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
#include <hdf5serie/matrixserie.h>
#include <cstdlib>
#include <cstring>

using namespace std;

namespace H5 {

  // template definitions

  template<class T>
  MatrixSerie<T>::MatrixSerie() : DataSet(), memDataSpace() {
    T dummy;
    memDataType=toH5Type(dummy);
    dims[0]=0;
    dims[1]=0;
    dims[2]=0;
  }

  template<class T>
  MatrixSerie<T>::MatrixSerie(const MatrixSerie<T>& dataset) : DataSet(dataset) {
    T dummy;
    memDataType=toH5Type(dummy);
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.getSimpleExtentDims(dims);
    hsize_t memDims[]={1, dims[1], dims[2]};
    memDataSpace=DataSpace(3, memDims);
  }

  template<class T>
  MatrixSerie<T>::MatrixSerie(const CommonFG& parent, const string& name) {
    T dummy;
    memDataType=toH5Type(dummy);
    open(parent, name);
  }

  template<class T>
  MatrixSerie<T>::MatrixSerie(const CommonFG& parent, const string& name, const int rows, const int cols, int compression) {
    T dummy;
    memDataType=toH5Type(dummy);
    create(parent, name, rows, cols, compression);
  }

  template<class T>
  void MatrixSerie<T>::create(const CommonFG& parent, const string& name, const int rows, const int cols, int compression) {
    dims[0]=0;
    dims[1]=rows;
    dims[2]=cols;
    hsize_t maxDims[]={H5S_UNLIMITED, dims[1], dims[2]};
    DataSpace fileDataSpace(3, dims, maxDims);
    DSetCreatPropList prop;
    hsize_t chunkDims[]={hdf5SerieChunkSize, dims[1], dims[2]};
    prop.setChunk(3, chunkDims);
    if(compression>0) prop.setDeflate(compression);
    DataSet dataset=parent.createDataSet(name, memDataType, fileDataSpace, prop);
    p_setId(dataset.getId());
    incRefCount();

    hsize_t memDims[]={1, dims[1], dims[2]};
    memDataSpace=DataSpace(3, memDims);
  }

  template<class T>
  void MatrixSerie<T>::open(const CommonFG& parent, const string& name) {
    DataSet dataset=parent.openDataSet(name);
    p_setId(dataset.getId());
    incRefCount();

    DataSpace fileDataSpace=getSpace();
    // Check if fileDataSpace and memDataType complies with the class
    assert(fileDataSpace.getSimpleExtentNdims()==3);
    hsize_t maxDims[3];
    fileDataSpace.getSimpleExtentDims(dims, maxDims);
    assert(maxDims[0]==H5S_UNLIMITED);
    assert(getDataType().getClass()==memDataType.getClass());

    hsize_t memDims[]={1, dims[1], dims[2]};
    memDataSpace=DataSpace(3, memDims);
  }

  template<class T>
  void MatrixSerie<T>::setDescription(const string& description) {
    SimpleAttribute<string> desc(*this, "Description", description);
  }

  template<class T>
  void MatrixSerie<T>::append(const vector<vector<T> > &matrix) {
    assert(matrix.size()==dims[1]);
    dims[0]++;
    DataSet::extend(dims);

    hsize_t start[]={dims[0]-1,0,0};
    hsize_t count[]={1, dims[1], dims[2]};
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, count, start);

    T* data=new T[dims[1]*dims[2]];
    for(unsigned int r=0; r<matrix.size(); r++) {
      assert(matrix[r].size()==dims[2]);
      memcpy(&data[r*dims[2]],&matrix[r][0],sizeof(double)*dims[2]);
    }
    write(data, memDataType, memDataSpace, fileDataSpace);
    delete[]data;
  }

  template<class T>
  vector<vector<T> > MatrixSerie<T>::getMatrix(const int number) {
    hsize_t start[]={number,0,0};
    hsize_t count[]={1, dims[1],dims[2]};
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, count, start);

    T* data=new T[dims[1]*dims[2]];
    read(data, memDataType, memDataSpace, fileDataSpace);
    vector<vector<T> > matrix;
    matrix.resize(dims[1]);
    for(unsigned int r=0; r<matrix.size(); r++) {
      matrix[r].resize(dims[2]);
      memcpy(&matrix[r][0],&data[r*dims[2]],sizeof(double)*dims[2]);
    }
    delete[]data;
    return matrix;
  }

  template<class T>
  string MatrixSerie<T>::getDescription() {
    // save and disable c error printing
    H5E_auto2_t func;
    void* client_data;
    Exception::getAutoPrint(func, &client_data);
    Exception::dontPrint();
    string ret;
    // catch error if Attribute is not found
    try {
      ret=SimpleAttribute<string>::getData(*this, "Description");
    }
    catch(AttributeIException e) {
      ret=string();
    }
    // restore c error printing
    Exception::setAutoPrint(func, client_data);
    return ret;
  }

  template<class T>
  void MatrixSerie<T>::extend(const hsize_t* size) {
    assert(1);
  }



  // explizit template instantations

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
  template class MatrixSerie<CTYPE>;
# include "hdf5serie/knowntypes.def"
# undef FOREACHKNOWNTYPE

}
