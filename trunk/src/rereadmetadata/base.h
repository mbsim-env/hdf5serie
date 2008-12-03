#ifndef _BASE_H_
#define _BASE_H_

#include <string>

namespace H5 {

  class GroupRO;
  class H5FileRO;

  class Base {
    protected:
      const GroupRO* parent;
      std::string name;
      H5FileRO* file;
    public:
      void reg(const GroupRO* parent, const std::string& name, H5FileRO* file);
      virtual void close()=0;
      virtual void reread()=0;
  };

}

#endif
