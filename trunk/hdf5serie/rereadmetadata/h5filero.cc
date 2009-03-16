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
#include <h5filero.h>
#include <string>

using namespace H5;
using namespace std;

H5FileRO::H5FileRO() : H5File() {
}

GroupRO H5FileRO::openGroup(const std::string& name) {
  GroupRO grp=H5File::openGroup(name);
  grp.reg((GroupRO*)this, name, this);
  return grp;
}

void H5FileRO::push_back(Base* obj) {
  object.push_back(obj);
}

void H5FileRO::reread() {
  for(int i=object.size()-1; i>=0; i--)
    object[i]->close();

  string filename=getFileName();
  close();
  openFile(filename, H5F_ACC_RDONLY);

  for(int i=0; i<object.size(); i++)
    object[i]->reread();
}
