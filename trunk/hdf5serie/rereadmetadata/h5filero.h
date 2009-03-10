#ifndef _H5FILERO_H_
#define _H5FILERO_H_

#include <H5Cpp.h>
#include <groupro.h>
#include <vector>

namespace H5 {
  
  class Base;

  class H5FileRO : public H5File {
    private:
      std::vector<Base*> object;
    public:
      H5FileRO();
      GroupRO openGroup(const std::string& name);
      void push_back(Base* obj);
      void reread();
  };

}

#endif
