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

#ifndef _HDF5SERIE_INTERFACE_H_
#define _HDF5SERIE_INTERFACE_H_

#include <fmatvec/atom.h>
#include <hdf5.h>
#include <map>
#include <set>
#include <utility>
#include <vector>

namespace H5 {

  #define HDF5SERIE_MAXCTORPARAMETERS 5

  class ScopedHID {
    protected:
      typedef herr_t (*CloseFunc)(hid_t id);
      hid_t id{-1};
      CloseFunc closeFunc{nullptr};
    public:
      ScopedHID()   = default;
      ScopedHID(hid_t id_, CloseFunc closeFunc_) : id(id_), closeFunc(closeFunc_) {}
      ~ScopedHID() {
        reset();
      }
      operator hid_t() {
        return id;
      }
      void reset(hid_t id_=-1, CloseFunc closeFunc_=nullptr) {
        if(id>=0)
          closeFunc(id);
        id=id_;
        closeFunc=closeFunc_;
      }
  };

  class Exception : public std::exception {
    protected:
      std::string path;
      std::string msg;
      mutable std::string whatMsg;
    public:
      explicit Exception(std::string path_, std::string msg_);
      ~Exception() noexcept override;
      const char* what() const noexcept override;
  };

  class File;
  class GroupBase;
  class Attribute;
  class Object;
  template<class Child, class Self> class Container;

  enum ElementType {
    simpleDatasetScalar,
    simpleDatasetVector,
    simpleDatasetMatrix,
    vectorSerie,
    simpleAttributeScalar,
    simpleAttributeVector,
    simpleAttributeMatrix
  };

  class Element : virtual public fmatvec::Atom {
    friend class Container<Attribute, Object>;
    friend class Container<Object, GroupBase>;
    protected:
      ScopedHID id;
      std::string name;
      Element(std::string name_);
      ~Element() override;
      virtual void close();
      virtual void open();
      virtual void refresh();
      virtual void flush();
    public:
      //! Note: use the returned hid_t only temporarily since its value may change, at least when File::reopenAsSWMR is called.
      hid_t getID() { return id; }
      std::string getName() { return name; }
  };

  // a container of objects of type Child which have itself a parent of type Self
  template<class Child, class Self>
  class Container {
    protected:
      Container() = default;
      ~Container() {
        for(auto it=childs.begin(); it!=childs.end(); ++it)
          delete it->second;
      }
      void close() {
        for(auto it=childs.begin(); it!=childs.end(); ++it)
          it->second->close();
      }
      void open() {
        for(auto it=childs.begin(); it!=childs.end(); ++it)
          it->second->open();
      }
      void refresh() {
        for(auto it=childs.begin(); it!=childs.end(); ++it)
          it->second->refresh();
      }
      void flush() {
        for(auto it=childs.begin(); it!=childs.end(); ++it)
          it->second->flush();
      }
      std::map<std::string, Child*> childs;

      // create a objet of class T which is derived from Child
      template<class T>
      class Creator {
        protected:
          Self *self;
          std::string name;
          std::map<std::string, Child*> &childs;
        public:
          Creator(Self *self_, std::string name_, std::map<std::string, Child*> &childs_) :
            self(self_), name(std::move(name_)), childs(childs_) {}

          template<typename... Args>
          T* operator()(Args&&... args) {
            auto ret=childs.insert(std::pair<std::string, Child*>(name, NULL));
            if(!ret.second)
              throw Exception(self->getPath(), "A element of name "+name+" already exists.");
            try {
              auto* r=new T(static_cast<Self*>(self), name, std::forward<Args>(args)...);
              ret.first->second=r;
              return r;
            }
            catch(...) {
              childs.erase(name);
              throw;
            }
          }
      };
      template<class T>
      Creator<T> createChild(const std::string &name_) {
        if(name_.find_first_of('/')!=std::string::npos)
          throw Exception(static_cast<Self*>(this)->getPath(), "Internal error: must be a relative name, not absolute or a path");
        return Creator<T>(static_cast<Self*>(this), name_, childs);
      }

      template<class T>
      T* openChild(const std::string &name_) {
        if(name_.find_first_of('/')!=std::string::npos)
          throw Exception(static_cast<Self*>(this)->getPath(), "Internal error: must be a relative name, not absolute or a path");
        std::pair<typename std::map<std::string, Child*>::iterator, bool> ret=childs.insert(std::pair<std::string, Child*>(name_, NULL));
        if(!ret.second) {
          auto *o=dynamic_cast<T*>(ret.first->second);
          if(!o)
            std::runtime_error("The element "+name_+" if of other type.");
          return o;
        }
        try {
          auto* r=new T(0, static_cast<Self*>(this), name_);
          ret.first->second=r;
          return r;
        }
        catch(...) {
          childs.erase(name_);
          throw;
        }
      }
  };

  class Object : public Element, public Container<Attribute, Object> {
    friend class Container<Object, GroupBase>;
    protected:
      Object(GroupBase *parent_, const std::string &name_);
      ~Object() override;
      void close() override;
      void open() override;
      void refresh() override;
      void flush() override;
      GroupBase *parent;
      File *file;
      Object *getFileAsObject(); // helper function used in openChildAttribute
      Object *getAttrParent(const std::string &path, size_t pos); // helper function used in openChildAttribute
    public:
      template<class T>
      Creator<T> createChildAttribute(const std::string &path) {
        if(path[0]=='/') // absolute path -> call openChildAttribute from file
          return getFileAsObject()->createChildAttribute<T>(path.substr(1));
        // now its a relative path
        size_t pos;
        if((pos=path.find_last_of('/'))==std::string::npos) // no / included -> call openChild from Container
          return createChild<T>(path);
        // now its a relative path including at least one /
        return getAttrParent(path, pos)->createChild<T>(path.substr(pos+1));
      }

      template<class T>
      T* openChildAttribute(const std::string &path) {
        if(path[0]=='/') // absolute path -> call openChildAttribute from file
          return getFileAsObject()->openChildAttribute<T>(path.substr(1));
        // now its a relative path
        size_t pos;
        if((pos=path.find_last_of('/'))==std::string::npos) // no / included -> call openChild from Container
          return openChild<T>(path);
        // now its a relative path including at least one /
        return getAttrParent(path, pos)->openChild<T>(path.substr(pos+1));
      }
      Attribute *openChildAttribute(const std::string &name_, ElementType *objectType=nullptr, hid_t *type=nullptr);
      std::set<std::string> getChildAttributeNames();
      bool hasChildAttribute(const std::string &name_);
      GroupBase *getParent() { return parent; }
      File *getFile() { return file; }
      std::string getPath();
  };

  class Attribute : public Element {
    friend class Container<Attribute, Object>;
    protected:
      Attribute(Object *parent_, const std::string &name_);
      ~Attribute() override;
      Object *parent;
      File *file;
      void close() override;
      void open() override;
      void refresh() override;
      void flush() override;
    public:
      Object *getParent() { return parent; }
      File *getFile() { return file; }
      std::string getPath();
  };

  class Dataset : public Object {
    protected:
      Dataset(GroupBase *parent_, const std::string &name_);
      Dataset(int dummy, GroupBase *parent_, const std::string &name_);
      ~Dataset() override;
      void close() override;
      void open() override;
      void refresh() override;
      void flush() override;
    public:
      std::vector<hsize_t> getExtentDims();
  };

}

#endif
