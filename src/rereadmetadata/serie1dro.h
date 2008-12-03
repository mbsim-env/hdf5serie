#ifndef _SERIE1DRO_H_
#define _SERIE1DRO_H_

#include <serie1d.h>

namespace H5 {

  class H5GroupRO;

  template<class S>
  class Serie1DRO : public Serie1D<S>, public Base {
    public:
      void open(const GroupRO& parent, const std::string& name);
      void close();
      void reread();
  };

  template<class S>
  void Serie1DRO<S>::open(const GroupRO& parent, const std::string& name) {
    Serie1D<S>::open(parent, name);
    reg(&parent, name, parent.getFile());
  }

  template<class S>
  void Serie1DRO<S>::close() {
    Serie1D<S>::close();
  }

  template<class S>
  void Serie1DRO<S>::reread() {
    Serie1D<S>::open(*parent, name);
  }

}

#endif
