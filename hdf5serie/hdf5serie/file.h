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
#include <boost/interprocess/sync/scoped_lock.hpp>
#ifdef _WIN32
  #include <boost/interprocess/windows_shared_memory.hpp>
#else
  #include <boost/interprocess/shared_memory_object.hpp>
#endif
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <thread>
#include <boost/thread/thread.hpp>

namespace H5 {

  namespace Internal {
#ifdef _WIN32
    using SharedMemory = boost::interprocess::windows_shared_memory;
    inline void SharedMemoryRemove(const char* shmName) {
    }
    inline SharedMemory SharedMemoryCreate(const char *shmName, boost::interprocess::mode_t mode,
                                 size_t size) {
      auto shm=SharedMemory(boost::interprocess::create_only, shmName, mode, size);
      return shm;
    }
#else
    using SharedMemory = boost::interprocess::shared_memory_object;
    inline void SharedMemoryRemove(const char* shmName) {
      SharedMemory::remove(shmName);
    }
    inline SharedMemory SharedMemoryCreate(const char *shmName, boost::interprocess::mode_t mode,
                                 size_t size) {
      auto shm=SharedMemory(boost::interprocess::create_only, shmName, mode);
      shm.truncate(size);
      return shm;
    }
#endif

    class ScopedLock;
  }

  class Dataset;

  /* A wrapper around a HDF5 file.
   * It handles automatically HDF5 SWMR access using a shared memory based inter-process communication:
   * This ensure that:
   * - only one writer is active (more writers are blocked until the writer closes the file)
   * - readers are blocked until no writer is active or the writer is in SWMR mode
   * - if readers are active and a writer wants to start writing the readers are notified by a reopen request.
   */
  class File : public GroupBase {
    friend class Internal::ScopedLock;
    public:
      enum FileAccess {
        read,            //!< Open file for reading with SWMR reading mode enabled
        write,           //!< Open file for writing. Calling enableSWMR will switch to SWMR writing mode.
                         //!< Note that enableSWMR will close all attributes.
        writeWithRename, //!< Open file for writing with a modified filename (<path>/<basename>.preSWMR.<ext>).
                         //!< Calling enableSWMR will close the file, rename it to the normal filename (<path>/<basename>.<ext>),
                         //!< reopen it in SWMR writing mode while all Elements of the file, except Attributes which are closed,
                         //!< are still available (but there hid_t will change, call getID() gain if needed).
                         //!< This is useful to avoid locking the normal filename during the none SWMR mode write tasks.
                         //!< This way the time the file is exclusively locked (in none SWMR mode) is minimized.
      };
      //! Opens the HDF5 file filename_ as a writer or reader dependent on type_.
      //! For a reader closeRequestCallback_ should be set!
      //! This callback is called if any writer want to write the file this reader also holds.
      //! If this callback is called you should close (destruct) this File object in time.
      //! After close (destruct) you can immediately reopen the file by constructing a File object (with the same HDF5 file) again.
      //! The inter process communication will ensure the the requested writer does its job before you can reopen the file for reading again.
      //! For a reader refreshCallback_ should also be set if the reader will call requestFlush.
      //! This this callback is called the reader should call refresh().
      //! Note that both callback functions will be called from of a thread created by this constructor.
      File(const boost::filesystem::path &filename_, FileAccess type_,
           const std::function<void()> &closeRequestCallback_=std::function<void()>(),
           const std::function<void()> &refreshCallback_=std::function<void()>());
      //! Closes the HDF5 file.
      ~File() override;
      //! Switch a writer from dataset creation mode to SWMR. After this call no datasets, groups or attributes can be created anymore
      //! and attributes are closed! But readers are no longer blocked and can read the file.
      void enableSWMR() override;

      static int getDefaultCompression() { return defaultCompression; }
      static void setDefaultCompression(int comp) { defaultCompression=comp; }
      static int getDefaultChunkSize() { return defaultChunkSize; }
      static void setDefaultChunkSize(int chunk) { defaultChunkSize=chunk; }
      static int getDefaultCacheSize() { return defaultCacheSize; }
      static void setDefaultCacheSize(int cache) { defaultCacheSize=cache; }

      //! Refresh the dataset of a reader
      void refresh() override;
      //! Request a flush of the writer.
      //! This is not blocking. If the writer has flushed the refreshCallback is called, see constructor.
      void requestFlush();
      //! Flush the file (the dataset) of a writer if this is requested by a reader.
      //! Does nothing if no reader has requested a flush.
      //! If a flush happend the reades are notified about the flush.
      void flushIfRequested();

