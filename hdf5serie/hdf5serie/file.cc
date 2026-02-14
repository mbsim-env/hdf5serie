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

// the following two lines are a workaround for a bug in boost 1.69
#define BOOST_PENDING_INTEGER_LOG2_HPP
#include <boost/integer/integer_log2.hpp>

#include <config.h>
#include <hdf5serie/file.h>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
  #  define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
  #include <io.h>
#else
  #include <sys/vfs.h>
  #include <linux/magic.h>
#endif
#if !defined(NDEBUG) && !defined(_WIN32)
  #include <boost/process.hpp>
#endif

using namespace std;
using namespace fmatvec;
using namespace H5::Internal;
namespace ipc = boost::interprocess;

namespace {

  auto now(const chrono::system_clock::time_point &currentTime = chrono::system_clock::now()) {
    auto t = chrono::system_clock::to_time_t(currentTime);
    auto local_t = *localtime(&t);
    auto ms = chrono::duration_cast<chrono::milliseconds>(currentTime.time_since_epoch()) % 1000;
    stringstream str;
    str<<put_time(&local_t, R"(%FT%T)")<<"."<<setfill('0')<<setw(3)<<ms.count();
    return str.str();
  }

#if !defined(NDEBUG) && !defined(_WIN32)
  // This function is only called in debug builds on linux when the program /usr/bin/lsof is installed.
  // If so, it checks if filename is opened by any process and throws a exception is this case.
  // The exception message will contain the filename, pid, cmd-name and user who owns a handle to filename.
  // This function is used in debug build (on linux) to check that the IPC to close a file by all running processes is working.
  void checkIfFileIsOpenedBySomeone(const string &msg, const boost::filesystem::path &filename) {
    static bool check = (getenv("HDF5SERIE_CHECKIFFILEISOPENEDBYSOMEONE")!=nullptr);
    if(!check)
      return;
    using namespace boost::process;
    if(!boost::filesystem::exists("/usr/bin/lsof"))
      return;
    ipstream outStr;
    child c("/usr/bin/lsof", "-F", "pcL", filename.string(), std_out > outStr, std_err > null);
    string line;
    string error;
    while(getline(outStr, line)) {
      if(line[0]=='p') error+=msg+": File "+boost::filesystem::absolute(filename).string()+" is opened by pid="+line.substr(1);
      if(line[0]=='c') error+=" cmd="+line.substr(1);
      if(line[0]=='L') error+=" user="+line.substr(1)+"\n";
    }
    c.wait();
    if(!error.empty())
      throw runtime_error(error);
  }
#else
  #define checkIfFileIsOpenedBySomeone(msg, filename)
#endif

  // call this function after opening a HDF5 file using H5F... to set the file flags to not to inherit a open
  // file descriptor in a started subprocess.
  void noFileHandleInherit(H5::ScopedHID &id) {
    // check if we are running on sec2 VFD
    H5::ScopedHID fapl(H5Fget_access_plist(id), &H5Pclose);
    auto driver = H5Pget_driver(fapl);
    // if so, get the file descriptor and mark the file flags as none inherit to child processes
    if(driver == H5FD_SEC2) {
      void *fileHandle;
      H5::checkCall(H5Fget_vfd_handle(id, H5P_DEFAULT, &fileHandle));
      auto fno = *static_cast<int*>(fileHandle);
      if(fno == -1)
        throw runtime_error("Unable to get file descriptor.");
      #ifdef _WIN32
        auto handle = reinterpret_cast<HANDLE>(_get_osfhandle(fno));
        if(handle == INVALID_HANDLE_VALUE)
          throw runtime_error("Unable to get Windows file handle from the file number.");
        if(SetHandleInformation(handle, HANDLE_FLAG_INHERIT, 0)==0)
          throw runtime_error("Unable to set Windows file handle flag.");
      #else
        int flags = fcntl(fno, F_GETFD);
        if(flags == -1)
          throw runtime_error("Unable to get Linux file flags.");
        flags |= FD_CLOEXEC;
        if(fcntl(fno, F_SETFD, flags) == -1)
          throw runtime_error("Unable to set Linux file flags.");
      #endif
    }
  }

  void initFileLock(ipc::file_lock &fl, const string &filename) {
#if _WIN32
    auto tmpDir=boost::filesystem::temp_directory_path();
#else
    auto checkTMPFS=[](const boost::filesystem::path &dir) {
      struct statfs buf;
      if(statfs(dir.string().c_str(), &buf)!=0)
        return false;
      return buf.f_type==TMPFS_MAGIC;
    };
    const char *XDG_RUNTIME_DIR=getenv("XDG_RUNTIME_DIR");
    boost::filesystem::path tmpDir;
    if(XDG_RUNTIME_DIR && checkTMPFS(XDG_RUNTIME_DIR))
      tmpDir=XDG_RUNTIME_DIR;
    if(tmpDir.empty() && checkTMPFS("/dev/shm"))
      tmpDir="/dev/shm";
    if(tmpDir.empty() && XDG_RUNTIME_DIR)
      tmpDir=XDG_RUNTIME_DIR;
    if(tmpDir.empty())
      tmpDir=boost::filesystem::temp_directory_path();
#endif

    boost::filesystem::path filepath = tmpDir/filename;
    if(!boost::filesystem::exists(filepath))
      ofstream f(filepath);
    fl = ipc::file_lock(filepath.string().c_str());
  }

