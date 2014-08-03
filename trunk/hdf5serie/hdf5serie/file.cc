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
#include <boost/format.hpp>
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/functional/hash.hpp>

using namespace std;
using namespace boost::interprocess;
using namespace boost::posix_time;
using namespace boost::filesystem;

namespace {
  void requestWriterFlush(H5::File::IPC &ipc, H5::File *me);
  void openIPC(H5::File::IPC &ipc, const boost::filesystem::path &filename);
  bool waitForWriterFlush(H5::File::IPC &ipc, H5::File *me);

  set<string> ipcRemove;
  void hdf5SerieAtExit();
  void addIPCRemove(const string &interprocessName);
}

namespace H5 {

int File::defaultCompression=1;
int File::defaultChunkSize=100;

set<File*> File::writerFiles;
set<File*> File::readerFiles;

File::File(const path &filename, FileAccess type_) : GroupBase(NULL, filename.string()), type(type_), isSWMR(false) {
  file=this;
  open();

  interprocessName=canonical(filename).string();
  interprocessName="hdf5serie_"+boost::lexical_cast<string>(boost::hash<string>()(interprocessName));

  if(type==write) {
    writerFiles.insert(this);
    // remove interprocess elements
    shared_memory_object::remove(interprocessName.c_str());
    // create interprocess elements
    ipc.filename=filename;
    ipc.shm=boost::make_shared<shared_memory_object>(create_only, interprocessName.c_str(), read_write);
    ipc.shm->truncate(sizeof(bool)+sizeof(interprocess_mutex)+sizeof(interprocess_condition));
    ipc.shmmap=boost::make_shared<mapped_region>(*ipc.shm, read_write);
    char *ptr=static_cast<char*>(ipc.shmmap->get_address());
    ipc.flushVar=new(ptr) bool(false);              ptr+=sizeof(bool);
    ipc.mutex   =new(ptr) interprocess_mutex();     ptr+=sizeof(interprocess_mutex);
    ipc.cond    =new(ptr) interprocess_condition(); ptr+=sizeof(interprocess_condition);
  }
  else {
    readerFiles.insert(this);
    // try to open interprocess elements
    openIPC(ipc, filename);
  }

  // register atExit
  static bool atExitRegistered=false;
  if(!atExitRegistered)
    if(atexit(hdf5SerieAtExit)!=0)
      throw Exception("Internal error: can not register atexit.");
  if(type==write)
    addIPCRemove(interprocessName);
}


File::~File() {
  if(type==write) {
    writerFiles.erase(this);
  }
  else
    readerFiles.erase(this);

  // remove interprocess elements
  ipc.shmmap.reset();
  ipc.shm.reset();
  if(type==write)
    shared_memory_object::remove(interprocessName.c_str());
  for(vector<IPC>::iterator it=ipcAdd.begin(); it!=ipcAdd.end(); ++it) {
    it->shmmap.reset();
    it->shm.reset();
    if(type==write)
      shared_memory_object::remove(it->interprocessName.c_str());
  }

  close();
}

void File::reopenAsSWMR() {
  if(type==read)
    throw Exception("Can only reopen files opened for writing in SWMR mode");
  if(isSWMR)
    throw Exception("Can only ones reopen files in SWMR mode");

  isSWMR=true;

  close();
  open();
}

void File::reopenAllFilesAsSWMR() {
  for(set<File*>::iterator it=writerFiles.begin(); it!=writerFiles.end(); ++it)
    (*it)->reopenAsSWMR();
}

void File::refresh() {
  if(type==write)
    throw Exception("refresh() can only be called for reading files");

  // refresh file
#ifdef HDF5_SWMR
  GroupBase::refresh();
#else
  close();
  open();
#endif
}

void File::flush() {
  if(type==read)
    throw Exception("flush() can only be called for writing files");

#ifdef HDF5_SWMR
  GroupBase::flush();
#else
  H5Fflush(id, H5F_SCOPE_GLOBAL);
#endif
}

void File::close() {
  GroupBase::close();
  ssize_t count=H5Fget_obj_count(id, H5F_OBJ_ALL | H5F_OBJ_LOCAL);
  if(count>1)
    throw Exception(str(boost::format("Internal error: Can not close file since %1% elements are still open.")%(count-1)));
  id.reset();
}

void File::open() {
  if(type==write) {
    if(!isSWMR) {
      ScopedHID faid(H5Pcreate(H5P_FILE_ACCESS), &H5Pclose);
#ifdef HDF5_SWMR
      H5Pset_libver_bounds(faid, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST); // We ALWAYS select the latest file format for SWMR
#endif
      id.reset(H5Fcreate(name.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, faid), &H5Fclose);
    }
    else {
      unsigned int flag=H5F_ACC_RDWR;
#ifdef HDF5_SWMR
      flag|=H5F_ACC_SWMR_WRITE;
#endif
      id.reset(H5Fopen(name.c_str(), flag, H5P_DEFAULT), &H5Fclose);
    }
  }
  else {
    unsigned int flag=H5F_ACC_RDONLY;
#ifdef HDF5_SWMR
    flag|=H5F_ACC_SWMR_READ;
#endif
    id.reset(H5Fopen(name.c_str(), flag, H5P_DEFAULT), &H5Fclose);
  }
  GroupBase::open();
}



void File::flushIfRequested() {
  bool localFlushVar;
  {
    scoped_lock<interprocess_mutex> lock(*ipc.mutex);
    localFlushVar=*ipc.flushVar;
  }
  if(localFlushVar) {
    if(msgAct(Debug))
      msg(Debug)<<"Flushing HDF5 file "+name+", requested by reader process, and send notification if flush finished."<<endl;
    flush();
    scoped_lock<interprocess_mutex> lock(*ipc.mutex);
    *ipc.flushVar=false;
    ipc.cond->notify_all();
  }
}

void File::flushAllFiles() {
  for(set<File*>::iterator it=writerFiles.begin(); it!=writerFiles.end(); ++it)
    (*it)->flush();
}

void File::flushAllFilesIfRequested() {
  for(set<File*>::iterator it=writerFiles.begin(); it!=writerFiles.end(); ++it)
    (*it)->flushIfRequested();
}

void File::refreshAfterWriterFlush() {
  requestWriterFlush();
  if(waitForWriterFlush())
    refresh();
}

void File::refreshAllFiles() {
  for(set<File*>::iterator it=readerFiles.begin(); it!=readerFiles.end(); ++it)
    (*it)->refresh();
}

void File::refreshAllFilesAfterWriterFlush() {
  for(set<File*>::iterator it=readerFiles.begin(); it!=readerFiles.end(); ++it)
    (*it)->requestWriterFlush();
  vector<bool> refreshNeeded;
  refreshNeeded.reserve(readerFiles.size());
  for(set<File*>::iterator it=readerFiles.begin(); it!=readerFiles.end(); ++it)
    refreshNeeded.push_back((*it)->waitForWriterFlush());
  vector<bool>::iterator nit=refreshNeeded.begin();
  for(set<File*>::iterator it=readerFiles.begin(); it!=readerFiles.end(); ++it, ++nit)
    if(*nit)
      (*it)->refresh();
}

void File::requestWriterFlush() {
  ::requestWriterFlush(ipc, this);
  // post also files with are linked by this file
  for(vector<IPC>::iterator it=ipcAdd.begin(); it!=ipcAdd.end(); ++it)
    ::requestWriterFlush(*it, this);
}

bool File::waitForWriterFlush() {
  bool ret=::waitForWriterFlush(ipc, this);
  // wait also for files with are linked by this file
  for(vector<IPC>::iterator it=ipcAdd.begin(); it!=ipcAdd.end(); ++it)
    if(::waitForWriterFlush(*it, this))
      ret=true;
  return ret;
}

void File::addFileToNotifyOnRefresh(const boost::filesystem::path &filename) {
  IPC ipc;
  openIPC(ipc, filename);
  if(!ipc.shm)
    return;
  ipcAdd.push_back(ipc);
}

}

