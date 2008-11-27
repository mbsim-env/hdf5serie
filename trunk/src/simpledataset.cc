#include <config.h>
#include <simpledataset.h>
#include <assert.h>

using namespace std;

namespace H5 {

  template<>
  void SimpleDataSet<string>::write(const string& data) {
    const char *buf[1]={data.c_str()};
    DataSet::write(buf, StrType(PredType::C_S1, H5T_VARIABLE), dataspace, dataspace);
  }

  template<>
  string SimpleDataSet<string>::read() {
    string data;
    char *buf[1];
    DataSet::read(buf, StrType(PredType::C_S1, H5T_VARIABLE), dataspace, dataspace);
    data=buf[0];
    free(buf[0]);
    return data;
  }



  template<>
  void SimpleDataSet<vector<string> >::write(const vector<string>& data) {
    hsize_t dims[]={data.size()};
    extend(dims);
    DataSpace dataspace=getSpace();
    char** buf=new char*[dims[0]];
    for(int i=0; i<dims[0]; i++) {
      buf[i]=new char[data[i].size()+1];
      strcpy(buf[i], data[i].c_str());
    }
    DataSet::write(buf, StrType(PredType::C_S1, H5T_VARIABLE), dataspace, dataspace);
    for(int i=0; i<dims[0]; i++) delete[]buf[i];
    delete[]buf;
  }

  template<>
  vector<string> SimpleDataSet<vector<string> >::read() {
    DataSpace dataspace=getSpace();
    hsize_t dims[1];
    dataspace.getSimpleExtentDims(dims);
    char** buf=new char*[dims[0]];
    DataSet::read(buf, StrType(PredType::C_S1, H5T_VARIABLE), dataspace, dataspace);
    vector<string> data;
    for(int i=0; i<dims[0]; i++) {
      data.push_back(buf[i]);
      free(buf[i]);
    }
    delete[]buf;
    return data;
  }



  template<>
  void SimpleDataSet<vector<vector<string> > >::write(const vector<vector<string> >& data) {
    hsize_t dims[]={data.size(),data[0].size()};
    extend(dims);
    DataSpace dataspace=getSpace();
    char** buf=new char*[dims[0]*dims[1]];
    for(int r=0; r<dims[0]; r++)
      for(int c=0; c<dims[1]; c++) {
	assert(data[r].size()==dims[1]);
	buf[r*dims[1]+c]=new char[data[r][c].size()+1];
	strcpy(buf[r*dims[1]+c],data[r][c].c_str());
      }
    DataSet::write(buf, StrType(PredType::C_S1, H5T_VARIABLE), dataspace, dataspace);
    for(int r=0; r<dims[0]; r++)
      for(int c=0; c<dims[1]; c++)
        delete[]buf[r*dims[1]+c];
    delete[]buf;
  }

  template<>
  vector<vector<string> > SimpleDataSet<vector<vector<string> > >::read() {
    DataSpace dataspace=getSpace();
    hsize_t dims[2];
    dataspace.getSimpleExtentDims(dims);
    char** buf=new char*[dims[0]*dims[1]];
    DataSet::read(buf, StrType(PredType::C_S1, H5T_VARIABLE), dataspace, dataspace);
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
