/* Copyright (C) 2009 Markus Friedrich
 *
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version. 
 *  
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * Lesser General Public License for more details. 
 *  
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Contact:
 *   mafriedrich@users.berlios.de
 *
 */

#include <config.h>
#include <hdf5serie/vectorserie.h>
#include <cstdlib>
#include <cstring>

using namespace std;

namespace H5 {

  // template definitions

  template<class T>
  VectorSerie<T>::VectorSerie() : DataSet(), memDataSpace() {
    T dummy;
    memDataType=toH5Type(dummy);
    dims[0]=0;
    dims[1]=0;
  }

  template<class T>
  VectorSerie<T>::VectorSerie(const VectorSerie<T>& dataset) : DataSet(dataset) {
    T dummy;
    memDataType=toH5Type(dummy);
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.getSimpleExtentDims(dims);
    hsize_t memDims[]={1, dims[1]};
    memDataSpace=DataSpace(2, memDims);
  }

  template<class T>
  VectorSerie<T>::VectorSerie(const CommonFG& parent, const std::string& name) {
    T dummy;
    memDataType=toH5Type(dummy);
    open(parent, name);
  }

  template<class T>
  VectorSerie<T>::VectorSerie(const CommonFG& parent, const std::string& name, const std::vector<std::string>& columnLabel, int compression, int chunkSize) {
    T dummy;
    memDataType=toH5Type(dummy);
    create(parent, name, columnLabel, compression, chunkSize);
  }

  template<class T>
  void VectorSerie<T>::create(const CommonFG& parent, const std::string& name, const std::vector<std::string>& columnLabel, int compression, int chunkSize) {
    dims[0]=0;
    dims[1]=columnLabel.size();
    hsize_t maxDims[]={H5S_UNLIMITED, dims[1]};
    DataSpace fileDataSpace(2, dims, maxDims);
    DSetCreatPropList prop;
    hsize_t chunkDims[]={chunkSize, dims[1]};
    prop.setChunk(2, chunkDims);
    if(compression>0) prop.setDeflate(compression);
    DataSet dataset=parent.createDataSet(name, memDataType, fileDataSpace, prop);
    p_setId(dataset.getId());
    incRefCount();

    SimpleAttribute<std::vector<std::string> > colHead(*this, "Column Label", columnLabel);

    hsize_t memDims[]={1, dims[1]};
    memDataSpace=DataSpace(2, memDims);
  }

  template<class T>
  void VectorSerie<T>::open(const CommonFG& parent, const std::string& name) {
    DataSet dataset=parent.openDataSet(name);
    p_setId(dataset.getId());
    incRefCount();

    DataSpace fileDataSpace=getSpace();
    // Check if fileDataSpace and memDataType complies with the class
    assert(fileDataSpace.getSimpleExtentNdims()==2);
    hsize_t maxDims[2];
    fileDataSpace.getSimpleExtentDims(dims, maxDims);
    assert(maxDims[0]==H5S_UNLIMITED);
    assert(getDataType().getClass()==memDataType.getClass());

    hsize_t memDims[]={1, dims[1]};
    memDataSpace=DataSpace(2, memDims);
  }

  template<class T>
  void VectorSerie<T>::setDescription(const std::string& description) {
    SimpleAttribute<std::string> desc(*this, "Description", description);
  }

  template<class T>
  void VectorSerie<T>::append(const std::vector<T> &data) {
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
  void VectorSerie<std::string>::append(const std::vector<std::string> &data);

  template<class T>
  std::vector<T> VectorSerie<T>::getRow(const int row) {
    hsize_t start[]={row,0};
    hsize_t count[]={1, dims[1]};
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, count, start);

    std::vector<T> data(dims[1]);
    read(&data[0], memDataType, memDataSpace, fileDataSpace);
    return data;
  }
  template<>
  std::vector<std::string> VectorSerie<std::string>::getRow(const int row);

  template<class T>
  std::vector<T> VectorSerie<T>::getColumn(const int column) {
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
  std::vector<std::string> VectorSerie<std::string>::getColumn(const int column);

  template<class T>
  std::string VectorSerie<T>::getDescription() {
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
  std::vector<std::string> VectorSerie<T>::getColumnLabel() {
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
  void VectorSerie<T>::extend(const hsize_t* size) {
    assert(1);
  }



  // explizit template spezialisations

  template<>
  void VectorSerie<string>::append(const vector<string> &data) {
    assert(data.size()==dims[1]);
    dims[0]++;
    DataSet::extend(dims);
  
    hsize_t start[]={dims[0]-1,0};
    hsize_t count[]={1, dims[1]};
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, count, start);
  
    char** dummy=new char*[dims[1]];
    for(unsigned int i=0; i<dims[1]; i++) {
      dummy[i]=new char[data[i].size()+1];
      strcpy(dummy[i], data[i].c_str());
    }
    write(dummy, memDataType, memDataSpace, fileDataSpace);
    for(unsigned int i=0; i<dims[1]; i++) delete[]dummy[i];
    delete[]dummy;
  }
  
  template<>
  vector<string> VectorSerie<string>::getRow(const int row) {
    hsize_t start[]={row,0};
    hsize_t count[]={1, dims[1]};
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, count, start);
  
    char** dummy=new char*[dims[1]];
    read(dummy, memDataType, memDataSpace, fileDataSpace);
    vector<string> data(dims[1]);
    for(unsigned int i=0; i<dims[1]; i++) {
      data[i]=dummy[i];
      free(dummy[i]);
    }
    delete[]dummy;
    return data;
  }
  
  template<>
  vector<string> VectorSerie<string>::getColumn(const int column) {
    hsize_t rows=getRows();
    hsize_t start[]={0, column};
    hsize_t count[]={rows, 1};
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, count, start);
  
    DataSpace coldataspace(2, count);
  
    char** dummy=new char*[rows];
    read(dummy, memDataType, coldataspace, fileDataSpace);
    vector<string> data(rows);
    for(unsigned int i=0; i<rows; i++) {
      data[i]=dummy[i];
      free(dummy[i]);
    }
    delete[]dummy;
    return data;
  }



  // explizit template instantations

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
  template class VectorSerie<CTYPE>;
# include "hdf5serie/knowntypes.def"
# undef FOREACHKNOWNTYPE

}
