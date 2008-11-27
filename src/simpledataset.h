#ifndef _SIMPLEDATASET_H_
#define _SIMPLEDATASET_H_

#include <H5Cpp.h>
#include <vector>
#include <string>
#include <toh5type.h>
#include <simpleattribute.h>
#include <assert.h>

namespace H5 {

  template<class T>
  class SimpleDataSet : public DataSet {
    private:
      DataSpace dataspace;
      DataType datatype;
    public:
      SimpleDataSet();
      SimpleDataSet(const SimpleDataSet<T>& dataset);
      SimpleDataSet(const CommonFG& parent, const std::string& name, bool create=false);
      SimpleDataSet(const CommonFG& parent, const std::string& name, const T& data);
      void create(const CommonFG& parent, const std::string& name);
      void open(const CommonFG& parent, const std::string& name);
      void write(const T& data);
      T read();
      void write(const CommonFG& parent, const std::string& name, const T& data);
      T read(const CommonFG& parent, const std::string& name);
      void setDescription(const std::string& description);
      std::string getDescription();
      static T getData(const CommonFG& parent, const std::string& name);
  };

  // a partial specialisation of class SimpleDataSet<T> for T=std::vector<T>
  template<class T>
  class SimpleDataSet<std::vector<T> > : public DataSet {
    private:
      DataType datatype;
    public:
      SimpleDataSet();
      SimpleDataSet(const SimpleDataSet<std::vector<T> >& dataset);
      SimpleDataSet(const CommonFG& parent, const std::string& name, bool create=false);
      SimpleDataSet(const CommonFG& parent, const std::string& name, const std::vector<T>& data);
      void create(const CommonFG& parent, const std::string& name);
      void open(const CommonFG& parent, const std::string& name);
      void write(const std::vector<T>& data);
      std::vector<T> read();
      void write(const CommonFG& parent, const std::string& name, const std::vector<T>& data);
      std::vector<T> read(const CommonFG& parent, const std::string& name);
      void setDescription(const std::string& description);
      std::string getDescription();
      static std::vector<T>  getData(const CommonFG& parent, const std::string& name);
  };

  // a partial specialisation of class SimpleDataSet<T> for T=std::vector<std::vector<T> >
  template<class T>
  class SimpleDataSet<std::vector<std::vector<T> > > : public DataSet {
    private:
      DataType datatype;
    public:
      SimpleDataSet();
      SimpleDataSet(const SimpleDataSet<std::vector<std::vector<T> > >& dataset);
      SimpleDataSet(const CommonFG& parent, const std::string& name, bool create=false);
      SimpleDataSet(const CommonFG& parent, const std::string& name, const std::vector<std::vector<T> >& data);
      void create(const CommonFG& parent, const std::string& name);
      void open(const CommonFG& parent, const std::string& name);
      void write(const std::vector<std::vector<T> >& data);
      std::vector<std::vector<T> > read();
      void write(const CommonFG& parent, const std::string& name, const std::vector<std::vector<T> >& data);
      std::vector<std::vector<T> > read(const CommonFG& parent, const std::string& name);
      void setDescription(const std::string& description);
      std::string getDescription();
      static std::vector<std::vector<T> > getData(const CommonFG& parent, const std::string& name);
  };



  template<class T>
  SimpleDataSet<T>::SimpleDataSet() : DataSet(), dataspace() {
    T dummy;
    datatype=toH5Type(dummy);
  }

  template<class T>
  SimpleDataSet<T>::SimpleDataSet(const SimpleDataSet<T>& dataset) : DataSet(dataset), dataspace(dataset.getSpace()) {
    T dummy;
    datatype=toH5Type(dummy);
  }

  template<class T>
  SimpleDataSet<T>::SimpleDataSet(const CommonFG& parent, const std::string& name, bool create_) {
    T dummy;
    datatype=toH5Type(dummy);
    if(create_)
      create(parent, name);
    else
      open(parent, name);
  }

  template<class T>
  SimpleDataSet<T>::SimpleDataSet(const CommonFG& parent, const std::string& name, const T& data) {
    T dummy;
    datatype=toH5Type(dummy);
    write(parent, name, data);
  }

  template<class T>
  void SimpleDataSet<T>::create(const CommonFG& parent, const std::string& name) {
    dataspace=DataSpace(0, NULL);
    DataSet dataset=parent.createDataSet(name, datatype, dataspace);
    p_setId(dataset.getId());
    incRefCount();
  }

  template<class T>
  void SimpleDataSet<T>::open(const CommonFG& parent, const std::string& name) {
    DataSet dataset=parent.openDataSet(name);
    dataspace=dataset.getSpace();
    p_setId(dataset.getId());
    incRefCount();
    // Check if dataspace and datatype complies with the class
    assert(dataspace.getSimpleExtentNdims()==0);
    T dummy;
    assert(getDataType()==toH5Type(dummy));
  }

  template<class T>
  void SimpleDataSet<T>::write(const T& data) {
    DataSet::write(&data, datatype, dataspace, dataspace);
  }
  template<>
  void SimpleDataSet<std::string>::write(const std::string& data);