  pair<std::mutex, ipc::file_lock> &getSyncPrimFileLock() {
    static pair<std::mutex, ipc::file_lock> ret;
    static bool firstCall = true;
    if(firstCall) {
      firstCall = false;
      initFileLock(ret.second, "hdf5serie_syncprim_filelock");
    }
    return ret;
  }

  pair<std::mutex, ipc::file_lock> &getSettingsFileLock() {
    static pair<std::mutex, ipc::file_lock> ret;
    static bool firstCall = true;
    if(firstCall) {
      firstCall = false;
      initFileLock(ret.second, "hdf5serie_settings_filelock");
    }
    return ret;
  }

}

namespace H5 {

int File::defaultCompression=1;
int File::defaultChunkSize=100;
int File::defaultCacheSize=100;

namespace Internal {
  // This class is similar to boost::interprocess::scoped_lock but prints debug messages.
  class ScopedLock : public ipc::scoped_lock<ipc::interprocess_mutex>  {
    public:
      ScopedLock(ipc::interprocess_mutex &mutex, File *self_, string_view msg_) :
                 ipc::scoped_lock<ipc::interprocess_mutex>((initMsg(self_, msg_), mutex)), self(self_), msg(msg_) {
        if(self->msgAct(Atom::Debug))
          self->msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<self->getFilename().string()<<": Mutex locked: "<<msg<<endl;
      }
      ScopedLock(const ScopedLock&) = delete;
      ScopedLock(ScopedLock&&) = delete;
      ScopedLock& operator=(const ScopedLock&) = delete;
      ScopedLock& operator=(ScopedLock&&) = delete;
      ~ScopedLock() {
        if(self->msgAct(Atom::Debug)) {
          if(mutex())
            self->msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<self->getFilename().string()<<": Unlock mutex: "<<msg<<endl;
          else
            self->msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<self->getFilename().string()<<": Nothing to unlock, moved to other lock: "<<msg<<endl;
        }
      }
    private:
      static void initMsg(File *self, string_view msg) {
        if(self->msgAct(Atom::Debug))
          self->msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<self->getFilename().string()<<": Trying to lock mutex: "<<msg<<endl;
      }
      File *self;
      string_view msg;
  };

  template<int N>
  void ConditionVariable<N>::wait(boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> &externLock,
                                  const function<bool()> &pred) {
    while(!wait_for(externLock, std::chrono::milliseconds::max(), pred)) {
    }
  }

  template<int N>
  bool ConditionVariable<N>::wait_for(boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> &externLock,
                                      const std::chrono::milliseconds& relTime,
                                      const std::function<bool()> &pred) {
    auto startTime = std::chrono::steady_clock::now();
    if(!externLock)
      throw boost::interprocess::lock_exception();
    auto waiterUUID=boost::uuids::random_generator()();
    while(!pred()) {
      {
        boost::interprocess::scoped_lock waiterLock(waiterMutex);
        if(waiter.size()==N)
          throw runtime_error("Too many threads are waiting in ConditionVariable.");
        if(find(waiter.begin(), waiter.end(), waiterUUID)==waiter.end())
          waiter.emplace_back(waiterUUID);
      }
      externLock.unlock();

      bool timeExceeded = false;
      bool exitLoop;
      do {
        using namespace chrono_literals;
        this_thread::sleep_for(1000ms/25);
        {
          boost::interprocess::scoped_lock waiterLock(waiterMutex);
          exitLoop = find(waiter.begin(), waiter.end(), waiterUUID)==waiter.end();
        }
        if(std::chrono::steady_clock::now()-startTime>relTime) {
          boost::interprocess::scoped_lock waiterLock(waiterMutex);
          if(auto it=find(waiter.begin(), waiter.end(), waiterUUID); it!=waiter.end())
            waiter.erase(it);
          timeExceeded = true;
          break;
        }
      }
      while(!exitLoop);

      externLock.lock();

      if(timeExceeded)
        return false;
    }
    return true;
  }

