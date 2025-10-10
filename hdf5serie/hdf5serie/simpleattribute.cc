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
#include <cstring>
#include <hdf5serie/simpleattribute.h>
#include <hdf5serie/utils.h>
#include <hdf5serie/toh5type.h>

using namespace std;

namespace H5 {

  #undef HDF5SERIE_DATASETTYPE
  #define HDF5SERIE_CLASS SimpleAttribute
  #define HDF5SERIE_BASECLASS Attribute
  #define HDF5SERIE_CONTAINERBASECLASS Attribute
  #define HDF5SERIE_PARENTCLASS Object
  #define HDF5SERIE_H5XCREATE H5Acreate2(parent->getID(), name.c_str(), memDataTypeID, memDataSpaceID, H5P_DEFAULT, H5P_DEFAULT)
  #define HDF5SERIE_H5XCLOSE H5Aclose
  #define HDF5SERIE_H5XOPEN H5Aopen(parent->getID(), name.c_str(), H5P_DEFAULT)
  #define HDF5SERIE_H5XWRITE(buf) H5Awrite(id, memDataTypeID, buf)
  #define HDF5SERIE_H5XREAD(buf) H5Aread(id, memDataTypeID, buf)
  #define HDF5SERIE_H5XGET_SPACE H5Aget_space(id)

  #include "simple.cc"

}
