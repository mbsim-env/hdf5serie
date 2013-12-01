#ifndef _HDF5SERIE_UTILS_H_
#define _HDF5SERIE_UTILS_H_

#include <vector>

namespace H5 {

class VecStr {
  public:
    VecStr(size_t size) : arr(size, NULL) {}
    ~VecStr() {
      for(std::vector<char*>::iterator it=arr.begin(); it!=arr.end(); it++)
        free(*it);
    }
    char *&operator[](size_t i) { return arr[i]; }
  private:
    std::vector<char*> arr;
};

}

#endif