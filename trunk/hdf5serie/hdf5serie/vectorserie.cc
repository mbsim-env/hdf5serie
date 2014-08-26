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
    T dummy2;
    memDataTypeID=toH5Type(dummy2);
    open();
  }

  template<class T>
  VectorSerie<T>::VectorSerie(GroupBase *parent_, const string &name_, int cols, int compression, int chunkSize) : Dataset(parent_, name_) {
    T dummy;
    memDataTypeID=toH5Type(dummy);
    // create dataset with chunk cache size = chunk size
    dims[0]=0;
    dims[1]=cols;
    hsize_t maxDims[]={H5S_UNLIMITED, dims[1]};
    ScopedHID fileDataSpaceID(H5Screate_simple(2, dims, maxDims), &H5Sclose);
    ScopedHID propID(H5Pcreate(H5P_DATASET_CREATE), &H5Pclose);
    hsize_t chunkDims[]={(hsize_t)chunkSize, (hsize_t)(dims[1])};
    H5Pset_chunk(propID, 2, chunkDims);
    if(compression>0) H5Pset_deflate(propID, compression);
    ScopedHID apl(H5Pcreate(H5P_DATASET_ACCESS), &H5Pclose);
    H5Pset_chunk_cache(apl, 521, sizeof(T)*dims[1]*chunkSize, 0.75);
    id.reset(H5Dcreate2(parent->getID(), name.c_str(), memDataTypeID,
                       fileDataSpaceID, H5P_DEFAULT, propID, apl), &H5Dclose);

    hsize_t memDims[]={1, dims[1]};
    memDataSpaceID.reset(H5Screate_simple(2, memDims, NULL), &H5Sclose);
    msg(Debug)<<"HDF5:\n"
              <<"Created object with name = "<<name<<", id = "<<id<<" at parent with id = "<<parent->getID()<<"."<<endl;
  }

  template<class T>
  VectorSerie<T>::~VectorSerie() {
  }

  template<class T>
  void VectorSerie<T>::close() {
    Dataset::close();
    memDataSpaceID.reset();
    id.reset();
  }

  template<class T>
  void VectorSerie<T>::open() {
    // open the dataset, get column size and chunk size, close dataset again
    id.reset(H5Dopen(parent->getID(), name.c_str(), H5P_DEFAULT), &H5Dclose);
    ScopedHID sid(H5Dget_space(id), &H5Sclose);
    if(H5Sget_simple_extent_ndims(sid)!=2)
      throw Exception("A VectorSerie dataset must have 2 dimensions.");
    hsize_t maxDims[2];
    H5Sget_simple_extent_dims(sid, dims, maxDims);
    if(maxDims[0]!=H5S_UNLIMITED)
      throw Exception("A VectorSerie dataset must have unlimited dimension in the first dimension.");
    ScopedHID cpl(H5Dget_create_plist(id), &H5Pclose);
    H5Pget_chunk(cpl, 2, maxDims);
    ScopedHID apl(H5Dget_access_plist(id), &H5Pclose);
    id.reset();
    // reopen the dataset with chunk cache == chunk size
    H5Pset_chunk_cache(apl, 521, sizeof(T)*dims[1]*maxDims[0], 0.75);
    id.reset(H5Dopen(parent->getID(), name.c_str(), apl), &H5Dclose);

    // create mem space
    hsize_t memDims[]={1, dims[1]};
    memDataSpaceID.reset(H5Screate_simple(2, memDims, NULL), &H5Sclose);
    msg(Debug)<<"HDF5:\n"
              <<"Opened object with name = "<<name<<", id = "<<id<<" at parent with id = "<<parent->getID()<<"."<<endl;
    Dataset::open();
  }

  template<class T>
  void VectorSerie<T>::setDescription(const string& description) {
    SimpleAttribute<string> *desc=createChildAttribute<SimpleAttribute<string> >("Description")();
    desc->write(description);
  }

  template<class T>
  void VectorSerie<T>::append(const T data[], size_t size) {
    if(size!=dims[1]) throw Exception("dataset dimension does not match");
    dims[0]++;
    H5Dset_extent(id, dims);

    hsize_t start[]={dims[0]-1,0};
    hsize_t count[]={1, dims[1]};
    ScopedHID fileDataSpaceID(H5Dget_space(id), &H5Sclose);
    H5Sselect_hyperslab(fileDataSpaceID, H5S_SELECT_SET, start, NULL, count, NULL);

    H5Dwrite(id, memDataTypeID, memDataSpaceID, fileDataSpaceID, H5P_DEFAULT, data);
  }

  template<class T>
  void VectorSerie<T>::getRow(const int row, size_t size, T data[]) {
    if(size!=dims[1])
      throw Exception("Size of data does not match");
    int rows=getRows();
    if(row<0 || row>=rows) {
      msg(Warn)<<"HDF5 object with id = "<<id<<":\n"
               <<"Requested row number "<<row<<" is out of range [0.."<<rows<<"[, returning a dummy vector."<<endl;
      for(size_t i=0; i<size; ++i)
         data[i]=T();
      return;
    }

    hsize_t start[]={(hsize_t)row,0};
    hsize_t count[]={1, dims[1]};
    ScopedHID fileDataSpaceID(H5Dget_space(id), &H5Sclose);
    H5Sselect_hyperslab(fileDataSpaceID, H5S_SELECT_SET, start, NULL, count, NULL);

    H5Dread(id, memDataTypeID, memDataSpaceID, fileDataSpaceID, H5P_DEFAULT, &data[0]);
    return;
  }

  template<class T>
  void VectorSerie<T>::getColumn(const int column, size_t size, T data[]) {
    hsize_t rows=getRows();
    if(size!=rows)
      throw Exception("dataset dimension does not match");
    hsize_t start[]={0, (hsize_t)column};
    hsize_t count[]={rows, 1};
    ScopedHID fileDataSpaceID(H5Dget_space(id), &H5Sclose);
    H5Sselect_hyperslab(fileDataSpaceID, H5S_SELECT_SET, start, NULL, count, NULL);

    ScopedHID colDataSpaceID(H5Screate_simple(2, count, NULL), &H5Sclose);

    H5Dread(id, memDataTypeID, colDataSpaceID, fileDataSpaceID, H5P_DEFAULT, data);
  }

  template<class T>
  string VectorSerie<T>::getDescription() {
    SimpleAttribute<string> *desc=openChildAttribute<SimpleAttribute<string> >("Description");
    return desc->read();
  }

  template<class T>
  void VectorSerie<T>::setColumnLabel(const vector<string>& columnLabel) {
    if(dims[1]!=columnLabel.size())
      throw Exception("Size of column labe does not match");
    SimpleAttribute<vector<string> > *col=createChildAttribute<SimpleAttribute<vector<string> > >("Column Label")(columnLabel.size());
    col->write(columnLabel);
  }

  template<class T>
  vector<string> VectorSerie<T>::getColumnLabel() {
    SimpleAttribute<vector<string> > *col=openChildAttribute<SimpleAttribute<vector<string> > >("Column Label");
    return col->read();
  }



  // explizit template spezialisations

  template<>
  void VectorSerie<string>::append(const string data[], size_t size) {
    if(size!=dims[1]) throw Exception("dataset dimension does not match");
    dims[0]++;
    H5Dset_extent(id, dims);
  
    hsize_t start[]={dims[0]-1,0};
    hsize_t count[]={1, dims[1]};
    ScopedHID fileDataSpaceID(H5Dget_space(id), &H5Sclose);
    H5Sselect_hyperslab(fileDataSpaceID, H5S_SELECT_SET, start, NULL, count, NULL);
  
    VecStr dummy(dims[1]);
    for(unsigned int i=0; i<dims[1]; i++) {
      dummy[i]=(char*)malloc((data[i].size()+1)*sizeof(char));
      strcpy(dummy[i], data[i].c_str());
    }
    H5Dwrite(id, memDataTypeID, memDataSpaceID, fileDataSpaceID, H5P_DEFAULT, &dummy[0]);
  }
  
  template<>
  void VectorSerie<string>::getRow(const int row, size_t size, string data[]) {
    if(size!=dims[1])
      throw Exception("Size of data does not match");
    int rows=getRows();
    if(row<0 || row>=rows) {
      msg(Warn)<<"HDF5 object with id = "<<id<<":\n"
               <<"Requested row number "<<row<<" is out of range [0.."<<rows<<"[, returning a dummy vector."<<endl;
      for(size_t i=0; i<size; ++i)
         data[i]=string();
      return;
    }

    hsize_t start[]={(hsize_t)row,0};
    hsize_t count[]={1, dims[1]};
    ScopedHID fileDataSpaceID(H5Dget_space(id), &H5Sclose);
    H5Sselect_hyperslab(fileDataSpaceID, H5S_SELECT_SET, start, NULL, count, NULL);
  
    VecStr dummy(dims[1]);
    H5Dread(id, memDataTypeID, memDataSpaceID, fileDataSpaceID, H5P_DEFAULT, &dummy[0]);
    for(unsigned int i=0; i<dims[1]; i++)
      data[i]=dummy[i];
    return;
  }
  
  template<>
  void VectorSerie<string>::getColumn(const int column, size_t size, string data[]) {
    hsize_t rows=getRows();
    if(size!=rows)
      throw Exception("dataset dimension does not match");
    hsize_t start[]={0, (hsize_t)column};
    hsize_t count[]={rows, 1};
    ScopedHID fileDataSpaceID(H5Dget_space(id), &H5Sclose);
    H5Sselect_hyperslab(fileDataSpaceID, H5S_SELECT_SET, start, NULL, count, NULL);
  
    ScopedHID colDataSpaceID(H5Screate_simple(2, count, NULL), &H5Sclose);
  
    VecStr dummy(rows);
    H5Dread(id, memDataTypeID, colDataSpaceID, fileDataSpaceID, H5P_DEFAULT, &dummy[0]);
    for(unsigned int i=0; i<rows; i++)
      data[i]=dummy[i];
  }



  // explizit template instantations

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
  template class VectorSerie<CTYPE>;
# include "hdf5serie/knowntypes.def"
# undef FOREACHKNOWNTYPE

}
