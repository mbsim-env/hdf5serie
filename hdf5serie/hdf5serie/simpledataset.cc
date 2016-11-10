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
#include <hdf5serie/simpledataset.h>
#include <hdf5serie/group.h>
#include <hdf5serie/utils.h>
#include <hdf5serie/toh5type.h>

using namespace std;

namespace H5 {

  #define HDF5SERIE_DATASETTYPE 1
  #define HDF5SERIE_CLASS SimpleDataset
  #define HDF5SERIE_BASECLASS Dataset
  #define HDF5SERIE_CONTAINERBASECLASS Object
  #define HDF5SERIE_PARENTCLASS GroupBase
  #define HDF5SERIE_H5XCREATE H5Dcreate2(parent->getID(), name.c_str(), memDataTypeID, memDataSpaceID, H5P_DEFAULT, propID, H5P_DEFAULT)
  #define HDF5SERIE_H5XCLOSE H5Dclose
  #define HDF5SERIE_H5XOPEN H5Dopen(parent->getID(), name.c_str(), H5P_DEFAULT)
  #define HDF5SERIE_H5XWRITE(buf) H5Dwrite(id, memDataTypeID, memDataSpaceID, memDataSpaceID, H5P_DEFAULT, buf)
  #define HDF5SERIE_H5XREAD(buf) H5Dread(id, memDataTypeID, memDataSpaceID, memDataSpaceID, H5P_DEFAULT, buf)
  #define HDF5SERIE_H5XGET_SPACE H5Dget_space(id)

  #include "simple.cc"

}
