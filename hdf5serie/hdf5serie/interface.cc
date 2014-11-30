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
#include <hdf5serie/interface.h>
#include <hdf5serie/group.h>
#include <hdf5serie/file.h>
#include <hdf5serie/simpleattribute.h>
#include <hdf5serie/toh5type.h>
#include <sstream>

using namespace std;

namespace {
  herr_t getChildNamesACB(hid_t, const char *name, const H5A_info_t *, void *op_data) {
    set<string> &ret=*static_cast<set<string>*>(op_data);
    ret.insert(name);
    return 0;
  }

  herr_t errorWalk(unsigned n, const H5E_error2_t *err, void *data) {
    stringstream &ret=*static_cast<stringstream*>(data);
    ret<<"HDF5 error in file "<<err->file_name<<":"<<err->line<<" function "<<err->func_name<<endl<<
         err->desc<<endl;
    return 0;
  }

  herr_t errorHandler(hid_t estack, void *client_data) {
    stringstream str;
    H5Ewalk2(H5E_DEFAULT, H5E_WALK_UPWARD, &errorWalk, &str);
    throw runtime_error(str.str());
  }
}

namespace H5 {

Exception::Exception(const std::string &path_, const std::string &msg_) : path(path_), msg(msg_) {}

Exception::~Exception() throw() {}

const char* Exception::what() const throw() {
  whatMsg="In element "+path+": "+msg;
  return whatMsg.c_str();
}

Element::Element(const std::string &name_) : id(), name(name_) {
  // print errors as exceptions
  static bool firstCall=true;
  if(firstCall) {
    H5Eset_auto2(H5E_DEFAULT, &errorHandler, NULL);
    firstCall=false;
  }
}

Element::~Element() {
}

void Element::close() {
}

void Element::open() {
}

void Element::refresh() {
}

void Element::flush() {
}

Object::Object(GroupBase *parent_, const std::string &name_) : Element(name_), Container<Attribute, Object>(),
  parent(parent_), file(parent?parent->file:NULL) { // parent is NULL only for File which sets file by itself
}

Object::~Object() {
}

Attribute *Object::openChildAttribute(const std::string &name_, ElementType *attributeType, hid_t *type) {
  ScopedHID d(H5Aopen(id, name_.c_str(), H5P_DEFAULT), &H5Dclose);
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
      if(attributeType) *attributeType=simpleAttributeScalar;
#     define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
      if(H5Tequal(ntd, H5TYPE)) \
        return openChildAttribute<SimpleAttribute<CTYPE> >(name_);
#     include "hdf5serie/knowntypes.def"
#     undef FOREACHKNOWNTYPE
      throw Exception(getPath(), "unknown type of dataset");
    case 1:
      if(dims[0]==maxDims[0] && dims[0]!=H5S_UNLIMITED) {
        if(attributeType) *attributeType=simpleAttributeVector;
#       define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
        if(H5Tequal(ntd, H5TYPE)) \
          return openChildAttribute<SimpleAttribute<vector<CTYPE> > >(name_);
#       include "hdf5serie/knowntypes.def"
#       undef FOREACHKNOWNTYPE
        throw Exception(getPath(), "unknown type of attribute");
      }
      throw Exception(getPath(), "unknown dimension of attribute");
    case 2:
      if(dims[0]==maxDims[0] && dims[0]!=H5S_UNLIMITED &&
         dims[1]==maxDims[1] && dims[1]!=H5S_UNLIMITED) {
        if(attributeType) *attributeType=simpleAttributeMatrix;
#       define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
        if(H5Tequal(ntd, H5TYPE)) \
          return openChildAttribute<SimpleAttribute<vector<vector<CTYPE> > > >(name_);
#       include "hdf5serie/knowntypes.def"
#       undef FOREACHKNOWNTYPE
        throw Exception(getPath(), "unknown type of attribute");
      }
      throw Exception(getPath(), "unknown dimension of attribute");
    default:
      throw Exception(getPath(), "unknown dimension of attribute");
  }
}

set<string> Object::getChildAttributeNames() {
  set<string> ret;
  hsize_t idx=0;
  H5Aiterate2(id, H5_INDEX_NAME, H5_ITER_NATIVE, &idx, &getChildNamesACB, &ret);
  return ret;
}

bool Object::hasChildAttribute(const std::string &name_) {
   set<string> names=getChildAttributeNames();
   if(names.find(name_)!=names.end())
     return true;
   return false;
}

void Object::open() {
  Element::open();
  Container<Attribute, Object>::open();
}

void Object::refresh() {
  Element::refresh();
  Container<Attribute, Object>::refresh();
}

void Object::flush() {
  Element::flush();
  Container<Attribute, Object>::flush();
}

void Object::close() {
  Container<Attribute, Object>::close();
  Element::close();
}

Object *Object::getFileAsObject() {
  return getFile();
}

Object *Object::getAttrParent(const string &path, size_t pos) {
  GroupBase *group=dynamic_cast<GroupBase*>(this);
  if(!group)
    throw Exception(getPath(), "Got a path (including /) but this object is not a group");
  return group->openChildObject(path.substr(0, pos));
}

string Object::getPath() {
  return parent ? parent->getPath()+"/"+name : name;
}



Attribute::Attribute(Object *parent_, const std::string &name_) : Element(name_), parent(parent_), file(parent->getFile()) {
}

Attribute::~Attribute() {
}

void Attribute::close() {
  Element::close();
}

void Attribute::open() {
  Element::open();
}

void Attribute::refresh() {
  Element::refresh();
}

void Attribute::flush() {
  Element::flush();
}

string Attribute::getPath() {
  return parent ? parent->getPath()+"/"+name : name;
}

Dataset::Dataset(GroupBase *parent_, const std::string &name_) : Object(parent_, name_) {
}

Dataset::~Dataset() {
}

void Dataset::open() {
  Object::open();
}

void Dataset::refresh() {
  Object::refresh();
#ifdef HDF5_SWMR
  H5Drefresh(id);
#endif
}

void Dataset::flush() {
  Object::flush();
#ifdef HDF5_SWMR
  H5Dflush(id);
#endif
}

void Dataset::close() {
  Object::close();
}

vector<hsize_t> Dataset::getExtentDims() {
  ScopedHID ds(H5Dget_space(id), &H5Sclose);
  hsize_t ndim=H5Sget_simple_extent_ndims(ds);
  vector<hsize_t> dims(ndim);
  vector<hsize_t> maxDims(ndim);
  H5Sget_simple_extent_dims(ds, &dims[0], &maxDims[0]);
  return dims;
}

}
