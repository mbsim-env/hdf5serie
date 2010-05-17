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

#ifndef _H5FILERO_H_
#define _H5FILERO_H_

#include <H5Cpp.h>
#include <groupro.h>
#include <vector>

namespace H5 {
  
  class Base;

  class H5FileRO : public H5File {
    private:
      std::vector<Base*> object;
    public:
      H5FileRO();
      GroupRO openGroup(const std::string& name);
      void push_back(Base* obj);
      void reread();
  };

}

#endif
