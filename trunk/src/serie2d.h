#ifndef _TIMESERIE2D_H_
#define _TIMESERIE2D_H_

#include <H5Cpp.h>
#include <vector>
#include <string>
#include <assert.h>
#include <simpleattribute.h>

namespace H5 {

  template<class T>
  class Serie2D : public DataSet {
    private:
      DataType datatype;
      DataSpace memdataspace;
      hsize_t dims[2];
    public:
      Serie2D();
      Serie2D(const Serie2D<T>& dataset);
      Serie2D(const CommonFG& parent, const std::string& name);
      Serie2D(const CommonFG& parent, const std::string& name, const std::vector<std::string>& columnLabel);
      void create(const CommonFG& parent, const std::string& name, const std::vector<std::string>& columnLabel);
      void open(const CommonFG& parent, const std::string& name);
      void setDescription(const std::string& desc);
      void append(const std::vector<T> &data);
      inline int getRows();
      inline int getColumns();
      std::vector<T> getRow(const int row);
      std::vector<T> getColumn(const int column);
      std::string getDescription();
      std::vector<std::string> getColumnLabel();

      void extend(const hsize_t* size);
  };



  template<class T>
  Serie2D<T>::Serie2D() : DataSet(), memdataspace() {
    T dummy;
    datatype=toH5Type(dummy);
    dims[0]=0;
    dims[1]=0;
  }

  template<class T>
  Serie2D<T>::Serie2D(const Serie2D<T>& dataset) : DataSet(dataset) {
    T dummy;
    datatype=toH5Type(dummy);
    DataSpace dataspace=getSpace();
    dataspace.getSimpleExtentDims(dims);
    hsize_t memDims[]={1, dims[1]};
    memdataspace=DataSpace(2, memDims);
  }

  template<class T>
  Serie2D<T>::Serie2D(const CommonFG& parent, const std::string& name) {
    T dummy;
    datatype=toH5Type(dummy);
    open(parent, name);
  }

  template<class T>
  Serie2D<T>::Serie2D(const CommonFG& parent, const std::string& name, const std::vector<std::string>& columnLabel) {
    T dummy;
    datatype=toH5Type(dummy);
    create(parent, name, columnLabel);
  }

  template<class T>
  void Serie2D<T>::create(const CommonFG& parent, const std::string& name, const std::vector<std::string>& columnLabel) {
    dims[0]=0;
    dims[1]=columnLabel.size();
    hsize_t maxDims[]={H5S_UNLIMITED, dims[1]};
    DataSpace dataspace(2, dims, maxDims);
    DSetCreatPropList prop;
    hsize_t chunkDims[]={1000, dims[1]};
    prop.setChunk(2, chunkDims);
    DataSet dataset=parent.createDataSet(name, datatype, dataspace, prop);
    p_setId(dataset.getId());
    incRefCount();

    SimpleAttribute<std::vector<std::string> > colHead(*this, "Column Label", columnLabel);

    hsize_t memDims[]={1, dims[1]};
    memdataspace=DataSpace(2, memDims);
  }

  template<class T>
  void Serie2D<T>::open(const CommonFG& parent, const std::string& name) {
    DataSet dataset=parent.openDataSet(name);
    p_setId(dataset.getId());
    incRefCount();

    DataSpace dataspace=getSpace();
    // Check if dataspace and datatype complies with the class
    assert(dataspace.getSimpleExtentNdims()==2);
    hsize_t maxDims[2];
    dataspace.getSimpleExtentDims(dims, maxDims);
    assert(maxDims[0]==H5S_UNLIMITED);
    T dummy;
    assert(getDataType()==toH5Type(dummy));

    hsize_t memDims[]={1, dims[1]};
    memdataspace=DataSpace(2, memDims);
  }

  template<class T>
  void Serie2D<T>::setDescription(const std::string& description) {
    SimpleAttribute<std::string> desc(*this, "Description", description);
  }

  template<class T>
  void Serie2D<T>::append(const std::vector<T> &data) {
    assert(data.size()==dims[1]);
    dims[0]++;
    DataSet::extend(dims);

    hsize_t start[]={dims[0]-1,0};
    hsize_t count[]={1, dims[1]};
    DataSpace dataspace=getSpace();
    dataspace.selectHyperslab(H5S_SELECT_SET, count, start);

    write(&data[0], datatype, memdataspace, dataspace);
  }
  template<>
  void Serie2D<std::string>::append(const std::vector<std::string> &data);

  template<class T>
  int Serie2D<T>::getRows() {
    //////////
    return dims[0];
    //////////
    //DataSpace dataspace=getSpace();
    //dataspace.getSimpleExtentDims(dims);
    //return dims[0];
    //////////
  }

  template<class T>
  int Serie2D<T>::getColumns() {
    return dims[1];
  }

  template<class T>
  std::vector<T> Serie2D<T>::getRow(const int row) {
    hsize_t start[]={row,0};
    hsize_t count[]={1, dims[1]};
    DataSpace dataspace=getSpace();
    dataspace.selectHyperslab(H5S_SELECT_SET, count, start);

    std::vector<T> data(dims[1]);
    read(&data[0], datatype, memdataspace, dataspace);
    return data;
  }
  template<>
  std::vector<std::string> Serie2D<std::string>::getRow(const int row);

  template<class T>
  std::vector<T> Serie2D<T>::getColumn(const int column) {
    hsize_t rows=getRows();
    hsize_t start[]={0, column};
    hsize_t count[]={rows, 1};
    DataSpace dataspace=getSpace();
    dataspace.selectHyperslab(H5S_SELECT_SET, count, start);

    DataSpace coldataspace(2, count);

    std::vector<T> data(rows);
    read(&data[0], datatype, coldataspace, dataspace);
    return data;
  }
  template<>
  std::vector<std::string> Serie2D<std::string>::getColumn(const int column);

  template<class T>
  std::string Serie2D<T>::getDescription() {
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
  std::vector<std::string> Serie2D<T>::getColumnLabel() {
    // save and disable c error printing
    H5E_auto2_t func;
    void* client_data;
    Exception::getAutoPrint(func, &client_data);
    Exception::dontPrint();
    std::vector<std::string> ret;
    // catch error if Attribute is not found
    try {
      ret=SimpleAttribute<std::vector<std::string> >::getData(*this, "Column Label");
    }
    catch(AttributeIException e) {
      ret=std::vector<std::string>(getColumns());
    }
    // restore c error printing
    Exception::setAutoPrint(func, client_data);
    return ret;
  }

  template<class T>
  void Serie2D<T>::extend(const hsize_t* size) {
    assert(1);
  }

}

#endif // _TIMESERIE_H_
