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

#include <boost/interprocess/shared_memory_object.hpp>
#include <config.h>
#include <hdf5serie/file.h>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace std;
using namespace fmatvec;
namespace ipc = boost::interprocess;

namespace H5 {

int File::defaultCompression=1;
int File::defaultChunkSize=100;

File::File(const boost::filesystem::path &filename_, FileAccess type_,
           const std::function<void()> &closeRequestCallback_) :
  GroupBase(nullptr, filename_.string()),
  filename(filename_),
  type(type_),
  closeRequestCallback(closeRequestCallback_),
  shmName(createShmName(filename)),
  processUUID(boost::uuids::random_generator()()) {

  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Process UUID = "<<processUUID<<endl;

  if(getenv("HDF5SERIE_DEBUG"))
    setMessageStreamActive(Atom::Debug, true);

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
  // create file -> to ensure is exists for file locking
  { ofstream str(filename.string(), ios_base::app); }
  // exclusively lock the file to atomically create or open a shared memory associated with this file
  ipc::file_lock fileLock(filename.c_str());
  {
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Locking HDF5 file"<<endl;
    ipc::scoped_lock lock(fileLock);
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": HDF5 file locked"<<endl;
    // convert filename to valid boost interprocess name (cname)
    try {
      // try to open the shared memory ...
      msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Try to open shared memory named "<<shmName<<endl;
      shm=ipc::shared_memory_object(ipc::open_only, shmName.c_str(), ipc::read_write);
      region=ipc::mapped_region(shm, ipc::read_write); // map memory
      sharedData=static_cast<SharedMemObject*>(region.get_address()); // get pointer
    }
    catch(...) {
      // ... if it failed, create the shared memory
      msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Opening shared memory failed, create now"<<endl;
      shm=ipc::shared_memory_object(ipc::create_only, shmName.c_str(), ipc::read_write);
      shm.truncate(sizeof(SharedMemObject)); // size the shared memory
      region=ipc::mapped_region(shm, ipc::read_write); // map memory
      sharedData=new(region.get_address())SharedMemObject; // initialize shared memory (by placement new)
    }
    {
      msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": lock mutex"<<endl;
      ipc::scoped_lock mutexLock(sharedData->mutex);
      msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": incrment shmUseCount and unlock mutex"<<endl;
      sharedData->shmUseCount++;
    }
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": HDF5 file unlocked"<<endl;
  }
  // now the process shared memory is created or opened atomically and the file lock is releases
  // from now on this shared memory is used for any syncronization/communiation between the processes
}

void File::openWriter() {
  {
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Lock mutex for writer open"<<endl;
    ipc::scoped_lock lock(sharedData->mutex);
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Mutex locked"<<endl;
    initProcessInfo();
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Unlock mutex and wait for writerState==none"<<endl;
    // open a writer
    // wait until no other writer is active
    wait(lock, "Blocking until other writer has finished.", [this](){
      return sharedData->writerState==WriterState::none;
    });
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": writerState==none; number of readers "<<sharedData->activeReaders<<endl;
    // now we are the single writer on this file
    if(sharedData->activeReaders>0) {
      // if readers are still active set the writer state to writeRequest and notify to request that all readers close
      msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Set writerState=writeRequest, notify and wait for activeReaders==0"<<endl;
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
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Release mutex and create HDF5 file"<<endl;
  }

  // create file
  ScopedHID faid(H5Pcreate(H5P_FILE_ACCESS), &H5Pclose);
  H5Pset_libver_bounds(faid, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
  ScopedHID file_creation_plist(H5Pcreate(H5P_FILE_CREATE), &H5Pclose);
  H5Pset_link_creation_order(file_creation_plist, H5P_CRT_ORDER_TRACKED | H5P_CRT_ORDER_INDEXED);
  id.reset(H5Fcreate(name.c_str(), H5F_ACC_TRUNC, file_creation_plist, faid), &H5Fclose);
}

void File::initProcessInfo() {
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": init process info"<<endl;
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
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": deinit process info. Interrupt and join keep alive thread"<<endl;
  stillAlivePingThread.interrupt();
  stillAlivePingThread.join();
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": keep alive thread joined. Remove process info from shared memory"<<endl;
  auto it=std::find_if(sharedData->processes.begin(), sharedData->processes.end(), [this](const ProcessInfo &pi) {
    return pi.processUUID==processUUID;
  });
  sharedData->processes.erase(it);
}

void File::stillAlivePing() {
  while(1) {
    boost::this_thread::sleep_for(boost::chrono::milliseconds(1000)); // this is a boost thread interruption point // mfmf configure time
//    if(msgAct(Atom::Debug)) msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Lock mutex for still alive ping"<<endl;
    ipc::scoped_lock lock(sharedData->mutex);
//    if(msgAct(Atom::Debug)) msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Mutex locked and update keep alive timestamp"<<endl;
    auto curIt=std::find_if(sharedData->processes.begin(), sharedData->processes.end(), [this](const ProcessInfo &pi) {
      return pi.processUUID==processUUID;
    });
    curIt->lastAliveTime=boost::posix_time::microsec_clock::universal_time();

    auto it=sharedData->processes.begin();
    while(it!=sharedData->processes.end()) {
      if(it->lastAliveTime+boost::posix_time::milliseconds(3000)<curIt->lastAliveTime) { // mfmf configure time
        msg(Atom::Info)<<"HDF5Serie: Found process with too old keep alive timestamp: "<<it->processUUID<<
                          " Assume that this process crashed. Remove it from shared memory."<<endl;
        if(it->type==read) {
//          msg(Atom::Debug)<<"HDF5Serie: decrement reader state, since a reader seem to have crashed"<<endl;
          sharedData->activeReaders--;
        }
        else if(it->type==write) {
//          msg(Atom::Debug)<<"HDF5Serie: set writer state=none, since a writer seem to have crashed"<<endl;
          sharedData->writerState=WriterState::none;
        }
//        msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Decrement shmUseCount"<<endl;
        sharedData->shmUseCount--;
        it=sharedData->processes.erase(it);
        sharedData->cond.notify_all();
      }
      else
        it++;
    }

//    if(msgAct(Atom::Debug)) msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Unlock mutex"<<endl;
  }
}

void File::openReader() {
  {
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Lock mutex for reader open"<<endl;
    ipc::scoped_lock lock(sharedData->mutex);
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Mutex locked"<<endl;
    initProcessInfo();
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Unlock mutex and wait for writerState==none or swmr"<<endl;
    // open file as a reader
    // wait until either no writer exists or the writer is in SWMR state
    wait(lock, "Blocking until no writer exists or the writer is in SWMR state.", [this](){
      return sharedData->writerState==WriterState::none || sharedData->writerState==WriterState::swmr;
    });
    // increment the active readers count and notify about this change
    sharedData->activeReaders++;
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": increment activeReaders to "<<sharedData->activeReaders<<" and notify"<<endl;
    sharedData->cond.notify_all();
    // open a thread which listens for futher writer which want to start writing
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": start thread and pass lock to thread"<<endl;
    exitThread=false; // flag indicating that the thread should close -> false at thread start
    listenForWriterRequestThread=thread(&File::listenForWriterRequest, this, move(lock));
  }

  // open file
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": open HDF5 file"<<endl;
  id.reset(H5Fopen(filename.string().c_str(), H5F_ACC_RDONLY | H5F_ACC_SWMR_READ, H5P_DEFAULT), &H5Fclose);
}

File::~File() {
  try
  {
    switch(type) {
      case write:  closeWriter(); break;
      case read:   closeReader(); break;
    }

    ipc::file_lock fileLock(filename.c_str());
    {
      msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Locking HDF5 file"<<endl;
      ipc::scoped_lock lockF(fileLock);
      msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": file locked"<<endl;
      size_t localShmUseCount;
      {
        msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": lock mutex"<<endl;
        ipc::scoped_lock lock(sharedData->mutex);
        msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": mutex locked, and decrement shmUseCount"<<endl;
        sharedData->shmUseCount--;
        localShmUseCount=sharedData->shmUseCount;
        msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": unlock mutex"<<endl;
      }
      // sharedData->shmUseCount cannot be incremente by another process since we sill own the file lock (but the mutex is unlocked now)
      if(localShmUseCount==0) {
        msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": shared memory no longer used, remove it"<<endl;
        sharedData->~SharedMemObject(); // call destructor of SharedMemObject
        // region does not need destruction
        ipc::shared_memory_object::remove(shmName.c_str()); // effectively destructs shm
      }
      msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": unlock file"<<endl;
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
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": close HDF5 writer file"<<endl;
  close();

  {
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": lock mutex"<<endl;
    ipc::scoped_lock lock(sharedData->mutex);
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": set writerState==none and notify"<<endl;
    // close a writer
    // set the writer state to none and notify about this change
    sharedData->writerState=WriterState::none;
    deinitProcessInfo();
    sharedData->cond.notify_all();
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": unlock mutex"<<endl;
  }
}

void File::closeReader() {
  // close reader file
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": close HDF5 reader file"<<endl;
  close();

  {
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": lock mutex"<<endl;
    ipc::scoped_lock lock(sharedData->mutex);
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": mutex locked"<<endl;
    // close a reader
    // decrements the number of active readers and notify about this change
    sharedData->activeReaders--;
    deinitProcessInfo();
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": decrement activeReaders to "<<sharedData->activeReaders<<
                                                    ", set thread exit flag and notify"<<endl;
    exitThread=true; // set the thread exit flag before notifying to ensure that the thread gets closed
    sharedData->cond.notify_all();
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": unlock mutex"<<endl;
  }
  // the thread should close now and we wait for it to join.
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": join thread"<<endl;
  listenForWriterRequestThread.join();
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": thread joined"<<endl;
}

void File::refresh() {
  if(type!=read)
    throw Exception(getPath(), "refresh() can only be called for reading files");

  // refresh file
  if(msgActStatic(Atom::Debug))
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": refresh reader"<<endl;
  GroupBase::refresh();
}

void File::flush() {
  if(type!=write)
    throw Exception(getPath(), "flush() can only be called for writing files");

  if(msgActStatic(Atom::Debug))
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": flush writer"<<endl;
  GroupBase::flush();
}

void File::enableSWMR() {
  if(type!=write)
    throw Exception(getPath(), "enableSWMR() can only be called for writing files");
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": enableSWMR in writer and call H5Fstart_swmr_write"<<endl;
  GroupBase::enableSWMR();

  if(H5Fstart_swmr_write(id)<0)
    throw Exception(getPath(), "enableSWMR() failed: still opened attributes, ...");

  {
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": lock mutex"<<endl;
    ipc::scoped_lock lock(sharedData->mutex);
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": mutex locked, set writerState=swmr and notify"<<endl;
    // switch the writer state from active to swmr and notify about this change
    sharedData->writerState=WriterState::swmr;
    sharedData->cond.notify_all();
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": unlock mutex"<<endl;
  }
}

void File::wait(ipc::scoped_lock<ipc::interprocess_mutex> &lock,
                const string &blockingMsg, const function<bool()> &pred) {
  // for for pred to become true but timeout immediately
  if(!sharedData->cond.timed_wait(lock, boost::posix_time::microsec_clock::universal_time(), [&pred](){ return pred(); })) {
    // if timed-out print the blocking message and wait again (for without a timeout)
    if(!blockingMsg.empty())
      msg(Atom::Info)<<"HDF5Serie: "<<filename.string()<<": "<<blockingMsg<<endl;
    sharedData->cond.wait(lock, [&pred](){ return pred(); });
    msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Not blocking for: "<<blockingMsg<<endl;
  }
}

void File::listenForWriterRequest(ipc::scoped_lock<ipc::interprocess_mutex> &&lock) {
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": THREAD: started, move lock to thread scope, unlock and wait "
                                                  "for writerState==writeRequest or thread exit flag"<<endl;
  // this thread just listens for all notifications and ...
  ipc::scoped_lock threadLock(move(lock));
  // ... waits until write request happens (or this thread is to be closed)
  wait(threadLock, "", [this](){
    return sharedData->writerState==WriterState::writeRequest || exitThread;
  });
  // if a write request has happen (we are not here due to a thread exit request) ...
  if(sharedData->writerState==WriterState::writeRequest) {
    // ... call the callback to notify the caller of this reader about this request
    if(closeRequestCallback) {
      msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": A writer wants to write this file. Notify this reader."<<endl;
      closeRequestCallback();
    }
    else
      msg(Atom::Info)<<"HDF5Serie: "<<filename.string()<<": A writer wants to write this file but this reader does not handle such requests."<<endl;
  }
  msg(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": THREAD: unlock mutex and end thread"<<endl;
}

void File::dumpSharedMemory(const boost::filesystem::path &filename) {
  // exclusively lock the file to atomically open the shared memory associated with this file
  ipc::file_lock fileLock(filename.c_str());
  {
    msgStatic(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Locking HDF5 file"<<endl;
    ipc::scoped_lock lock(fileLock);
    msgStatic(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": HDF5 file locked"<<endl;
    // convert filename to valid boost interprocess name (cname)
    string shmName=createShmName(filename);
    boost::interprocess::shared_memory_object shm;
    boost::interprocess::mapped_region region;
    SharedMemObject *sharedData=nullptr;
    try {
      // try to open the shared memory ...
      msgStatic(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": Try to open shared memory named "<<shmName<<endl;
      shm=ipc::shared_memory_object(ipc::open_only, shmName.c_str(), ipc::read_write);
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

    msgStatic(Atom::Debug)<<"HDF5Serie: "<<filename.string()<<": HDF5 file unlocked"<<endl;
  }
}

string File::createShmName(const boost::filesystem::path &filename) {
  string shmName="hdf5serieShm_";
  auto absFilename=boost::filesystem::absolute(filename).generic_string();
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
  ipc::shared_memory_object::remove(shmName.c_str());
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