  template<int N>
  void ConditionVariable<N>::notify_all() {
    boost::interprocess::scoped_lock waiterLock(waiterMutex);
    // no wakeup of all others needed since polling is used
    waiter.clear();
  }
}

class Settings {
  private:
    static boost::filesystem::path getFileName() {
      static boost::filesystem::path base;
      if(base.empty()) {
#if _WIN32
        base=getenv("APPDATA");
#else
        base=getenv("XDG_CONFIG_HOME")?getenv("XDG_CONFIG_HOME"):"";
        if(base.empty())
          base=boost::filesystem::path(getenv("HOME"))/".config";
#endif
      }
      return base/"mbsim-env"/"hdf5serie.ini";
    }
  public:
    // Get configuration value of path (e.g. "keepAlive/pingFrequency").
    // If the path does not exits it is added to config files with defaultValue and defaultValue is returned.
    template<class T>
    static T getValue(string path, const T& defaultValue) {
      boost::algorithm::replace_all(path, "/", ".");

      Atom::msgStatic(Atom::Debug)<<"HDF5Serie: "<<now()<<": Trying to lock the named mutex 'hdf5serie_mutex_settings_file'"<<endl;
      ipc::scoped_lock lock1(getSettingsFileLock().first);
      ipc::scoped_lock lock2(getSettingsFileLock().second);
      Atom::msgStatic(Atom::Debug)<<"HDF5Serie: "<<now()<<": locked"<<endl;

      auto filename = getFileName();
      if(!boost::filesystem::is_directory(filename.parent_path()))
        boost::filesystem::create_directories(filename.parent_path());

      boost::property_tree::ptree pt;
      if(boost::filesystem::exists(filename))
        boost::property_tree::ini_parser::read_ini(filename.string(), pt);
      boost::optional<T> v=pt.get_optional<T>(path);
      if(v)
        return v.get();
      pt.put(path, defaultValue);
      boost::property_tree::ini_parser::write_ini(filename.string(), pt);
      return defaultValue;
    }
};

File::File(const boost::filesystem::path &filename_, FileAccess type_,
           const function<void()> &closeRequestCallback_,
           const function<void()> &refreshCallback_,
           const function<void()> &renameAtomicFunc_) :
  GroupBase(nullptr, filename_.string()),
  filename(filename_),
  type(type_),
  closeRequestCallback(closeRequestCallback_),
  refreshCallback(refreshCallback_),
  renameAtomicFunc(renameAtomicFunc_),
  processUUID(boost::uuids::random_generator()()) {

  if(getType()==read && !boost::filesystem::exists(filename))
    throw Exception({}, "No such HDF5 file to open: "+filename.string());

  if(type==writeWithRename)
    preSWMR = true;

  if(getenv("HDF5SERIE_DEBUG"))
    setMessageStreamActive(Atom::Debug, true);

  msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Process UUID = "<<processUUID<<endl;

  file=this;

  openOrCreateShm(getFilename(), this,
                  shmName, shm, region, sharedData);

  switch(getType()) {
    case write:
      initProcessInfo();
      preOpenWriter();
      openWriter();
      break;
    case read:
      initProcessInfo();
      preOpenReader();
      openReader();
      break;
    default: throw runtime_error("internal error");
  }
}

void File::openOrCreateShm(const boost::filesystem::path &filename, File *self,
                           string &shmName, Internal::SharedMemory &shm, boost::interprocess::mapped_region &region,
                           SharedMemObject *&sharedData) {
  // create inter process shared memory atomically
  self->msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<filename.string()<<": Touch file"<<endl;

  shmName=createShmName(filename);
  // exclusively lock the global shm mutex
  {
    self->msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<filename.string()<<": Trying to lock the global shm mutex: openOrCreateShm"<<endl;
    ipc::scoped_lock lock1(getSyncPrimFileLock().first);
    ipc::scoped_lock lock2(getSyncPrimFileLock().second);
    self->msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<filename.string()<<": Locked: openOrCreateShm"<<endl;
    // convert filename to valid boost interprocess name (cname)
    try {
      // try to open the shared memory ...
        self->msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<filename.string()<<": Try to open shared memory named "<<shmName<<endl;
        shm=SharedMemory(ipc::open_only, shmName.c_str(), ipc::read_write);
        region=ipc::mapped_region(shm, ipc::read_write); // map memory
        sharedData=static_cast<SharedMemObject*>(region.get_address()); // get pointer
    }
    catch(...) {
      // ... if it failed, create the shared memory
      self->msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<filename.string()<<": Opening shared memory failed, create now"<<endl;
      shm=SharedMemoryCreate(shmName.c_str(), ipc::read_write, sizeof(SharedMemObject));
      region=ipc::mapped_region(shm, ipc::read_write); // map memory
      sharedData=new(region.get_address())SharedMemObject; // initialize shared memory (by placement new)
    }
    sharedData->shmUseCount++;
    self->msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<filename.string()<<": Unlock: openOrCreateShm"<<endl;
  }
  // now the process shared memory is created or opened atomically and the global mutex lock is releases
  // from now on this shared memory is used for any syncronization/communiation between the processes
}

boost::filesystem::path File::getFilename(bool originalFilename) {
  if(originalFilename)
    return filename;
  if(preSWMR)
    return filename.parent_path()/(filename.stem().string()+".preSWMR"+filename.extension().string());
  return filename;
}

File::FileAccess File::getType(bool originalType) {
  if(originalType)
    return type;
  if(type==read)
    return read;
  return write;
}

void File::preOpenWriter() {
  ScopedLock lock(sharedData->mutex, this, "preOpenWriter");
  // open the writer
  // wait until no other writer is active
  const static std::chrono::milliseconds showBlockMessageAfter(Settings::getValue("messages/showBlockMessageAfter", 500));
  wait(lock, showBlockMessageAfter, "Blocking until other writer has finished.", [this](){
    return sharedData->writerState==WriterState::none;
  });
  // now we are the single writer on this file
  if(sharedData->activeReaders>0) {
    // if readers are still active set the writer state to writeRequest and notify to request that all readers close
    msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Set writerState=writeRequest and notify"<<endl;
    sharedData->writerState=WriterState::writeRequest;
    sharedData->cond.notify_all();
      // now wait until all readers have closed
    wait(lock, showBlockMessageAfter, "Blocking until all readers haved closed.", [this](){
      return sharedData->activeReaders==0;
    });
  }
  msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Set writerState=active and notify"<<endl;
  // now set the writer state to active (creation of datasets/attributes) and notify about this change
  sharedData->writerState=WriterState::active;
  sharedData->cond.notify_all();
}

namespace {
  void retryOnLockError(const string &filenames, const function<void()> &run) {
    static const std::vector<double> retryDelay {0,0.01,0.1,0.5,1,5};
    for(size_t i=0; i<retryDelay.size(); ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<size_t>(retryDelay[i]*1000)));
      try {
        run();
        break; // if everything was ok break this loop (no retry needed)
      }
      catch(std::exception &ex) {
        // at the last retry -> throw
        if(i == retryDelay.size()-1)
          throw;

        // check if its a file lock error
        bool isLockError = false;
        // check for HDF5 file lock errors
        if(auto exHDF5 = dynamic_cast<Exception*>(&ex); exHDF5)
          for(auto &ei : exHDF5->getErrorStack()) {
            if(ei.min_num == H5E_CANTLOCKFILE) {
              isLockError = true;
              break;
            }
          }
        // check for boost::filesystem::rename error due to locking

        // if its not a file lock error -> throw
        if(!isLockError)
          throw;
        // if its a file lock error -> print warning
        Atom::msgStatic(Atom::Warn)<<filenames<<": File locked by unknown process. Retry "
                                   <<i+1<<"/"<<retryDelay.size()-1<<" in "<<retryDelay[i+1]<<"s."<<endl;
      }
    }
  }
}

