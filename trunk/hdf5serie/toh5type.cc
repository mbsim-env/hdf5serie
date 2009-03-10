#include <config.h>
#include <hdf5serie/toh5type.h> 

using namespace std;

namespace H5 {

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
  DataType toH5Type(const CTYPE& dummy) { \
    return H5TYPE; \
  }
# include "hdf5serie/knowntypes.def"
# undef FOREACHKNOWNTYPE

}