      //! Internal helper function which dumps the content of the shared memory associated with filename.
      //! !!! Note that the shared memory mutex is NOT locked for this operation but the global named mutex to create/open and destroy lock is accquired.
      static void dumpSharedMemory(const boost::filesystem::path &filename);
      //! Internal helper function which removes the shared memory associated with filename, !!!EVEN if other process still use it!!!
      static void removeSharedMemory(const boost::filesystem::path &filename);

      FileAccess getType(bool originalType=false); // returns write for write and writeWithRename and read for read

    private:
      static int defaultCompression;
      static int defaultChunkSize;
      static int defaultCacheSize;

      void close() override;

      //! The name of the file
      boost::filesystem::path filename;
      boost::filesystem::path getFilename(bool originalFilename=false); // gets the filename dependent on the current preSWMR

      //! Flag if this instance is a writer or reader
      FileAccess type;
      //! This flag is set if the file was opened in writeWithRename mode and is currently in this mode (enableSWMR was not called yet)
      //! (note that type is reset to write in this case in the ctor, hence you need to used this flag)
      bool preSWMR { false };

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
        // the follwing members are only used for flush/refresh handling
        bool flushRequest { false }; //<! Is set to true by reader if a flush of the writer should be done. The writer resets to false after a flush.
      };

      //! This callback is called when a writer requested a close of all readers
      const std::function<void()> closeRequestCallback;
      //! This callback is called when a writer has flushed
      const std::function<void()> refreshCallback;

      //! Name of the shared memory
      std::string shmName;
      //!< a globally unique identifier for this process
      const boost::uuids::uuid processUUID;

      //! Shared memory object holding the shared memory
      //! Open/create and destroy of shm must bu guarded by a mutex
      Internal::SharedMemory shm;
      //! Memory region holding the shared memory map
      //! Open/create and destroy of region must bu guarded by a mutex
      boost::interprocess::mapped_region region;

      //! Pointer to the shared memory object
      SharedMemObject *sharedData {nullptr};

      //! True if this reader has requested a flush.
      bool flushRequested { false };
      //! The last wrtierState known by this object.
      WriterState lastWriterState { WriterState::none };

      //! transform filename to a valid boost interprocess name.
      static std::string createShmName(const boost::filesystem::path &filename);

      //! Helper function to open the file as a reader
      void openReader();
      //! Helper function to close the file as a reader
      void closeReader();
      //! Helper function to allow this process to open for writing (wait for other writers and for all readers to close)
      void allowOpenWriter();
      //! Helper function to open the file as a writer. allowOpenWriter must be called before
      void openWriter();
      //! Helper function to close the file as a writer
      void closeWriter();

      //! Helper function which waits until the condition pred is true.
      //! On entry the lock lock is released and re-aquired if waiting ends (the pred() == true).
      //! If this call blocks for more then relTime then then the message blockingMsg is printed
      //! but wait will not return (its blocking until pred is true)
      //! (use a empty string to avoid printing)
      void wait(Internal::ScopedLock &lock, const std::chrono::milliseconds& relTime,
                std::string_view blockingMsg, const std::function<bool()> &pred);

      // still alive pings use boost::thread since interruption point of boost, which are not availabe for std::thread, are used
      boost::thread stillAlivePingThread; // the thread for still alive pings
      void stillAlivePing(); // the worker function

      //! A thread created for a reader to listen when a new writer process requests a write of has flushed the file.
      //! boost thread interruption points does not help here since its not working with ConditionVariable
      //! -> hence we implement it ourself using the exitThread flag
      std::thread listenForRequestThread;
      //! The worker function for the thread listenForRequestThread.
      void listenForRequest();
      //! Flag which is set to true to enforce the thread to exit (on the next condition notify signal)
      bool exitThread { false }; // access is object is guarded by sharedData->mutex (interprocess wide)

      //! Write process information of this process to the shared memory
      void initProcessInfo();
      //! Remove process information of this process from the shared memory
      void deinitProcessInfo();

      //! open or create the shared memory atomically (guarded by a global named mutex)
      static void openOrCreateShm(const boost::filesystem::path &filename, File *self,
                                  std::string &shmName, Internal::SharedMemory &shm, boost::interprocess::mapped_region &region,
                                  SharedMemObject *&sharedData);
      static void deinitShm(SharedMemObject *sharedData, const boost::filesystem::path &filename, File *self, const std::string &shmName);
  };
}

#endif
