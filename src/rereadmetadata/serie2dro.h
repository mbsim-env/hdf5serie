#ifndef _SERIE2DRO_H_
#define _SERIE2DRO_H_

#include <serie2d.h>

namespace H5 {

  class H5GroupRO;

  template<class T>
  class Serie2DRO : public Serie2D<T>, public Base {
    public:
      void open(const GroupRO& parent, const std::string& name);
      void close();
      void reread();
  };

  template<class T>
  void Serie2DRO<T>::open(const GroupRO& parent, const std::string& name) {
    Serie2D<T>::open(parent, name);
    reg(&parent, name, parent.getFile());
  }

  template<class T>
  void Serie2DRO<T>::close() {
    Serie2D<T>::close();
  }

  template<class T>
  void Serie2DRO<T>::reread() {
    Serie2D<T>::open(*parent, name);
  }

}

#endif
