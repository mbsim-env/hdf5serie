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

#ifndef _HDF5SERIE_GROUP_H_
#define _HDF5SERIE_GROUP_H_

#include <hdf5serie/interface.h>
#include <boost/filesystem.hpp>
#include <list>

namespace H5 {

  class GroupBase : public Object, public Container<Object, GroupBase> {
    protected:
      GroupBase(int dummy, GroupBase *parent_, const std::string &name_);
      GroupBase(GroupBase *parent_, const std::string &name_);
      ~GroupBase() override;
      void close() override;
      void open() override;
      void refresh() override;
      void flush() override;
      Dataset *openChildDataset(const std::string &name_, ElementType *objectType, hid_t *type);
      void handleExternalLink(const std::string &name_);
      GroupBase *getFileAsGroup();
    public:
      template<class T>
      Container<Object, GroupBase>::Creator<T> createChildObject(const std::string &path) {
        if(path[0]=='/') // absolute path -> call createChildObject from file
          return getFileAsGroup()->createChildObject<T>(path.substr(1));
        // now its a relative path
        size_t pos;
        if((pos=path.find_last_of('/'))==std::string::npos) // no / included -> call createChild from Container<Object, GroupBase>
          return Container<Object, GroupBase>::createChild<T>(path);
        // now its a relative path including at least one /
        GroupBase *group=dynamic_cast<GroupBase*>(openChildObject(path.substr(0, pos)));
        if(!group)
          throw Exception(getPath(), "Got a path (including /) but this object is not a group");
        return group->createChildObject<T>(path.substr(pos+1));
      }

      template<class T>
      T* openChildObject(const std::string &path) {
        handleExternalLink(path);
        if(path[0]=='/') // absolute path -> call openChildObject from file
          return getFileAsGroup()->openChildObject<T>(path.substr(1));
        // now its a relative path
        size_t pos;
        if((pos=path.find_first_of('/'))==std::string::npos) // no / included -> call openChild from Container<Object, GroupBase>
          return Container<Object, GroupBase>::openChild<T>(path);
        // now its a relative path including at least one /
        GroupBase *group=dynamic_cast<GroupBase*>(openChildObject(path.substr(0, pos)));
        if(!group)
          throw Exception(getPath(), "Got a path (including /) but this object is not a group");
        return group->openChildObject<T>(path.substr(pos+1));
      }
      Object *openChildObject(const std::string &name_, ElementType *objectType=nullptr, hid_t *type=nullptr);
      std::list<std::string> getChildObjectNames();
      bool hasChildObject(const std::string &name_);

      bool isExternalLink(const std::string &name_);
      std::pair<boost::filesystem::path, std::string> getExternalLink(const std::string &name_);
      void createExternalLink(const std::string &name_, const std::pair<boost::filesystem::path, std::string> &target);
      void createSoftLink(const std::string &name_, const std::string &target);
  };

  class Group : public GroupBase {
    friend class Container<Object, GroupBase>;
    protected:
      Group(int dummy, GroupBase *parent_, const std::string &name_);
      Group(GroupBase *parent_, const std::string &name_);
      ~Group() override;
      void close() override;
      void open() override;
      void refresh() override;
      void flush() override;
  };

}

#endif
