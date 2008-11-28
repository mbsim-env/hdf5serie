#include <config.h>
#include <serie1d.h>

using namespace std;
using namespace H5;

template<>
void Serie1D<char*>::create(const CommonFG& parent, const string& name, const vector<MemberNameType>& nameType) {
  dims[0]=0;
  hsize_t maxDims[]={H5S_UNLIMITED};
  DataSpace dataspace(1, dims, maxDims);
  DSetCreatPropList prop;
  hsize_t chunkDims[]={1000};
  prop.setChunk(1, chunkDims);

  size_t size=0;
  for(int i=0; i<nameType.size(); i++)
    size+=strToH5Type(nameType[i].type).getSize();
  CompType comptype(size);
  size=0;
  for(int i=0; i<nameType.size(); i++) {
    comptype.insertMember(nameType[i].name, size, strToH5Type(nameType[i].type));
    size+=strToH5Type(nameType[i].type).getSize();
  }
  datatype=comptype;

  DataSet dataset=parent.createDataSet(name, datatype, dataspace, prop);
  p_setId(dataset.getId());
  incRefCount();
}

template<>
void Serie1D<char*>::open(const CommonFG& parent, const string& name) {
  DataSet dataset=parent.openDataSet(name);
  p_setId(dataset.getId());
  incRefCount();

  DataSpace dataspace=getSpace();
  // Check if dataspace and datatype complies with the class
  assert(dataspace.getSimpleExtentNdims()==1);
  hsize_t maxDims[1];
  dataspace.getSimpleExtentDims(dims, maxDims);
  assert(maxDims[0]==H5S_UNLIMITED);

  assert(dataset.getDataType().getClass()==H5T_COMPOUND);
  datatype=getCompType();
}

template<>
void Serie1D<char*>::append(char*const& data) {
  dims[0]++;
  DataSet::extend(dims);

  hsize_t start[]={dims[0]-1};
  hsize_t count[]={1};
  DataSpace dataspace=getSpace();
  dataspace.selectHyperslab(H5S_SELECT_SET, count, start);

  std::vector<char*> delMe;
  int offset=0, offsetCStr=0;
  char buf[10000];
  for(int i=0; i<datatype.getNmembers(); i++) {
    int size=datatype.getMemberDataType(i).getSize();
    if(!(datatype.getMemberDataType(i)==strToH5Type("String"))) {
      memcpy(buf+offsetCStr, ((char*)data)+offset, size);
      offsetCStr+=size;
    }
    else {
      *(char**)(buf+offsetCStr)=new char[datatype.getMemberDataType(i).getSize()+1];
      delMe.push_back(*(char**)(buf+offsetCStr));
      strcpy(*(char**)(buf+offsetCStr), (*(std::string*)(((char*)data)+offset)).c_str());
      offsetCStr+=sizeof(char*);
    }
    offset+=size;
  }
  write(buf, datatype, memdataspace, dataspace);
  for(int i=0; i<delMe.size(); i++)
    delete[]delMe[i];
}

template<>
char* Serie1D<char*>::getRow(const int row) {
  hsize_t start[]={row};
  hsize_t count[]={1};
  DataSpace dataspace=getSpace();
  dataspace.selectHyperslab(H5S_SELECT_SET, count, start);

  char* data=new char[datatype.getSize()];
  char* buf=new char[datatype.getSize()+datatype.getNmembers()*sizeof(char*)]; // this is always enough memory
  read(buf, datatype, memdataspace, dataspace);
  int offset=0, offsetCStr=0;
  for(int i=0; i<datatype.getNmembers(); i++) {
    int size=datatype.getMemberDataType(i).getSize();
    if(!(datatype.getMemberDataType(i)==strToH5Type("String"))) {
      memcpy(((char*)data)+offset, buf+offsetCStr, size);
      offsetCStr+=size;
    }
    else {
      (*(std::string*)(((char*)data)+offset))=*(char**)(buf+offsetCStr);
      free(*(char**)(buf+offsetCStr));
      offsetCStr+=sizeof(char*);
    }
    offset+=size;
  }
  delete[]buf;
  return data;
}
