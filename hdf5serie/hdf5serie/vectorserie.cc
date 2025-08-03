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
#include <hdf5serie/simpleattribute.h>
#include <hdf5serie/toh5type.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include "utils.h"

using namespace std;

namespace H5 {

  // template definitions

  template<class T>
  VectorSerie<T>::VectorSerie(int dummy, GroupBase *parent_, const string &name_) : Dataset(parent_, name_) {
    memDataTypeID=toH5Type<T>();

    // open the dataset, get column size and chunk size, close dataset again
    openIDandFileDataSpaceID();

    // create mem space
    hsize_t memDims[]={1, dims[1]};
    memDataSpaceID.reset(H5Screate_simple(2, memDims, nullptr), &H5Sclose);
    msg(Debug)<<"HDF5:"<<endl
              <<"Opened object with name = "<<name<<", id = "<<id<<" at parent with id = "<<parent->getID()<<"."<<endl;
  }

  template<class T>
  void VectorSerie<T>::openIDandFileDataSpaceID() {
    id.reset(H5Dopen(parent->getID(), name.c_str(), H5P_DEFAULT), &H5Dclose);
    ScopedHID sid(H5Dget_space(id), &H5Sclose);
    if(H5Sget_simple_extent_ndims(sid)!=2)
      throw Exception(getPath(), "A VectorSerie dataset must have 2 dimensions.");
    hsize_t maxDims[2];
    checkCall(H5Sget_simple_extent_dims(sid, dims, maxDims));
    if(maxDims[0]!=H5S_UNLIMITED)
      throw Exception(getPath(), "A VectorSerie dataset must have unlimited dimension in the first dimension.");
    ScopedHID cpl(H5Dget_create_plist(id), &H5Pclose);
    checkCall(H5Pget_chunk(cpl, 2, maxDims));
    ScopedHID apl(H5Dget_access_plist(id), &H5Pclose);
    id.reset();
    // reopen the dataset with chunk cache == chunk size
    checkCall(H5Pset_chunk_cache(apl, 521, sizeof(T)*dims[1]*maxDims[0], 0.75));
    id.reset(H5Dopen(parent->getID(), name.c_str(), apl), &H5Dclose);
    fileDataSpaceID.reset(H5Dget_space(id), &H5Sclose);
  }

  template<class T>
  VectorSerie<T>::VectorSerie(GroupBase *parent_, const string &name_, int cols, int compression, int chunkSize, int cacheSize) : Dataset(parent_, name_) {
    memDataTypeID=toH5Type<T>();
    // create dataset with chunk cache size = chunk size
    dims[0]=0;
    dims[1]=cols;
    hsize_t maxDims[]={H5S_UNLIMITED, dims[1]};
    fileDataSpaceID.reset(H5Screate_simple(2, dims, maxDims), &H5Sclose);
    ScopedHID propID(H5Pcreate(H5P_DATASET_CREATE), &H5Pclose);
    checkCall(H5Pset_attr_phase_change(propID, 0, 0));
    hsize_t chunkDims[]={(hsize_t)chunkSize, (hsize_t)(dims[1])};
    checkCall(H5Pset_chunk(propID, 2, chunkDims));
    if(compression>0) checkCall(H5Pset_deflate(propID, compression));
    ScopedHID apl(H5Pcreate(H5P_DATASET_ACCESS), &H5Pclose);
    checkCall(H5Pset_chunk_cache(apl, 521, sizeof(T)*dims[1]*chunkSize, 0.75));
    id.reset(H5Dcreate2(parent->getID(), name.c_str(), memDataTypeID,
                       fileDataSpaceID, H5P_DEFAULT, propID, apl), &H5Dclose);

    hsize_t memDims[]={1, dims[1]};
    memDataSpaceID.reset(H5Screate_simple(2, memDims, nullptr), &H5Sclose);
    if(cacheSize>1) {
      cache.resize(boost::extents[cacheSize][cols]);
      cacheRow=0;
      memDims[0]=cacheSize;
      memDataSpaceCacheID.reset(H5Screate_simple(2, memDims, nullptr), &H5Sclose);
    }
    msg(Debug)<<"HDF5:"<<endl
              <<"Created object with name = "<<name<<", id = "<<id<<" at parent with id = "<<parent->getID()<<"."<<endl;
  }

  template<class T>
  VectorSerie<T>::~VectorSerie() = default;

