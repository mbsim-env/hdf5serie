#ifndef _SIMPLEATTRIBUTE_H_
#define _SIMPLEATTRIBUTE_H_

#include <H5Cpp.h>
#include <vector>
#include <string>
#include <toh5type.h>
#include <assert.h>

namespace H5 {

  template<class T>
  class SimpleAttribute : public Attribute {
    private:
      DataType datatype;
    public:
      SimpleAttribute();
      SimpleAttribute(const SimpleAttribute<T>& attribute);
      SimpleAttribute(const DataSet& parent, const std::string& name, bool create=false);
      SimpleAttribute(const DataSet& parent, const std::string& name, const T& data);
      void create(const DataSet& parent, const std::string& name);
      void open(const DataSet& parent, const std::string& name);
      void write(const T& data);
      T read();
      void write(const DataSet& parent, const std::string& name, const T& data);
      T read(const DataSet& parent, const std::string& name);
      static T getData(const DataSet& parent, const std::string& name);
  };

  // a partial specialisation of class SimpleAttribute<T> for T=std::vector<T>
  template<class T>
  class SimpleAttribute<std::vector<T> > : public Attribute {
    private:
      DataType datatype;
    public:
      SimpleAttribute();
      SimpleAttribute(const SimpleAttribute<std::vector<T> >& attribute);
      SimpleAttribute(const DataSet& parent, const std::string& name, const int count=0);
      SimpleAttribute(const DataSet& parent, const std::string& name, const std::vector<T>& data);
      void create(const DataSet& parent, const std::string& name, const int count);
      void open(const DataSet& parent, const std::string& name);
      void write(const std::vector<T>& data);
      std::vector<T> read();
      void write(const DataSet& parent, const std::string& name, const std::vector<T>& data);
      std::vector<T> read(const DataSet& parent, const std::string& name);
      static std::vector<T>  getData(const DataSet& parent, const std::string& name);
  };

  // a partial specialisation of class SimpleAttribute<T> for T=std::vector<std::vector<T> >
  template<class T>
  class SimpleAttribute<std::vector<std::vector<T> > > : public Attribute {
    private:
      DataType datatype;
    public:
      SimpleAttribute();
      SimpleAttribute(const SimpleAttribute<std::vector<std::vector<T> > >& attribute);
      SimpleAttribute(const DataSet& parent, const std::string& name, const int rows=0, const int columns=0);
      SimpleAttribute(const DataSet& parent, const std::string& name, const std::vector<std::vector<T> >& data);
      void create(const DataSet& parent, const std::string& name, const int rows, const int columns);
      void open(const DataSet& parent, const std::string& name);
      void write(const std::vector<std::vector<T> >& data);
      std::vector<std::vector<T> > read();
      void write(const DataSet& parent, const std::string& name, const std::vector<std::vector<T> >& data);
      std::vector<std::vector<T> > read(const DataSet& parent, const std::string& name);
      static std::vector<std::vector<T> > getData(const DataSet& parent, const std::string& name);
  };



  template<class T>
  SimpleAttribute<T>::SimpleAttribute() : Attribute() {
    T dummy;
    datatype=toH5Type(dummy);
  }

  template<class T>
  SimpleAttribute<T>::SimpleAttribute(const SimpleAttribute<T>& attribute) : Attribute(attribute) {
    T dummy;
    datatype=toH5Type(dummy);
  }

  template<class T>
  SimpleAttribute<T>::SimpleAttribute(const DataSet& parent, const std::string& name, bool create_) : Attribute() {
    T dummy;
    datatype=toH5Type(dummy);
    if(create_)
      create(parent, name);
    else
      open(parent, name);
  }

  template<class T>
  SimpleAttribute<T>::SimpleAttribute(const DataSet& parent, const std::string& name, const T& data) : Attribute() {
    T dummy;
    datatype=toH5Type(dummy);
    write(parent, name, data);
  }

  template<class T>
  void SimpleAttribute<T>::create(const DataSet& parent, const std::string& name) {
    DataSpace dataspace(0, NULL);
    Attribute attribute=parent.createAttribute(name, datatype, dataspace);
    p_setId(attribute.getId());
    incRefCount();
  }

