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
#include <stdexcept>
#include "toh5type.h"
#include "interface.h"

using namespace std;

namespace H5 {

hid_t returnVarLenStrDatatypeID() {
  static hid_t varLenStrDataTypeID=-1;
  if(varLenStrDataTypeID<0) {
    varLenStrDataTypeID=H5Tcopy(H5T_C_S1);
    if(H5Tset_size(varLenStrDataTypeID, H5T_VARIABLE)<0)
      throw Exception("Internal error: Can not create varaible length string datatype.");
  }
  return varLenStrDataTypeID;
}

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
  hid_t toH5Type(const CTYPE& dummy) { \
    return H5TYPE; \
  }
# include "hdf5serie/knowntypes.def"
# undef FOREACHKNOWNTYPE

}