  template<class T>
  void VectorSerie<T>::close() {
    int cacheSize=cache.shape()[0];
    if(cacheSize>1 && cacheRow>0) {
      writeToHDF5(cacheRow, cacheSize, cache.data());
      cacheRow=0;
    }

    Dataset::close();
    // memDataSpaceID.reset(); do not close this since its not file related (to avoid the need for reopen it in writetemp mode)
    // memDataSpaceCacheID.reset(); do not close this since its not file related (to avoid the need for reopen it in writetemp mode)
    id.reset();
  }

  template<class T>
  void VectorSerie<T>::refresh() {
    Dataset::refresh();
    fileDataSpaceID.reset(H5Dget_space(id), &H5Sclose);
  }

  template<class T>
  void VectorSerie<T>::flush() {
    int cacheSize=cache.shape()[0];
    if(cacheSize>1 && cacheRow>0) {
      writeToHDF5(cacheRow, cacheSize, cache.data());
      cacheRow=0;
    }

    Dataset::flush();
  }

  template<class T>
  void VectorSerie<T>::setDescription(const string& description) {
    SimpleAttribute<string> *desc=createChildAttribute<SimpleAttribute<string> >("Description")();
    desc->write(description);
  }

  template<class T>
  void VectorSerie<T>::writeToHDF5(size_t nrRows, size_t cacheSize, const T* data) {
    dims[0]+=nrRows;
    checkCall(H5Dset_extent(id, dims)); // this invalidates fileDataSpaceID -> get it again
    fileDataSpaceID.reset(H5Dget_space(id), &H5Sclose);

    hsize_t start[]={dims[0]-nrRows,0};
    hsize_t count[]={nrRows, dims[1]};
    checkCall(H5Sselect_hyperslab(fileDataSpaceID, H5S_SELECT_SET, start, nullptr, count, nullptr));

    if(nrRows==1) // use memDataSpeceID (1 row)
      checkCall(H5Dwrite(id, memDataTypeID, memDataSpaceID, fileDataSpaceID, H5P_DEFAULT, data));
    else if(nrRows==cacheSize) // use memDataSpaceCacheID (cacheSize rows)
      checkCall(H5Dwrite(id, memDataTypeID, memDataSpaceCacheID, fileDataSpaceID, H5P_DEFAULT, data));
    else { // create new memDataSpaceLocalID (nrRows rows) (this is only called once by close())
      hsize_t memDims[]={nrRows, dims[1]};
      ScopedHID memDataSpaceLocalID(H5Screate_simple(2, memDims, nullptr), &H5Sclose);
      checkCall(H5Dwrite(id, memDataTypeID, memDataSpaceLocalID, fileDataSpaceID, H5P_DEFAULT, data));
    }
  }

  template<class T>
  void VectorSerie<T>::append(const T data[], size_t size) {
    if(size!=dims[1]) throw Exception(getPath(), "dataset dimension does not match");

    auto cacheSize=cache.shape()[0];
    if(cacheSize>1) {
      for(size_t i=0; i<size; ++i)
        cache[cacheRow][i]=data[i];
      cacheRow++;
      if(cacheRow>=cacheSize) {
        writeToHDF5(cacheSize, cacheSize, cache.data());
        cacheRow=0;
      }
    }
    else
      writeToHDF5(1, cacheSize, data);
  }

  template<class T>
  void VectorSerie<T>::getRow(const int row, size_t size, T data[]) {
    if(size!=dims[1])
      throw Exception(getPath(), "Size of data does not match");
    int rows=getRows();
    if(row<0 || row>=rows) {
      msg(Debug)<<"HDF5 object with id = "<<id<<":"<<endl
                <<"Requested row number "<<row<<" is out of range [0.."<<rows<<"[, returning a dummy vector."<<endl;
      for(size_t i=0; i<size; ++i)
         data[i]=T();
      return;
    }

    hsize_t start[]={(hsize_t)row,0};
    hsize_t count[]={1, dims[1]};
    checkCall(H5Sselect_hyperslab(fileDataSpaceID, H5S_SELECT_SET, start, nullptr, count, nullptr));

    checkCall(H5Dread(id, memDataTypeID, memDataSpaceID, fileDataSpaceID, H5P_DEFAULT, &data[0]));
 }

  template<class T>
  void VectorSerie<T>::getColumn(const int column, size_t size, T data[]) {
    hsize_t rows=getRows();
    if(size!=rows)
      throw Exception(getPath(), "dataset dimension does not match");
    hsize_t start[]={0, (hsize_t)column};
    hsize_t count[]={rows, 1};
    checkCall(H5Sselect_hyperslab(fileDataSpaceID, H5S_SELECT_SET, start, nullptr, count, nullptr));

    ScopedHID colDataSpaceID(H5Screate_simple(2, count, nullptr), &H5Sclose);

    checkCall(H5Dread(id, memDataTypeID, colDataSpaceID, fileDataSpaceID, H5P_DEFAULT, data));
  }

