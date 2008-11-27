#ifndef _H5FILERO_H_
#define _H5FILERO_H_

#include <H5Cpp.h>
#include <groupro.h>
#include <vector>

namespace H5 {

  template<class T>
  class Serie2DRO;

  class H5FileRO : public H5File {
    template<class T> friend class Serie2DRO;
    friend class GroupRO;
    private:
#     define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
      std::vector<Serie2DRO<CTYPE>*> ts##TYPE; \
      std::vector<std::string> ts##TYPE##Name; \
      std::vector<GroupRO*> ts##TYPE##Parent;
#     include "knowntypes.def"
#     undef FOREACHKNOWNTYPE

      std::vector<GroupRO*> grp;
      std::vector<std::string> grpName;
      std::vector<GroupRO*> grpParent;
    public:
      H5FileRO();
      H5FileRO(const H5FileRO& org);
      H5FileRO(const std::string& name, unsigned int flags, const FileCreatPropList& cpl=FileCreatPropList::DEFAULT, const FileAccPropList& apl=FileAccPropList::DEFAULT);
      GroupRO openGroup(const std::string& name);
      void rereadMetadata();
  };

}

#endif
