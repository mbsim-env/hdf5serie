#include <config.h>
#include <serie2d.h>
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace H5;
using namespace std;

string comment="#";
string quote="\"";
string nan="nan";

int string2int(string str) {
  stringstream sstr(str);
  int i;
  sstr>>i;
  return i;
}

template<class T>
void dumpHeader(Serie2D<T>& ts, vector<int>& column, int& col) {
  string desc=ts.getDescription();
  if(desc!="") cout<<comment<<"   Description: "<<ts.getDescription()<<endl;
  vector<string> colLabel=ts.getColumnLabel();
  cout<<comment<<"   Column Label: "<<endl;
  for(int i=0; i<column.size(); i++)
    cout<<comment<<"     "<<setfill('0')<<setw(4)<<col++<<": "<<colLabel[column[i]-1]<<endl;
}

template<class T>
void dumpSubRow(Serie2D<T>& ts, vector<int>& column, int r) {
  int rows=ts.getRows();
  if(r<rows) {
    std::vector<T> row=ts.getRow(r);
    for(int j=0; j<column.size(); j++)
      cout<<row[column[j]-1]<<" ";
  }
  else
    for(int j=0; j<column.size(); j++)
      cout<<nan<<" ";
}
template<>
void dumpSubRow(Serie2D<string>& ts, vector<int>& column, int r) {
  int rows=ts.getRows();
  if(r<rows) {
    std::vector<string> row=ts.getRow(r);
    for(int j=0; j<column.size(); j++)
      cout<<quote<<row[column[j]-1]<<quote<<" ";
  }
  else
    for(int j=0; j<column.size(); j++)
      cout<<nan<<" ";
}

int main(int argc, char* argv[]) {
  vector<string> arg;
  for(int i=1; i<argc; i++)
    arg.push_back(argv[i]);

  if(arg.size()==0 ||
     find(arg.begin(), arg.end(), "-h")!=arg.end() ||
     find(arg.begin(), arg.end(), "--help")!=arg.end() ) {
    cout<<"Dumps 2 dimensional datasets of a HDF5 file as a space separated table."<<endl;
    cout<<"If more file/datasets (DATA) are given the output tabel will be merged"<<endl;
    cout<<"row-wise. If the number of rows in multi DATA output differs, <nan> will be"<<endl;
    cout<<"appended for to short DATAs."<<endl;
    cout<<endl;
    cout<<"Usage:"<<endl;
    cout<<"  h5dumptimeserie [options] DATA [DATA] ..."<<endl;
    cout<<"    DATA: FILENAME/DATASET[:COLUMNS]"<<endl;
    cout<<"      FILENAME: <dir/to/hdf5/file/filetodump.h5>"<<endl;
    cout<<"      DATASET: <dir/to/dataset/in/hdf5file/datasettodump>"<<endl;
    cout<<"      COLUMNS: COL|RANGE[,COL|RANGE]... (dump all columns if not given)"<<endl;
    cout<<"        COL: column number in dataset to dump (starting with 1)"<<endl;
    cout<<"        RANGE: [BEGINCOL]-[ENDCOL] dump columns from BEGINCOL to ENDCOL"<<endl;
    cout<<"          (use 1/last if not given)"<<endl;
    cout<<"    options:"<<endl;
    cout<<"      -h, --help: show this help"<<endl;
    cout<<"      -c <string>: use <string> as comment in header (Default '#')"<<endl;
    cout<<"      -s: suppress header"<<endl;
    cout<<"      -q <quote>: use <quote> to quote string in output (Default '\"')"<<endl;
    cout<<"      -n <nan>: use <nan> for not a number in output (Default 'nan')"<<endl;
    cout<<endl;
    cout<<"Example:"<<endl;
    cout<<"  h5dumptimeserie dir/test1.h5/grp1/grp2/mydata:1,3,5-,2 dir/test1.h5/mydata:-4"<<endl;
    return 0;
  }

  vector<string>::iterator i;

  i=find(arg.begin(), arg.end(), "-c");
  if(i!=arg.end()) {
    comment=*(i+1);
    arg.erase(i, i+2);
  }

  bool header=true;
  i=find(arg.begin(), arg.end(), "-s");
  if(i!=arg.end()) {
    header=false;
    arg.erase(i);
  }

  i=find(arg.begin(), arg.end(), "-q");
  if(i!=arg.end()) {
    quote=*(i+1);
    arg.erase(i, i+2);
  }

  i=find(arg.begin(), arg.end(), "-n");
  if(i!=arg.end()) {
    nan=*(i+1);
    arg.erase(i, i+2);
  }

  DataType* datatype=new DataType[arg.size()];
  vector<int>* column=new vector<int>[arg.size()];
  int maxrows=0;
  int col=1;

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
  vector<Serie2D<CTYPE> > ts##TYPE; \
  int k##TYPE=0;
# include "../knowntypes.def"
# undef FOREACHKNOWNTYPE

  for(int k=0; k<arg.size(); k++) {
    string para=arg[k];

    int i;
    i=para.find(".h5/");
    string filename=para.substr(0, i+3);
    H5File file(filename, H5F_ACC_RDONLY);

    string dummy=para.substr(i+3);
    i=dummy.find(':');
    string datasetname, columnname;
    if(i<0) {
      datasetname=dummy;
      columnname="-,";
    }
    else {
      datasetname=dummy.substr(0, i);
      columnname=dummy.substr(i+1)+',';
    }

    DataSet ds=file.openDataSet(datasetname);
    DataSpace space=ds.getSpace();
    int N=space.getSimpleExtentNdims();
    hsize_t* dims=new hsize_t[N];
    space.getSimpleExtentDims(dims);
    int columns=dims[1];
    maxrows=maxrows>dims[0]?maxrows:dims[0];
    delete[]dims;
    while((i=columnname.find(','))>0) {
      string columnstr=columnname.substr(0,i);
      columnname=columnname.substr(i+1);
      i=columnstr.find('-');
      if(i<0)
        column[k].push_back(string2int(columnstr));
      else {
        int begin, end;
        if(columnstr.substr(0,i)=="") begin=1; else begin=string2int(columnstr.substr(0,i));
        if(columnstr.substr(i+1)=="") end=columns; else end=string2int(columnstr.substr(i+1));
        for(int j=begin; j<=end; j++)
          column[k].push_back(j);
      }
    }

    datatype[k]=ds.getDataType();

    if(header) cout<<comment<<" File/DataSet: "<<filename<<datasetname<<endl;
    if(0);
#   define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
    else if(datatype[k]==H5TYPE) { \
      ts##TYPE.push_back(Serie2D<CTYPE>(file, datasetname)); \
      if(header) dumpHeader(ts##TYPE[k##TYPE], column[k], col); \
      k##TYPE++; \
    }
#   include "../knowntypes.def"
#   undef FOREACHKNOWNTYPE
    else cerr<<"Unknown datatype"<<endl;

    file.close();
  }

  for(int r=0; r<maxrows; r++) {
#   define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
    k##TYPE=0;
#   include "../knowntypes.def"
#   undef FOREACHKNOWNTYPE

    for(int k=0; k<arg.size(); k++) {
      if(0);
#     define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
      else if(datatype[k]==H5TYPE) { \
        dumpSubRow(ts##TYPE[k##TYPE], column[k], r); \
        k##TYPE++; \
      }
#     include "../knowntypes.def"
#     undef FOREACHKNOWNTYPE
      else cerr<<"Unknown datatype"<<endl;
    }
    cout<<endl;
  }

  delete[]datatype;
  delete[]column;
}
