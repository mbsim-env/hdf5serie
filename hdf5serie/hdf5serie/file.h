/* Copyright (C) 2009 Markus Friedrich
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Contact:
 *   friedrich.at.gc@googlemail.com
 *
 */

#ifndef _HDF5SERIE_FILE_H_
#define _HDF5SERIE_FILE_H_

#include <hdf5serie/group.h>
#include <boost/filesystem.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <thread>
#include <boost/thread/thread.hpp>

namespace H5 {

  class Dataset;

  /* A wrapper around a HDF5 file.
   * It handles automatically HDF5 SWMR access using a shared memory based inter-process communication:
   * This ensure that:
   * - only one writer is active (more writers are blocked until the writer closes the file)
   * - readers are blocked until no writer is active or the writer is in SWMR mode
   * - if readers are active and a writer wants to start writing the readers are notified by a reopen request.
   */
  class File : public GroupBase {
    friend class Dataset;
    friend class GroupBase;
    public:
      enum FileAccess {
        read,  //!< open file for reading
        write, //!< open file for writing
      };
      //! Opens the HDF5 file filename_ as a writer or reader dependent on type_.
      File(const boost::filesystem::path &filename_, FileAccess type_);
      //! Closes the HDF5 file.
      ~File() override;
      //! Switch a writer from dataset creation mode to SWMR. After this call no datasets, groups or attributes can be created anymore
      //! and attributes are closed! But readers are no longer blocked and can read the file.
      void enableSWMR() override;
      //! A reader should call this function periodically to check if it should reopen the file.
      //! If true, reopen should be called in time.
      bool shouldReopen();
      //! Close the file and wait until a writer has rewritten the file (until the writer is again in SWMR mode).
      //! Hence, the call blocks until has done this action. Afterwards the file is reopened.
      //! Note that only the File object is reopened. NO dataset or other HDF5 objects are available after this call.
      void reopen();

      static int getDefaultCompression() { return defaultCompression; }
      static void setDefaultCompression(int comp) { defaultCompression=comp; }
      static int getDefaultChunkSize() { return defaultChunkSize; }
      static void setDefaultChunkSize(int chunk) { defaultChunkSize=chunk; }

      //! Refresh the dataset of a reader
      void refresh() override;
      //! Flush the datasets of a writer
      void flush() override;

      //! Internal helper function which dumps the content of the shared memory associated with filename.
      //! !!! Note that the mutex is NOT locked for this operation but the file lock is accquired.
      static void dumpSharedMemory(const boost::filesystem::path &filename);
      //! Internal helper function which removes the shared memory associated with filename, !!!EVEN if other process still use it!!!
      static void removeSharedMemory(const boost::filesystem::path &filename);

    private:
      static int defaultCompression;
      static int defaultChunkSize;

      boost::filesystem::path filename;

      //! Flag if this instance is a writer or reader
      FileAccess type;

      //! A writer can be in the following state:
      enum class WriterState {
        none,         //<! no writer is currently active
        writeRequest, //<! a writer wants to write the file but readers still exists
        active,       //<! a writer exists and is currently in dateset/attribute creation mode
        swmr,         //<! a writer exists and is currently in SWMR mode (readers can use the file using SWMR)
      };
      //! the maximal number of readers which can access the file simultanously
      constexpr static size_t MAXREADERS { 100 };
      //! Information about a process accessing the shared memory (a process means here an instance of a File class)
      struct ProcessInfo {
        boost::uuids::uuid processUUID;         //!< a globally unique identifier for each process
        boost::posix_time::ptime lastAliveTime; //!< the last still alive timestamp of the process
        FileAccess type;                        //!< the type of the process read or write
      };
      //! This struct holds synchronization primitives and states for inter-process communication.
      //! One such object exists in process shared memory for each file (so multiple instances of the File object can share it).
      //! All access to all members of this object must be guarded by locking sharedData->mutex (interprocess wide)
      struct SharedMemObject {
        // the following member is only used for life-time handling of the shared memroy object itself
        size_t shmUseCount { 0 }; //<! the number users of this shared memory object
        // the following members are used to synchronize the writer and all readers.
        boost::interprocess::interprocess_mutex mutex;    //<! mutex for synchronization handling.
        boost::interprocess::interprocess_condition cond; //<! a condition variable for signaling state changes.
        // the following members represent the state of the writer and readers
        // after setting any of these variables sharedData->cond.notify_all() must be called to notify all waiting process about the change
        WriterState writerState { WriterState::none }; //<! the current state of the write of this file.
        size_t activeReaders { 0 };                    //<! the number of active readers on this file.
        // the follwing members are only used for still-alive/crash detection handling
        boost::container::static_vector<ProcessInfo, MAXREADERS+1> processes; //<! a list of all processes accessing the shared memory
      };
      //! Name of the shared memory
      std::string shmName;

      //! Shared memory object holding the shared memory
      //! Access to shm (and region) must bu guarded by locking the boost filelock of filename.
      boost::interprocess::shared_memory_object shm;
      //! Memory region holding the shared memory map
      //! Access to region (and shm) must bu guarded by locking the boost filelock of filename.
      boost::interprocess::mapped_region region;

      //! Pointer to the shared memory object
      SharedMemObject *sharedData {nullptr};

      //! transform filename to a valid boost interprocess name.
      static std::string createShmName(const boost::filesystem::path &filename);

      //! Helper function to open the file as a reader
      void openReader();
      //! Helper function to close the file as a reader
      void closeReader();
      //! Helper function to open the file as a writer
      void openWriter();
      //! Helper function to close the file as a writer
      void closeWriter();

      //! Helper function which waits until the condition pred is true.
      //! On entry the lock lock is released and re-aquired if waiting ends (the pred() == true).
      //! If this call blocks (pred is not already true on entry) then the message blockingMsg is printed
      //! (use a empty string to avoid printing)
      void wait(boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> &lock,
                const std::string &blockingMsg, const std::function<bool()> &pred);

      // still alive pings use boost::thread since interruption point of boost, which are not availabe for std::thread, are used
      boost::thread stillAlivePingThread; // the thread for still alive pings
      void stillAlivePing(); // the worker function

      //! A thread created for a reader to listen when a new writer process requests
      //! boost thread interruption points does not help here since its not working with boost::interprocess::interprocess_condition
      //! -> hence we implement it ourself using the exitThread flag
      std::thread listenForWriterRequestThread;
      //! The worker function for the thread listenForWriterRequestThread.
      void listenForWriterRequest(boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> &&lock);
      //! Flag which is set to true to enforce the thread to exit (on the next condition notify signal)
      bool exitThread { false }; // access is object is guarded by sharedData->mutex (interprocess wide)

      //! This flag is set to true by the thread when a new writer requests action.
      std::atomic<bool> readerShouldClose { false }; // access to this object is handled atomically (within the current process)

      boost::uuids::uuid processUUID; //!< a globally unique identifier for this process

      //! Write process information of this process to the shared memory
      void initProcessInfo();
      //! Remove process information of this process from the shared memory
      void deinitProcessInfo();

      //! open or create the shared memory atomically (process with using file lock)
      void openOrCreateShm();
  };
}

#endif
