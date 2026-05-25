#ifndef _HDF5SERIE_UTILS_H_
#define _HDF5SERIE_UTILS_H_

#include "interface.h"
#include "toh5type.h"
#include <boost/core/demangle.hpp>
#include <vector>
#include <cstdlib>

namespace H5 {

class VecStr {
  public:
    VecStr(size_t size) : arr(size, nullptr) {}
    ~VecStr() {
      for(auto & it : arr)
        free(it);
    }
    void alloc(size_t i, size_t size) { free(arr[i]); arr[i]=static_cast<char*>(malloc((size+1)*sizeof(char))); }
    char *&operator[](size_t i) { return arr[i]; }
  private:
    std::vector<char*> arr;
};

template<class T>
ScopedHID getMemDataTypeID(ScopedHID filedt, const std::string &path, const std::string &className) {
  // HDF5 does not convert between different string character encodings but be only support reading of ASCII and UTF-8 file data
  // to UTF-8 memory data, so we can always the file datatype.
  if(H5Tget_class(filedt) == H5T_STRING && std::is_same_v<T,std::string>)
    return filedt;
  // for anything else we just the type T and HDF5 does the conversion
  return ScopedHID(H5Tcopy(toH5Type<T>()), &H5Tclose);
}

}

#endif