  template<class T>
  void SimpleAttribute<T>::open(const DataSet& parent, const std::string& name) {
    Attribute attribute=parent.openAttribute(name);
    p_setId(attribute.getId());
    incRefCount();
    // Check if dataspace and datatype complies with the class
    DataSpace dataspace=getSpace();
    assert(dataspace.getSimpleExtentNdims()==0);
    T dummy;
    assert(getDataType()==toH5Type(dummy));
  }

  template<class T>
  void SimpleAttribute<T>::write(const T& data) {
    Attribute::write(datatype, &data);
  }
  template<>
  void SimpleAttribute<std::string>::write(const std::string& data);

  template<class T>
  T SimpleAttribute<T>::read() {
    T data;
    Attribute::read(datatype, &data);
    return data;
  }
  template<>
  std::string SimpleAttribute<std::string>::read();

  template<class T>
  void SimpleAttribute<T>::write(const DataSet& parent, const std::string& name, const T& data) {
    create(parent, name);
    write(data);
  }

  template<class T>
  T SimpleAttribute<T>::read(const DataSet& parent, const std::string& name)  {
    open(parent, name);
    return read();
  }

  template<class T>
  T SimpleAttribute<T>::getData(const DataSet& parent, const std::string& name) {
    SimpleAttribute<T> attribute;
    return attribute.read(parent, name);
  }



  template<class T>
  SimpleAttribute<std::vector<T> >::SimpleAttribute() : Attribute() {
    T dummy;
    datatype=toH5Type(dummy);
  }

  template<class T>
  SimpleAttribute<std::vector<T> >::SimpleAttribute(const SimpleAttribute<std::vector<T> >& attribute) : Attribute(attribute) {
    T dummy;
    datatype=toH5Type(dummy);
  }

  template<class T>
  SimpleAttribute<std::vector<T> >::SimpleAttribute(const DataSet& parent, const std::string& name, const int count) : Attribute() {
    T dummy;
    datatype=toH5Type(dummy);
    if(count)
      create(parent, name, count);
    else
      open(parent, name);
  }

  template<class T>
  SimpleAttribute<std::vector<T> >::SimpleAttribute(const DataSet& parent, const std::string& name, const std::vector<T>& data) : Attribute() {
    T dummy;
    datatype=toH5Type(dummy);
    write(parent, name, data);
  }

  template<class T>
  void SimpleAttribute<std::vector<T> >::create(const DataSet& parent, const std::string& name, const int count) {
    hsize_t dims[]={count};
    DataSpace dataspace(1, dims);
    Attribute attribute=parent.createAttribute(name, datatype, dataspace);
    p_setId(attribute.getId());
    incRefCount();
  }

  template<class T>
  void SimpleAttribute<std::vector<T> >::open(const DataSet& parent, const std::string& name) {
    Attribute attribute=parent.openAttribute(name);
    p_setId(attribute.getId());
    incRefCount();
    // Check if dataspace and datatype complies with the class
    DataSpace dataspace=getSpace();
    assert(dataspace.getSimpleExtentNdims()==1);
    T dummy;
    assert(getDataType()==toH5Type(dummy));
  }

  template<class T>
  void SimpleAttribute<std::vector<T> >::write(const std::vector<T>& data) {
    DataSpace dataspace=getSpace();
    hsize_t dims[1];
    dataspace.getSimpleExtentDims(dims);
    assert(data.size()==dims[0]);
    Attribute::write(datatype, &data[0]);
  }
  template<>
  void SimpleAttribute<std::vector<std::string> >::write(const std::vector<std::string>& data);

  template<class T>
  std::vector<T> SimpleAttribute<std::vector<T> >::read() {
    DataSpace dataspace=getSpace();
    hsize_t dims[1];
    dataspace.getSimpleExtentDims(dims);
    std::vector<T> data(dims[0]);
    Attribute::read(datatype, &data[0]);
    return data;
  }
  template<>
  std::vector<std::string> SimpleAttribute<std::vector<std::string> >::read();

  template<class T>
  void SimpleAttribute<std::vector<T> >::write(const DataSet& parent, const std::string& name, const std::vector<T>& data) {
    create(parent, name, data.size());
    write(data);
  }

  template<class T>
  std::vector<T> SimpleAttribute<std::vector<T> >::read(const DataSet& parent, const std::string& name) {
    open(parent, name);
    return read();
  }

