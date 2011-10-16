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

#ifndef _HDF5SERIE_TOH5TYPE_H_
#define _HDF5SERIE_TOH5TYPE_H_


/** \mainpage HDF5Serie - A HDF5 Wrapper for Time Series - API Documentation
 *
 * \section purpse Purpose of this Project
 *
 * This project has the purpose to provide a template based high level C++
 * interface/wrapper to HDF5 (http://www.hdfgroup.org) especially/just for
 * reading and writing series of data. It provides a simple interface to
 * append data to an existing HDF5-Dataset with one unlimited dimension.
 * The data to append can be vectors/matrices of elementary type or structs.
 * The use of the HDF5 DataType and DataSpace is totally encapsulated by the
 * templates structure of this project (of course only for the provided
 * functionality).
 *
 * \section install Download/Installation
 *
 * The source code of this project is only avaliable via svn. See
 * http://code.google.com/p/openmbv for more informations.
 *
 * For installation instructions see the README file of the "checkout" from svn.
 *
 * \section license License
 *
 * The library is licensed under the GNU Lesser General Public License (LGPL)
 *
 * \section example Example Code
 *
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