namespace {

void requestWriterFlush(H5::File::IPC &ipc, H5::File *me) {
  if(!ipc.shm)
    return;
  if(me->msgAct(me->Debug))
    me->msg(me->Debug)<<"Ask writer process to flush hdf5 file "<<ipc.filename.string()<<"."<<endl;
  // post this file
  {
    scoped_lock<interprocess_mutex> lock(*ipc.mutex);
    *ipc.flushVar=true;
  }
  // save current time for later use in timed_wait
  ipc.flushRequestTime=microsec_clock::universal_time();
}

bool waitForWriterFlush(H5::File::IPC &ipc, H5::File *me) {
  if(!ipc.shm)
    return false;
  // get time to wait
  int msec=1000/25; // default wait time is 1/25Hz
  static char *waitTime=getenv("HDF5SERIE_REFRESHWAITTIME");
  if(waitTime)
    msec=boost::lexical_cast<int>(waitTime);
  // wait for file
  bool flushReady=true;
  {
    scoped_lock<interprocess_mutex> lock(*ipc.mutex);
    if(*ipc.flushVar) {
      flushReady=ipc.cond->timed_wait(lock, ipc.flushRequestTime+milliseconds(msec));
    }
  }
  // print message
  if(flushReady) {
    time_duration d=microsec_clock::universal_time()-ipc.flushRequestTime;
    if(me->msgAct(me->Debug))
      me->msg(me->Debug)<<"Flush of writer succsessfull after "<<d.total_milliseconds()<<" msec. Using newest data now."<<endl;
  }
  else
    if(me->msgAct(me->Warn))
      me->msg(me->Warn)<<"Writer process has not flushed hdf5 file "<<ipc.filename.string()<<" after "<<msec<<" msec, continue with maybe not newest data."<<endl;

  return true;
}

void openIPC(H5::File::IPC &ipc, const path &filename) {
  string interprocessName=canonical(filename).string();
  interprocessName="hdf5serie_"+boost::lexical_cast<string>(boost::hash<string>()(interprocessName));
  try {
    ipc.filename=filename;
    ipc.interprocessName=interprocessName;
    ipc.shm=boost::make_shared<shared_memory_object>(open_only, interprocessName.c_str(), read_write);
    ipc.shmmap=boost::make_shared<mapped_region>(*ipc.shm, read_write);
    char *ptr=static_cast<char*>(ipc.shmmap->get_address());
    ipc.flushVar=reinterpret_cast<bool*>                  (ptr); ptr+=sizeof(bool);
    ipc.mutex   =reinterpret_cast<interprocess_mutex*>    (ptr); ptr+=sizeof(interprocess_mutex);
    ipc.cond    =reinterpret_cast<interprocess_condition*>(ptr); ptr+=sizeof(interprocess_condition);
  }
  catch(const interprocess_exception &ex) {
    ipc.shm.reset();
    ipc.shmmap.reset();
  }
}

void hdf5SerieAtExit() {
  for(set<string>::iterator it=ipcRemove.begin(); it!=ipcRemove.end(); ++it)
    shared_memory_object::remove(it->c_str());
  ipcRemove.clear();
}

void addIPCRemove(const string &interprocessName) {
  ipcRemove.insert(interprocessName);
}

}
