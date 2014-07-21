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
#include <boost/filesystem/fstream.hpp>
#include <boost/thread/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <signal.h>

using namespace std;
namespace bfs=boost::filesystem;

namespace {
  void sendUserSignal(pid_t pid) {
    if(kill(pid, SIGUSR2)==0) {
#ifndef HDF5_SWMR
      int msec=50;
      static char *sleep=getenv("HDF5SERIE_REFRESHSLEEP");
      if(sleep)
        msec=boost::lexical_cast<int>(sleep);
      //only newer boost version: boost::this_thread::sleep_for(boost::chrono::milliseconds(msec));
      boost::this_thread::sleep(boost::posix_time::milliseconds(msec)); // boost deprecated
#endif
    }
  }
}

namespace H5 {

int File::defaultCompression=1;
int File::defaultChunkSize=100;
set<File*> File::writerFiles;
bool File::flushAllFilesRequested=false;

File::File(const bfs::path &filename, FileAccess type_) : GroupBase(NULL, filename.string()), type(type_), isSWMR(false) {
  file=this;

  pidFilename=filename.parent_path()/("."+filename.filename().string()+".pid");
  writerExists=false;
  if(type==write) {
    writerFiles.insert(this);
    // create pid file
    bfs::ofstream f(pidFilename);
    f<<getpid()<<endl;
    // install signal handler
    signal(SIGUSR2, &sigUsr2Handler);
  }
  else {
    // get pid of writing process
    bfs::ifstream f(pidFilename);
    if(!f.fail()) {
      f>>writerPID;
      writerExists=true;
      // flush writer file in writer process
      sendUserSignal(writerPID);
    }
  }

  open();
}


File::~File() {
  if(type==write) {
    // remove pid file
    bfs::remove(pidFilename);
  }

  close();

  writerFiles.erase(this);
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

void File::refresh() {
  if(type==write)
    throw Exception("refresh() can only be called for reading files");

  // flush writer file in writer process
  if(writerExists)
    sendUserSignal(writerPID);

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

void File::sigUsr2Handler(int) {
  flushAllFilesRequested=true;
}

void File::flushAllFiles(bool onlyIfRequrestedBySignal) {
  if(onlyIfRequrestedBySignal && !flushAllFilesRequested)
    return;
  if(onlyIfRequrestedBySignal && flushAllFilesRequested)
    msgStatic(Info)<<"Flushing all HDF5 files (requested by signal)."<<endl;
  flushAllFilesRequested=false;
  for(set<File*>::iterator it=File::writerFiles.begin(); it!=File::writerFiles.end(); ++it)
    (*it)->flush();
}

}