void File::openWriter() {
  // create file
  msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Create HDF5 file"<<endl;
  ScopedHID faid(H5Pcreate(H5P_FILE_ACCESS), &H5Pclose);
  checkCall(H5Pset_libver_bounds(faid, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST));
  checkCall(H5Pset_fclose_degree(faid, H5F_CLOSE_SEMI));
  checkIfFileIsOpenedBySomeone("File::openWriter::precreate/preopen", getFilename());
  if(type==write || (type==writeWithRename && preSWMR)) {
    ScopedHID file_creation_plist(H5Pcreate(H5P_FILE_CREATE), &H5Pclose);
    checkCall(H5Pset_link_creation_order(file_creation_plist, H5P_CRT_ORDER_TRACKED | H5P_CRT_ORDER_INDEXED));
    retryOnLockError(getFilename().string(), [this, &faid, &file_creation_plist](){
      id.reset(H5Fcreate(getFilename().string().c_str(), H5F_ACC_TRUNC, file_creation_plist, faid), &H5Fclose);
    });
  }
  else
    retryOnLockError(getFilename().string(), [this, &faid](){
      id.reset(H5Fopen(getFilename().string().c_str(), H5F_ACC_RDWR, faid), &H5Fclose);
    });
  noFileHandleInherit(id);
  msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Create HDF5 file: done"<<endl;
}

void File::initProcessInfo() {
  ScopedLock lock(sharedData->mutex, this, "initProcessInfo");
  msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": init process info and start thread"<<endl;
  // save the process info of this process in shared memory
  if(sharedData->processes.size()==MAXREADERS+1)
    throw Exception(getPath(), "Too many process are accessing this file.");
  sharedData->processes.push_back(ProcessInfo{processUUID,
                                              boost::posix_time::microsec_clock::universal_time(),
                                              getType()});
  // start thread which updates the still alive timestamp
  stillAlivePingThread=boost::thread(&File::stillAlivePing, this);
}

void File::deinitProcessInfo() {
  msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Interrupt thread and join"<<endl;
  stillAlivePingThread.interrupt();
  stillAlivePingThread.join();
  msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Thread joined"<<endl;

  ScopedLock lock(sharedData->mutex, this, "deinitProcessInfo");
  auto it=std::find_if(sharedData->processes.begin(), sharedData->processes.end(), [this](const ProcessInfo &pi) {
    return pi.processUUID==processUUID;
  });
  if(it!=sharedData->processes.end())
    sharedData->processes.erase(it);
  else
    msg(Atom::Error)<<"HDF5Serie: "<<getFilename().string()<<": Another process has remove this ProcessInfo, maybe because it thought that this process has crashed, but I'm this alive."<<endl;
}

