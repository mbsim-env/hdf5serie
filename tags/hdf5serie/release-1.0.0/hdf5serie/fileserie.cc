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
 *   mafriedrich@users.berlios.de
 *
 */

#include <config.h>
#include <hdf5serie/fileserie.h>
#include <fstream>
#include <unistd.h>
#include <libgen.h>
#include <iostream>
#ifdef HAVE_ANSICSIGNAL
#  include <signal.h>
#endif

using namespace H5;
using namespace std;

list<FileSerie*> FileSerie::openedFile;
bool FileSerie::flushOnes=false;

void FileSerie::sigUSR2Handler(int i) {
  cout<<"HDF5Serie: Received USR2 signal! Ask for flushing files!"<<endl;
  flushOnes=true;
}

void FileSerie::flushAllFiles() {
  cout<<"HDF5Serie: Flushing files!"<<endl;
  for(list<FileSerie*>::iterator i=openedFile.begin(); i!=openedFile.end(); ++i)
    (*i)->flush(H5F_SCOPE_GLOBAL);
  flushOnes=false;
}

FileSerie::FileSerie(const char *name, unsigned int flags,
                     const FileCreatPropList &create_plist,
                     const FileAccPropList &access_plist) : H5File(name, flags, create_plist, access_plist) {
  ofstream lockFile((string(dirname((char*)name))+"/."+basename((char*)name)+".pid").c_str());
  lockFile<<getpid()<<endl;
  lockFile.close();
  openedFile.push_back(this);
#ifdef HAVE_ANSICSIGNAL
  signal(SIGUSR2, sigUSR2Handler);
#endif
}

FileSerie::FileSerie(const H5std_string &name, unsigned int flags,
                     const FileCreatPropList &create_plist,
                     const FileAccPropList &access_plist) : H5File(name, flags, create_plist, access_plist) {
  ofstream lockFile((string(dirname((char*)name.c_str()))+"/."+basename((char*)name.c_str())+".pid").c_str());
  lockFile<<getpid()<<endl;
  lockFile.close();
  openedFile.push_back(this);
#ifdef HAVE_ANSICSIGNAL
  signal(SIGUSR2, sigUSR2Handler);
#endif
}

FileSerie::~FileSerie() {
  ::unlink((string(dirname((char*)getFileName().c_str()))+"/."+basename((char*)getFileName().c_str())+".pid").c_str());
  openedFile.remove(this);
}

void FileSerie::close() {
  ::unlink((string(dirname((char*)getFileName().c_str()))+"/."+basename((char*)getFileName().c_str())+".pid").c_str());
  H5File::close();
  openedFile.remove(this);
}

void FileSerie::openFile(const H5std_string &name, unsigned int flags, const FileAccPropList &access_plist) {
  H5File::openFile(name, flags, access_plist);
  ofstream lockFile((string(dirname((char*)name.c_str()))+"/."+basename((char*)name.c_str())+".pid").c_str());
  lockFile<<getpid()<<endl;
  lockFile.close();
  openedFile.push_back(this);
}

void FileSerie::openFile(const char *name, unsigned int flags, const FileAccPropList &access_plist) {
  H5File::openFile(name, flags, access_plist);
  ofstream lockFile((string(dirname((char*)name))+"/."+basename((char*)name)+".pid").c_str());
  lockFile<<getpid()<<endl;
  lockFile.close();
  openedFile.push_back(this);
}

void FileSerie::deletePIDFiles() {
  for(list<FileSerie*>::iterator i=openedFile.begin(); i!=openedFile.end(); ++i)
    ::unlink((string(dirname((char*)(*i)->getFileName().c_str()))+"/."+basename((char*)(*i)->getFileName().c_str())+".pid").c_str());
}
