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
#include <groupro.h>
#include <h5filero.h>

using namespace H5;

GroupRO::GroupRO(const Group& org) : Group(org) {
}

GroupRO GroupRO::openGroup(const std::string& name) {
  GroupRO grp=Group::openGroup(name);
  grp.reg(this, name, getFile());
  return grp;
}

H5FileRO* GroupRO::getFile() const {
  return file;
}

void GroupRO::close() {
  Group::close();
}

void GroupRO::reread() {
  const GroupRO* parentSave=parent;
  std::string nameSave=name;
  H5FileRO* fileSave=file;
  *this=parent->Group::openGroup(name);
  parent=parentSave;
  name=nameSave;
  file=fileSave;
}