  template<class T>
  string VectorSerie<T>::getDescription() {
    auto *desc=openChildAttribute<SimpleAttribute<string> >("Description");
    return desc->read();
  }

  template<class T>
  void VectorSerie<T>::setColumnLabel(const vector<string>& columnLabel) {
    if(dims[1]!=columnLabel.size())
      throw Exception(getPath(), "Size of column labe does not match");
    SimpleAttribute<vector<string> > *col=createChildAttribute<SimpleAttribute<vector<string> > >("Column Label")(columnLabel.size());
    col->write(columnLabel);
  }

  template<class T>
  vector<string> VectorSerie<T>::getColumnLabel() {
    auto *col=openChildAttribute<SimpleAttribute<vector<string> > >("Column Label");
    return col->read();
  }

  template<class T>
  void VectorSerie<T>::enableSWMR() {
    if(file->getType(true) == File::writeTempNoneSWMR)
      openIDandFileDataSpaceID();
    Dataset::enableSWMR();
  }



  // explizit template spezialisations

  template<>
  void VectorSerie<string>::append(const string data[], size_t size) {
    if(size!=dims[1]) throw Exception(getPath(), "dataset dimension does not match");
    dims[0]++;
    checkCall(H5Dset_extent(id, dims)); // this invalidates fileDataSpaceID -> get it again
    fileDataSpaceID.reset(H5Dget_space(id), &H5Sclose);
  
    hsize_t start[]={dims[0]-1,0};
    hsize_t count[]={1, dims[1]};
    checkCall(H5Sselect_hyperslab(fileDataSpaceID, H5S_SELECT_SET, start, nullptr, count, nullptr));
  
    VecStr dummy(dims[1]);
    for(unsigned int i=0; i<dims[1]; i++) {
      dummy.alloc(i, data[i].size());
      strcpy(dummy[i], data[i].c_str());
    }
    checkCall(H5Dwrite(id, memDataTypeID, memDataSpaceID, fileDataSpaceID, H5P_DEFAULT, &dummy[0]));
  }
  
  template<>
  void VectorSerie<string>::getRow(const int row, size_t size, string data[]) {
    if(size!=dims[1])
      throw Exception(getPath(), "Size of data does not match");
    int rows=getRows();
    if(row<0 || row>=rows) {
      msg(Debug)<<"HDF5 object with id = "<<id<<":"<<endl
                <<"Requested row number "<<row<<" is out of range [0.."<<rows<<"[, returning a dummy vector."<<endl;
      for(size_t i=0; i<size; ++i)
         data[i]=string();
      return;
    }

    hsize_t start[]={(hsize_t)row,0};
    hsize_t count[]={1, dims[1]};
    checkCall(H5Sselect_hyperslab(fileDataSpaceID, H5S_SELECT_SET, start, nullptr, count, nullptr));
  
    VecStr dummy(dims[1]);
    checkCall(H5Dread(id, memDataTypeID, memDataSpaceID, fileDataSpaceID, H5P_DEFAULT, &dummy[0]));
    for(unsigned int i=0; i<dims[1]; i++)
      data[i]=dummy[i];
 }
  
  template<>
  void VectorSerie<string>::getColumn(const int column, size_t size, string data[]) {
    hsize_t rows=getRows();
    if(size!=rows)
      throw Exception(getPath(), "dataset dimension does not match");
    hsize_t start[]={0, (hsize_t)column};
    hsize_t count[]={rows, 1};
    checkCall(H5Sselect_hyperslab(fileDataSpaceID, H5S_SELECT_SET, start, nullptr, count, nullptr));
  
    ScopedHID colDataSpaceID(H5Screate_simple(2, count, nullptr), &H5Sclose);
  
    VecStr dummy(rows);
    checkCall(H5Dread(id, memDataTypeID, colDataSpaceID, fileDataSpaceID, H5P_DEFAULT, &dummy[0]));
    for(unsigned int i=0; i<rows; i++)
      data[i]=dummy[i];
  }



  // explizit template instantations

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
  template class VectorSerie<CTYPE>;
# include "hdf5serie/knowntypes.def"
# undef FOREACHKNOWNTYPE

}
