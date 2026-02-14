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
#include <utility>
#include <optional>

using namespace std;

namespace {
  vector<H5::ErrorInfo> globalErrorStack;

  herr_t getChildNamesACB(hid_t, const char *name, const H5A_info_t *, void *op_data) {
    pair<exception_ptr, set<string>> &ret=*static_cast<pair<exception_ptr, set<string>>*>(op_data);
    try {
      ret.second.insert(name);
    }
    catch(...) {
      ret.first=current_exception();
    }
    return 0;
  }

  herr_t errorWalk(unsigned n, const H5E_error2_t *err, void *data) {
    auto &globalErrorStack = *static_cast<vector<H5::ErrorInfo>*>(data);
    try {
      globalErrorStack.emplace_back(err->cls_id, err->maj_num, err->min_num, err->line, err->func_name, err->file_name, err->desc);
    }
    catch(...) {
      cerr<<"Error printing error message. This should never happen."<<endl;
    }
    return 0;
  }

  herr_t errorHandler(hid_t estack, void *client_data) {
    if(H5Ewalk2(H5E_DEFAULT, H5E_WALK_UPWARD, &errorWalk, client_data)<0)
      cerr<<"Error printing error message (H5Ewalk2 failed). This should never happen."<<endl;
    return 0;
  }
}

