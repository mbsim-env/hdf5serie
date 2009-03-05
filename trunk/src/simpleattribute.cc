#include <config.h>
#include <simpleattribute.h>
#include <assert.h>

using namespace std;

namespace H5 {
  
  // template definitions

  template<class T>
  SimpleAttribute<T>::SimpleAttribute() : Attribute() {
    T dummy;
    memDataType=toH5Type(dummy);
  }

  template<class T>
  SimpleAttribute<T>::SimpleAttribute(const SimpleAttribute<T>& attribute) : Attribute(attribute) {
    T dummy;
    memDataType=toH5Type(dummy);
  }

  template<class T>
  SimpleAttribute<T>::SimpleAttribute(const H5Object& parent, const string& name, bool create_) : Attribute() {
    T dummy;
    memDataType=toH5Type(dummy);
    if(create_)
      create(parent, name);
    else
      open(parent, name);
  }

  template<class T>
  SimpleAttribute<T>::SimpleAttribute(const H5Object& parent, const string& name, const T& data) : Attribute() {
    T dummy;
    memDataType=toH5Type(dummy);
    write(parent, name, data);
  }

  template<class T>
  void SimpleAttribute<T>::create(const H5Object& parent, const string& name) {
    DataSpace dataSpace(0, NULL);
    Attribute attribute=parent.createAttribute(name, memDataType, dataSpace);
    p_setId(attribute.getId());
    incRefCount();
  }

  template<class T>
  void SimpleAttribute<T>::open(const H5Object& parent, const string& name) {
    Attribute attribute=parent.openAttribute(name);
    p_setId(attribute.getId());
    incRefCount();
    // Check if dataSpace and memDataType complies with the class
    DataSpace dataSpace=getSpace();
    assert(dataSpace.getSimpleExtentNdims()==0);
    T dummy;
    assert(getDataType().getClass()==memDataType.getClass());
  }

  template<class T>
  void SimpleAttribute<T>::write(const T& data) {
    Attribute::write(memDataType, &data);
  }
  template<>
  void SimpleAttribute<string>::write(const string& data);

  template<class T>
  T SimpleAttribute<T>::read() {
    T data;
    Attribute::read(memDataType, &data);
    return data;
  }
  template<>
  string SimpleAttribute<string>::read();

  template<class T>
  void SimpleAttribute<T>::write(const H5Object& parent, const string& name, const T& data) {
    create(parent, name);
    write(data);
  }

  template<class T>
  T SimpleAttribute<T>::read(const H5Object& parent, const string& name)  {
    open(parent, name);
    return read();
  }

  template<class T>
  T SimpleAttribute<T>::getData(const H5Object& parent, const string& name) {
    SimpleAttribute<T> attribute;
    return attribute.read(parent, name);
  }

  template<class T>
  void SimpleAttribute<T>::setData(const H5Object& parent, const std::string& name, const T& data) {
    SimpleAttribute<T> attribute;
    attribute.write(parent, name, data);
  }



  template<class T>
  SimpleAttribute<vector<T> >::SimpleAttribute() : Attribute() {
    T dummy;
    memDataType=toH5Type(dummy);
  }

  template<class T>
  SimpleAttribute<vector<T> >::SimpleAttribute(const SimpleAttribute<vector<T> >& attribute) : Attribute(attribute) {
    T dummy;
    memDataType=toH5Type(dummy);
  }

  template<class T>
  SimpleAttribute<vector<T> >::SimpleAttribute(const H5Object& parent, const string& name, const int count) : Attribute() {
    T dummy;
    memDataType=toH5Type(dummy);
    if(count)
      create(parent, name, count);
    else
      open(parent, name);
  }

  template<class T>
  SimpleAttribute<vector<T> >::SimpleAttribute(const H5Object& parent, const string& name, const vector<T>& data) : Attribute() {
    T dummy;
    memDataType=toH5Type(dummy);
    write(parent, name, data);
  }

  template<class T>
  void SimpleAttribute<vector<T> >::create(const H5Object& parent, const string& name, const int count) {
    hsize_t dims[]={count};
    DataSpace dataSpace(1, dims);
    Attribute attribute=parent.createAttribute(name, memDataType, dataSpace);
    p_setId(attribute.getId());
    incRefCount();
  }

  template<class T>
  void SimpleAttribute<vector<T> >::open(const H5Object& parent, const string& name) {
    Attribute attribute=parent.openAttribute(name);
    p_setId(attribute.getId());
    incRefCount();
    // Check if dataSpace and memDataType complies with the class
    DataSpace dataSpace=getSpace();
    assert(dataSpace.getSimpleExtentNdims()==1);
    T dummy;
    assert(getDataType().getClass()==memDataType.getClass());
  }

