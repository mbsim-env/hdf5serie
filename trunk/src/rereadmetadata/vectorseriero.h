#ifndef _VECTORSERIERO_H_
#define _VECTORSERIERO_H_

#include <vectorserie.h>

namespace H5 {

  class H5GroupRO;

  template<class T>
  class VectorSerieRO : public VectorSerie<T>, public Base {
    public:
      void open(const GroupRO& parent, const std::string& name);
      void close();
      void reread();
  };

  template<class T>
  void VectorSerieRO<T>::open(const GroupRO& parent, const std::string& name) {
    VectorSerie<T>::open(parent, name);
    reg(&parent, name, parent.getFile());
  }

  template<class T>
  void VectorSerieRO<T>::close() {
    VectorSerie<T>::close();
  }

  template<class T>
  void VectorSerieRO<T>::reread() {
    VectorSerie<T>::open(*parent, name);
  }

}

#endif