  template<class T>
  T SimpleDataSet<T>::read() {
    T data;
    DataSet::read(&data, datatype, dataspace, dataspace);
    return data;
  }
  template<>
  std::string SimpleDataSet<std::string>::read();

  template<class T>
  void SimpleDataSet<T>::write(const CommonFG& parent, const std::string& name, const T& data) {
    create(parent, name);
    write(data);
  }

  template<class T>
  T SimpleDataSet<T>::read(const CommonFG& parent, const std::string& name)  {
    open(parent, name);
    return read();
  }

  template<class T>
  void SimpleDataSet<T>::setDescription(const std::string& description) {
    SimpleAttribute<std::string> attr(*this, "Description", description);
  }

  template<class T>
  std::string SimpleDataSet<T>::getDescription() {
    return SimpleAttribute<std::string>::getData(*this, "Description");
  }

  template<class T>
  T SimpleDataSet<T>::getData(const CommonFG& parent, const std::string& name) {
    SimpleDataSet<T> dataset;
    return dataset.read(parent, name);
  }



  template<class T>
  SimpleDataSet<std::vector<T> >::SimpleDataSet() : DataSet() {
    T dummy;
    datatype=toH5Type(dummy);
  }

  template<class T>
  SimpleDataSet<std::vector<T> >::SimpleDataSet(const SimpleDataSet<std::vector<T> >& dataset) : DataSet(dataset) {
    T dummy;
    datatype=toH5Type(dummy);
  }

  template<class T>
  SimpleDataSet<std::vector<T> >::SimpleDataSet(const CommonFG& parent, const std::string& name, bool create_) : DataSet() {
    T dummy;
    datatype=toH5Type(dummy);
    if(create_)
      create(parent, name);
    else
      open(parent, name);
  }

  template<class T>
  SimpleDataSet<std::vector<T> >::SimpleDataSet(const CommonFG& parent, const std::string& name, const std::vector<T>& data) : DataSet() {
    T dummy;
    datatype=toH5Type(dummy);
    write(parent, name, data);
  }

  template<class T>
  void SimpleDataSet<std::vector<T> >::create(const CommonFG& parent, const std::string& name) {
    hsize_t dims[]={0};
    hsize_t maxDims[]={H5S_UNLIMITED};
    DataSpace dataspace(1, dims, maxDims);
    DSetCreatPropList prop;
    hsize_t chunkDims[]={1};
    prop.setChunk(1, chunkDims);
    DataSet dataset=parent.createDataSet(name, datatype, dataspace, prop);
    p_setId(dataset.getId());
    incRefCount();
  }

  template<class T>
  void SimpleDataSet<std::vector<T> >::open(const CommonFG& parent, const std::string& name) {
    DataSet dataset=parent.openDataSet(name);
    p_setId(dataset.getId());
    incRefCount();
    // Check if dataspace and datatype complies with the class
    DataSpace dataspace=getSpace();
    assert(dataspace.getSimpleExtentNdims()==1);
    hsize_t dims[1], maxDims[1];
    dataspace.getSimpleExtentDims(dims, maxDims);
    assert(maxDims[0]==H5S_UNLIMITED);
    T dummy;
    assert(getDataType()==toH5Type(dummy));
  }

  template<class T>
  void SimpleDataSet<std::vector<T> >::write(const std::vector<T>& data) {
    hsize_t dims[]={data.size()};
    extend(dims);
    DataSpace dataspace=getSpace();
    DataSet::write(&data[0], datatype, dataspace, dataspace);
  }
  template<>
  void SimpleDataSet<std::vector<std::string> >::write(const std::vector<std::string>& data);

  template<class T>
  std::vector<T> SimpleDataSet<std::vector<T> >::read() {
    DataSpace dataspace=getSpace();
    hsize_t dims[1];
    dataspace.getSimpleExtentDims(dims);
    std::vector<T> data(dims[0]);
    DataSet::read(&data[0], datatype, dataspace, dataspace);
    return data;
  }
  template<>
  std::vector<std::string> SimpleDataSet<std::vector<std::string> >::read();

  template<class T>
  void SimpleDataSet<std::vector<T> >::write(const CommonFG& parent, const std::string& name, const std::vector<T>& data) {
    create(parent, name);
    write(data);
  }

  template<class T>
  std::vector<T> SimpleDataSet<std::vector<T> >::read(const CommonFG& parent, const std::string& name) {
    open(parent, name);
    return read();
  }

  template<class T>
  void SimpleDataSet<std::vector<T> >::setDescription(const std::string& description) {
    SimpleAttribute<std::string> attr(*this, "Description", description);
  }

  template<class T>
  std::string SimpleDataSet<std::vector<T> >::getDescription() {
    return SimpleAttribute<std::string>::getData(*this, "Description");
  }

  template<class T>
  std::vector<T> SimpleDataSet<std::vector<T> >::getData(const CommonFG& parent, const std::string& name) {
    SimpleDataSet<std::vector<T> > dataset;
    return dataset.read(parent, name);
  }