  template<class T>
  std::vector<T> SimpleAttribute<std::vector<T> >::getData(const DataSet& parent, const std::string& name) {
    SimpleAttribute<std::vector<T> > attribute;
    return attribute.read(parent, name);
  }


  
  template<class T>
  SimpleAttribute<std::vector<std::vector<T> > >::SimpleAttribute() : Attribute() {
    T dummy;
    datatype=toH5Type(dummy);
  }

  template<class T>
  SimpleAttribute<std::vector<std::vector<T> > >::SimpleAttribute(const SimpleAttribute<std::vector<std::vector<T> > >& attribute) : Attribute(attribute) {
    T dummy;
    datatype=toH5Type(dummy);
  }

  template<class T>
  SimpleAttribute<std::vector<std::vector<T> > >::SimpleAttribute(const DataSet& parent, const std::string& name, const int rows, const int columns) : Attribute() {
    T dummy;
    datatype=toH5Type(dummy);
    if(rows && columns)
      create(parent, name, rows, columns);
    else
      open(parent, name);
  }

  template<class T>
  SimpleAttribute<std::vector<std::vector<T> > >::SimpleAttribute(const DataSet& parent, const std::string& name, const std::vector<std::vector<T> >& data) : Attribute() {
    T dummy;
    datatype=toH5Type(dummy);
    write(parent, name, data);
  }

  template<class T>
  void SimpleAttribute<std::vector<std::vector<T> > >::create(const DataSet& parent, const std::string& name, const int rows, const int columns) {
    hsize_t dims[]={rows, columns};
    DataSpace dataspace(2, dims);
    Attribute attribute=parent.createAttribute(name, datatype, dataspace);
    p_setId(attribute.getId());
    incRefCount();
  }

  template<class T>
  void SimpleAttribute<std::vector<std::vector<T> > >::open(const DataSet& parent, const std::string& name) {
    Attribute attribute=parent.openAttribute(name);
    p_setId(attribute.getId());
    incRefCount();
    // Check if dataspace and datatype complies with the class
    DataSpace dataspace=getSpace();
    assert(dataspace.getSimpleExtentNdims()==2);
    T dummy;
    assert(getDataType()==toH5Type(dummy));
  }

  template<class T>
  void SimpleAttribute<std::vector<std::vector<T> > >::write(const std::vector<std::vector<T> >& data) {
    DataSpace dataspace=getSpace();
    hsize_t dims[2];
    dataspace.getSimpleExtentDims(dims);
    T* buf=new T[dims[0]*dims[1]];
    assert(data.size()==dims[0]);
    T dummy;
    for(int r=0; r<dims[0]; r++) {
      assert(data[r].size()==dims[1]);
      memcpy(&buf[r*dims[1]], &data[r][0], sizeof(dummy)*dims[1]);
    }
    Attribute::write(datatype, buf);
    delete[]buf;
  }
  template<>
  void SimpleAttribute<std::vector<std::vector<std::string> > >::write(const std::vector<std::vector<std::string> >& data);

  template<class T>
  std::vector<std::vector<T> > SimpleAttribute<std::vector<std::vector<T> > >::read() {
    DataSpace dataspace=getSpace();
    hsize_t dims[2];
    dataspace.getSimpleExtentDims(dims);
    std::vector<std::vector<T> > data(dims[0]);
    T* buf=new T[dims[0]*dims[1]];
    Attribute::read(datatype, buf);
    for(int r=0; r<dims[0]; r++) {
      data[r].resize(dims[1]);
      memcpy(&data[r][0], &buf[r*dims[1]], sizeof(T)*dims[1]);
    }
    delete[]buf;
    return data;
  }
  template<>
  std::vector<std::vector<std::string> > SimpleAttribute<std::vector<std::vector<std::string> > >::read();

  template<class T>
  void SimpleAttribute<std::vector<std::vector<T> > >::write(const DataSet& parent, const std::string& name, const std::vector<std::vector<T> >& data) {
    create(parent, name, data.size(), data[0].size());
    write(data);
  }

  template<class T>
  std::vector<std::vector<T> > SimpleAttribute<std::vector<std::vector<T> > >::read(const DataSet& parent, const std::string& name) {
    open(parent, name);
    return read();
  }

  template<class T>
  std::vector<std::vector<T> > SimpleAttribute<std::vector<std::vector<T> > >::getData(const DataSet& parent, const std::string& name) {
    SimpleAttribute<std::vector<std::vector<T> > > attribute;
    return attribute.read(parent, name);
  }
}

#endif
