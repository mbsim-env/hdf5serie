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
#include <thread>

namespace H5 {

  class Dataset;

  /* A wrapper around a HDF5 file.
   * It handles automatically HDF5 SWMR access using a shared memory based inter-process communication:
   * This ensure that:
   * - only one writer is active (more writers are blocked until the writer closes the file)
   * - readers are blocked until no writer is active or the writer is in SWMR mode
   * - if readers are active and a writer wants to start writing the file the readers are notified by a close and reopen request.
   */
  class File : public GroupBase {
    friend class Dataset;
    friend class GroupBase;
    public:
      enum FileAccess {
        read,
        write
      };
      //! Opens the HDF5 file filename_ as a writer or reader dependent on type_.
      File(const boost::filesystem::path &filename_, FileAccess type_);
      //! Closes the HDF5 file.
      ~File() override;
      //! Switch a writer from dataset build-up to SWMR. After this call no datasets, groups or attributes can be created anymore
      //! and attributes are closed! But readers are no longer blocked and can read the file.
      void enableSWMR() override;
      //! A reader should call this function periodically to check if the reader should close and reopen the file (if true is returned).
      //! If true is returned reloadAfterRequest should be called.
      bool shouldClose();
      //! close the file and reopen it gain.
      void reloadAfterRequest();

      static int getDefaultCompression() { return defaultCompression; }
      static void setDefaultCompression(int comp) { defaultCompression=comp; }
      static int getDefaultChunkSize() { return defaultChunkSize; }
      static void setDefaultChunkSize(int chunk) { defaultChunkSize=chunk; }
      void refresh() override;
      void flush() override;

      //! Internal helper function which dumps the content of the shared memory
      static void dumpSharedMemory(const boost::filesystem::path &filename);
      //! Internal helper function which removes the shared memory, even if other processes still need it
      static void removeSharedMemory(const boost::filesystem::path &filename);

    private:
      static int defaultCompression;
      static int defaultChunkSize;

      boost::filesystem::path filename;

      //! Flag if this instance is a writer or reader
      FileAccess type;

      //! A file can be in the following states regarding the writer:
      enum class WriterState {
        none,         //<! no writer is currently active on the file
        writeRequest, //<! a writer wants to write the file but readers still exists
        active,       //<! a writer exists and is currently in dateset/attribute creation mode
        swmr,         //<! a writer exists and is currently in SWMR mode
      };
      //! This struct holds synchronization primitives for inter-process communication
      //! One such object exists in process shared memory for each file (not for each instance of File).
      struct SharedMemObject { // access to all members of this object is guarded by sharedData->mutex (interprocess wide)
        boost::interprocess::interprocess_mutex mutex;    //<! mutex for synchronization handling.
        boost::interprocess::interprocess_condition cond; //<! a condition variable for signaling state changes.
        WriterState writerState { WriterState::none };    //<! the current state of the write of this file.
        size_t activeReaders { 0 };                       //<! the number of active readers on this file.
      };
      //! Shared memory object holding the shared memory
      boost::interprocess::shared_memory_object shm;
      //! Memory region holding the shared memory map
      boost::interprocess::mapped_region region;
      //! Pointer to the shared memory object
      SharedMemObject *sharedData {nullptr};

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

      //! A thread created for a reader to listen when a new writer process requests
      std::thread listenForWriterRequestThread;
      //! The worker function for the thread listenForWriterRequestThread.
      void listenForWriterRequest(boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> &&lock);
      //! Flag which is set to true to enforce the thread to exit (on the next condition notify signal)
      bool exitThread { false }; // access is object is guarded by sharedData->mutex (interprocess wide)

      //! This flag is set to true by the thread when a new writer requests action.
      std::atomic<bool> readerShouldClose { false }; // access to this object is handled atomically (within the current process)

      //! Get pointer to shared memory object SharedMemoryObject. Returns the used shm and region object.
      //! Also returned the shared memory name if provided.
      //! The shared memory is get atomically using a file lock on filename
      static SharedMemObject* getSharedMemory(File *self,
                                              const boost::filesystem::path &filename,
                                              boost::interprocess::shared_memory_object &shm,
                                              boost::interprocess::mapped_region &region,
                                              bool openOnly=false,
                                              std::string *name=nullptr);
  };
}

#endif
