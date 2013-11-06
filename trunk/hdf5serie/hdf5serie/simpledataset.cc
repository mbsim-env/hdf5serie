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
#include <hdf5serie/simpledataset.h>
#include <assert.h>
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include "utils.h"

using namespace std;

namespace H5 {

  // template definitions

  template<class T>
  SimpleDataSet<T>::SimpleDataSet() : DataSet(), dataSpace() {
    T dummy;
    memDataType=toH5Type(dummy);
  }

  template<class T>
  SimpleDataSet<T>::SimpleDataSet(const SimpleDataSet<T>& dataset) : DataSet(dataset), dataSpace(dataset.getSpace()) {
    T dummy;
    memDataType=toH5Type(dummy);
  }

  template<class T>
  SimpleDataSet<T>::SimpleDataSet(const CommonFG& parent, const string& name, bool create_) {
    T dummy;
    memDataType=toH5Type(dummy);
    if(create_)
      create(parent, name);
    else
      open(parent, name);
  }

  template<class T>
  SimpleDataSet<T>::SimpleDataSet(const CommonFG& parent, const string& name, const T& data) {
    T dummy;
    memDataType=toH5Type(dummy);
    write(parent, name, data);
  }

  template<class T>
  void SimpleDataSet<T>::create(const CommonFG& parent, const string& name) {
    dataSpace=DataSpace(0, NULL);
    DataSet dataset=parent.createDataSet(name, memDataType, dataSpace);
    p_setId(dataset.getId()); // increments the ref count
  }

  template<class T>
  void SimpleDataSet<T>::open(const CommonFG& parent, const string& name) {
    DataSet dataset=parent.openDataSet(name);
    dataSpace=dataset.getSpace();
    p_setId(dataset.getId()); // increments the ref count
    // Check if dataSpace and memDataType complies with the class
    assert(dataSpace.getSimpleExtentNdims()==0);
    assert(getDataType().getClass()==memDataType.getClass());
  }

  template<class T>
  void SimpleDataSet<T>::write(const T& data) {
    DataSet::write(&data, memDataType, dataSpace, dataSpace);
  }
  template<>
  void SimpleDataSet<string>::write(const string& data);

  template<class T>
  T SimpleDataSet<T>::read() {
    T data;
    DataSet::read(&data, memDataType, dataSpace, dataSpace);
    return data;
  }
  template<>
  string SimpleDataSet<string>::read();

  template<class T>
  void SimpleDataSet<T>::write(const CommonFG& parent, const string& name, const T& data) {
    create(parent, name);
    write(data);
  }

  template<class T>
  T SimpleDataSet<T>::read(const CommonFG& parent, const string& name)  {
    open(parent, name);
    return read();
  }

  template<class T>
  void SimpleDataSet<T>::setDescription(const string& description) {
    SimpleAttribute<string> attr(*this, "Description", description);
  }

