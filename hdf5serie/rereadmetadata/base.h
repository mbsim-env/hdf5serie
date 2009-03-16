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

#ifndef _BASE_H_
#define _BASE_H_

#include <string>

namespace H5 {

  class GroupRO;
  class H5FileRO;

  class Base {
    protected:
      const GroupRO* parent;
      std::string name;
      H5FileRO* file;
    public:
      void reg(const GroupRO* parent, const std::string& name, H5FileRO* file);
      virtual void close()=0;
      virtual void reread()=0;
  };

}

#endif