// executed in a thread
void File::stillAlivePing() {
  while(true) {
    {
      ScopedLock lock(sharedData->mutex, this, "stillAlivePing");
      auto curIt=std::find_if(sharedData->processes.begin(), sharedData->processes.end(), [this](const ProcessInfo &pi) {
        return pi.processUUID==processUUID;
      });
      curIt->lastAliveTime=boost::posix_time::microsec_clock::universal_time();

      auto it=sharedData->processes.begin();
      while(it!=sharedData->processes.end()) {
        const static int HDF5SERIE_FIXAFTER=getenv("HDF5SERIE_FIXAFTER") ? boost::lexical_cast<int>(getenv("HDF5SERIE_FIXAFTER")) : 0;
        const static int fixAfter = Settings::getValue("keepAlive/fixAfter", 3000);
        if(it->lastAliveTime+boost::posix_time::milliseconds(
           HDF5SERIE_FIXAFTER>0 ? HDF5SERIE_FIXAFTER : fixAfter)<curIt->lastAliveTime) {
          msg(Atom::Info)<<"HDF5Serie: Found process with too old keep alive timestamp: "<<it->processUUID<<
                           " Assume that this process crashed. Remove it from shared memory."<<endl;
          if(it->type==read) {
            msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": Decrement activeReaders and shmUseCount, since a reader seem to have crashed, and notify"<<endl;
            sharedData->activeReaders--;
          }
          else if(it->type==write) {
            msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": Set writerState=none and decrement shmUseCount, since the writer seem to have crashed, and notify"<<endl;
            sharedData->writerState=WriterState::none;
          }
          sharedData->shmUseCount--;
          it=sharedData->processes.erase(it);
          sharedData->cond.notify_all();
        }
        else
          it++;
      }
    }
    // this is a boost thread interruption point
    const static int pingFreq = Settings::getValue("keepAlive/pingFrequency", 1000);
    boost::this_thread::sleep_for(boost::chrono::milliseconds(pingFreq));
  }
}

void File::preOpenReader() {
  ScopedLock lock(sharedData->mutex, this, "preOpenReader");
  // open file as a reader
  // wait until either no writer exists or the writer is in SWMR state
  const static std::chrono::milliseconds showBlockMessageAfter(Settings::getValue("messages/showBlockMessageAfter", 500));
  wait(lock, showBlockMessageAfter, "Blocking until no writer exists or the writer is in SWMR state.", [this](){
    return sharedData->writerState==WriterState::none || sharedData->writerState==WriterState::swmr;
  });
  lastWriterState=sharedData->writerState;
  // increment the active readers count and notify about this change
  msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Increment activeReaders and notify"<<endl;
  sharedData->activeReaders++;
  sharedData->cond.notify_all();
  // open a thread which listens for futher writer which want to start writing
  msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Start thread"<<endl;
  exitThread=false; // flag indicating that the thread should close -> false at thread start
  listenForRequestThread=thread(&File::listenForRequest, this);
}

void File::openReader() {
  // open file
  msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Open HDF5 file"<<endl;
  ScopedHID faid(H5Pcreate(H5P_FILE_ACCESS), &H5Pclose);
  checkCall(H5Pset_fclose_degree(faid, H5F_CLOSE_SEMI));
  retryOnLockError(getFilename().string(), [this, &faid](){
    id.reset(H5Fopen(getFilename().string().c_str(), H5F_ACC_RDONLY | H5F_ACC_SWMR_READ, faid), &H5Fclose);
  });
  noFileHandleInherit(id);
  msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Open HDF5 file: done"<<endl;
}

void File::deinitShm(SharedMemObject *sharedData, const boost::filesystem::path &filename, File *self, const std::string &shmName) {
  self->msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<filename.string()<<": Trying to lock the global mutex: dtor"<<endl;
  ipc::scoped_lock lock1(getSyncPrimFileLock().first);
  ipc::scoped_lock lock2(getSyncPrimFileLock().second);
  self->msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<filename.string()<<": File locked: dtor"<<endl;
  size_t localShmUseCount;
  sharedData->shmUseCount--;
  localShmUseCount=sharedData->shmUseCount;
  self->msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<filename.string()<<": Decrement shmUseCount"<<endl;
  // sharedData->shmUseCount cannot be incremente by another process since we sill own the global named mutex
  if(localShmUseCount==0) {
    self->msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<filename.string()<<": Shared memory is no longer used, remove it"<<endl;
    sharedData->~SharedMemObject(); // call destructor of SharedMemObject
    // region does not need destruction
    SharedMemoryRemove(shmName.c_str()); // effectively destructs shm
  }
  self->msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<filename.string()<<": Unlock: dtor"<<endl;
}

