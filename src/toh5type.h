#ifndef _TOH5TYPE_H_
#define _TOH5TYPE_H_

#include <H5Cpp.h>

#if H5_VERS_MAJOR!=1 || H5_VERS_MINOR!=8 || H5_VERS_RELEASE<2
  #error "Need HDF5 version ==1.8; >=1.8.2"
#endif

namespace H5 {

  template<class T>
  DataType toH5Type(const T dummy);

  /////////// not needed till noy BEGIN
  //template<class T>
  //std::string toType(const T dummy);

  DataType strToH5Type(const std::string& type);

  //std::string toType(const DataType& datatype);
  /////////// not needed till noy END

}

#endif
