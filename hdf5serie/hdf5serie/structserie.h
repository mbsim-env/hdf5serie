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

#ifndef _HDF5SERIE_STRUCTSERIE_H_
#define _HDF5SERIE_STRUCTSERIE_H_

#include <fmatvec/atom.h>
#include <H5Cpp.h>
#include <vector>
#include <string>
#include <assert.h>
#include <hdf5serie/simpleattribute.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace H5 {

  /** \brief Serie of structs (compount datas).
   *
   * A HDF5 dataset for reading and writing a serie of structs (compount datas).
   * The type of each data of the struct can be of:
   *   - char
   *   - signed char
   *   - unsigned char
   *   - short
   *   - unsigned short
   *   - int
   *   - unsigned int
   *   - long
   *   - unsigned long
   *   - long long
   *   - unsigned long long
   *   - float
   *   - double
   *   - long double
   *   - std::string
   *
   * Or a vector of fixed lenght of this type.
   *
   * The data is stored as a 1D array in the HDF5 file. Each element in the
   * array is one struct.
  */
  template<class S>
  class StructSerie : public fmatvec::Atom, public DataSet {
    private:
      CompType memDataType;
      DataSpace memDataSpace;
      hsize_t dims[1];
      bool firstCall;
      std::vector<int> structOffset;
    public:
      /** \brief A stub constructor
       *
       * Creates a empty object.
      */
      StructSerie();

#ifdef PARSED_BY_DOXYGEN
// Use this code section if doxygen is running to generate documentation.
      /** \brief Register a scalar member of the struct
       *
       * Register a scalar member of type CTYPE of the struct S and set the corrospondending member
       * label to \a name.
       * The type CTYPE can be one of the types defined in the class description.
       *
       * Example:
       * \code
struct S {
  int i;
  std::string s;
  std::vector<double> vd;
}
StructSerie<S> serie;
S s;
serie.registerMember(s, s.i, "Integer Value");
serie.registerMember(s, s.s, "String Value");
serie.registerMember(s, s.vd, 3, "Vector of 3 doubles");
serie.create(parent, "mystructserie");
       * \endcode
      */
      void registerMember(const S& s, const CTYPE& e, const std::string name);

      /** \brief Register a vector member of the struct
       *
       * Register a vector member of lenght \a N of type std::vector<CTYPE> of the struct S
       * and set the corrospondending member label to \a name.
       * Also see registerMember(const S& s, const CTYPE& e, const std::string name);
      */
      void registerMember(const S& s, const std::vector<CTYPE>& e, unsigned int N, const std::string name);
#else
// Use this code section else
#     define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
      void registerMember(const S& s, const CTYPE& e, const std::string name);
#     include "hdf5serie/knowntypes.def"
#     undef FOREACHKNOWNTYPE

#     define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
      void registerMember(const S& s, const std::vector<CTYPE>& e, unsigned int N, const std::string name);
#     include "hdf5serie/knowntypes.def"
#     undef FOREACHKNOWNTYPE
#endif

      /** \brief Creating a dataset
       *
       * Creates a dataset named \a name as a child of position \a parent.
       * By default the dataset is compressed using deflate (gzip) with compression level
       * FileSerie::defaultCompression. Use \a compression to adjuste the compression level [1-9] or 0 to disable compression.
       * NOTE that the object can not be created before the members of the struct
       * are registered using registerMember(const S& s, const CTYPE& e, const std::string name) or
       * registerMember(const S& s, const std::vector<CTYPE>& e, unsigned int N, const std::string name)
      */
      void create(const CommonFG& parent, const std::string& name, int compression=FileSerie::getDefaultCompression(), int chunkSize=FileSerie::getDefaultChunkSize());

      /** \brief Open a dataset
       *
       * Opens the dataset named \a name as a child of position \a parent.
       * NOTE that the object can not be opened before the members of the struct
       * are registered using registerMember(const S& s, const CTYPE& e, const std::string name) or
       * registerMember(const S& s, const std::vector<CTYPE>& e, unsigned int N, const std::string name)
       */
      void open(const CommonFG& parent, const std::string& name);

      /** \brief Sets a description for the dataset
       *
       * The value of \a desc is stored as an string attribute named \p Description in the dataset.
       */
      void setDescription(const std::string& desc);

      /** \brief Return the description for the dataset
       *
       * Returns the value of the string attribute named \p Description of the dataset.
       */
      std::string getDescription();

      /** \brief Append a data struct
       *
       * Appends the data struct \a data at the end of the dataset.
       * The number of elements of the HDF5 array will be incremented by this operation.
       */
      void append(const S& data);

      /** \brief Returns the number of elements in the dataset */
      inline int getRows();

      /** \brief Returns the number of members in the struct */
      inline unsigned int getMembers();

      /** \brief Returns the data struct at element \a row
       *
       * The first element is 0. The last avaliable element ist getRows()-1.
       */
      S getRow(const int row);

      /** \brief Returns the member labels
       *
       * Returns the value of the string vector of all member labels in
       * the dataset.
       */
      std::vector<std::string> getMemberLabel();

      void extend(const hsize_t* size);
  };



  template<class S>
  StructSerie<S>::StructSerie() : DataSet(), memDataType(), firstCall(true) {
    dims[0]=0;
    hsize_t memDims[]={1};
    memDataSpace=DataSpace(1, memDims);
  }

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
  template<class S> \
  void StructSerie<S>::registerMember(const S& s, const CTYPE& e, const std::string name) { \
    int size; \
    if(!firstCall) size=memDataType.getSize(); else size=0; \
    CompType oldMemDataType(memDataType); \
    memDataType=CompType(size+toH5Type(e).getSize()); \
    for(int i=0; i<(firstCall?0:oldMemDataType.getNmembers()); i++) \
      memDataType.insertMember(oldMemDataType.getMemberName(i), oldMemDataType.getMemberOffset(i), oldMemDataType.getMemberDataType(i)); \
    memDataType.insertMember(name, size, toH5Type(e)); \
    structOffset.push_back((char*)&e-(char*)&s); \
    firstCall=false; \
  }
