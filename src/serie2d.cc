#include <config.h>
#include <serie2d.h>

using namespace std;
using namespace H5;

template<>
void Serie2D<string>::append(const vector<string> &data) {
  assert(data.size()==dims[1]);
  dims[0]++;
  DataSet::extend(dims);

  hsize_t start[]={dims[0]-1,0};
  hsize_t count[]={1, dims[1]};
  DataSpace dataspace=getSpace();
  dataspace.selectHyperslab(H5S_SELECT_SET, count, start);

  char** dummy=new char*[dims[1]];
  for(int i=0; i<dims[1]; i++) {
    dummy[i]=new char[data[i].size()+1];
    strcpy(dummy[i], data[i].c_str());
  }
  write(dummy, datatype, memdataspace, dataspace);
  for(int i=0; i<dims[1]; i++) delete[]dummy[i];
  delete[]dummy;
}

template<>
vector<string> Serie2D<string>::getRow(const int row) {
  hsize_t start[]={row,0};
  hsize_t count[]={1, dims[1]};
  DataSpace dataspace=getSpace();
  dataspace.selectHyperslab(H5S_SELECT_SET, count, start);

  char** dummy=new char*[dims[1]];
  read(dummy, datatype, memdataspace, dataspace);
  vector<string> data(dims[1]);
  for(int i=0; i<dims[1]; i++) {
    data[i]=dummy[i];
    free(dummy[i]);
  }
  delete[]dummy;
  return data;
}

template<>
vector<string> Serie2D<string>::getColumn(const int column) {
  hsize_t rows=getRows();
  hsize_t start[]={0, column};
  hsize_t count[]={rows, 1};
  DataSpace dataspace=getSpace();
  dataspace.selectHyperslab(H5S_SELECT_SET, count, start);

  DataSpace coldataspace(2, count);

  char** dummy=new char*[rows];
  read(dummy, datatype, coldataspace, dataspace);
  vector<string> data(rows);
  for(int i=0; i<rows; i++) {
    data[i]=dummy[i];
    free(dummy[i]);
  }
  delete[]dummy;
  return data;
}
