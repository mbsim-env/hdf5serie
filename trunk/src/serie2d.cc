#include <config.h>
#include <serie2d.h>

using namespace std;

namespace H5 {

  // template definitions

  template<class T>
  Serie2D<T>::Serie2D() : DataSet(), memDataSpace() {
    T dummy;
    memDataType=toH5Type(dummy);
    dims[0]=0;
    dims[1]=0;
  }

  template<class T>
  Serie2D<T>::Serie2D(const Serie2D<T>& dataset) : DataSet(dataset) {
    T dummy;
    memDataType=toH5Type(dummy);
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.getSimpleExtentDims(dims);
    hsize_t memDims[]={1, dims[1]};
    memDataSpace=DataSpace(2, memDims);
  }

  template<class T>
  Serie2D<T>::Serie2D(const CommonFG& parent, const std::string& name) {
    T dummy;
    memDataType=toH5Type(dummy);
    open(parent, name);
  }

  template<class T>
  Serie2D<T>::Serie2D(const CommonFG& parent, const std::string& name, const std::vector<std::string>& columnLabel) {
    T dummy;
    memDataType=toH5Type(dummy);
    create(parent, name, columnLabel);
  }

  template<class T>
  void Serie2D<T>::create(const CommonFG& parent, const std::string& name, const std::vector<std::string>& columnLabel) {
    dims[0]=0;
    dims[1]=columnLabel.size();
    hsize_t maxDims[]={H5S_UNLIMITED, dims[1]};
    DataSpace fileDataSpace(2, dims, maxDims);
    DSetCreatPropList prop;
    hsize_t chunkDims[]={1000, dims[1]};
    prop.setChunk(2, chunkDims);
    DataSet dataset=parent.createDataSet(name, memDataType, fileDataSpace, prop);
    p_setId(dataset.getId());
    incRefCount();

    SimpleAttribute<std::vector<std::string> > colHead(*this, "Column Label", columnLabel);

    hsize_t memDims[]={1, dims[1]};
    memDataSpace=DataSpace(2, memDims);
  }

  template<class T>
  void Serie2D<T>::open(const CommonFG& parent, const std::string& name) {
    DataSet dataset=parent.openDataSet(name);
    p_setId(dataset.getId());
    incRefCount();

    DataSpace fileDataSpace=getSpace();
    // Check if fileDataSpace and memDataType complies with the class
    assert(fileDataSpace.getSimpleExtentNdims()==2);
    hsize_t maxDims[2];
    fileDataSpace.getSimpleExtentDims(dims, maxDims);
    assert(maxDims[0]==H5S_UNLIMITED);
    T dummy;
    assert(getDataType().getClass()==memDataType.getClass());

    hsize_t memDims[]={1, dims[1]};
    memDataSpace=DataSpace(2, memDims);
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
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, count, start);

    write(&data[0], memDataType, memDataSpace, fileDataSpace);
  }
  template<>
  void Serie2D<std::string>::append(const std::vector<std::string> &data);

  template<class T>
  void Serie2D<T>::operator<<(const std::vector<T>& data) {
    append(data);
  }

  template<class T>
  std::vector<T> Serie2D<T>::getRow(const int row) {
    hsize_t start[]={row,0};
    hsize_t count[]={1, dims[1]};
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, count, start);

    std::vector<T> data(dims[1]);
    read(&data[0], memDataType, memDataSpace, fileDataSpace);
    return data;
  }
  template<>
  std::vector<std::string> Serie2D<std::string>::getRow(const int row);

  template<class T>
  std::vector<T> Serie2D<T>::getColumn(const int column) {
    hsize_t rows=getRows();
    hsize_t start[]={0, column};
    hsize_t count[]={rows, 1};
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, count, start);

    DataSpace coldataspace(2, count);

    std::vector<T> data(rows);
    read(&data[0], memDataType, coldataspace, fileDataSpace);
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



  // explizit template spezialisations

  template<>
  void Serie2D<string>::append(const vector<string> &data) {
    assert(data.size()==dims[1]);
    dims[0]++;
    DataSet::extend(dims);
  
    hsize_t start[]={dims[0]-1,0};
    hsize_t count[]={1, dims[1]};
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, count, start);
  
    char** dummy=new char*[dims[1]];
    for(int i=0; i<dims[1]; i++) {
      dummy[i]=new char[data[i].size()+1];
      strcpy(dummy[i], data[i].c_str());
    }
    write(dummy, memDataType, memDataSpace, fileDataSpace);
    for(int i=0; i<dims[1]; i++) delete[]dummy[i];
    delete[]dummy;
  }
  
  template<>
  vector<string> Serie2D<string>::getRow(const int row) {
    hsize_t start[]={row,0};
    hsize_t count[]={1, dims[1]};
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, count, start);
  
    char** dummy=new char*[dims[1]];
    read(dummy, memDataType, memDataSpace, fileDataSpace);
    vector<string> data(dims[1]);
    for(int i=0; i<dims[1]; i++) {
      data[i]=dummy[i];
      free(dummy[i]);
    }
    delete[]dummy;
    return data;
  }
  
  template<>
  vector<string> Serie2D<string>::getColumn(const int column) {
    hsize_t rows=getRows();
    hsize_t start[]={0, column};
    hsize_t count[]={rows, 1};
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, count, start);
  
    DataSpace coldataspace(2, count);
  
    char** dummy=new char*[rows];
    read(dummy, memDataType, coldataspace, fileDataSpace);
    vector<string> data(rows);
    for(int i=0; i<rows; i++) {
      data[i]=dummy[i];
      free(dummy[i]);
    }
    delete[]dummy;
    return data;
  }



  // explizit template instantations

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
  template class Serie2D<CTYPE>;
# include "knowntypes.def"
# undef FOREACHKNOWNTYPE

}
