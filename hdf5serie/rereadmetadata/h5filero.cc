#include <config.h>
#include <h5filero.h>
#include <string>

using namespace H5;
using namespace std;

H5FileRO::H5FileRO() : H5File() {
}

GroupRO H5FileRO::openGroup(const std::string& name) {
  GroupRO grp=H5File::openGroup(name);
  grp.reg((GroupRO*)this, name, this);
  return grp;
}

void H5FileRO::push_back(Base* obj) {
  object.push_back(obj);
}

void H5FileRO::reread() {
  for(int i=object.size()-1; i>=0; i--)
    object[i]->close();

  string filename=getFileName();
  close();
  openFile(filename, H5F_ACC_RDONLY);

  for(int i=0; i<object.size(); i++)
    object[i]->reread();
}
