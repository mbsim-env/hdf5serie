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

using namespace std;
using namespace boost::filesystem;

namespace H5 {

int File::defaultCompression=1;
int File::defaultChunkSize=100;

File::File(const path &filename, FileAccess type_) : GroupBase(nullptr, filename.string()), type(type_) {
  file=this;

  if(type==write) {
    ScopedHID faid(H5Pcreate(H5P_FILE_ACCESS), &H5Pclose);
    H5Pset_libver_bounds(faid, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
    ScopedHID file_creation_plist(H5Pcreate(H5P_FILE_CREATE), &H5Pclose);
    H5Pset_link_creation_order(file_creation_plist, H5P_CRT_ORDER_TRACKED | H5P_CRT_ORDER_INDEXED);
    id.reset(H5Fcreate(name.c_str(), H5F_ACC_TRUNC, file_creation_plist, faid), &H5Fclose);
  }
  else {
    id.reset(H5Fopen(name.c_str(), H5F_ACC_RDONLY | H5F_ACC_SWMR_READ, H5P_DEFAULT), &H5Fclose);
  }
}


File::~File() {
}

void File::refresh() {
  if(type==write)
    throw Exception(getPath(), "refresh() can only be called for reading files");

  // refresh file
  GroupBase::refresh();
}

void File::flush() {
  if(type==read)
    throw Exception(getPath(), "flush() can only be called for writing files");

  GroupBase::flush();
}

void File::enableSWMR() {
  if(type==read)
    throw Exception(getPath(), "enableSWMR() can only be called for writing files");
  GroupBase::enableSWMR();

  if(H5Fstart_swmr_write(id)<0)
    throw Exception(getPath(), "enableSWMR() failed: still opened attributes, ...");
}

}
