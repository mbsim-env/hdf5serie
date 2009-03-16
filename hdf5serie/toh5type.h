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

#ifndef _TOH5TYPE_H_
#define _TOH5TYPE_H_


/** \mainpage HDF5Serie - A HDF5 Wrapper for Time Series - API Documentation
 *
 * This library is a high level programming interface to the HDF5 library for
 * reading and writing data series. A data consists of a vector of
 * equal simple types or a compount of diffrent simple types.
 *
 * The library is licensed under the GNU Lesser General Public License (LGPL)
 *
 * The following code shows a usage example of the library.
 * \code
#include <H5Cpp.h>
#include <hdf5serie/vectorserie.h>
#include <string>

using namespace H5;
using namespace std;

int main() {
  H5File file("test.h5", H5F_ACC_TRUNC);
  Group grp=file.createGroup("mygrp");
  VectorSerie<double> vs;
  vector<string> colhead;
  colhead.push_back("col1");
  colhead.push_back("col2");
  colhead.push_back("col3");
  vs.create(grp, "vectorserie", colhead);
  vs.setDescription("mydesctipsldfk");
  vector<double> data;
  data.push_back(1.2);
  data.push_back(2.3);
  data.push_back(3.4);
  vs.append(data);
  file.close();
  return 0;
}
 * \endcode
 */



#include <H5Cpp.h>

#if H5_VERS_MAJOR!=1 || H5_VERS_MINOR!=8 || H5_VERS_RELEASE<2
  #error "Need HDF5 version ==1.8; >=1.8.2"
#endif

namespace H5 {

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
  DataType toH5Type(const CTYPE& dummy);
# include "hdf5serie/knowntypes.def"
# undef FOREACHKNOWNTYPE

}

#endif
