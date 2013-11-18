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
 *   friedrich.at.gc@googlemail.com
 *
 */

#include <config.h>
#include <hdf5serie/vectorserie.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include "utils.h"

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
  void VectorSerie<T>::create(const CommonFG& parent, const std::string& name,
                              const std::vector<std::string>& columnLabel, int compression, int chunkSize) {
    // create dataset with chunk cache size = chunk size
    dims[0]=0;
    dims[1]=columnLabel.size();
    hsize_t maxDims[]={H5S_UNLIMITED, dims[1]};
    DataSpace fileDataSpace(2, dims, maxDims);
    DSetCreatPropList prop;
    hsize_t chunkDims[]={(hsize_t)chunkSize, (hsize_t)(dims[1])};
    prop.setChunk(2, chunkDims);
    if(compression>0) prop.setDeflate(compression);
    hid_t apl=H5Pcreate(H5P_DATASET_ACCESS);
    H5Pset_chunk_cache(apl, 521, sizeof(T)*dims[1]*chunkSize, 0.75);
    hid_t did=H5Dcreate(dynamic_cast<const Group&>(parent).getId(), name.c_str(), memDataType.getId(),
                        fileDataSpace.getId(), H5P_DEFAULT, prop.getId(), apl); // increments the refcount
    H5Pclose(apl);
    p_setId(did); // transfer the c handle did to c++ (do not increment the refcount since H5Dclose(did) is not called)

    SimpleAttribute<std::vector<std::string> > colHead(*this, "Column Label", columnLabel);

    hsize_t memDims[]={1, dims[1]};
    memDataSpace=DataSpace(2, memDims);
    //cout<<"INFO from HDF5:"<<endl
    //    <<"  Created object with name = "<<name<<", id = "<<getId()<<" at parent with id = "<<((Group*)&parent)->getId()<<"."<<endl;
  }

  template<class T>
  void VectorSerie<T>::open(const CommonFG& parent, const std::string& name) {
    // open the dataset, get column size and chunk size, close dataset again
    hid_t did=H5Dopen(dynamic_cast<const Group&>(parent).getId(), name.c_str(), H5P_DEFAULT); // increments the refcount
    hid_t sid=H5Dget_space(did);
    assert(H5Sget_simple_extent_ndims(sid)==2);
    hsize_t maxDims[2];
    H5Sget_simple_extent_dims(sid, dims, maxDims);
    assert(maxDims[0]==H5S_UNLIMITED);
    H5Sclose(sid);
    hid_t cpl=H5Dget_create_plist(did);
    H5Pget_chunk(cpl, 2, maxDims);
    H5Pclose(cpl);
    hid_t apl=H5Dget_access_plist(did);
    H5Dclose(did); // decrements the refcount
    // reopen the dataset with chunk cache == chunk size
    H5Pset_chunk_cache(apl, 521, sizeof(T)*dims[1]*maxDims[0], 0.75);
    did=H5Dopen(dynamic_cast<const Group&>(parent).getId(), name.c_str(), apl); // increments the refcount
    H5Pclose(apl);
    // assigne the dataset to the c++ object
    p_setId(did); // transfer the c handle did to c++ (do not increment the refcount since H5Dclose(did) is not called)

    // get file space
    DataSpace fileDataSpace=getSpace();
    assert(getDataType().getClass()==memDataType.getClass());

    // create mem space
    hsize_t memDims[]={1, dims[1]};
    memDataSpace=DataSpace(2, memDims);
    //cout<<"INFO from HDF5:"<<endl
    //    <<"  Opened object with name = "<<name<<", id = "<<getId()<<" at parent with id = "<<((Group*)&parent)->getId()<<"."<<endl;
  }

  template<class T>
  void VectorSerie<T>::setDescription(const std::string& description) {
    SimpleAttribute<std::string> desc(*this, "Description", description);
  }

  template<class T>
  void VectorSerie<T>::append(const std::vector<T> &data) {
    if(data.size()!=dims[1]) throw runtime_error("dataset dimension does not match");
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
    std::vector<T> data(dims[1], T());
    if(row<0 || row>=(int)dims[0]) {
      cerr<<"WARNING from HDF5 object with id = "<<getId()<<":"<<endl
          <<"  Requested vector number is out of range, returning a dummy vector."<<endl;
      return data;
    }

    hsize_t start[]={(hsize_t)row,0};
    hsize_t count[]={1, dims[1]};
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, count, start);

    read(&data[0], memDataType, memDataSpace, fileDataSpace);
    return data;
  }
  template<>
  std::vector<std::string> VectorSerie<std::string>::getRow(const int row);

  template<class T>
  std::vector<T> VectorSerie<T>::getColumn(const int column) {
    hsize_t rows=getRows();
    hsize_t start[]={0, (hsize_t)column};
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
    if(data.size()!=dims[1]) throw runtime_error("dataset dimension does not match");
    dims[0]++;
    DataSet::extend(dims);
  
    hsize_t start[]={dims[0]-1,0};
    hsize_t count[]={1, dims[1]};
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, count, start);
  
    VecStr dummy(dims[1]);
    for(unsigned int i=0; i<dims[1]; i++) {
      dummy[i]=(char*)malloc((data[i].size()+1)*sizeof(char));
      strcpy(dummy[i], data[i].c_str());
    }
    write(&dummy[0], memDataType, memDataSpace, fileDataSpace);
  }
  
  template<>
  vector<string> VectorSerie<string>::getRow(const int row) {
    vector<string> data(dims[1], string());
    if(row<0 || row>=(int)dims[0]) {
      cerr<<"WARNING from HDF5 object with id = "<<getId()<<":"<<endl
          <<"  Requested vector number is out of range, returning a dummy vector."<<endl;
      return data;
    }

    hsize_t start[]={(hsize_t)row,0};
    hsize_t count[]={1, dims[1]};
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, count, start);
  
    VecStr dummy(dims[1]);
    read(&dummy[0], memDataType, memDataSpace, fileDataSpace);
    for(unsigned int i=0; i<dims[1]; i++)
      data[i]=dummy[i];
    return data;
  }
  
  template<>
  vector<string> VectorSerie<string>::getColumn(const int column) {
    hsize_t rows=getRows();
    hsize_t start[]={0, (hsize_t)column};
    hsize_t count[]={rows, 1};
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, count, start);
  
    DataSpace coldataspace(2, count);
  
    VecStr dummy(rows);
    read(&dummy[0], memDataType, coldataspace, fileDataSpace);
    vector<string> data(rows);
    for(unsigned int i=0; i<rows; i++)
      data[i]=dummy[i];
    return data;
  }



  // explizit template instantations

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
  template class VectorSerie<CTYPE>;
# include "hdf5serie/knowntypes.def"
# undef FOREACHKNOWNTYPE

}
