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
#include <boost/optional.hpp>

using namespace std;
using namespace boost::filesystem;

namespace {
  herr_t getChildNamesLCB(hid_t, const char *name, const H5L_info_t *, void *op_data) {
    pair<boost::optional<exception>, set<string>> &ret=*static_cast<pair<boost::optional<exception>, set<string>>*>(op_data);
    try {
      ret.second.insert(name);
    }
    catch(exception &ex) {
      ret.first=ex;
    }
    catch(...) {
      ret.first=runtime_error("Unknown exception during getChildNamesLCB");
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
  H5Sget_simple_extent_dims(sd, &dims[0], &maxDims[0]);
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
      throw Exception(getPath(), "unknown type of dataset");
    case 1:
      if(dims[0]==maxDims[0] && dims[0]!=H5S_UNLIMITED) {
        if(objectType) *objectType=simpleDatasetVector;
#       define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
        if(H5Tequal(ntd, H5TYPE)) \
          return openChildObject<SimpleDataset<vector<CTYPE> > >(name_);
#       include "hdf5serie/knowntypes.def"
#       undef FOREACHKNOWNTYPE
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
        throw Exception(getPath(), "unknown type of dataset");
      }
      throw Exception(getPath(), "unknown dimension of dataset");
    default:
      throw Exception(getPath(), "unknown dimension of dataset");
  }
}

set<string> GroupBase::getChildObjectNames() {
  pair<boost::optional<exception>, set<string>> ret;
  hsize_t idx=0;
  H5Literate(id, H5_INDEX_NAME, H5_ITER_NATIVE, &idx, &getChildNamesLCB, &ret);
  if(ret.first)
    throw ret.first.get();
  return ret.second;
}

bool GroupBase::hasChildObject(const string &name_) {
   set<string> names=getChildObjectNames();
   return names.find(name_)!=names.end();
}

void GroupBase::close() {
  Container<Object, GroupBase>::close();
  Object::close();
}

void GroupBase::open() {
  Object::open();
  Container<Object, GroupBase>::open();
}

void GroupBase::refresh() {
  Object::refresh();
  Container<Object, GroupBase>::refresh();
}

void GroupBase::flush() {
  Object::flush();
  Container<Object, GroupBase>::flush();
}

bool GroupBase::isExternalLink(const string &name_) {
  H5L_info_t link;
  H5Lget_info(id, name_.c_str(), &link, H5P_DEFAULT);
  return link.type==H5L_TYPE_EXTERNAL;
}

pair<path, string> GroupBase::getExternalLink(const string &name_) {
  H5L_info_t link_buff;
  H5Lget_info(id, name_.c_str(), &link_buff, H5P_DEFAULT);
  vector<char> buff(link_buff.u.val_size);
  H5Lget_val(id, name_.c_str(), &buff[0], link_buff.u.val_size, H5P_DEFAULT);
  const char *linkFilename;
  const char *obj_path;
  H5Lunpack_elink_val(&buff[0], link_buff.u.val_size, nullptr, &linkFilename, &obj_path);
  return make_pair(absolute(linkFilename, path(getFile()->getName()).parent_path()), obj_path);
  //MFMF use same algo for retrun.first as in doc of H5Lcreate_external
}

void GroupBase::createExternalLink(const string &name_, const pair<boost::filesystem::path, string> &target) {
  H5Lcreate_external(target.first.string().c_str(), target.second.c_str(), id, name_.c_str(), H5P_DEFAULT, H5P_DEFAULT);
}

void GroupBase::createSoftLink(const string &name_, const string &target) {
  H5Lcreate_soft( target.c_str(), id, name_.c_str(), H5P_DEFAULT, H5P_DEFAULT);
}

void GroupBase::handleExternalLink(const string &name_) {
  if(!isExternalLink(name_))
    return;
  pair<path, string> link=getExternalLink(name_);
  getFile()->addFileToNotifyOnRefresh(link.first);
}

GroupBase *GroupBase::getFileAsGroup() {
  return getFile();
}



Group::Group(int dummy, GroupBase *parent_, const string &name_) : GroupBase(parent_, name_) {
  open();
}

Group::Group(GroupBase *parent_, const string &name_) : GroupBase(parent_, name_) {
  id.reset(H5Gcreate2(parent->getID(), name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), &H5Gclose);
}

Group::~Group() {
  close();
}

void Group::close() {
  GroupBase::close();
  id.reset();
}

void Group::open() {
  id.reset(H5Gopen2(parent->getID(), name.c_str(), H5P_DEFAULT), &H5Gclose);
  GroupBase::open();
}

void Group::refresh() {
  GroupBase::refresh();
}

void Group::flush() {
  GroupBase::flush();
}

}
