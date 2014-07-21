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

#ifndef _HDF5SERIE_SIMPLEDATASET_H_
#define _HDF5SERIE_SIMPLEDATASET_H_

#include <hdf5serie/interface.h>
#include <hdf5serie/simpleattribute.h>
#include <vector>

namespace H5 {

  #define HDF5SERIE_DATASETTYPE 1
  #define HDF5SERIE_CLASS SimpleDataset
  #define HDF5SERIE_BASECLASS Dataset
  #define HDF5SERIE_CONTAINERBASECLASS Object
  #define HDF5SERIE_PARENTCLASS GroupBase

  #include "simple.h"

  #undef HDF5SERIE_DATASETTYPE
  #undef HDF5SERIE_CLASS
  #undef HDF5SERIE_BASECLASS
  #undef HDF5SERIE_CONTAINERBASECLASS
  #undef HDF5SERIE_PARENTCLASS

}

#endif
