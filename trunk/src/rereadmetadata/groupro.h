#ifndef _GROUPRO_H_
#define _GROUPRO_H_

#include <H5Cpp.h>
#include <base.h>

namespace H5 {

  class H5FileRO;

  class GroupRO : public Group, public Base {
    public:
      GroupRO(const Group& org);
      GroupRO openGroup(const std::string& name);
      H5FileRO* getFile() const;
      void close();
      void reread();
  };

}

#endif
