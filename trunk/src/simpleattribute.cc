#include <config.h>
#include <simpleattribute.h>
#include <assert.h>

using namespace std;

namespace H5 {

  template<>
  void SimpleAttribute<string>::write(const string& data) {
    const char *buf[1]={data.c_str()};
    Attribute::write(StrType(PredType::C_S1, H5T_VARIABLE), buf);
  }

  template<>
  string SimpleAttribute<string>::read() {
    string data;
    char *buf[1];
    Attribute::read(StrType(PredType::C_S1, H5T_VARIABLE), buf);
    data=buf[0];
    free(buf[0]);
    return data;
  }



  template<>
  void SimpleAttribute<vector<string> >::write(const vector<string>& data) {
    DataSpace dataspace=getSpace();
    hsize_t dims[1];
    dataspace.getSimpleExtentDims(dims);
    assert(data.size()==dims[0]);
    char** buf=new char*[dims[0]];
    for(int i=0; i<dims[0]; i++) {
      buf[i]=new char[data[i].size()+1];
      strcpy(buf[i], data[i].c_str());
    }
    Attribute::write(StrType(PredType::C_S1, H5T_VARIABLE), buf);
    for(int i=0; i<dims[0]; i++) delete[]buf[i];
    delete[]buf;
  }

  template<>
  vector<string> SimpleAttribute<vector<string> >::read() {
    DataSpace dataspace=getSpace();
    hsize_t dims[1];
    dataspace.getSimpleExtentDims(dims);
    char** buf=new char*[dims[0]];
    Attribute::read(StrType(PredType::C_S1, H5T_VARIABLE), buf);
    vector<string> data;
    for(int i=0; i<dims[0]; i++) {
      data.push_back(buf[i]);
      free(buf[i]);
    }
    delete[]buf;
    return data;
  }



  template<>
  void SimpleAttribute<vector<vector<string> > >::write(const vector<vector<string> >& data) {
    DataSpace dataspace=getSpace();
    hsize_t dims[2];
    dataspace.getSimpleExtentDims(dims);
    assert(data.size()==dims[0]);
    char** buf=new char*[dims[0]*dims[1]];
    for(int r=0; r<dims[0]; r++)
      for(int c=0; c<dims[1]; c++) {
	assert(data[r].size()==dims[1]);
	buf[r*dims[1]+c]=new char[data[r][c].size()+1];
	strcpy(buf[r*dims[1]+c],data[r][c].c_str());
      }
    Attribute::write(StrType(PredType::C_S1, H5T_VARIABLE), buf);
    for(int r=0; r<dims[0]; r++)
      for(int c=0; c<dims[1]; c++)
        delete[]buf[r*dims[1]+c];
    delete[]buf;
  }

  template<>
  vector<vector<string> > SimpleAttribute<vector<vector<string> > >::read() {
    DataSpace dataspace=getSpace();
    hsize_t dims[2];
    dataspace.getSimpleExtentDims(dims);
    char** buf=new char*[dims[0]*dims[1]];
    Attribute::read(StrType(PredType::C_S1, H5T_VARIABLE), buf);
    vector<vector<string> > data(dims[0]);
    for(int r=0; r<dims[0]; r++)
      for(int c=0; c<dims[1]; c++) {
        data[r].push_back(buf[r*dims[1]+c]);
        free(buf[r*dims[1]+c]);
      }
    delete[]buf;
    return data;
  }

}