  template<class T>
  string SimpleDataSet<T>::getDescription() {
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
  T SimpleDataSet<T>::getData(const CommonFG& parent, const string& name) {
    SimpleDataSet<T> dataset;
    return dataset.read(parent, name);
  }



  template<class T>
  SimpleDataSet<vector<T> >::SimpleDataSet() : DataSet() {
    T dummy;
    memDataType=toH5Type(dummy);
  }

  template<class T>
  SimpleDataSet<vector<T> >::SimpleDataSet(const SimpleDataSet<vector<T> >& dataset) : DataSet(dataset) {
    T dummy;
    memDataType=toH5Type(dummy);
  }

  template<class T>
  SimpleDataSet<vector<T> >::SimpleDataSet(const CommonFG& parent, const string& name, bool create_) : DataSet() {
    T dummy;
    memDataType=toH5Type(dummy);
    if(create_)
      create(parent, name);
    else
      open(parent, name);
  }

  template<class T>
  SimpleDataSet<vector<T> >::SimpleDataSet(const CommonFG& parent, const string& name, const vector<T>& data) : DataSet() {
    T dummy;
    memDataType=toH5Type(dummy);
    write(parent, name, data);
  }

  template<class T>
  void SimpleDataSet<vector<T> >::create(const CommonFG& parent, const string& name) {
    hsize_t dims[]={0};
    hsize_t maxDims[]={H5S_UNLIMITED};
    DataSpace dataSpace(1, dims, maxDims);
    DSetCreatPropList prop;
    hsize_t chunkDims[]={1};
    prop.setChunk(1, chunkDims);
    DataSet dataset=parent.createDataSet(name, memDataType, dataSpace, prop);
    p_setId(dataset.getId()); // increments the ref count
  }

  template<class T>
  void SimpleDataSet<vector<T> >::open(const CommonFG& parent, const string& name) {
    DataSet dataset=parent.openDataSet(name);
    p_setId(dataset.getId()); // increments the ref count
    // Check if dataSpace and memDataType complies with the class
    DataSpace dataSpace=getSpace();
    assert(dataSpace.getSimpleExtentNdims()==1);
    hsize_t dims[1], maxDims[1];
    dataSpace.getSimpleExtentDims(dims, maxDims);
    assert(maxDims[0]==H5S_UNLIMITED);
    assert(getDataType().getClass()==memDataType.getClass());
  }

  template<class T>
  void SimpleDataSet<vector<T> >::write(const vector<T>& data) {
    hsize_t dims[]={data.size()};
    extend(dims);
    DataSpace dataSpace=getSpace();
    DataSet::write(&data[0], memDataType, dataSpace, dataSpace);
  }
  template<>
  void SimpleDataSet<vector<string> >::write(const vector<string>& data);

  template<class T>
  vector<T> SimpleDataSet<vector<T> >::read() {
    DataSpace dataSpace=getSpace();
    hsize_t dims[1];
    dataSpace.getSimpleExtentDims(dims);
    vector<T> data(dims[0]);
    DataSet::read(&data[0], memDataType, dataSpace, dataSpace);
    return data;
  }
  template<>
  vector<string> SimpleDataSet<vector<string> >::read();

  template<class T>
  void SimpleDataSet<vector<T> >::write(const CommonFG& parent, const string& name, const vector<T>& data) {
    create(parent, name);
    write(data);
  }

  template<class T>
  vector<T> SimpleDataSet<vector<T> >::read(const CommonFG& parent, const string& name) {
    open(parent, name);
    return read();
  }

  template<class T>
  void SimpleDataSet<vector<T> >::setDescription(const string& description) {
    SimpleAttribute<string> attr(*this, "Description", description);
  }

  template<class T>
  string SimpleDataSet<vector<T> >::getDescription() {
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
  vector<T> SimpleDataSet<vector<T> >::getData(const CommonFG& parent, const string& name) {
    SimpleDataSet<vector<T> > dataset;
    return dataset.read(parent, name);
  }



  template<class T>
  SimpleDataSet<vector<vector<T> > >::SimpleDataSet() : DataSet() {
    T dummy;
    memDataType=toH5Type(dummy);
  }

  template<class T>
  SimpleDataSet<vector<vector<T> > >::SimpleDataSet(const SimpleDataSet<vector<vector<T> > >& dataset) : DataSet(dataset) {
    T dummy;
    memDataType=toH5Type(dummy);
  }

  template<class T>
  SimpleDataSet<vector<vector<T> > >::SimpleDataSet(const CommonFG& parent, const string& name, bool create_) : DataSet() {
    T dummy;
    memDataType=toH5Type(dummy);
    if(create_)
      create(parent, name);
    else
      open(parent, name);
  }

  template<class T>
  SimpleDataSet<vector<vector<T> > >::SimpleDataSet(const CommonFG& parent, const string& name, const vector<vector<T> >& data) : DataSet() {
    T dummy;
    memDataType=toH5Type(dummy);
    write(parent, name, data);
  }

  template<class T>
  void SimpleDataSet<vector<vector<T> > >::create(const CommonFG& parent, const string& name) {
    hsize_t dims[]={0,0};
    hsize_t maxDims[]={H5S_UNLIMITED,H5S_UNLIMITED};
    DataSpace dataSpace(2, dims, maxDims);
    DSetCreatPropList prop;
    hsize_t chunkDims[]={1,1};
    prop.setChunk(2, chunkDims);
    DataSet dataset=parent.createDataSet(name, memDataType, dataSpace, prop);
    p_setId(dataset.getId()); // increments the ref count
  }

  template<class T>
  void SimpleDataSet<vector<vector<T> > >::open(const CommonFG& parent, const string& name) {
    DataSet dataset=parent.openDataSet(name);
    p_setId(dataset.getId()); // increments the ref count
    // Check if dataSpace and memDataType complies with the class
    DataSpace dataSpace=getSpace();
    assert(dataSpace.getSimpleExtentNdims()==2);
    hsize_t dims[2], maxDims[2];
    dataSpace.getSimpleExtentDims(dims, maxDims);
    assert(maxDims[0]==H5S_UNLIMITED);
    assert(maxDims[1]==H5S_UNLIMITED);
    assert(getDataType().getClass()==memDataType.getClass());
  }

  template<class T>
  void SimpleDataSet<vector<vector<T> > >::write(const vector<vector<T> >& data) {
    hsize_t dims[]={data.size(),data[0].size()};
    extend(dims);
    DataSpace dataSpace=getSpace();
    vector<T> buf(dims[0]*dims[1]);
    T dummy;
    for(unsigned int r=0; r<dims[0]; r++) {
      if(data[r].size()!=dims[1]) throw runtime_error("all row of the input must have the same length");
      memcpy(&buf[r*dims[1]], &data[r][0], sizeof(dummy)*dims[1]);
    }
    DataSet::write(&buf[0], memDataType, dataSpace, dataSpace);
  }
  template<>
  void SimpleDataSet<vector<vector<string> > >::write(const vector<vector<string> >& data);

  template<class T>
  vector<vector<T> > SimpleDataSet<vector<vector<T> > >::read() {
    DataSpace dataSpace=getSpace();
    hsize_t dims[2];
    dataSpace.getSimpleExtentDims(dims);
    vector<vector<T> > data(dims[0]);
    vector<T> buf(dims[0]*dims[1]);
    DataSet::read(&buf[0], memDataType, dataSpace, dataSpace);
    for(unsigned int r=0; r<dims[0]; r++) {
      data[r].resize(dims[1]);
      memcpy(&data[r][0], &buf[r*dims[1]], sizeof(T)*dims[1]);
    }
    return data;
  }
  template<>
  vector<vector<string> > SimpleDataSet<vector<vector<string> > >::read();

  template<class T>
  void SimpleDataSet<vector<vector<T> > >::write(const CommonFG& parent, const string& name, const vector<vector<T> >& data) {
    create(parent, name);
    write(data);
  }

  template<class T>
  vector<vector<T> > SimpleDataSet<vector<vector<T> > >::read(const CommonFG& parent, const string& name) {
    open(parent, name);
    return read();
  }

  template<class T>
  void SimpleDataSet<vector<vector<T> > >::setDescription(const string& description) {
    SimpleAttribute<string> attr(*this, "Description", description);
  }

  template<class T>
  string SimpleDataSet<vector<vector<T> > >::getDescription() {
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
  vector<vector<T> > SimpleDataSet<vector<vector<T> > >::getData(const CommonFG& parent, const string& name) {
    SimpleDataSet<vector<vector<T> > > dataset;
    return dataset.read(parent, name);
  }



  // explizit template spezialisations

  template<>
  void SimpleDataSet<string>::write(const string& data) {
    const char *buf[1]={data.c_str()};
    DataSet::write(buf, StrType(PredType::C_S1, H5T_VARIABLE), dataSpace, dataSpace);
  }

  template<>
  string SimpleDataSet<string>::read() {
    string data;
    VecStr buf(1);
    DataSet::read(&buf[0], StrType(PredType::C_S1, H5T_VARIABLE), dataSpace, dataSpace);
    data=buf[0];
    return data;
  }



  template<>
  void SimpleDataSet<vector<string> >::write(const vector<string>& data) {
    hsize_t dims[]={data.size()};
    extend(dims);
    DataSpace dataSpace=getSpace();
    VecStr buf(dims[0]);
    for(unsigned int i=0; i<dims[0]; i++) {
      buf[i]=(char*)malloc((data[i].size()+1)*sizeof(char));
      strcpy(buf[i], data[i].c_str());
    }
    DataSet::write(&buf[0], StrType(PredType::C_S1, H5T_VARIABLE), dataSpace, dataSpace);
  }

  template<>
  vector<string> SimpleDataSet<vector<string> >::read() {
    DataSpace dataSpace=getSpace();
    hsize_t dims[1];
    dataSpace.getSimpleExtentDims(dims);
    VecStr buf(dims[0]);
    DataSet::read(&buf[0], StrType(PredType::C_S1, H5T_VARIABLE), dataSpace, dataSpace);
    vector<string> data;
    for(unsigned int i=0; i<dims[0]; i++) {
      data.push_back(buf[i]);
    }
    return data;
  }



  template<>
  void SimpleDataSet<vector<vector<string> > >::write(const vector<vector<string> >& data) {
    hsize_t dims[]={data.size(),data[0].size()};
    extend(dims);
    DataSpace dataSpace=getSpace();
    VecStr buf(dims[0]*dims[1]);
    for(unsigned int r=0; r<dims[0]; r++)
      for(unsigned int c=0; c<dims[1]; c++) {
	if(data[r].size()!=dims[1]) throw runtime_error("all rows of the input must have the same size");
	buf[r*dims[1]+c]=(char*)malloc((data[r][c].size()+1)*sizeof(char));
	strcpy(buf[r*dims[1]+c],data[r][c].c_str());
      }
    DataSet::write(&buf[0], StrType(PredType::C_S1, H5T_VARIABLE), dataSpace, dataSpace);
  }

  template<>
  vector<vector<string> > SimpleDataSet<vector<vector<string> > >::read() {
    DataSpace dataSpace=getSpace();
    hsize_t dims[2];
    dataSpace.getSimpleExtentDims(dims);
    VecStr buf(dims[0]*dims[1]);
    DataSet::read(&buf[0], StrType(PredType::C_S1, H5T_VARIABLE), dataSpace, dataSpace);
    vector<vector<string> > data(dims[0]);
    for(unsigned int r=0; r<dims[0]; r++)
      for(unsigned int c=0; c<dims[1]; c++) {
        data[r].push_back(buf[r*dims[1]+c]);
      }
    return data;
  }



  // explizit template instantations

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
  template class SimpleDataSet<CTYPE>;
# include "knowntypes.def"
# undef FOREACHKNOWNTYPE

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
  template class SimpleDataSet<vector<CTYPE> >;
# include "knowntypes.def"
# undef FOREACHKNOWNTYPE

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
  template class SimpleDataSet<vector<vector<CTYPE> > >;
# include "knowntypes.def"
# undef FOREACHKNOWNTYPE

}
