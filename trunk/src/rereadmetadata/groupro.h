#ifndef _GROUPRO_H_
#define _GROUPRO_H_

#include <H5Cpp.h>

namespace H5 {

  class H5FileRO;

  class GroupRO : public Group {
    template<class T> friend class Serie2DRO;
    friend class H5FileRO;
    private:
      H5FileRO* roFile;
      void setROFile(H5FileRO* file) { roFile=file; }
      H5FileRO* getROFile() { return roFile; }
    public:
      GroupRO();
      GroupRO(const GroupRO& org);
      GroupRO(const Group& org);
      GroupRO openGroup(const std::string& name);
      void closePermanent();
  };

}

#endif