File::~File() {
  try {
    switch(getType()) {
      case write:
        closeWriter();
        postCloseWriter();
        deinitProcessInfo();
        break;
      case read:
        closeReader();
        postCloseReader();
        deinitProcessInfo();
        // the thread should close now and we wait for it to join.
        msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Join thread"<<endl;
        listenForRequestThread.join();
        msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Thread joined"<<endl;
        break;
      default:
        throw runtime_error("internal error");
    }

    // if opened with writeWithRename but enableSWMR was not called -> rename the file now
    if(type == writeWithRename && preSWMR == true) {
      std::string tmpShmName;
      Internal::SharedMemory tmpShm;
      boost::interprocess::mapped_region tmpRegion;
      SharedMemObject *tmpSharedData;
      openOrCreateShm(filename, this, tmpShmName, tmpShm, tmpRegion, tmpSharedData);
      std::swap(shmName, tmpShmName);
      std::swap(shm, tmpShm);
      std::swap(region, tmpRegion);
      std::swap(sharedData, tmpSharedData);
      initProcessInfo();
      preOpenWriter();
      checkIfFileIsOpenedBySomeone("File::~File::prerename", getFilename());
      checkIfFileIsOpenedBySomeone("File::~File::prerename", filename);
      retryOnLockError(getFilename().string()+","+filename.string(), [this](){
        boost::filesystem::rename(getFilename(), filename);
      });
      if(renameAtomicFunc)
        renameAtomicFunc();
      deinitShm(tmpSharedData, getFilename(), this, tmpShmName);
      postCloseWriter();
      deinitProcessInfo();
    }
 
    deinitShm(sharedData, getFilename(), this, shmName);
  }
  catch(const exception &ex) {
    msg(Atom::Error)<<"HDF5Serie: Exception during destructor: "<<ex.what()<<endl;
  }
  catch(...) {
    msg(Atom::Error)<<"HDF5Serie: Unknown exception during destructor."<<endl;
  }
}

void File::closeWriter() {
  // close writer file
  msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Close HDF5 writer file"<<endl;
  close();
}

void File::postCloseWriter() {
  {
    ScopedLock lock(sharedData->mutex, this, "postCloseWriter");
    // close the writer
    // set the writer state to none and notify about this change
    msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Set writerState=none and notify"<<endl;
    sharedData->writerState=WriterState::none;
    sharedData->cond.notify_all();
  }
}

void File::closeReader() {
  // close reader file
  msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Close HDF5 reader file"<<endl;
  close();
}

void File::postCloseReader() {
  ScopedLock lock(sharedData->mutex, this, "postCloseReader");
  // close a reader
  // decrements the number of active readers and notify about this change
  msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Decrement activeReaders, set thread exit flag and notify"<<endl;
  sharedData->activeReaders--;
  exitThread=true; // set the thread exit flag before notifying to ensure that the thread gets closed
  sharedData->cond.notify_all();
}

void File::refresh() {
  assert(getType()==read && "refresh() can only be called on files opened for reading");
  GroupBase::refresh();
}

bool File::requestFlush() {
  assert(getType()==read && "requestFlush() can only be called on files opened for reading");
  ScopedLock lock(sharedData->mutex, this, "requestFlush");
  if(msgAct(Atom::Debug))
    msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Set flushRequest and notify"<<endl;
  sharedData->flushRequest=true;
  flushRequested=true;
  sharedData->cond.notify_all(); // not really needed since we assume that the writer is polling on this flag frequently.
  return sharedData->writerState==WriterState::swmr;
}

void File::flushIfRequested(const function<void(File*)> &postFlushFunc) {
  assert(getType()==write && "flushIfRequested() can only be called on files opened for writing");
  {
    ScopedLock lock(sharedData->mutex, this, "flushIfRequested, before flush");
    if(!sharedData->flushRequest) {
      if(msgAct(Atom::Debug))
        msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": No flush request"<<endl;
      return;
    }
  }

  // flush file (and datasets) and reset flushRequest flag and notify
  if(msgAct(Atom::Debug))
    msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Flushing now"<<endl;
  GroupBase::flush();

  if(postFlushFunc)
    postFlushFunc(this);

  ScopedLock lock(sharedData->mutex, this, "flushIfRequested, after flush");
  if(msgAct(Atom::Debug))
    msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Unset flushRequest and notify"<<endl;
  sharedData->flushRequest=false;
  sharedData->cond.notify_all();
}

