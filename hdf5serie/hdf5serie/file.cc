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
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

using namespace std;
using namespace fmatvec;
using namespace H5::Internal;
namespace ipc = boost::interprocess;

namespace H5 {

int File::defaultCompression=1;
int File::defaultChunkSize=100;

namespace Internal {
  // This class is exaclty like boost::interprocess::scoped_lock but prints debug messages.
  class ScopedLock : public ipc::scoped_lock<ipc::interprocess_mutex>  {
    public:
      ScopedLock(ipc::interprocess_mutex &mutex, File *self_, string_view msg_) :
                 ipc::scoped_lock<ipc::interprocess_mutex>((initMsg(self_, msg_), mutex)), self(self_), msg(msg_) {
        if(self->msgAct(Atom::Debug))
          self->msg(Atom::Debug)<<"HDF5Serie: "<<self->filename.string()<<": Mutex locked: "<<msg<<endl;
      }
      ScopedLock(ScopedLock &&src, string_view msg_="") :
                 ipc::scoped_lock<ipc::interprocess_mutex>(move(src)), self(src.self), msg(msg_!="" ? msg_ : src.msg) {
        if(self->msgAct(Atom::Debug))
          self->msg(Atom::Debug)<<"HDF5Serie: "<<self->filename.string()<<": Move lock "<<src.msg<<" to "<<msg<<endl;
      }
      ~ScopedLock() {
        if(self->msgAct(Atom::Debug)) {
          if(mutex())
            self->msg(Atom::Debug)<<"HDF5Serie: "<<self->filename.string()<<": Unlock mutex: "<<msg<<endl;
          else
            self->msg(Atom::Debug)<<"HDF5Serie: "<<self->filename.string()<<": Nothing to unlock, moved to other lock: "<<msg<<endl;
        }
      }
    private:
      static void initMsg(File *self, string_view msg) {
        if(self->msgAct(Atom::Debug))
          self->msg(Atom::Debug)<<"HDF5Serie: "<<self->filename.string()<<": Trying to lock mutex: "<<msg<<endl;
      }
      File *self;
      string_view msg;
  };
}

class Settings {
  private:
    static boost::filesystem::path getFileName() {
#if _WIN32
      boost::filesystem::path base(getenv("APPDATA"));
#else
      boost::filesystem::path base(getenv("HOME"));
      base/=".config";
#endif
      return base/"mbsim-env"/"hdf5serie.ini";
    }
  public:
    // Get configuration value of path (e.g. "keepAlive/pingFrequency").
    // If the path does not exits it is added to config files with defaultValue and defaultValue is returned.
    template<class T>
    static T getValue(string path, const T& defaultValue) {
      boost::algorithm::replace_all(path, "/", ".");
      boost::property_tree::ptree pt;
      if(boost::filesystem::exists(getFileName()))
        boost::property_tree::ini_parser::read_ini(getFileName().string(), pt);
      else
        boost::filesystem::create_directories(getFileName().parent_path());
      boost::optional<T> v=pt.get_optional<T>(path);
      if(v)
        return v.get();
      pt.put(path, defaultValue);
      boost::property_tree::ini_parser::write_ini(getFileName().string(), pt);
      return defaultValue;
    }
};

File::File(const boost::filesystem::path &filename_, FileAccess type_,
           const std::function<void()> &closeRequestCallback_,
           const std::function<void()> &refreshCallback_) :
  GroupBase(nullptr, filename_.string()),
  filename(filename_),
  type(type_),
  closeRequestCallback(closeRequestCallback_),
  refreshCallback(refreshCallback_),
  processUUID(boost::uuids::random_generator()()) {
  // HDF5 file locking seems not to work propably, so we dislabe it. We do not need it anyway since proper 
  // file access is fully handled by this class.
  // (with hdf5 >= 1.10.7 this is also possilbe using H5Pset_file_locking
  // but setting this envvar works with hdf5 >= 1.10.0 and even overwrites H5Pset_file_locking
  putenv(const_cast<char*>("HDF5_USE_FILE_LOCKING=FALSE"));

  if(getenv("HDF5SERIE_DEBUG"))
    setMessageStreamActive(Atom::Debug, true);

  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Process UUID = "<<processUUID<<endl;

  file=this;

  openOrCreateShm();

  switch(type) {
    case write: openWriter(); break;
    case read:  openReader(); break;
  }
}

void File::openOrCreateShm() {
  // create inter process shared memory atomically
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Touch file"<<endl;
  // create file -> to ensure is exists for file locking and canonical path
  { ofstream str(filename.string(), ios_base::app); }
  // now the file exists and we can create the shm name (which uses boost::filesystem::canonical)
  shmName=createShmName(filename);
  // exclusively lock the file to atomically create or open a shared memory associated with this file
  ipc::file_lock fileLock(filename.string().c_str());
  {
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Trying to lock file: openOrCreateShm"<<endl;
    ipc::scoped_lock lock(fileLock);
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": File locked: openOrCreateShm"<<endl;
    // convert filename to valid boost interprocess name (cname)
    try {
      // try to open the shared memory ...
      msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Try to open shared memory named "<<shmName<<endl;
      shm=SharedMemory(ipc::open_only, shmName.c_str(), ipc::read_write);
      region=ipc::mapped_region(shm, ipc::read_write); // map memory
      sharedData=static_cast<SharedMemObject*>(region.get_address()); // get pointer
    }
    catch(...) {
      // ... if it failed, create the shared memory
      msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Opening shared memory failed, create now"<<endl;
      shm=SharedMemoryCreate(shmName.c_str(), ipc::read_write, sizeof(SharedMemObject));
      region=ipc::mapped_region(shm, ipc::read_write); // map memory
      sharedData=new(region.get_address())SharedMemObject; // initialize shared memory (by placement new)
    }
    {
      ScopedLock mutexLock(sharedData->mutex, this, "openOrCreateShm, increment shmUseCount");
      sharedData->shmUseCount++;
    }
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Unlock file: openOrCreateShm"<<endl;
  }
  // now the process shared memory is created or opened atomically and the file lock is releases
  // from now on this shared memory is used for any syncronization/communiation between the processes
}

void File::openWriter() {
  {
    ScopedLock lock(sharedData->mutex, this, "openWriter");
    initProcessInfo();
    // open a writer
    // wait until no other writer is active
    wait(lock, "Blocking until other writer has finished.", [this](){
      return sharedData->writerState==WriterState::none;
    });
    // now we are the single writer on this file
    if(sharedData->activeReaders>0) {
      // if readers are still active set the writer state to writeRequest and notify to request that all readers close
      msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Set writerState=writeRequest and notify"<<endl;
      sharedData->writerState=WriterState::writeRequest;
      sharedData->cond.notify_all();
        // now wait until all readers have closed
      wait(lock, "Blocking until all readers haved closed.", [this](){
        return sharedData->activeReaders==0;
      });
    }
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Set writerState=active and notify"<<endl;
    // now set the writer state to active (creation of datasets/attributes) and notify about this change
    sharedData->writerState=WriterState::active;
    sharedData->cond.notify_all();
  }

  // create file
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Create HDF5 file"<<endl;
  ScopedHID faid(H5Pcreate(H5P_FILE_ACCESS), &H5Pclose);
  H5Pset_libver_bounds(faid, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
  H5Pset_fclose_degree(faid, H5F_CLOSE_SEMI);
  ScopedHID file_creation_plist(H5Pcreate(H5P_FILE_CREATE), &H5Pclose);
  H5Pset_link_creation_order(file_creation_plist, H5P_CRT_ORDER_TRACKED | H5P_CRT_ORDER_INDEXED);
  id.reset(H5Fcreate(name.c_str(), H5F_ACC_TRUNC, file_creation_plist, faid), &H5Fclose);
}

void File::initProcessInfo() {
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": init process info and start thread"<<endl;
  // save the process info of this process in shared memory
  if(sharedData->processes.size()==MAXREADERS+1)
    throw Exception(getPath(), "Too many process are accessing this file.");
  sharedData->processes.push_back(ProcessInfo{processUUID,
                                              boost::posix_time::microsec_clock::universal_time(),
                                              type});
  // start thread which updates the still alive timestamp
  stillAlivePingThread=boost::thread(&File::stillAlivePing, this);
}

void File::deinitProcessInfo() {
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Interrupt thread and join"<<endl;
  stillAlivePingThread.interrupt();
  stillAlivePingThread.join();
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Thread joined"<<endl;
  {
    ScopedLock lock(sharedData->mutex, this, "deinitProcessInfo");
    auto it=std::find_if(sharedData->processes.begin(), sharedData->processes.end(), [this](const ProcessInfo &pi) {
      return pi.processUUID==processUUID;
    });
    sharedData->processes.erase(it);
  }
}

void File::stillAlivePing() {
  while(1) {
    {
      ScopedLock lock(sharedData->mutex, this, "stillAlivePing");
      auto curIt=std::find_if(sharedData->processes.begin(), sharedData->processes.end(), [this](const ProcessInfo &pi) {
        return pi.processUUID==processUUID;
      });
      curIt->lastAliveTime=boost::posix_time::microsec_clock::universal_time();

      auto it=sharedData->processes.begin();
      while(it!=sharedData->processes.end()) {
        if(it->lastAliveTime+boost::posix_time::milliseconds(Settings::getValue("keepAlive/fixAfter", 3000))<curIt->lastAliveTime) {
          const static char* HDF5SERIE_NOCRASHFIX=getenv("HDF5SERIE_NOCRASHFIX");
          if(!HDF5SERIE_NOCRASHFIX) {
            msg(Atom::Info)<<"HDF5Serie: Found process with too old keep alive timestamp: "<<it->processUUID<<
                             " Assume that this process crashed. Remove it from shared memory."<<endl;
            if(it->type==read) {
              msg(Atom::Debug)<<"HDF5Serie: Decrement activeReaders and shmUseCount, since a reader seem to have crashed, and notify"<<endl;
              sharedData->activeReaders--;
            }
            else if(it->type==write) {
              msg(Atom::Debug)<<"HDF5Serie: Set writerState=none and decrement shmUseCount, since a writer seem to have crashed, and notify"<<endl;
              sharedData->writerState=WriterState::none;
            }
            sharedData->shmUseCount--;
            it=sharedData->processes.erase(it);
            sharedData->cond.notify_all();
          }
          else {
            msg(Atom::Info)<<"HDF5Serie: Found process with too old keep alive timestamp: "<<it->processUUID<<
                             " Assume that this process crashed but DO NOT remove it from shared memory"
                             " since the envvar HDF5SERIE_NOCRASHFIX is set."<<endl;
          }
        }
        else
          it++;
      }
    }
    // this is a boost thread interruption point
    boost::this_thread::sleep_for(boost::chrono::milliseconds(Settings::getValue("keepAlive/pingFrequency", 1000)));
  }
}

void File::openReader() {
  {
    ScopedLock lock(sharedData->mutex, this, "openReader");
    initProcessInfo();
    // open file as a reader
    // wait until either no writer exists or the writer is in SWMR state
    wait(lock, "Blocking until no writer exists or the writer is in SWMR state.", [this](){
      return sharedData->writerState==WriterState::none || sharedData->writerState==WriterState::swmr;
    });
    lastWriterState=sharedData->writerState;
    // increment the active readers count and notify about this change
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Increment activeReaders and notify"<<endl;
    sharedData->activeReaders++;
    sharedData->cond.notify_all();
    // open a thread which listens for futher writer which want to start writing
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Start thread pass pass lock (openReader) to thread (listenForRequest)"<<endl;
    exitThread=false; // flag indicating that the thread should close -> false at thread start
    listenForRequestThread=thread(&File::listenForRequest, this, move(lock));
  }

  // open file
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Open HDF5 file"<<endl;
  ScopedHID faid(H5Pcreate(H5P_FILE_ACCESS), &H5Pclose);
  H5Pset_fclose_degree(faid, H5F_CLOSE_SEMI);
  id.reset(H5Fopen(filename.string().c_str(), H5F_ACC_RDONLY | H5F_ACC_SWMR_READ, faid), &H5Fclose);
}

File::~File() {
  try
  {
    switch(type) {
      case write:  closeWriter(); break;
      case read:   closeReader(); break;
    }
 
    ipc::file_lock fileLock(filename.string().c_str());//mfmf file_lock are not very portable -> use a named mutex (the mutex in the shm may be obsolte than)
    {
      msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Trying to lock file: dtor"<<endl;
      ipc::scoped_lock lockF(fileLock);
      msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": File locked: dtor"<<endl;
      size_t localShmUseCount;
      {
        ScopedLock lock(sharedData->mutex, this, "dtor");
        sharedData->shmUseCount--;
        localShmUseCount=sharedData->shmUseCount;
        msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Decrement shmUseCount"<<endl;
      }
      // sharedData->shmUseCount cannot be incremente by another process since we sill own the file lock (but the mutex is unlocked now)
      if(localShmUseCount==0) {
        msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Shared memory is no longer used, remove it"<<endl;
        sharedData->~SharedMemObject(); // call destructor of SharedMemObject
        // region does not need destruction
        SharedMemoryRemove(shmName.c_str()); // effectively destructs shm
      }
      msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Unlock file: dtor"<<endl;
    }
  }
  catch(const exception &ex) {
    msg(Atom::Warn)<<"HDF5Serie: Exception during destructor: "<<ex.what()<<endl;
  }
  catch(...) {
    msg(Atom::Warn)<<"HDF5Serie: Unknown exception during destructor."<<endl;
  }
}

void File::closeWriter() {
  // close writer file
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Close HDF5 writer file"<<endl;
  close();

  {
    ScopedLock lock(sharedData->mutex, this, "closeWriter");
    // close a writer
    // set the writer state to none and notify about this change
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Set writerState=none and notify"<<endl;
    sharedData->writerState=WriterState::none;
    sharedData->cond.notify_all();
  }
  deinitProcessInfo();
}

void File::closeReader() {
  // close reader file
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Close HDF5 reader file"<<endl;
  close();

  {
    ScopedLock lock(sharedData->mutex, this, "closeReader");
    // close a reader
    // decrements the number of active readers and notify about this change
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Decrement activeReaders, set thread exit flag and notify"<<endl;
    sharedData->activeReaders--;
    exitThread=true; // set the thread exit flag before notifying to ensure that the thread gets closed
    sharedData->cond.notify_all();
  }
  deinitProcessInfo();
  // the thread should close now and we wait for it to join.
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Join thread"<<endl;
  listenForRequestThread.join();
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Thread joined"<<endl;
}

void File::refresh() {
  if(type!=read)
    throw Exception(getPath(), "refresh() can only be called for reading files");

  GroupBase::refresh();
}

void File::requestFlush() {
  ScopedLock lock(sharedData->mutex, this, "requestFlush");
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Set flushRequest and notify"<<endl;
  sharedData->flushRequest=true;
  flushRequested=true;
  sharedData->cond.notify_all(); // not really needed since we assume that the writer is polling on this flag frequently.
}

void File::flushIfRequested() {
  if(type!=write)
    throw Exception(getPath(), "flushIfRequested() can only be called for writing files");

  {
    ScopedLock lock(sharedData->mutex, this, "flushIfRequested, before flush");
    if(!sharedData->flushRequest) {
      if(msgAct(Atom::Debug)) msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": No flush request"<<endl;
      return;
    }
  }

  // flush file (and datasets) and reset flushRequest flag and notify
  if(msgAct(Atom::Debug)) msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Flushing now"<<endl;
  GroupBase::flush();

  ScopedLock lock(sharedData->mutex, this, "flushIfRequested, after flush");
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Unset flushRequest and notify"<<endl;
  sharedData->flushRequest=false;
  sharedData->cond.notify_all();
}

void File::enableSWMR() {
  if(type!=write)
    throw Exception(getPath(), "enableSWMR() can only be called for writing files");
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": enableSWMR in writer and call H5Fstart_swmr_write"<<endl;
  GroupBase::enableSWMR();

  if(H5Fstart_swmr_write(id)<0)
    throw Exception(getPath(), "enableSWMR() failed: still opened attributes, ...");

  {
    ScopedLock lock(sharedData->mutex, this, "enableSWMR");
    // switch the writer state from active to swmr and notify about this change
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Set writerState=swmr and notify"<<endl;
    sharedData->writerState=WriterState::swmr;
    sharedData->cond.notify_all();
  }
}

void File::wait(ScopedLock &lock,
                string_view blockingMsg, const function<bool()> &pred) {
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Waiting for: "<<blockingMsg<<endl;
  // for for pred to become true but timeout immediately
  if(!sharedData->cond.timed_wait(lock, boost::posix_time::microsec_clock::universal_time(), [&pred](){ return pred(); })) {
    // if timed-out print the blocking message and wait again (for without a timeout)
    if(!blockingMsg.empty())
      msg(Atom::Info)<<"HDF5Serie: "<<filename.string()<<": "<<blockingMsg<<endl;
    sharedData->cond.wait(lock, [&pred](){ return pred(); });
  }
}

void File::listenForRequest(ScopedLock &&lock) {
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Thread started, move lock to thread scope"<<endl;
  // this thread just listens for all notifications and ...
  ScopedLock threadLock(move(lock), "listenForRequest");
  while(1) {
    // ... waits until write request happens (or this thread is to be closed)
    wait(threadLock, "", [this](){
      return sharedData->writerState==WriterState::writeRequest || // a writer wants to write
             (flushRequested && sharedData->flushRequest==false) || // this object has requested a flush which is done now
             (lastWriterState!=WriterState::none && sharedData->writerState==WriterState::none) || // writer has finished
             exitThread; // this thread should exit
    });
    // if a writer has done is flush after a reqeust OR
    // a writer has finished ...
    if((flushRequested && sharedData->flushRequest==false) ||
       (lastWriterState!=WriterState::none && sharedData->writerState==WriterState::none)) {
      flushRequested=false;
      // ... call the callback to notify the caller of this reader about the finished flush
      if(refreshCallback) {
        msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": A writer has flushed this file. Notify the reader."<<endl;
        refreshCallback();
      }
      else
        msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": A writer has flushed this file but the reader does not handle such nofitications."<<endl;
      // do no exit the thread
    }
    lastWriterState=sharedData->writerState;
    // if a write request has happen (we are not here due to a thread exit request) ...
    if(sharedData->writerState==WriterState::writeRequest) {
      // ... call the callback to notify the caller of this reader about this request
      if(closeRequestCallback) {
        msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": A writer wants to write this file. Notify the reader."<<endl;
        closeRequestCallback();
      }
      else
        msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": A writer wants to write this file but the reader does not handle such requests."<<endl;
      break; // exit the thread
    }
    if(exitThread)
      break; // exit the thread
  }
}

void File::dumpSharedMemory(const boost::filesystem::path &filename) {
  // exclusively lock the file to atomically open the shared memory associated with this file
  ipc::file_lock fileLock(filename.string().c_str());
  {
    msgStatic(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Trying to lock file: dumpSharedMemory"<<endl;
    ipc::scoped_lock lock(fileLock);
    msgStatic(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": File locked: dumpSharedMemory"<<endl;
    // convert filename to valid boost interprocess name (cname)
    string shmName=createShmName(filename);
    SharedMemory shm;
    boost::interprocess::mapped_region region;
    SharedMemObject *sharedData=nullptr;
    try {
      // try to open the shared memory ...
      msgStatic(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Try to open shared memory named "<<shmName<<endl;
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
    // cond: nothing to dump
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

    msgStatic(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Unlock file: dumpSharedMemory"<<endl;
  }
}

string File::createShmName(const boost::filesystem::path &filename) {
  string shmName="hdf5serieShm_";
  auto absFilename=boost::filesystem::canonical(filename).generic_string();
  for(const char &c : absFilename) {
    if(('a'<=c && c<='z') || ('A'<=c && c<='Z') || ('0'<=c && c<='9'))
      shmName+=c;
    else if(c=='/')
      shmName+='_';
    else
      shmName+="_"+to_string(static_cast<unsigned char>(c))+"_";
  }
  return shmName;
}

void File::removeSharedMemory(const boost::filesystem::path &filename) {
  string shmName=createShmName(filename);
  cout<<"Remove shared memory for HDF5Serie HDF5 file."<<endl;
  cout<<"filename: "<<filename.string()<<endl;
  cout<<"shared memory name: "<<shmName<<endl;
  SharedMemoryRemove(shmName.c_str());
}


void File::close() {
  // close everything (except the file itself)
  GroupBase::close();

  // check if all object are closed now: if not -> throw internal error (with details about the opened objects)
  ssize_t count=H5Fget_obj_count(id, H5F_OBJ_DATASET | H5F_OBJ_GROUP | H5F_OBJ_DATATYPE | H5F_OBJ_ATTR | H5F_OBJ_LOCAL);
  if(count<0)
    throw Exception(getPath(), "Internal error: H5Fget_obj_count failed");
  if(count>0) {
    vector<hid_t> obj(count, 0);
    ssize_t ret=H5Fget_obj_ids(id, H5F_OBJ_DATASET | H5F_OBJ_GROUP | H5F_OBJ_DATATYPE | H5F_OBJ_ATTR | H5F_OBJ_LOCAL, count, &obj[0]);
    if(ret<0)
      throw Exception(getPath(), "Internal error: H5Fget_obj_ids failed");
    vector<char> name(1000+1);
    stringstream err;
    err<<"Internal error: Can not close file since "<<count<<" elements are still open:"<<endl;
    for(auto it : obj) {
      size_t ret=H5Iget_name(it, &name[0], 1000);
      if(ret<=0)
        throw Exception(getPath(), "Internal error: H5Iget_name");
      err<<"type="<<H5Iget_type(it)<<" name="<<(ret>0?&name[0]:"<no name>")<<endl;
    }
    throw Exception(getPath(), err.str());
  }

  // now close also the file with is now the last opened identifier
  id.reset();
}

}
