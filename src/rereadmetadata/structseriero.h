#ifndef _STRUCTSERIERO_H_
#define _STRUCTSERIERO_H_

#include <structserie.h>

namespace H5 {

  class H5GroupRO;

  template<class S>
  class StructSerieRO : public StructSerie<S>, public Base {
    public:
      void open(const GroupRO& parent, const std::string& name);
      void close();
      void reread();
  };

  template<class S>
  void StructSerieRO<S>::open(const GroupRO& parent, const std::string& name) {
    StructSerie<S>::open(parent, name);
    reg(&parent, name, parent.getFile());
  }

  template<class S>
  void StructSerieRO<S>::close() {
    StructSerie<S>::close();
  }

  template<class S>
  void StructSerieRO<S>::reread() {
    StructSerie<S>::open(*parent, name);
  }

}

#endif