namespace H5 {

Exception::Exception(std::string path_, std::string msg_, const std::vector<ErrorInfo> &errorStack_) : path(std::move(path_)), msg(std::move(msg_)) {
  if(errorStack_.empty()) {
    errorStack = std::move(globalErrorStack);
    globalErrorStack.clear();
  }
  else
    errorStack = errorStack_;
}

Exception::~Exception() noexcept = default;

const char* Exception::what() const noexcept {
  if(!path.empty())
    whatMsg = "HDF5 error at: "+path+":";
  else
    whatMsg = "HDF5 error:";

  whatMsg += "\n"+msg;

  if(!errorStack.empty()) {
    whatMsg += "\nError details from HDF5 (stacktrace):";
    auto tostr = [](const char* s) {
      return s ? s : "<unknown>";
    };
    for(auto &ei : errorStack) {
      whatMsg += "\n- "+ei.desc;
      if(fmatvec::Atom::msgActStatic(fmatvec::Atom::Debug)) {
        whatMsg += "\n  major: "s+tostr(H5Eget_major(ei.maj_num));
        whatMsg += "\n  minor: "s+tostr(H5Eget_minor(ei.min_num));
        whatMsg += "\n  source file: "s+ei.file_name;
        whatMsg += "\n  source func: "s+ei.func_name;
        whatMsg += "\n  source line: "s+to_string(ei.line);
        whatMsg += "\n  class id: "s+to_string(ei.cls_id);
      }
    }
  }

  return whatMsg.c_str();
}

Element::Element(std::string name_) :  name(std::move(name_)) {
  // print errors as exceptions
  static bool firstCall=true;
  if(firstCall) {
    checkCall(H5Eset_auto2(H5E_DEFAULT, &errorHandler, &globalErrorStack));
    firstCall=false;
  }
}

Element::~Element() = default;

void Element::refresh() {
}

void Element::close() {
}

void Element::flush() {
}

void Element::enableSWMR() {
}

Object::Object(GroupBase *parent_, const std::string &name_) : Element(name_), 
  parent(parent_), file(parent?parent->file:nullptr) { // parent is NULL only for File which sets file by itself
}

Object::~Object() = default;

Attribute *Object::openChildAttribute(const std::string &name_, ElementType *attributeType, ScopedHID *type) {
  ScopedHID d(H5Aopen(id, name_.c_str(), H5P_DEFAULT), &H5Dclose);
  ScopedHID sd(H5Dget_space(d), &H5Sclose);
  hsize_t ndim=H5Sget_simple_extent_ndims(sd);
  vector<hsize_t> dims(ndim);
  vector<hsize_t> maxDims(ndim);
  checkCall(H5Sget_simple_extent_dims(sd, &dims[0], &maxDims[0]));
  ScopedHID td(H5Dget_type(d), &H5Tclose);
  ScopedHID ntdTmp(H5Tget_native_type(td, H5T_DIR_ASCEND), &H5Tclose);
  reference_wrapper<ScopedHID> ntd(ntdTmp);
  if(type) {
    *type = std::move(ntdTmp);
    ntd = *type; // rebind the reference_wrapper
  }
  switch(ndim) {
    case 0:
      if(attributeType) *attributeType=simpleAttributeScalar;
#     define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
      if(H5Tequal(ntd.get(), H5TYPE)) \
        return openChildAttribute<SimpleAttribute<CTYPE> >(name_);
#     include "hdf5serie/knowntypes.def"
#     undef FOREACHKNOWNTYPE
      // fixed length string types need special handling
      if(H5Tget_class(ntd.get()) == H5T_STRING) // a variable length string is already handled
        return openChildAttribute<SimpleAttribute<string> >(name_);
      throw Exception(getPath(), "unknown type of dataset");
    case 1:
      if(dims[0]==maxDims[0] && dims[0]!=H5S_UNLIMITED) {
        if(attributeType) *attributeType=simpleAttributeVector;
#       define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
        if(H5Tequal(ntd.get(), H5TYPE)) \
          return openChildAttribute<SimpleAttribute<vector<CTYPE> > >(name_);
#       include "hdf5serie/knowntypes.def"
#       undef FOREACHKNOWNTYPE
        // fixed length string types need special handling
        if(H5Tget_class(ntd.get()) == H5T_STRING) // a variable length string is already handled
          return openChildAttribute<SimpleAttribute<vector<string> > >(name_);
        throw Exception(getPath(), "unknown type of attribute");
      }
      throw Exception(getPath(), "unknown dimension of attribute");
    case 2:
      if(dims[0]==maxDims[0] && dims[0]!=H5S_UNLIMITED &&
         dims[1]==maxDims[1] && dims[1]!=H5S_UNLIMITED) {
        if(attributeType) *attributeType=simpleAttributeMatrix;
#       define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
        if(H5Tequal(ntd.get(), H5TYPE)) \
          return openChildAttribute<SimpleAttribute<vector<vector<CTYPE> > > >(name_);
#       include "hdf5serie/knowntypes.def"
#       undef FOREACHKNOWNTYPE
        // fixed length string types need special handling
        if(H5Tget_class(ntd.get()) == H5T_STRING) // a variable length string is already handled
          return openChildAttribute<SimpleAttribute<vector<vector<string> > > >(name_);
        throw Exception(getPath(), "unknown type of attribute");
      }
      throw Exception(getPath(), "unknown dimension of attribute");
    default:
      throw Exception(getPath(), "unknown dimension of attribute");
  }
}

set<string> Object::getChildAttributeNames() {
  pair<exception_ptr, set<string>> ret { nullptr, {} };
  hsize_t idx=0;
  checkCall(H5Aiterate2(id, H5_INDEX_NAME, H5_ITER_NATIVE, &idx, &getChildNamesACB, &ret));
  if(ret.first)
    rethrow_exception(ret.first);
  return ret.second;
}

bool Object::hasChildAttribute(const std::string &name_) {
   set<string> names=getChildAttributeNames();
   return names.find(name_)!=names.end();
}

void Object::refresh() {
  Element::refresh();
  Container<Attribute, Object>::refresh();
}

void Object::flush() {
  Element::flush();
  Container<Attribute, Object>::flush();
}

void Object::enableSWMR() {
  Element::enableSWMR();
  Container<Attribute, Object>::enableSWMR();
}

void Object::close() {
  Container<Attribute, Object>::close();
  Element::close();
}

Object *Object::getFileAsObject() {
  return getFile();
}

Object *Object::getAttrParent(const string &path, size_t pos) {
  auto *group=dynamic_cast<GroupBase*>(this);
  if(!group)
    throw Exception(getPath(), "Got a path (including /) but this object is not a group");
  return group->openChildObject(path.substr(0, pos));
}

string Object::getPath() {
  return parent ? parent->getPath()+"/"+name : name;
}



Attribute::Attribute(Object *parent_, const std::string &name_) : Element(name_), parent(parent_), file(parent->getFile()) {
}

Attribute::~Attribute() = default;

void Attribute::close() {
  Element::close();
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

Dataset::~Dataset() = default;

void Dataset::refresh() {
  Object::refresh();
  checkCall(H5Drefresh(id));
}

void Dataset::flush() {
  Object::flush();
  checkCall(H5Dflush(id));
}

void Dataset::enableSWMR() {
  Object::enableSWMR();
}

void Dataset::close() {
  Object::close();
}

vector<hsize_t> Dataset::getExtentDims() {
  ScopedHID ds(H5Dget_space(id), &H5Sclose);
  hsize_t ndim=H5Sget_simple_extent_ndims(ds);
  vector<hsize_t> dims(ndim);
  vector<hsize_t> maxDims(ndim);
  checkCall(H5Sget_simple_extent_dims(ds, &dims[0], &maxDims[0]));
  return dims;
}

}
