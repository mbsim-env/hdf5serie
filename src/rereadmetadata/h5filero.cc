#include <config.h>
#include <h5filero.h>
#include <h5rereadmetadata.h>

using namespace H5;

H5FileRO::H5FileRO() : H5File() {
}

H5FileRO::H5FileRO(const H5FileRO& org) : H5File(org) {
}

H5FileRO::H5FileRO(const std::string& name, unsigned int flags, const FileCreatPropList& cpl, const FileAccPropList& apl) : H5File(name, flags, cpl, apl) {
}

GroupRO H5FileRO::openGroup(const std::string& name) {
  GroupRO newGroup(H5File::openGroup(name));
  newGroup.setROFile(this);
  return newGroup;
}

void H5FileRO::rereadMetadata() {
# define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
  for(int i=ts##TYPE.size()-1; i>=0; i--) \
    ts##TYPE[i]->close();
# include "knowntypes.def"
# undef FOREACHKNOWNTYPE

  for(int i=grp.size()-1; i>=0; i--) \
    grp[i]->close();

  std::string name=getFileName();
  close();
  openFile(name, H5F_ACC_RDONLY);

  for(int i=0; i<grp.size(); i++)
    *grp[i]=grpParent[i]->Group::openGroup(grpName[i]);

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
  for(int i=0; i<ts##TYPE.size(); i++) \
    ts##TYPE[i]->Serie2D<CTYPE>::open(*ts##TYPE##Parent[i], ts##TYPE##Name[i]);
# include "knowntypes.def"
# undef FOREACHKNOWNTYPE
}