  template<class T>
  void SimpleAttribute<vector<T> >::write(const vector<T>& data) {
    DataSpace dataSpace=getSpace();
    hsize_t dims[1];
    dataSpace.getSimpleExtentDims(dims);
    assert(data.size()==dims[0]);
    Attribute::write(memDataType, &data[0]);
  }
  template<>
  void SimpleAttribute<vector<string> >::write(const vector<string>& data);

  template<class T>
  vector<T> SimpleAttribute<vector<T> >::read() {
    DataSpace dataSpace=getSpace();
    hsize_t dims[1];
    dataSpace.getSimpleExtentDims(dims);
    vector<T> data(dims[0]);
    Attribute::read(memDataType, &data[0]);
    return data;
  }
  template<>
  vector<string> SimpleAttribute<vector<string> >::read();

  template<class T>
  void SimpleAttribute<vector<T> >::write(const H5Object& parent, const string& name, const vector<T>& data) {
    create(parent, name, data.size());
    write(data);
  }

  template<class T>
  vector<T> SimpleAttribute<vector<T> >::read(const H5Object& parent, const string& name) {
    open(parent, name);
    return read();
  }

  template<class T>
  vector<T> SimpleAttribute<vector<T> >::getData(const H5Object& parent, const string& name) {
    SimpleAttribute<vector<T> > attribute;
    return attribute.read(parent, name);
  }

  template<class T>
  void SimpleAttribute<vector<T> >::setData(const H5Object& parent, const std::string& name, const vector<T>& data) {
    SimpleAttribute<vector<T> > attribute;
    attribute.write(parent, name, data);
  }


  
  template<class T>
  SimpleAttribute<vector<vector<T> > >::SimpleAttribute() : Attribute() {
    T dummy;
    memDataType=toH5Type(dummy);
  }

  template<class T>
  SimpleAttribute<vector<vector<T> > >::SimpleAttribute(const SimpleAttribute<vector<vector<T> > >& attribute) : Attribute(attribute) {
    T dummy;
    memDataType=toH5Type(dummy);
  }

  template<class T>
  SimpleAttribute<vector<vector<T> > >::SimpleAttribute(const H5Object& parent, const string& name, const int rows, const int columns) : Attribute() {
    T dummy;
    memDataType=toH5Type(dummy);
    if(rows && columns)
      create(parent, name, rows, columns);
    else
      open(parent, name);
  }

  template<class T>
  SimpleAttribute<vector<vector<T> > >::SimpleAttribute(const H5Object& parent, const string& name, const vector<vector<T> >& data) : Attribute() {
    T dummy;
    memDataType=toH5Type(dummy);
    write(parent, name, data);
  }

  template<class T>
  void SimpleAttribute<vector<vector<T> > >::create(const H5Object& parent, const string& name, const int rows, const int columns) {
    hsize_t dims[]={rows, columns};
    DataSpace dataSpace(2, dims);
    Attribute attribute=parent.createAttribute(name, memDataType, dataSpace);
    p_setId(attribute.getId());
    incRefCount();
  }

  template<class T>
  void SimpleAttribute<vector<vector<T> > >::open(const H5Object& parent, const string& name) {
    Attribute attribute=parent.openAttribute(name);
    p_setId(attribute.getId());
    incRefCount();
    // Check if dataSpace and memDataType complies with the class
    DataSpace dataSpace=getSpace();
    assert(dataSpace.getSimpleExtentNdims()==2);
    T dummy;
    assert(getDataType().getClass()==memDataType.getClass());
  }

  template<class T>
  void SimpleAttribute<vector<vector<T> > >::write(const vector<vector<T> >& data) {
    DataSpace dataSpace=getSpace();
    hsize_t dims[2];
    dataSpace.getSimpleExtentDims(dims);
    T* buf=new T[dims[0]*dims[1]];
    assert(data.size()==dims[0]);
    T dummy;
    for(int r=0; r<dims[0]; r++) {
      assert(data[r].size()==dims[1]);
      memcpy(&buf[r*dims[1]], &data[r][0], sizeof(dummy)*dims[1]);
    }
    Attribute::write(memDataType, buf);
    delete[]buf;
  }
  template<>
  void SimpleAttribute<vector<vector<string> > >::write(const vector<vector<string> >& data);

  template<class T>
  vector<vector<T> > SimpleAttribute<vector<vector<T> > >::read() {
    DataSpace dataSpace=getSpace();
    hsize_t dims[2];
    dataSpace.getSimpleExtentDims(dims);
    vector<vector<T> > data(dims[0]);
    T* buf=new T[dims[0]*dims[1]];
    Attribute::read(memDataType, buf);
    for(int r=0; r<dims[0]; r++) {
      data[r].resize(dims[1]);
      memcpy(&data[r][0], &buf[r*dims[1]], sizeof(T)*dims[1]);
    }
    delete[]buf;
    return data;
  }
  template<>
  vector<vector<string> > SimpleAttribute<vector<vector<string> > >::read();

