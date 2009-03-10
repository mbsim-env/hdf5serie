#include <config.h>
#include <groupro.h>
#include <h5filero.h>

using namespace H5;

GroupRO::GroupRO(const Group& org) : Group(org) {
}

GroupRO GroupRO::openGroup(const std::string& name) {
  GroupRO grp=Group::openGroup(name);
  grp.reg(this, name, getFile());
  return grp;
}

H5FileRO* GroupRO::getFile() const {
  return file;
}

void GroupRO::close() {
  Group::close();
}

void GroupRO::reread() {
  const GroupRO* parentSave=parent;
  std::string nameSave=name;
  H5FileRO* fileSave=file;
  *this=parent->Group::openGroup(name);
  parent=parentSave;
  name=nameSave;
  file=fileSave;
}