# include "hdf5serie/knowntypes.def"
# undef FOREACHKNOWNTYPE

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
  template<class S> \
  void StructSerie<S>::registerMember(const S& s, const std::vector<CTYPE>& e, unsigned int N, const std::string name) { \
    if(e.size()!=0 && e.size()!=N) throw Exception("wrong dimension"); \
    int size; \
    if(!firstCall) size=memDataType.getSize(); else size=0; \
    CTYPE dummy; \
    hsize_t dims[]={N}; \
    CompType oldMemDataType(memDataType); \
    memDataType=CompType(size+ArrayType(toH5Type(dummy), 1, dims).getSize()); \
    for(int i=0; i<(firstCall?0:oldMemDataType.getNmembers()); i++) \
      memDataType.insertMember(oldMemDataType.getMemberName(i), oldMemDataType.getMemberOffset(i), oldMemDataType.getMemberDataType(i)); \
    memDataType.insertMember(name, size, ArrayType(toH5Type(dummy), 1, dims)); \
    structOffset.push_back((char*)&e-(char*)&s); \
    firstCall=false; \
  }
# include "hdf5serie/knowntypes.def"
# undef FOREACHKNOWNTYPE

  template<class S>
  void StructSerie<S>::create(const CommonFG& parent, const std::string& name, int compression, int chunkSize) {
    if(firstCall) throw Exception("wrong call sequence");
    dims[0]=0;
    hsize_t maxDims[]={H5S_UNLIMITED};
    DataSpace fileDataSpace(1, dims, maxDims);
    DSetCreatPropList prop;
    hsize_t chunkDims[]={static_cast<hsize_t>(chunkSize)};
    prop.setChunk(1, chunkDims);
    if(compression>0) prop.setDeflate(compression);
    DataSet dataSet=parent.createDataSet(name, memDataType, fileDataSpace, prop); // increments the refcount
    setId(dataSet.getId()); // increment the ref count (the ctor of dataset decrements it again)
    fmatvec::Atom::msg(fmatvec::Atom::Debug)<<"HDF5:\n"
      <<"Created object with name = "<<name<<", id = "<<getId()<<" at parent with id = "<<((Group*)&parent)->getId()<<"."<<std::endl;
  }
  
  template<class S>
  void StructSerie<S>::open(const CommonFG& parent, const std::string& name) {
    if(firstCall) throw Exception("wrong call sequence");
    DataSet dataSet=parent.openDataSet(name); // increments the refcount
    setId(dataSet.getId()); // increment the ref count (the ctor of dataset decrements it again)
  
    DataSpace fileDataSpace=getSpace();
    // Check if dataspace and datatype complies with the class
    assert(fileDataSpace.getSimpleExtentNdims()==1);
    hsize_t maxDims[1];
    fileDataSpace.getSimpleExtentDims(dims, maxDims);
    assert(maxDims[0]==H5S_UNLIMITED);
  
    assert(dataSet.getDataType().getClass()==H5T_COMPOUND);
    assert((int)structOffset.size()==getCompType().getNmembers());
    for(unsigned int i=0; i<structOffset.size(); i++) {
      assert(getCompType().getMemberName(i)==memDataType.getMemberName(i));
      assert(getCompType().getMemberOffset(i)==memDataType.getMemberOffset(i));
      assert(getCompType().getMemberDataType(i).getClass()==memDataType.getMemberDataType(i).getClass());
    }
    fmatvec::Atom::msg(fmatvec::Atom::Debug)<<"HDF5:\n"
      <<"Opened object with name = "<<name<<", id = "<<getId()<<" at parent with id = "<<((Group*)&parent)->getId()<<"."<<std::endl;
  }
  
  template<class S>
  void StructSerie<S>::setDescription(const std::string& description) {
    SimpleAttribute<std::string> desc(*this, "Description", description);
  }
  
  template<class S>
  std::string StructSerie<S>::getDescription() {
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
  
  template<class S>
  void StructSerie<S>::append(const S& data) {
    dims[0]++;
    DataSet::extend(dims);
  
    hsize_t start[]={dims[0]-1};
    hsize_t count[]={1};
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, count, start);
  
    std::vector<char> buf(memDataType.getSize());
    std::list<std::vector<char> > charptr;
    for(int i=0; i<memDataType.getNmembers(); i++) {
#     define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
      if(memDataType.getMemberDataType(i)==H5TYPE) \
        *(CTYPE*)(&buf[0]+memDataType.getMemberOffset(i))=*(CTYPE*)((char*)&data+structOffset[i]);
#     include "knownpodtypes.def"
#     undef FOREACHKNOWNTYPE
      if(memDataType.getMemberDataType(i)==StrType(PredType::C_S1, H5T_VARIABLE)) {
        charptr.push_back(std::vector<char>(((std::string*)((char*)&data+structOffset[i]))->size()+1));
        char* str=&(*--charptr.end())[0];
        strcpy(str, ((std::string*)((char*)&data+structOffset[i]))->c_str());
        *(char**)(&buf[0]+memDataType.getMemberOffset(i))=str;
      }
      if(memDataType.getMemberDataType(i).getClass()==H5T_ARRAY) {
        hsize_t dims[1];
        memDataType.getMemberArrayType(i).getArrayDims(dims);
#       define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
        if(memDataType.getMemberDataType(i)==ArrayType(H5TYPE,1,dims)) { \
          std::vector<CTYPE>* vec=(std::vector<CTYPE>*)((char*)&data+structOffset[i]); \
          if(vec->size()!=dims[0]) throw Exception("the dimension does not match"); \
          memcpy(&buf[0]+memDataType.getMemberOffset(i), &((*vec)[0]), dims[0]*sizeof(CTYPE)); \
        }
#       include "knownpodtypes.def"
#       undef FOREACHKNOWNTYPE
        if(memDataType.getMemberDataType(i)==ArrayType(StrType(PredType::C_S1, H5T_VARIABLE),1,dims))
          for(unsigned int j=0; j<dims[0]; j++) {
            std::vector<std::string>* vec=(std::vector<std::string>*)((char*)&data+structOffset[i]);
            if(vec->size()!=dims[0]) throw Exception("the dimension does not match");
            charptr.push_back(std::vector<char>((*vec)[j].size()+1));
            char* str=&(*--charptr.end())[0];
            strcpy(str, (*vec)[j].c_str());
            *(char**)(&buf[0]+memDataType.getMemberOffset(i)+j*sizeof(char*))=str;
          }
      }
    }
    write(&buf[0], memDataType, memDataSpace, fileDataSpace);
  }

  template<class S>
  int StructSerie<S>::getRows() {
    // get current dims from dataspace (maybe another (single-)writer process has increased the number of rows)
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.getSimpleExtentDims(dims);
    // return current value
    return dims[0];
  }
  
  template<class S>
  unsigned int StructSerie<S>::getMembers() {
    return memDataType.getNmembers();
  }
  
  template<class S>
  S StructSerie<S>::getRow(const int row) {
    S data;
    if(row<0 || row>=(int)dims[0]) {
      fmatvec::Atom::msg(fmatvec::Atom::Warn)<<"HDF5 object with id = "<<getId()<<":\n"
                                             <<"Requested struct number is out of range, returning a dummy struct."<<std::endl;
      return data;
    }

    hsize_t start[]={static_cast<hsize_t>(row)};
    hsize_t count[]={1};
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, count, start);
    
    std::vector<char> buf(memDataType.getSize());
    read(&buf[0], memDataType, memDataSpace, fileDataSpace);
    for(int i=0; i<memDataType.getNmembers(); i++) {
#     define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
      if(memDataType.getMemberDataType(i)==H5TYPE) \
        *(CTYPE*)((char*)&data+structOffset[i])=*(CTYPE*)(&buf[0]+memDataType.getMemberOffset(i));
#     include "knownpodtypes.def"
#     undef FOREACHKNOWNTYPE
      if(memDataType.getMemberDataType(i)==StrType(PredType::C_S1, H5T_VARIABLE)) {
        char* str=*(char**)(&buf[0]+memDataType.getMemberOffset(i));
        *((std::string*)((char*)&data+structOffset[i]))=str;
        free(str);
      }
      if(memDataType.getMemberDataType(i).getClass()==H5T_ARRAY) {
        hsize_t dims[1];
        memDataType.getMemberArrayType(i).getArrayDims(dims);
#       define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
        if(memDataType.getMemberDataType(i)==ArrayType(H5TYPE,1,dims)) { \
          std::vector<CTYPE>* vec=(std::vector<CTYPE>*)((char*)&data+structOffset[i]); \
          vec->resize(dims[0]); \
          memcpy(&((*vec)[0]), &buf[0]+memDataType.getMemberOffset(i), dims[0]*sizeof(CTYPE)); \
        }
#       include "knownpodtypes.def"
#       undef FOREACHKNOWNTYPE
        if(memDataType.getMemberDataType(i)==ArrayType(StrType(PredType::C_S1, H5T_VARIABLE),1,dims)) {
          std::vector<std::string>* vec=(std::vector<std::string>*)((char*)&data+structOffset[i]);
          vec->resize(dims[0]);
          for(unsigned int j=0; j<dims[0]; j++) {
            char* str=*(char**)(&buf[0]+memDataType.getMemberOffset(i)+j*sizeof(char*));
            (*vec)[j]=str;
            free(str);
          }
        }
      }
    }
    return data;
  }
  
  template<class S>
  std::vector<std::string> StructSerie<S>::getMemberLabel() {
    std::vector<std::string> ret;
    for(int i=0; i<memDataType.getNmembers(); i++)
      ret.push_back(memDataType.getMemberName(i));
    return ret;
  }
  
  template<class S>
  void StructSerie<S>::extend(const hsize_t* size) {
    assert(1);
  }

}

#endif // _TIMESERIE_H_