  template<class T>
  SimpleDataSet<std::vector<std::vector<T> > >::SimpleDataSet() : DataSet() {
    T dummy;
    datatype=toH5Type(dummy);
  }

  template<class T>
  SimpleDataSet<std::vector<std::vector<T> > >::SimpleDataSet(const SimpleDataSet<std::vector<std::vector<T> > >& dataset) : DataSet(dataset) {
    T dummy;
    datatype=toH5Type(dummy);
  }

  template<class T>
  SimpleDataSet<std::vector<std::vector<T> > >::SimpleDataSet(const CommonFG& parent, const std::string& name, bool create_) : DataSet() {
    T dummy;
    datatype=toH5Type(dummy);
    if(create_)
      create(parent, name);
    else
      open(parent, name);
  }

  template<class T>
  SimpleDataSet<std::vector<std::vector<T> > >::SimpleDataSet(const CommonFG& parent, const std::string& name, const std::vector<std::vector<T> >& data) : DataSet() {
    T dummy;
    datatype=toH5Type(dummy);
    write(parent, name, data);
  }

  template<class T>
  void SimpleDataSet<std::vector<std::vector<T> > >::create(const CommonFG& parent, const std::string& name) {
    hsize_t dims[]={0,0};
    hsize_t maxDims[]={H5S_UNLIMITED,H5S_UNLIMITED};
    DataSpace dataspace(2, dims, maxDims);
    DSetCreatPropList prop;
    hsize_t chunkDims[]={1,1};
    prop.setChunk(2, chunkDims);
    DataSet dataset=parent.createDataSet(name, datatype, dataspace, prop);
    p_setId(dataset.getId());
    incRefCount();
  }

  template<class T>
  void SimpleDataSet<std::vector<std::vector<T> > >::open(const CommonFG& parent, const std::string& name) {
    DataSet dataset=parent.openDataSet(name);
    p_setId(dataset.getId());
    incRefCount();
    // Check if dataspace and datatype complies with the class
    DataSpace dataspace=getSpace();
    assert(dataspace.getSimpleExtentNdims()==2);
    hsize_t dims[2], maxDims[2];
    dataspace.getSimpleExtentDims(dims, maxDims);
    assert(maxDims[0]==H5S_UNLIMITED);
    assert(maxDims[1]==H5S_UNLIMITED);
    T dummy;
    assert(getDataType()==toH5Type(dummy));
  }

  template<class T>
  void SimpleDataSet<std::vector<std::vector<T> > >::write(const std::vector<std::vector<T> >& data) {
    hsize_t dims[]={data.size(),data[0].size()};
    extend(dims);
    DataSpace dataspace=getSpace();
    T* buf=new T[dims[0]*dims[1]];
    T dummy;
    for(int r=0; r<dims[0]; r++) {
      assert(data[r].size()==dims[1]);
      memcpy(&buf[r*dims[1]], &data[r][0], sizeof(dummy)*dims[1]);
    }
    DataSet::write(buf, datatype, dataspace, dataspace);
  }
  template<>
  void SimpleDataSet<std::vector<std::vector<std::string> > >::write(const std::vector<std::vector<std::string> >& data);

  template<class T>
  std::vector<std::vector<T> > SimpleDataSet<std::vector<std::vector<T> > >::read() {
    DataSpace dataspace=getSpace();
    hsize_t dims[2];
    dataspace.getSimpleExtentDims(dims);
    std::vector<std::vector<T> > data(dims[0]);
    T* buf=new T[dims[0]*dims[1]];
    DataSet::read(buf, datatype, dataspace, dataspace);
    for(int r=0; r<dims[0]; r++) {
      data[r].resize(dims[1]);
      memcpy(&data[r][0], &buf[r*dims[1]], sizeof(T)*dims[1]);
    }
    delete[]buf;
    return data;
  }
  template<>
  std::vector<std::vector<std::string> > SimpleDataSet<std::vector<std::vector<std::string> > >::read();

  template<class T>
  void SimpleDataSet<std::vector<std::vector<T> > >::write(const CommonFG& parent, const std::string& name, const std::vector<std::vector<T> >& data) {
    create(parent, name);
    write(data);
  }

  template<class T>
  std::vector<std::vector<T> > SimpleDataSet<std::vector<std::vector<T> > >::read(const CommonFG& parent, const std::string& name) {
    open(parent, name);
    return read();
  }

  template<class T>
  void SimpleDataSet<std::vector<std::vector<T> > >::setDescription(const std::string& description) {
    SimpleAttribute<std::string> attr(*this, "Description", description);
  }

  template<class T>
  std::string SimpleDataSet<std::vector<std::vector<T> > >::getDescription() {
    return SimpleAttribute<std::string>::getData(*this, "Description");
  }

  template<class T>
  std::vector<std::vector<T> > SimpleDataSet<std::vector<std::vector<T> > >::getData(const CommonFG& parent, const std::string& name) {
    SimpleDataSet<std::vector<std::vector<T> > > dataset;
    return dataset.read(parent, name);
  }

}

#endif
