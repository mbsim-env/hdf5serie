#include <config.h>
#include <groupro.h>
#include <h5filero.h>

using namespace H5;

GroupRO::GroupRO() : Group(), roFile(NULL) {
}

GroupRO::GroupRO(const GroupRO& org) : Group(org), roFile(NULL) {
}

GroupRO::GroupRO(const Group& org) : Group(org), roFile(NULL) {
}

GroupRO GroupRO::openGroup(const std::string& name) {
  GroupRO newGroup(Group::openGroup(name));
  newGroup.setROFile(roFile);
  roFile->grp.push_back(&newGroup);
  roFile->grpName.push_back(name);
  roFile->grpParent.push_back(this);
  return newGroup;
}

void GroupRO::closePermanent() {
  if(roFile) {
    int i;
    for(i=0; i<roFile->grp.size(); i++)
      if(roFile->grp[i]==this) break;
    roFile->grp.erase(roFile->grp.begin()+i);
    roFile->grpName.erase(roFile->grpName.begin()+i);
    roFile->grpParent.erase(roFile->grpParent.begin()+i);
  }
  Group::close();
}
