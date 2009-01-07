#ifndef _STRUCTSERIE_H_
#define _STRUCTSERIE_H_

#include <H5Cpp.h>
#include <vector>
#include <string>
#include <assert.h>
#include <simpleattribute.h>

namespace H5 {

  template<class S>
  class StructSerie : public DataSet {
    private:
      CompType memDataType;
      DataSpace memDataSpace;
      hsize_t dims[1];
      bool firstCall;
      std::vector<int> structOffset;
    public:
      StructSerie();

#     define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
      void insertMember(const S& s, const CTYPE& e, const std::string name);
#     include "knowntypes.def"
#     undef FOREACHKNOWNTYPE

#     define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
      void insertMember(const S& s, const std::vector<CTYPE>& e, int N, const std::string name);
#     include "knowntypes.def"
#     undef FOREACHKNOWNTYPE

      void create(const CommonFG& parent, const std::string& name);
      void open(const CommonFG& parent, const std::string& name);
      void setDescription(const std::string& desc);
      std::string getDescription();
      void append(const S& data);
      inline int getRows();
      inline int getMembers();
      S getRow(const int row);
      std::vector<std::string> getMemberLabel();

      void extend(const hsize_t* size);
  };



  template<class S>
  StructSerie<S>::StructSerie() : DataSet(), memDataType(), firstCall(true) {
    dims[0]=0;
    hsize_t memDims[]={1};
    memDataSpace=DataSpace(1, memDims);
  }

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
  template<class S> \
  void StructSerie<S>::insertMember(const S& s, const CTYPE& e, const std::string name) { \
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
# include "knowntypes.def"
# undef FOREACHKNOWNTYPE

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
  template<class S> \
  void StructSerie<S>::insertMember(const S& s, const std::vector<CTYPE>& e, int N, const std::string name) { \
    assert(e.size()==0 || e.size()==N); \
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
# include "knowntypes.def"
# undef FOREACHKNOWNTYPE

  template<class S>
  void StructSerie<S>::create(const CommonFG& parent, const std::string& name) {
    assert(!firstCall);
    dims[0]=0;
    hsize_t maxDims[]={H5S_UNLIMITED};
    DataSpace fileDataSpace(1, dims, maxDims);
    DSetCreatPropList prop;
    hsize_t chunkDims[]={1000};
    prop.setChunk(1, chunkDims);
  
    DataSet dataSet=parent.createDataSet(name, memDataType, fileDataSpace, prop);
    p_setId(dataSet.getId());
    incRefCount();
  }
  
  template<class S>
  void StructSerie<S>::open(const CommonFG& parent, const std::string& name) {
    DataSet dataSet=parent.openDataSet(name);
    p_setId(dataSet.getId());
    incRefCount();
  
    DataSpace fileDataSpace=getSpace();
    // Check if dataspace and datatype complies with the class
    assert(fileDataSpace.getSimpleExtentNdims()==1);
    hsize_t maxDims[1];
    fileDataSpace.getSimpleExtentDims(dims, maxDims);
    assert(maxDims[0]==H5S_UNLIMITED);
  
    assert(dataSet.getDataType().getClass()==H5T_COMPOUND);
    assert(structOffset.size()==getCompType().getNmembers());
    for(int i=0; i<structOffset.size(); i++) {
      assert(getCompType().getMemberName(i)==memDataType.getMemberName(i));
      assert(getCompType().getMemberOffset(i)==memDataType.getMemberOffset(i));
      assert(getCompType().getMemberDataType(i).getClass()==memDataType.getMemberDataType(i).getClass());
    }
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
  
    char* buf=new char[memDataType.getSize()];
    std::vector<char*> charptr;
    for(int i=0; i<memDataType.getNmembers(); i++) {
#     define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
      if(memDataType.getMemberDataType(i)==H5TYPE) \
        *(CTYPE*)(buf+memDataType.getMemberOffset(i))=*(CTYPE*)((char*)&data+structOffset[i]);
#     include "knownpodtypes.def"
#     undef FOREACHKNOWNTYPE
      if(memDataType.getMemberDataType(i)==StrType(PredType::C_S1, H5T_VARIABLE)) {
        char* str=new char[((std::string*)((char*)&data+structOffset[i]))->size()+1];
        charptr.push_back(str);
        strcpy(str, ((std::string*)((char*)&data+structOffset[i]))->c_str());
        *(char**)(buf+memDataType.getMemberOffset(i))=str;
      }
      if(memDataType.getMemberDataType(i).getClass()==H5T_ARRAY) {
        hsize_t dims[1];
        memDataType.getMemberArrayType(i).getArrayDims(dims);
#       define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
        if(memDataType.getMemberDataType(i)==ArrayType(H5TYPE,1,dims)) { \
          std::vector<CTYPE>* vec=(std::vector<CTYPE>*)((char*)&data+structOffset[i]); \
          assert(vec->size()==dims[0]); \
          memcpy(buf+memDataType.getMemberOffset(i), &((*vec)[0]), dims[0]*sizeof(CTYPE)); \
        }
#       include "knownpodtypes.def"
#       undef FOREACHKNOWNTYPE
        if(memDataType.getMemberDataType(i)==ArrayType(StrType(PredType::C_S1, H5T_VARIABLE),1,dims))
          for(int j=0; j<dims[0]; j++) {
            std::vector<std::string>* vec=(std::vector<std::string>*)((char*)&data+structOffset[i]);
            assert(vec->size()==dims[0]);
            char* str=new char[(*vec)[j].size()+1];
            charptr.push_back(str);
            strcpy(str, (*vec)[j].c_str());
            *(char**)(buf+memDataType.getMemberOffset(i)+j*sizeof(char*))=str;
          }
      }
    }
    write(buf, memDataType, memDataSpace, fileDataSpace);
    for(int i=0; i<charptr.size(); i++)
      delete[]charptr[i];
    delete[]buf;
  }

  template<class S>
  int StructSerie<S>::getRows() {
    //////////
    return dims[0];
    //////////
    //DataSpace dataspace=getSpace();
    //dataspace.getSimpleExtentDims(dims);
    //return dims[0];
    //////////
  }
  
  template<class S>
  int StructSerie<S>::getMembers() {
    return memDataType.getNmembers();
  }
  
  template<class S>
  S StructSerie<S>::getRow(const int row) {
    hsize_t start[]={row};
    hsize_t count[]={1};
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, count, start);
    
    char* buf=new char[memDataType.getSize()];
    read(buf, memDataType, memDataSpace, fileDataSpace);
    S data;
    for(int i=0; i<memDataType.getNmembers(); i++) {
#     define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
      if(memDataType.getMemberDataType(i)==H5TYPE) \
        *(CTYPE*)((char*)&data+structOffset[i])=*(CTYPE*)(buf+memDataType.getMemberOffset(i));
#     include "knownpodtypes.def"
#     undef FOREACHKNOWNTYPE
      if(memDataType.getMemberDataType(i)==StrType(PredType::C_S1, H5T_VARIABLE)) {
        char* str=*(char**)(buf+memDataType.getMemberOffset(i));
        *((std::string*)((char*)&data+structOffset[i]))=str;
        free(str);
      }
      if(memDataType.getMemberDataType(i).getClass()==H5T_ARRAY) {
        hsize_t dims[1];
        memDataType.getMemberArrayType(i).getArrayDims(dims);
#       define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
        if(memDataType.getMemberDataType(i)==ArrayType(H5TYPE,1,dims)) { \
          std::vector<CTYPE>* vec=(std::vector<CTYPE>*)((char*)&data+structOffset[i]); \
          vec->resize(dims[0]); \
          memcpy(&((*vec)[0]), buf+memDataType.getMemberOffset(i), dims[0]*sizeof(CTYPE)); \
        }
#       include "knownpodtypes.def"
#       undef FOREACHKNOWNTYPE
        if(memDataType.getMemberDataType(i)==ArrayType(StrType(PredType::C_S1, H5T_VARIABLE),1,dims)) {
          std::vector<std::string>* vec=(std::vector<std::string>*)((char*)&data+structOffset[i]);
          vec->resize(dims[0]);
          for(int j=0; j<dims[0]; j++) {
            char* str=*(char**)(buf+memDataType.getMemberOffset(i)+j*sizeof(char*));
            (*vec)[j]=str;
            free(str);
          }
        }
      }
    }
    delete[]buf;
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