  template<class T>
  void SimpleAttribute<vector<vector<T> > >::write(const H5Object& parent, const string& name, const vector<vector<T> >& data) {
    create(parent, name, data.size(), data[0].size());
    write(data);
  }

  template<class T>
  vector<vector<T> > SimpleAttribute<vector<vector<T> > >::read(const H5Object& parent, const string& name) {
    open(parent, name);
    return read();
  }

  template<class T>
  vector<vector<T> > SimpleAttribute<vector<vector<T> > >::getData(const H5Object& parent, const string& name) {
    SimpleAttribute<vector<vector<T> > > attribute;
    return attribute.read(parent, name);
  }

  template<class T>
  void SimpleAttribute<vector<vector<T> > >::setData(const H5Object& parent, const std::string& name, const vector<vector<T> >& data) {
    SimpleAttribute<vector<vector<T> > > attribute;
    attribute.write(parent, name, data);
  }



  // explizit template spezialisations

  template<>
  void SimpleAttribute<string>::write(const string& data) {
    const char *buf[1]={data.c_str()};
    Attribute::write(StrType(PredType::C_S1, H5T_VARIABLE), buf);
  }

  template<>
  string SimpleAttribute<string>::read() {
    string data;
    char *buf[1];
    Attribute::read(StrType(PredType::C_S1, H5T_VARIABLE), buf);
    data=buf[0];
    free(buf[0]);
    return data;
  }

 

  template<>
  void SimpleAttribute<vector<string> >::write(const vector<string>& data) {
    DataSpace dataSpace=getSpace();
    hsize_t dims[1];
    dataSpace.getSimpleExtentDims(dims);
    assert(data.size()==dims[0]);
    char** buf=new char*[dims[0]];
    for(int i=0; i<dims[0]; i++) {
      buf[i]=new char[data[i].size()+1];
      strcpy(buf[i], data[i].c_str());
    }
    Attribute::write(StrType(PredType::C_S1, H5T_VARIABLE), buf);
    for(int i=0; i<dims[0]; i++) delete[]buf[i];
    delete[]buf;
  }

  template<>
  vector<string> SimpleAttribute<vector<string> >::read() {
    DataSpace dataSpace=getSpace();
    hsize_t dims[1];
    dataSpace.getSimpleExtentDims(dims);
    char** buf=new char*[dims[0]];
    Attribute::read(StrType(PredType::C_S1, H5T_VARIABLE), buf);
    vector<string> data;
    for(int i=0; i<dims[0]; i++) {
      data.push_back(buf[i]);
      free(buf[i]);
    }
    delete[]buf;
    return data;
  }



  template<>
  void SimpleAttribute<vector<vector<string> > >::write(const vector<vector<string> >& data) {
    DataSpace dataSpace=getSpace();
    hsize_t dims[2];
    dataSpace.getSimpleExtentDims(dims);
    assert(data.size()==dims[0]);
    char** buf=new char*[dims[0]*dims[1]];
    for(int r=0; r<dims[0]; r++)
      for(int c=0; c<dims[1]; c++) {
	assert(data[r].size()==dims[1]);
	buf[r*dims[1]+c]=new char[data[r][c].size()+1];
	strcpy(buf[r*dims[1]+c],data[r][c].c_str());
      }
    Attribute::write(StrType(PredType::C_S1, H5T_VARIABLE), buf);
    for(int r=0; r<dims[0]; r++)
      for(int c=0; c<dims[1]; c++)
        delete[]buf[r*dims[1]+c];
    delete[]buf;
  }

  template<>
  vector<vector<string> > SimpleAttribute<vector<vector<string> > >::read() {
    DataSpace dataSpace=getSpace();
    hsize_t dims[2];
    dataSpace.getSimpleExtentDims(dims);
    char** buf=new char*[dims[0]*dims[1]];
    Attribute::read(StrType(PredType::C_S1, H5T_VARIABLE), buf);
    vector<vector<string> > data(dims[0]);
    for(int r=0; r<dims[0]; r++)
      for(int c=0; c<dims[1]; c++) {
        data[r].push_back(buf[r*dims[1]+c]);
        free(buf[r*dims[1]+c]);
      }
    delete[]buf;
    return data;
  }



  // explizit template instantations

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
  template class SimpleAttribute<CTYPE>;
# include "knowntypes.def"
# undef FOREACHKNOWNTYPE

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
  template class SimpleAttribute<vector<CTYPE> >;
# include "knowntypes.def"
# undef FOREACHKNOWNTYPE

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
  template class SimpleAttribute<vector<vector<CTYPE> > >;
# include "knowntypes.def"
# undef FOREACHKNOWNTYPE

}
