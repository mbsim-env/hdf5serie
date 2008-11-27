#ifndef _TIMESERIE1D_H_
#define _TIMESERIE1D_H_

#include <H5Cpp.h>
#include <vector>
#include <string>
#include <assert.h>
#include <simpleattribute.h>

namespace H5 {

  class MemberNameType {
    public:
      MemberNameType() {}
      MemberNameType(const std::string& name_, const std::string& type_) { name=name_; type=type_; }
      std::string name;
      std::string type;
  };

  template<class T>
  class Serie1D : public DataSet {
    private:
      CompType datatype;
      DataSpace memdataspace;
      hsize_t dims[1];
    public:
      Serie1D();
      Serie1D(const Serie1D<T>& dataset);
      Serie1D(const CommonFG& parent, const std::string& name);
      Serie1D(const CommonFG& parent, const std::string& name, const std::vector<MemberNameType>& nameType);
      void create(const CommonFG& parent, const std::string& name, const std::vector<MemberNameType>& nameType);
      void open(const CommonFG& parent, const std::string& name);
      void setDescription(const std::string& desc);
      void append(const T& data);
      inline int getRows();
      inline int getMembers();
      T getRow(const int row);
      //std::vector<T> getColumn(const int column);
      std::string getDescription();
      std::vector<std::string> getMemberLabel();

      void extend(const hsize_t* size);
  };



  template<class T>
  Serie1D<T>::Serie1D() : DataSet(), datatype() {
    dims[0]=0;
    hsize_t memDims[]={1};
    memdataspace=DataSpace(1, memDims);
  }

  template<class T>
  Serie1D<T>::Serie1D(const Serie1D<T>& dataset) : DataSet(dataset) {
    assert(dataset.getDataType().getClass()==H5T_COMPOUND);
    datatype=dataset.getCompType();
    DataSpace dataspace=getSpace();
    dataspace.getSimpleExtentDims(dims);
    hsize_t memDims[]={1};
    memdataspace=DataSpace(1, memDims);
  }

  template<class T>
  Serie1D<T>::Serie1D(const CommonFG& parent, const std::string& name) {
    hsize_t memDims[]={1};
    memdataspace=DataSpace(1, memDims);
    open(parent, name);
  }

  template<class T>
  Serie1D<T>::Serie1D(const CommonFG& parent, const std::string& name, const std::vector<MemberNameType>& nameType) {
    hsize_t memDims[]={1};
    memdataspace=DataSpace(1, memDims);
    create(parent, name, nameType);
  }

  template<class T>
  void Serie1D<T>::create(const CommonFG& parent, const std::string& name, const std::vector<MemberNameType>& nameType) {
    dims[0]=0;
    hsize_t maxDims[]={H5S_UNLIMITED};
    DataSpace dataspace(1, dims, maxDims);
    DSetCreatPropList prop;
    hsize_t chunkDims[]={1000};
    prop.setChunk(1, chunkDims);

    size_t size=0;
    for(int i=0; i<nameType.size(); i++)
      size+=strToH5Type(nameType[i].type).getSize();
    assert(size=sizeof(T));
    CompType comptype(size);
    size=0;
    for(int i=0; i<nameType.size(); i++) {
      comptype.insertMember(nameType[i].name, size, strToH5Type(nameType[i].type));
      size+=strToH5Type(nameType[i].type).getSize();
    }
    datatype=comptype;

    DataSet dataset=parent.createDataSet(name, datatype, dataspace, prop);
    p_setId(dataset.getId());
    incRefCount();
  }

  template<class T>
  void Serie1D<T>::open(const CommonFG& parent, const std::string& name) {
    DataSet dataset=parent.openDataSet(name);
    p_setId(dataset.getId());
    incRefCount();

    DataSpace dataspace=getSpace();
    // Check if dataspace and datatype complies with the class
    assert(dataspace.getSimpleExtentNdims()==1);
    hsize_t maxDims[1];
    dataspace.getSimpleExtentDims(dims, maxDims);
    assert(maxDims[0]==H5S_UNLIMITED);

    assert(dataset.getDataType().getClass()==H5T_COMPOUND);
    datatype=getCompType();
    assert(datatype.getSize()==sizeof(T));
  }

  template<class T>
  void Serie1D<T>::setDescription(const std::string& description) {
    SimpleAttribute<std::string> desc(*this, "Description", description);
  }

  template<class T>
  void Serie1D<T>::append(const T& data) {
    dims[0]++;
    DataSet::extend(dims);

    hsize_t start[]={dims[0]-1};
    hsize_t count[]={1};
    DataSpace dataspace=getSpace();
    dataspace.selectHyperslab(H5S_SELECT_SET, count, start);

    write(&data, datatype, memdataspace, dataspace);
  }

  template<class T>
  int Serie1D<T>::getRows() {
    //////////
    return dims[0];
    //////////
    //DataSpace dataspace=getSpace();
    //dataspace.getSimpleExtentDims(dims);
    //return dims[0];
    //////////
  }

  template<class T>
  int Serie1D<T>::getMembers() {
    return datatype.getNmembers();
  }

  template<class T>
  T Serie1D<T>::getRow(const int row) {
    hsize_t start[]={row};
    hsize_t count[]={1};
    DataSpace dataspace=getSpace();
    dataspace.selectHyperslab(H5S_SELECT_SET, count, start);

    T data;
    read(&data, datatype, memdataspace, dataspace);
    return data;
  }

/*  template<class T>
  std::vector<T> Serie1D<T>::getColumn(const int column) {
    hsize_t rows=getRows();
    hsize_t start[]={0, column};
    hsize_t count[]={rows, 1};
    DataSpace dataspace=getSpace();
    dataspace.selectHyperslab(H5S_SELECT_SET, count, start);

    DataSpace coldataspace(2, count);

    std::vector<T> data(rows);
    read(&data[0], datatype, coldataspace, dataspace);
    return data;
  }*/

  template<class T>
  std::string Serie1D<T>::getDescription() {
    // save and disable c error printing
    H5E_auto2_t func;
    void* client_data;
    Exception::getAutoPrint(func, &client_data);
    Exception::dontPrint();
    std::string ret;
    // catch error if Attribute is not found
    try {
      ret=SimpleAttribute<std::string>::getData(*this, "Description");
    }
    catch(AttributeIException e) {
      ret=std::string();
    }
    // restore c error printing
    Exception::setAutoPrint(func, client_data);
    return ret;
  }

  template<class T>
  std::vector<std::string> Serie1D<T>::getMemberLabel() {
    std::vector<std::string> ret;
    for(int i=0; i<datatype.getNmembers(); i++)
      ret.push_back(datatype.getMemberName(i));
    return ret;
  }

  template<class T>
  void Serie1D<T>::extend(const hsize_t* size) {
    assert(1);
  }

}

#endif // _TIMESERIE_H_
