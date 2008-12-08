#ifndef _TOH5TYPE_H_
#define _TOH5TYPE_H_

#include <H5Cpp.h>

#if H5_VERS_MAJOR!=1 || H5_VERS_MINOR!=8 || H5_VERS_RELEASE<2
  #error "Need HDF5 version ==1.8; >=1.8.2"
#endif

namespace H5 {

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
  DataType toH5Type(const CTYPE& dummy);
# include "knowntypes.def"
# undef FOREACHKNOWNTYPE

}

#endif
