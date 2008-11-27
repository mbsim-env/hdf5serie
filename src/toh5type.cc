#include <config.h>
#include <toh5type.h> 

using namespace std;

namespace H5 {

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
  template<> \
  DataType toH5Type(CTYPE dummy) { \
    return H5TYPE; \
  }
# include "knowntypes.def"
# undef FOREACHKNOWNTYPE

///////////// not needed till noy BEGIN
//# define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
//  template<> \
//  string toType(CTYPE dummy) { \
//    return #TYPE; \
//  }
//# include "knowntypes.def"
//# undef FOREACHKNOWNTYPE
//
  DataType strToH5Type(const string& type) {
#   define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
    if(type==#TYPE) return H5TYPE;
#   include "knowntypes.def"
#   undef FOREACHKNOWNTYPE
  }
//
//  string toType(const DataType& datatype) {
//#   define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
//    if(datatype==H5TYPE) return #TYPE;
//#   include "knowntypes.def"
//#   undef FOREACHKNOWNTYPE
//  }
///////////// not needed till noy END

}
