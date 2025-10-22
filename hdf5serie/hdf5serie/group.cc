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
#include <hdf5serie/group.h>
#include <hdf5serie/simpledataset.h>
#include <hdf5serie/vectorserie.h>
#include <hdf5serie/toh5type.h>
#include <vector>
#include <optional>

using namespace std;
using namespace boost::filesystem;

namespace {
  herr_t getChildNamesLCB(hid_t, const char *name, const H5L_info_t *, void *op_data) {
    pair<exception_ptr, list<string>> &ret=*static_cast<pair<exception_ptr, list<string>>*>(op_data);
    try {
      ret.second.emplace_back(name);
    }
    catch(...) {
      ret.first=current_exception();
    }
    return 0;
  }
}

namespace H5 {

GroupBase::GroupBase(int dummy, GroupBase *parent_, const string &name_) : Object(parent_, name_) {
}

GroupBase::GroupBase(GroupBase *parent_, const string &name_) : Object(parent_, name_) {
}

GroupBase::~GroupBase() = default;

Object *GroupBase::openChildObject(const string &name_, ElementType *objectType, hid_t *type) {
  ScopedHID o(H5Oopen(id, name_.c_str(), H5P_DEFAULT), &H5Oclose);
  H5I_type_t t=H5Iget_type(o);
  if(t==H5I_BADID)
    throw Exception(getPath(), "Can not get type");
  switch(t) {
    case H5I_GROUP:
      return openChildObject<Group>(name_);
    case H5I_DATASET:
      return openChildDataset(name_, objectType, type);
    default:
      throw Exception(getPath(), "internal error: unknown type");
  }
}

Dataset *GroupBase::openChildDataset(const string &name_, ElementType *objectType, hid_t *type) {
  ScopedHID d(H5Dopen(id, name_.c_str(), H5P_DEFAULT), &H5Dclose);
  ScopedHID sd(H5Dget_space(d), &H5Sclose);
  hsize_t ndim=H5Sget_simple_extent_ndims(sd);
  vector<hsize_t> dims(ndim);
  vector<hsize_t> maxDims(ndim);
  checkCall(H5Sget_simple_extent_dims(sd, &dims[0], &maxDims[0]));
  ScopedHID td(H5Dget_type(d), &H5Tclose);
  ScopedHID ntd(H5Tget_native_type(td, H5T_DIR_ASCEND), &H5Tclose);
  if(type) *type=ntd;
  switch(ndim) {
    case 0:
      if(objectType) *objectType=simpleDatasetScalar;
#     define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
      if(H5Tequal(ntd, H5TYPE)) \
        return openChildObject<SimpleDataset<CTYPE> >(name_);
#     include "hdf5serie/knowntypes.def"
#     undef FOREACHKNOWNTYPE
      // fixed length string types need special handling
      if(H5Tget_class(ntd) == H5T_STRING) // a variable length string is already handled
        return openChildObject<SimpleDataset<string> >(name_);
      throw Exception(getPath(), "unknown type of dataset");
    case 1:
      if(dims[0]==maxDims[0] && dims[0]!=H5S_UNLIMITED) {
        if(objectType) *objectType=simpleDatasetVector;
#       define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
        if(H5Tequal(ntd, H5TYPE)) \
          return openChildObject<SimpleDataset<vector<CTYPE> > >(name_);
#       include "hdf5serie/knowntypes.def"
#       undef FOREACHKNOWNTYPE
        // fixed length string types need special handling
        if(H5Tget_class(ntd) == H5T_STRING) // a variable length string is already handled
          return openChildObject<SimpleDataset<vector<string> > >(name_);
        throw Exception(getPath(), "unknown type of dataset");
      }
      throw Exception(getPath(), "unknown dimension of dataset");
    case 2:
      if(dims[0]==maxDims[0] && dims[0]!=H5S_UNLIMITED &&
         dims[1]==maxDims[1] && dims[1]!=H5S_UNLIMITED) {
        if(objectType) *objectType=simpleDatasetMatrix;
#       define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
        if(H5Tequal(ntd, H5TYPE)) \
          return openChildObject<SimpleDataset<vector<vector<CTYPE> > > >(name_);
#       include "hdf5serie/knowntypes.def"
#       undef FOREACHKNOWNTYPE
        // fixed length string types need special handling
        if(H5Tget_class(ntd) == H5T_STRING) // a variable length string is already handled
          return openChildObject<SimpleDataset<vector<vector<string> > > >(name_);
        throw Exception(getPath(), "unknown type of dataset");
      }
      if(maxDims[0]==H5S_UNLIMITED &&
         dims[1]==maxDims[1] && dims[1]!=H5S_UNLIMITED) {
        if(objectType) *objectType=vectorSerie;
#       define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
        if(H5Tequal(ntd, H5TYPE)) \
          return openChildObject<VectorSerie<CTYPE> >(name_);
#       include "hdf5serie/knowntypes.def"
#       undef FOREACHKNOWNTYPE
        // fixed length string types need special handling
        if(H5Tget_class(ntd) == H5T_STRING) // a variable length string is already handled
          return openChildObject<VectorSerie<string> >(name_);
        throw Exception(getPath(), "unknown type of dataset");
      }
      throw Exception(getPath(), "unknown dimension of dataset");
    default:
      throw Exception(getPath(), "unknown dimension of dataset");
  }
}

list<string> GroupBase::getChildObjectNames() {
  pair<exception_ptr, list<string>> ret { nullptr, {} };
  hsize_t idx=0;
  checkCall(H5Literate(id, H5_INDEX_CRT_ORDER, H5_ITER_INC, &idx, &getChildNamesLCB, &ret));
  if(ret.first)
    rethrow_exception(ret.first);
  return ret.second;
}

void GroupBase::close() {
  Container<Object, GroupBase>::close();
  Object::close();
}

void GroupBase::refresh() {
  Object::refresh();
  Container<Object, GroupBase>::refresh();
}

void GroupBase::flush() {
  Object::flush();
  Container<Object, GroupBase>::flush();
}

void GroupBase::enableSWMR() {
  Object::enableSWMR();
  Container<Object, GroupBase>::enableSWMR();
}

GroupBase *GroupBase::getFileAsGroup() {
  return getFile();
}



Group::Group(int dummy, GroupBase *parent_, const string &name_) : GroupBase(parent_, name_) {
  id.reset(H5Gopen2(parent->getID(), name.c_str(), H5P_DEFAULT), &H5Gclose);
}

Group::Group(GroupBase *parent_, const string &name_) : GroupBase(parent_, name_) {
  ScopedHID group_creation_plist(H5Pcreate(H5P_GROUP_CREATE), &H5Pclose);
  checkCall(H5Pset_link_creation_order(group_creation_plist, H5P_CRT_ORDER_TRACKED | H5P_CRT_ORDER_INDEXED));
  id.reset(H5Gcreate2(parent->getID(), name.c_str(), H5P_DEFAULT, group_creation_plist, H5P_DEFAULT), &H5Gclose);
}

Group::~Group() = default;

void Group::close() {
  GroupBase::close();
  id.reset();
}

void Group::refresh() {
  GroupBase::refresh();
}

void Group::flush() {
  GroupBase::flush();
}

void Group::enableSWMR() {
  if(file->getType(true) == File::writeWithRename)
    id.reset(H5Gopen2(parent->getID(), name.c_str(), H5P_DEFAULT), &H5Gclose);
  GroupBase::enableSWMR();
}

}
