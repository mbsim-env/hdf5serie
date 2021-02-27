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

#include <config.h>
#include <hdf5serie/file.h>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
using namespace fmatvec;
namespace ipc = boost::interprocess;

namespace H5 {

int File::defaultCompression=1;
int File::defaultChunkSize=100;

File::File(const boost::filesystem::path &filename_, FileAccess type_) : GroupBase(nullptr, filename_.string()), filename(filename_), type(type_) {
  if(getenv("HDF5SERIE_DEBUG"))
    setMessageStreamActive(Atom::Debug, true);

  file=this;

  // create inter process shared memory atomically
  {
    msg(Atom::Debug)<<"HDF5Serie: Touch file"<<endl;
    // create file -> to ensure is exists for file locking
    { ofstream str(filename, ios_base::app); }
    // exclusively lock the file to atomically create or open a shared memory associated with this file
    msg(Atom::Debug)<<"HDF5Serie: Getting HDF5 file lock"<<endl;
    ipc::file_lock fileLock(filename.c_str());
    msg(Atom::Debug)<<"HDF5Serie: HDF5 file locked"<<endl;
    ipc::scoped_lock lock(fileLock);
    // convert filename to valid boost interprocess name (cname)
    shmName="hdf5serieShm_";
    auto absFilename=boost::filesystem::absolute(filename).generic_string();
    for(const char &c : absFilename) {
      if(('a'<=c && c<='z') || ('A'<=c && c<='Z') || ('0'<=c && c<='9'))
        shmName+=c;
      else if(c=='/')
        shmName+='_';
      else
        shmName+="_"+to_string(static_cast<unsigned char>(c))+"_";
    }
    try {
      // try to open the shared memory ...
      msg(Atom::Debug)<<"HDF5Serie: Try to open shared memory named "<<shmName<<endl;
      shm=ipc::shared_memory_object(ipc::open_only, shmName.c_str(), ipc::read_write);
      region=ipc::mapped_region(shm, ipc::read_write); // map memory
      sharedData=static_cast<SharedMemObject*>(region.get_address()); // get pointer
    }
    catch(...) {
      // ... if it failed, create the shared memory
      msg(Atom::Debug)<<"HDF5Serie: Opening shared memory failed, create now"<<endl;
      shm=ipc::shared_memory_object(ipc::create_only, shmName.c_str(), ipc::read_write);
      shm.truncate(sizeof(SharedMemObject)); // size the shared memory
      region=ipc::mapped_region(shm, ipc::read_write); // map memory
      sharedData=new(region.get_address())SharedMemObject; // initialize shared memory (by placement new)
    }
    msg(Atom::Debug)<<"HDF5Serie: HDF5 file unlocked"<<endl;
  }
  // now the process shared memory is created or opened atomically and the file lock is releases
  // from now on this shared memory is used for any syncronization/communiation between the processes

  switch(type) {
    case write: openWriter(); break;
    case read:  openReader(); break;
    case dump:  dumpSharedMemory(); break;
  }
}

void File::openWriter() {
  {
    msg(Atom::Debug)<<"HDF5Serie: Lock mutex for writer open"<<endl;
    ipc::scoped_lock lock(sharedData->mutex);
    msg(Atom::Debug)<<"HDF5Serie: Mutex locked, unlock and wait for writerState==none"<<endl;
    // open a writer
    // wait until no other writer is active
    wait(lock, "Blocking until other writer, waiting for reader processes to finish, has finished.", [this](){
      return sharedData->writerState==WriterState::none;
    });
    msg(Atom::Debug)<<"HDF5Serie: writerState==none; number of readers "<<sharedData->activeReaders<<endl;
    // now we are the single writer on this file
    if(sharedData->activeReaders>0) {
      // if readers are still active set the writer state to writeRequest and notify to request that all readers close
      msg(Atom::Debug)<<"HDF5Serie: Set writerState=writeRequest, notify and wait for activeReaders==0"<<endl;
      sharedData->writerState=WriterState::writeRequest;
      sharedData->cond.notify_all();
        // now wait until all readers have closed
      wait(lock, "Blocking until all readers haved closed.", [this](){
        return sharedData->activeReaders==0;
      });
    }
    msg(Atom::Debug)<<"HDF5Serie: Set writerState=active and notify"<<endl;
    // now set the writer state to active (creation of datasets/attributes) and notify about this change
    sharedData->writerState=WriterState::active;
    sharedData->cond.notify_all();
    msg(Atom::Debug)<<"HDF5Serie: Release mutex and create HDF5 file"<<endl;
  }

  // create file
  ScopedHID faid(H5Pcreate(H5P_FILE_ACCESS), &H5Pclose);
  H5Pset_libver_bounds(faid, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
  ScopedHID file_creation_plist(H5Pcreate(H5P_FILE_CREATE), &H5Pclose);
  H5Pset_link_creation_order(file_creation_plist, H5P_CRT_ORDER_TRACKED | H5P_CRT_ORDER_INDEXED);
  id.reset(H5Fcreate(name.c_str(), H5F_ACC_TRUNC, file_creation_plist, faid), &H5Fclose);
}

void File::openReader() {
  {
    msg(Atom::Debug)<<"HDF5Serie: Lock mutex for reader open"<<endl;
    ipc::scoped_lock lock(sharedData->mutex);
    msg(Atom::Debug)<<"HDF5Serie: Mutex locked, unlock and wait for writerState==none or swmr"<<endl;
    // open file as a reader
    // wait until either no writer exists or the writer is in SWMR state
    wait(lock, "Blocking until no writer exists or the writer is in SWMR state.", [this](){
      return sharedData->writerState==WriterState::none || sharedData->writerState==WriterState::swmr;
    });
    // increment the active readers count and notify about this change
    sharedData->activeReaders++;
    msg(Atom::Debug)<<"HDF5Serie: increment activeReaders to "<<sharedData->activeReaders<<" and notify"<<endl;
    sharedData->cond.notify_all();
    // open a thread which listens for futher writer which want to start writing
    msg(Atom::Debug)<<"HDF5Serie: start thread and pass lock to thread"<<endl;
    exitThread=false; // flag indicating that the thread should close -> false at thread start
    readerShouldClose=false; // at start of a reader its ensured that no writer wants to write
    listenForWriterRequestThread=thread(&File::listenForWriterRequest, this, move(lock));
  }

  // open file
  msg(Atom::Debug)<<"HDF5Serie: open HDF5 file"<<endl;
  id.reset(H5Fopen(filename.string().c_str(), H5F_ACC_RDONLY | H5F_ACC_SWMR_READ, H5P_DEFAULT), &H5Fclose);
}

File::~File() {
  switch(type) {
    case write: closeWriter(); break;
    case read:  closeReader(); break;
    case dump:  break;
  }
}

void File::closeWriter() {
  // close writer file
  msg(Atom::Debug)<<"HDF5Serie: close HDF5 writer file"<<endl;
  id.reset();

  {
    msg(Atom::Debug)<<"HDF5Serie: lock mutex"<<endl;
    ipc::scoped_lock lock(sharedData->mutex);
    msg(Atom::Debug)<<"HDF5Serie: set writerState==none and notify"<<endl;
    // close a writer
    // set the writer state to none and notify about this change
    sharedData->writerState=WriterState::none;
    sharedData->cond.notify_all();
    msg(Atom::Debug)<<"HDF5Serie: unlock mutex"<<endl;
  }
}

void File::closeReader() {
  // close reader file
  msg(Atom::Debug)<<"HDF5Serie: close HDF5 reader file"<<endl;
  id.reset();

  {
    msg(Atom::Debug)<<"HDF5Serie: lock mutex"<<endl;
    ipc::scoped_lock lock(sharedData->mutex);
    // close a reader
    // decrements the number of active readers and notify about this change
    sharedData->activeReaders--;
    msg(Atom::Debug)<<"HDF5Serie: decrement activeReaders to "<<sharedData->activeReaders<<
                                                    ", set thread exit flag and notify"<<endl;
    exitThread=true; // set the thread exit flag before notifying to ensure that the thread gets closed
    sharedData->cond.notify_all();
    msg(Atom::Debug)<<"HDF5Serie: unlock mutex"<<endl;
  }
  // the thread should close now and we wait for it to join.
  msg(Atom::Debug)<<"HDF5Serie: join thread"<<endl;
  listenForWriterRequestThread.join();
  msg(Atom::Debug)<<"HDF5Serie: thread joined"<<endl;
}

void File::reloadAfterRequest() {
  if(type!=read)
    throw runtime_error("H5::File::reloadAfterRequest is only possible for readers.");
  closeReader(); // close the file such that a writer which wait can start writing if all reader have closed
  openReader(); // this call blocks until the writer has switched to SWMR mode; then this reader reopens the file
}

void File::refresh() {
  if(type!=read)
    throw Exception(getPath(), "refresh() can only be called for reading files");

  // refresh file
  if(msgActStatic(Atom::Debug))
    msg(Atom::Debug)<<"HDF5Serie: refresh reader"<<endl;
  GroupBase::refresh();
}

void File::flush() {
  if(type!=write)
    throw Exception(getPath(), "flush() can only be called for writing files");

  if(msgActStatic(Atom::Debug))
    msg(Atom::Debug)<<"HDF5Serie: flush writer"<<endl;
  GroupBase::flush();
}

void File::enableSWMR() {
  if(type!=write)
    throw Exception(getPath(), "enableSWMR() can only be called for writing files");
  msg(Atom::Debug)<<"HDF5Serie: enableSWMR in writer and call H5Fstart_swmr_write"<<endl;
  GroupBase::enableSWMR();

  if(H5Fstart_swmr_write(id)<0)
    throw Exception(getPath(), "enableSWMR() failed: still opened attributes, ...");

  {
    msg(Atom::Debug)<<"HDF5Serie: lock mutex"<<endl;
    ipc::scoped_lock lock(sharedData->mutex);
    msg(Atom::Debug)<<"HDF5Serie: set writerState=swmr and notify"<<endl;
    // switch the writer state from active to swmr and notify about this change
    sharedData->writerState=WriterState::swmr;
    sharedData->cond.notify_all();
    msg(Atom::Debug)<<"HDF5Serie: unlock mutex"<<endl;
  }
}

void File::wait(ipc::scoped_lock<ipc::interprocess_mutex> &lock,
                const string &blockingMsg, const function<bool()> &pred) {
  // for for pred to become true but timeout immediately
  if(!sharedData->cond.timed_wait(lock, boost::posix_time::microsec_clock::universal_time(), [&pred](){ return pred(); })) {
    // if timed-out print the blocking message and wait again (for without a timeout)
    if(!blockingMsg.empty())
      msg(Atom::Info)<<blockingMsg<<endl;
    sharedData->cond.wait(lock, [&pred](){ return pred(); });
  }
}

void File::listenForWriterRequest(ipc::scoped_lock<ipc::interprocess_mutex> &&lock) {
  msg(Atom::Debug)<<"HDF5Serie: THREAD: started, move lock to thread scope, unlock and wait "
                                                  "for writerState==writeRequest or thread exit flag"<<endl;
  // this thread just listens for all notifications and ...
  ipc::scoped_lock threadLock(move(lock));
  // ... waits until write request happens (or this thread is to be closed)
  wait(threadLock, "", [this](){
    return sharedData->writerState==WriterState::writeRequest || exitThread;
  });
  // if a write request has happen (we are not here due to a thread exit request) ...
  if(sharedData->writerState==WriterState::writeRequest) {
    // ... set the should close flag which is (hopefully) checked by the caller periodically.
    msg(Atom::Info)<<"HDF5Serie: THREAD: writer wants to write, set readerShouldClose"<<endl;
    readerShouldClose=true; //MFMF maybe a callback to the caller should be used here
  }
  msg(Atom::Debug)<<"HDF5Serie: THREAD: unlock mutex and end thread"<<endl;
}

bool File::shouldClose() {
  return readerShouldClose;
}

void File::dumpSharedMemory() {
  cout<<endl;

  if(!sharedData) {
    cout<<"The HDF5Serie HDF5 file of the following name does not have an associated shared memory."<<endl;
    cout<<"filename: "<<filename.string()<<endl;
    return;
  }

  cout<<"Dump of the shared memory associated with a HDF5Serie HDF5 file."<<endl;
  cout<<"This dump prints the shared memory WITHOUT locking the memory."<<endl;

  cout<<"filename: "<<filename.string()<<endl;

  cout<<"shared memory name: "<<shmName<<endl;

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
}

//mfmf  cout<<"Remove the shared memory associated with a HDF5Serie HDF5 file."<<endl;
//mfmf  cout<<"This is a unsecure operation and may cause other processes using this file to fail."<<endl;
//mfmf  boost::interprocess::shared_memory_object::remove(name.c_str());
}
