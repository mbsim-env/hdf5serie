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

namespace H5 {

  class GroupBase : public Object, public Container<Object, GroupBase> {
    protected:
      GroupBase(int dummy, GroupBase *parent_, const std::string &name_);
      GroupBase(GroupBase *parent_, const std::string &name_);
      ~GroupBase();
      void close();
      void open();
      void refresh();
      void flush();
      Dataset *openChildDataset(const std::string &name_, ElementType *objectType, hid_t *type);
      void handleExternalLink(const std::string &name_);
    public:
      template<class T>
      Container<Object, GroupBase>::Creator<T> createChildObject(const std::string &name_) {
        return Container<Object, GroupBase>::createChild<T>(name_);
      }

      template<class T>
      T* openChildObject(const std::string &name_) {
        handleExternalLink(name_);
        return Container<Object, GroupBase>::openChild<T>(name_);
      }
      Object *openChildObject(const std::string &name_, ElementType *objectType=NULL, hid_t *type=NULL);
      std::set<std::string> getChildObjectNames();
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
      ~Group();
      void close();
      void open();
      void refresh();
      void flush();
  };

}

#endif