void File::enableSWMR() {
  if(getType()!=write)
    throw Exception(getPath(), "enableSWMR() can only be called for writing files");
  msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": enableSWMR: start"<<endl;

  msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": flush before enableSWMR"<<endl;
  flush();

  if(type == writeWithRename) {
    try {
      msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": enableSWMR: type=writeWithRename: close all elements"<<endl;
      closeWriter();
      postCloseWriter();
      deinitProcessInfo();
  
      msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<filename.string()<<": enableSWMR: type=writeWithRename: create shm for original filename"<<endl;
      std::string tmpShmName;
      Internal::SharedMemory tmpShm;
      boost::interprocess::mapped_region tmpRegion;
      SharedMemObject *tmpSharedData;
      openOrCreateShm(filename, this, tmpShmName, tmpShm, tmpRegion, tmpSharedData);
      msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<filename.string()<<": enableSWMR: type=writeWithRename: wait to allow writing for original filename"<<endl;
      std::swap(shmName, tmpShmName);
      std::swap(shm, tmpShm);
      std::swap(region, tmpRegion);
      std::swap(sharedData, tmpSharedData);
      initProcessInfo();
      preOpenWriter();
      msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": enableSWMR: type=writeWithRename: move temp to original file"<<endl;
      checkIfFileIsOpenedBySomeone("File::enableSWMR::prerename", getFilename());
      checkIfFileIsOpenedBySomeone("File::enableSWMR::prerename", filename);
      retryOnLockError(getFilename().string()+","+filename.string(), [this](){
        boost::filesystem::rename(getFilename(), filename);
      });
      if(renameAtomicFunc)
        renameAtomicFunc();
      msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": enableSWMR: type=writeWithRename: deinit shm for temp filename"<<endl;
      deinitShm(tmpSharedData, getFilename(), this, tmpShmName);
  
      preSWMR = false;
  
      msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": enableSWMR: type=writeWithRename: reopen all elements"<<endl;
      openWriter();
    }
    catch(const exception &ex) {
      msg(Atom::Error)<<"HDF5Serie: A exception was thrown during enableSWMR which is rethrown now. This may cause further undefined behaviour:\n"<<
                        "Exception message:\n"<<ex.what()<<endl;
      throw;
    }
    catch(...) {
      msg(Atom::Error)<<"HDF5Serie: A exception was thrown during enableSWMR which is rethrown now. This may cause further undefined behaviour.\n"<<
                        "Exception message: <unknown>"<<endl;
      rethrow_exception(current_exception());
    }
  }

  msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": enableSWMR: call enableSWMR recursively for all elements"<<endl;
  GroupBase::enableSWMR();

  msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": enableSWMR: call H5Fstart_swmr_write"<<endl;
  if(H5Fstart_swmr_write(id)<0)
    throw Exception(getPath(), "enableSWMR() failed: still opened attributes, ...");

  msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": enableSWMR: call H5Fstart_swmr_write done"<<endl;

  {
    ScopedLock lock(sharedData->mutex, this, "enableSWMR");
    // switch the writer state from active to swmr and notify about this change
    msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Set writerState=swmr and notify"<<endl;
    sharedData->writerState=WriterState::swmr;
    sharedData->cond.notify_all();
  }
}

void File::wait(ScopedLock &lock, const std::chrono::milliseconds& relTime,
                string_view blockingMsg, const function<bool()> &pred) {
  if(msgAct(Atom::Debug))
    msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Waiting for: "<<blockingMsg<<endl;
  bool blockMsgPrinted=false;
  auto blockTime = chrono::system_clock::now();
  if(!sharedData->cond.wait_for(lock, relTime, [&pred](){ return pred(); })) {
    // print a message if this call will block
    if(!blockingMsg.empty() && !pred()) {
      msg(Atom::Info)<<getFilename().filename().string()<<": "<<now(blockTime)<<": "<<blockingMsg<<endl;
      blockMsgPrinted=true;
    }
    sharedData->cond.wait(lock, [&pred](){ return pred(); });
  }
  if(blockMsgPrinted)
    msg(Atom::Info)<<getFilename().filename().string()<<": "<<now()<<": Waiting condition passed, continue: "<<blockingMsg<<endl;
  if(msgAct(Atom::Debug))
    msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Waiting condition passed, continue: "<<blockingMsg<<endl;
}

// executed in a thread
void File::listenForRequest() {
  if(msgAct(Atom::Debug))
    msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": Thread started"<<endl;
  // this thread just listens for all notifications and ...
  ScopedLock threadLock(sharedData->mutex, this, "listenForRequest");
  while(true) {
    // ... waits until write request happens (or this thread is to be closed)
    wait(threadLock, 1000*24*60*60*1000ms, "", [this](){ // use a reltime of 1000day for infinity
      return sharedData->writerState==WriterState::writeRequest || // the writer wants to write
             (flushRequested && sharedData->flushRequest==false) || // this object has requested a flush which is done now
             (lastWriterState!=WriterState::none && sharedData->writerState==WriterState::none) || // writer has finished
             exitThread; // this thread should exit
    });
    // if the writer has done is flush after a reqeust OR
    // the writer has finished ...
    if((flushRequested && sharedData->flushRequest==false) ||
       (lastWriterState!=WriterState::none && sharedData->writerState==WriterState::none)) {
      flushRequested=false;
      // ... call the callback to notify the caller of this reader about the finished flush
      if(refreshCallback) {
        if(msgAct(Atom::Debug))
          msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": The writer has flushed this file. Notify the reader."<<endl;
        refreshCallback();
      }
      else {
        if(msgAct(Atom::Debug))
          msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": The writer has flushed this file but the reader does not handle such nofitications."<<endl;
      }
      // do no exit the thread
    }
    lastWriterState=sharedData->writerState;
    // if a write request has happen (we are not here due to a thread exit request) ...
    if(sharedData->writerState==WriterState::writeRequest) {
      // ... call the callback to notify the caller of this reader about this request
      if(closeRequestCallback) {
        if(msgAct(Atom::Debug))
          msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": The writer wants to write this file. Notify the reader."<<endl;
        closeRequestCallback();
      }
      else {
        if(msgAct(Atom::Debug))
          msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": The writer wants to write this file but the reader does not handle such requests."<<endl;
      }
      break; // exit the thread
    }
    if(exitThread)
      break; // exit the thread
  }
}

