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
#include <boost/interprocess/sync/named_semaphore.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/named_condition.hpp>
#include <boost/functional/hash.hpp>

using namespace std;
using namespace boost::interprocess;
namespace bfs=boost::filesystem;

namespace {
  void requestWriterFlush(H5::File::IPC &ipc, H5::File *me);
  void openIPC(H5::File::IPC &ipc, const boost::filesystem::path &filename);
  bool waitForWriterFlush(H5::File::IPC &ipc, H5::File *me);
}

namespace H5 {

int File::defaultCompression=1;
int File::defaultChunkSize=100;

set<File*> File::writerFiles;
set<File*> File::readerFiles;

File::File(const bfs::path &filename, FileAccess type_) : GroupBase(NULL, filename.string()), type(type_), isSWMR(false) {
  file=this;
  open();

  interprocessBasename=bfs::canonical(filename).string();
  interprocessBasename="hdf5serie_"+boost::lexical_cast<string>(boost::hash<string>()(interprocessBasename));

  if(type==write) {
    writerFiles.insert(this);
    // remove interprocess elements
    named_semaphore::remove((interprocessBasename+"_sem").c_str());
    named_mutex::remove((interprocessBasename+"_condmutex").c_str());
    named_condition::remove((interprocessBasename+"_cond").c_str());
    // create interprocess elements
    ipc.filename=filename;
    ipc.sem  =boost::make_shared<named_semaphore>(create_only, (interprocessBasename+"_sem").c_str(), 0);
    ipc.mutex=boost::make_shared<named_mutex>    (create_only, (interprocessBasename+"_condmutex").c_str());
    ipc.cond =boost::make_shared<named_condition>(create_only, (interprocessBasename+"_cond").c_str());
  }
  else {
    readerFiles.insert(this);
    // try to open interprocess elements
    openIPC(ipc, filename);
    ::requestWriterFlush(ipc, this);
    if(::waitForWriterFlush(ipc, this));
      refresh();
  }
}


File::~File() {
  if(type==write) {
    writerFiles.erase(this);
    // remove interprocess elements
    named_semaphore::remove((interprocessBasename+"_sem").c_str());
    named_mutex::remove((interprocessBasename+"_condmutex").c_str());
    named_condition::remove((interprocessBasename+"_cond").c_str());
  }
  else
    readerFiles.erase(this);

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
    if(!isSWMR)
      id.reset(H5Fcreate(name.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT), &H5Fclose);
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
  if(!ipc.sem->try_wait())
    return;
  while(ipc.sem->try_wait()); // decrement up to zero
  msg(Info)<<"Flushing HDF5 file "+name+", requested by reader process, and send notification if flush finished."<<endl;
  flush();
  ipc.cond->notify_all();
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
  if(ipc.sem)
    ::requestWriterFlush(ipc, this);
  // post also files with are linked by this file
  for(vector<IPC>::iterator it=ipcAdd.begin(); it!=ipcAdd.end(); ++it)
    ::requestWriterFlush(*it, this);
}

bool File::waitForWriterFlush() {
  ::waitForWriterFlush(ipc, this);
  // wait also for files with are linked by this file
  for(vector<IPC>::iterator it=ipcAdd.begin(); it!=ipcAdd.end(); ++it)
    ::waitForWriterFlush(*it, this);

  return true;
}

void File::addFileToNotifyOnRefresh(const boost::filesystem::path &filename) {
  IPC ipc;
  openIPC(ipc, filename);
  if(!ipc.sem)
    return;
  ipcAdd.push_back(ipc);

  ::requestWriterFlush(ipc, this);
  if(::waitForWriterFlush(ipc, this))
    refresh();
}

}

namespace {

void requestWriterFlush(H5::File::IPC &ipc, H5::File *me) {
  if(!ipc.sem)
    return;
  me->msg(me->Info)<<"Ask writer process to flush hdf5 file "<<ipc.filename.string()<<"."<<endl;
  // post this file
  ipc.sem->post();
  // save current time for later use in timed_wait
  ipc.flushRequestTime=boost::posix_time::second_clock::universal_time();
}

bool waitForWriterFlush(H5::File::IPC &ipc, H5::File *me) {
  if(!ipc.sem)
    return false;
  // get time to wait
  int msec=1000;
  static char *sleep=getenv("HDF5SERIE_REFRESHWAITTIME");
  if(sleep)
    msec=boost::lexical_cast<int>(sleep);
  // wait for file
  bool flushReady;
  {
     scoped_lock<named_mutex> lock(*ipc.mutex);
     flushReady=ipc.cond->timed_wait(lock, ipc.flushRequestTime+boost::posix_time::milliseconds(msec));
  }
  // print message
  if(flushReady)
    me->msg(me->Info)<<"Flush of writer succsessfull. Using newest data now."<<endl;
  else
    me->msg(me->Warn)<<"Writer process has not flushed hdf5 file "<<ipc.filename.string()<<" after "<<msec<<" msec, continue with maybe not newest data."<<endl;

  return true;
}

void openIPC(H5::File::IPC &ipc, const bfs::path &filename) {
  string interprocessBasename=bfs::canonical(filename).string();
  interprocessBasename="hdf5serie_"+boost::lexical_cast<string>(boost::hash<string>()(interprocessBasename));
  try {
    ipc.filename=filename;
    ipc.sem  =boost::make_shared<named_semaphore>(open_only, (interprocessBasename+"_sem").c_str());
    ipc.mutex=boost::make_shared<named_mutex>    (open_only, (interprocessBasename+"_condmutex").c_str());
    ipc.cond =boost::make_shared<named_condition>(open_only, (interprocessBasename+"_cond").c_str());
  }
  catch(const interprocess_exception &ex) {
    ipc.sem  .reset();
    ipc.mutex.reset();
    ipc.cond .reset();
  }
}

}
