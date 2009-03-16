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

#ifndef _VECTORSERIERO_H_
#define _VECTORSERIERO_H_

#include <vectorserie.h>

namespace H5 {

  class H5GroupRO;

  template<class T>
  class VectorSerieRO : public VectorSerie<T>, public Base {
    public:
      void open(const GroupRO& parent, const std::string& name);
      void close();
      void reread();
  };

  template<class T>
  void VectorSerieRO<T>::open(const GroupRO& parent, const std::string& name) {
    VectorSerie<T>::open(parent, name);
    reg(&parent, name, parent.getFile());
  }

  template<class T>
  void VectorSerieRO<T>::close() {
    VectorSerie<T>::close();
  }

  template<class T>
  void VectorSerieRO<T>::reread() {
    VectorSerie<T>::open(*parent, name);
  }

}

#endif
