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

File::File(const path &filename, FileAccess type_) : GroupBase(nullptr, filename.string()), type(type_), isSWMR(false) {
  file=this;
  open();
}


File::~File() {
  close();
}

void File::reopenAsSWMR() {
  if(type==read)
    throw Exception(getPath(), "Can only reopen files opened for writing in SWMR mode");

  isSWMR=true;

  close();
  open();
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

void File::open() {
  if(type==write) {
    if(!isSWMR) {
      ScopedHID faid(H5Pcreate(H5P_FILE_ACCESS), &H5Pclose);
      H5Pset_libver_bounds(faid, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
      hid_t file_creation_plist = H5Pcreate(H5P_FILE_CREATE);
      H5Pset_link_creation_order(file_creation_plist, H5P_CRT_ORDER_TRACKED | H5P_CRT_ORDER_INDEXED);
      id.reset(H5Fcreate(name.c_str(), H5F_ACC_TRUNC, file_creation_plist, faid), &H5Fclose);
    }
    else {
      id.reset(H5Fopen(name.c_str(), H5F_ACC_RDWR | H5F_ACC_SWMR_WRITE, H5P_DEFAULT), &H5Fclose);
    }
  }
  else {
    id.reset(H5Fopen(name.c_str(), H5F_ACC_RDONLY | H5F_ACC_SWMR_READ, H5P_DEFAULT), &H5Fclose);
  }
  GroupBase::open();
}

}
