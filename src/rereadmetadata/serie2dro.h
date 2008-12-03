#ifndef _TIMESERIERO_H_
#define _TIMESERIERO_H_

#include <serie2d.h>
#include <h5filero.h>

namespace H5 {

  template<class T>
  class Serie2DRO : public Serie2D<T> {
    private:
      H5FileRO* roFile;
    public:
      Serie2DRO();
      Serie2DRO(const Serie2DRO<T>& dataset);
      Serie2DRO(const Serie2D<T>& dataset);
      Serie2DRO(const H5FileRO& parent, const std::string& name);
      Serie2DRO(const GroupRO& parent, const std::string& name);
      void open(H5FileRO& parent, const std::string& name);
      void open(GroupRO& parent, const std::string& name);
      void closePermanent();
  };



  template<class T>
  Serie2DRO<T>::Serie2DRO() : Serie2D<T>(), roFile(NULL) {
  }

  template<class T>
  Serie2DRO<T>::Serie2DRO(const Serie2DRO<T>& dataset) : Serie2D<T>(dataset), roFile(NULL) {
  }

  template<class T>
  Serie2DRO<T>::Serie2DRO(const Serie2D<T>& dataset) : Serie2D<T>(dataset), roFile(NULL) {
  }

  template<class T>
  Serie2DRO<T>::Serie2DRO(const H5FileRO& parent, const std::string& name) :Serie2D<T>(parent, name), roFile(NULL) {
  }

  template<class T>
  Serie2DRO<T>::Serie2DRO(const GroupRO& parent, const std::string& name) :Serie2D<T>(parent, name), roFile(NULL) {
  }

  template<class T>
  void Serie2DRO<T>::open(H5FileRO& parent, const std::string& name) {
    Serie2D<T>::open(parent, name);
    roFile=&parent;

#   define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
    if(DataSet::getDataType()==H5TYPE) roFile->ts##TYPE.push_back((Serie2DRO<CTYPE>*)this); \
    if(DataSet::getDataType()==H5TYPE) roFile->ts##TYPE##Name.push_back(name); \
    if(DataSet::getDataType()==H5TYPE) roFile->ts##TYPE##Parent.push_back((GroupRO*)&parent);
#   include "knowntypes.def"
#   undef FOREACHKNOWNTYPE
  }

  template<class T>
  void Serie2DRO<T>::open(GroupRO& parent, const std::string& name) {
    Serie2D<T>::open(parent, name);
    roFile=parent.getROFile();

#   define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
    if(DataSet::getDataType()==H5TYPE) roFile->ts##TYPE.push_back((Serie2DRO<CTYPE>*)this); \
    if(DataSet::getDataType()==H5TYPE) roFile->ts##TYPE##Name.push_back(name); \
    if(DataSet::getDataType()==H5TYPE) roFile->ts##TYPE##Parent.push_back(&parent);
#   include "knowntypes.def"
#   undef FOREACHKNOWNTYPE
  }

  template<class T>
  void Serie2DRO<T>::closePermanent() {
    if(roFile) {
      int i;

#     define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
      for(i=0; i<roFile->ts##TYPE.size(); i++) \
        if(roFile->ts##TYPE[i]==(Serie2DRO<CTYPE>*)this) break; \
      roFile->ts##TYPE.erase(roFile->ts##TYPE.begin()+i); \
      roFile->ts##TYPE##Name.erase(roFile->ts##TYPE##Name.begin()+i); \
      roFile->ts##TYPE##Parent.erase(roFile->ts##TYPE##Parent.begin()+i);
#     include "knowntypes.def"
#     undef FOREACHKNOWNTYPE

    }
    Serie2D<T>::close();
  }

}

#endif
