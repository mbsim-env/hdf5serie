#include <base.h>
#include <h5filero.h>

using namespace H5;

void Base::reg(const GroupRO* parent_, const std::string& name_, H5FileRO* file_) {
  parent=parent_;
  name=name_;
  file=file_;
  file->push_back(this);
}