void File::dumpSharedMemory(const boost::filesystem::path &filename) {
  {
    msgStatic(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<filename.string()<<": Trying to lock the global mutex: dumpSharedMemory"<<endl;
    ipc::scoped_lock lock1(getSyncPrimFileLock().first);
    ipc::scoped_lock lock2(getSyncPrimFileLock().second);
    msgStatic(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<filename.string()<<": File locked: dumpSharedMemory"<<endl;
    // convert filename to valid boost interprocess name (cname)
    string shmName=createShmName(filename);
    SharedMemory shm;
    boost::interprocess::mapped_region region;
    SharedMemObject *sharedData=nullptr;
    try {
      // try to open the shared memory ...
      msgStatic(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<filename.string()<<": Try to open shared memory named "<<shmName<<endl;
      shm=SharedMemory(ipc::open_only, shmName.c_str(), ipc::read_write);
      region=ipc::mapped_region(shm, ipc::read_write); // map memory
      sharedData=static_cast<SharedMemObject*>(region.get_address()); // get pointer
    }
    catch(...) {
      cout<<"The HDF5Serie HDF5 file of the following name does not have an associated shared memory."<<endl;
      cout<<"filename: "<<filename.string()<<endl;
      return;
    }

    cout<<"Dump of the shared memory associated with a HDF5Serie HDF5 file."<<endl;
    cout<<"This dump prints the shared memory WITHOUT locking the memory."<<endl;
    cout<<"filename: "<<filename.string()<<endl;
    cout<<"shared memory name: "<<shmName<<endl;
    cout<<"shmUseCount: "<<sharedData->shmUseCount<<endl;
    {
      ipc::scoped_lock lock(sharedData->mutex, ipc::try_to_lock);
      cout<<"mutex: "<<(lock.owns() ? "unlocked" : "locked")<<endl;
    }
    {
      ipc::scoped_lock lock(sharedData->cond.waiterMutex, ipc::try_to_lock);
      cout<<"cond.mutex: "<<(lock.owns() ? "unlocked" : "locked")<<endl;
    }
    for(auto &waiter : sharedData->cond.waiter)
      cout<<"cond.waiters: UUID="<<waiter<<endl;
    string str;
    switch(sharedData->writerState) {
      case WriterState::none:         str="none";         break;
      case WriterState::writeRequest: str="writeRequest"; break;
      case WriterState::active:       str="active";       break;
      case WriterState::swmr:         str="swmr";         break;
    }
    cout<<"writerState: "<<str<<endl;
    cout<<"activeReaders: "<<sharedData->activeReaders<<endl;
    for(auto &pi : sharedData->processes)
      cout<<"processes: UUID="<<pi.processUUID<<" lastAliveTime="<<pi.lastAliveTime<<" type="<<(pi.type == write ? "write": "read")<<endl;
    cout<<"flushRequest: "<<sharedData->flushRequest<<endl;

    msgStatic(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<filename.string()<<": Unlock: dumpSharedMemory"<<endl;
  }
}

string File::createShmName(const boost::filesystem::path &filename) {
  auto absFilename=boost::filesystem::absolute(filename).lexically_normal().generic_string();
  return "hdf5serie_shm_file_"+to_string(hash<string>{}(absFilename));
}

void File::removeSharedMemory(const boost::filesystem::path &filename) {
  string shmName=createShmName(filename);
  cout<<"Remove shared memory for HDF5Serie HDF5 file."<<endl;
  cout<<"filename: "<<filename.string()<<endl;
  cout<<"shared memory name: "<<shmName<<endl;
  SharedMemoryRemove(shmName.c_str());
}


void File::close() {
  msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": close file"<<endl;
  // close everything (except the file itself)
  GroupBase::close();

  if(id>=0) {
    // check if all object are closed now: if not -> throw internal error (with details about the opened objects)
    ssize_t count=H5Fget_obj_count(id, H5F_OBJ_FILE | H5F_OBJ_DATASET | H5F_OBJ_GROUP | H5F_OBJ_DATATYPE | H5F_OBJ_ATTR | H5F_OBJ_LOCAL);
    if(count<0)
      throw Exception(getPath(), "Internal error: H5Fget_obj_count failed");
    if(count!=1) {
      vector<hid_t> obj(count, 0);
      ssize_t ret=H5Fget_obj_ids(id, H5F_OBJ_DATASET | H5F_OBJ_GROUP | H5F_OBJ_DATATYPE | H5F_OBJ_ATTR | H5F_OBJ_LOCAL, count, &obj[0]);
      if(ret<0)
        throw Exception(getPath(), "Internal error: H5Fget_obj_ids failed");
      vector<char> name(1000+1);
      stringstream err;
      err<<"Internal error: Can not close file since "<<count<<" elements (including the file itself) are still open:"<<endl;
      for(auto it : obj) {
        size_t ret=H5Iget_name(it, &name[0], 1000);
        if(ret<=0)
          throw Exception(getPath(), "Internal error: H5Iget_name");
        err<<"type="<<H5Iget_type(it)<<" name="<<(ret>0?&name[0]:"<no name>")<<endl;
      }
      throw Exception(getPath(), err.str());
    }
  }

  // now close also the file with is now the last opened identifier
  id.reset();
  msg(Atom::Debug)<<"HDF5Serie: "<<now()<<": "<<getFilename().string()<<": close file: done"<<endl;
}

}
