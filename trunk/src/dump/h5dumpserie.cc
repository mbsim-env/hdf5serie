#include <config.h>
#include <serie2d.h>
#include <serie1d.h>
#include <simpledataset.h>
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

/*template<class T>
void dumpHeader(Serie2D<T>& s2d, vector<int>& column, int& col) {
  string desc=s2d.getDescription();
  if(desc!="") cout<<comment<<"   Description: "<<desc<<endl;
  vector<string> colLabel=s2d.getColumnLabel();
  cout<<comment<<"   Column Label: "<<endl;
  for(int i=0; i<column.size(); i++)
    cout<<comment<<"     "<<setfill('0')<<setw(4)<<col++<<": "<<colLabel[column[i]-1]<<endl;
}

void dumpHeader(Serie1D<char*> s1d, vector<int>& column, int& col) {
  string desc=s1d.getDescription();
  if(desc!="") cout<<comment<<"   Description: "<<desc<<endl;
  vector<string> memberLabel=s1d.getMemberLabel();
  cout<<comment<<"   Member Label: "<<endl;
  for(int i=0; i<column.size(); i++)
    cout<<comment<<"     "<<setfill('0')<<setw(4)<<col++<<": "<<memberLabel[column[i]-1]<<endl;
}

template<class T>
void dumpHeader(SimpleDataSet<vector<T> >& sdsv, vector<int>& column, int& col) {
  cout<<comment<<"   "<<setfill('0')<<setw(4)<<col++<<": "<<sdsv.getDescription()<<endl;
}

template<class T>
void dumpSubRow(Serie2D<T>& s2d, vector<int>& column, int r) {
  int rows=s2d.getRows();
  if(r<rows) {
    std::vector<T> row=s2d.getRow(r);
    for(int j=0; j<column.size(); j++)
      cout<<row[column[j]-1]<<" ";
  }
  else
    for(int j=0; j<column.size(); j++)
      cout<<nan<<" ";
}
template<>
void dumpSubRow(Serie2D<string>& s2d, vector<int>& column, int r) {
  int rows=s2d.getRows();
  if(r<rows) {
    std::vector<string> row=s2d.getRow(r);
    for(int j=0; j<column.size(); j++)
      cout<<quote<<row[column[j]-1]<<quote<<" ";
  }
  else
    for(int j=0; j<column.size(); j++)
      cout<<nan<<" ";
}

void dumpSubRow(Serie1D<char*>& s1d, vector<int>& column, int r) {
  int rows=s1d.getRows();
  if(r<rows) {
    char* buf;
    buf=s1d.getRow(r);
    CompType comptype=s1d.getCompType();
    for(int j=0; j<column.size(); j++) {
#     define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
      if(comptype.getMemberDataType(column[j]-1)==H5TYPE) \
        cout<<*(CTYPE*)((char*)buf+comptype.getMemberOffset(column[j]-1))<<" ";
#     include "../knowntypes.def"
#     undef FOREACHKNOWNTYPE
    }
    delete[]buf;
  }
  else
    for(int j=0; j<column.size(); j++)
      cout<<nan<<" ";
}

template<class T>
void dumpSubRow(SimpleDataSet<vector<T> > sdsv, vector<int>& column, int r) {
  vector<T> data=sdsv.read();
  if(r<data.size())
    cout<<data[r]<<" ";
  else
    cout<<nan<<" ";
}
template<>
void dumpSubRow(SimpleDataSet<vector<string> > sdsv, vector<int>& column, int r) {
  vector<string> data=sdsv.read();
  if(r<data.size())
    cout<<quote<<data[r]<<quote<<" ";
  else
    cout<<nan<<" ";
}*/

int main(int argc, char* argv[]) {
  vector<string> arg;
  for(int i=1; i<argc; i++)
    arg.push_back(argv[i]);

  if(arg.size()==0 ||
     find(arg.begin(), arg.end(), "-h")!=arg.end() ||
     find(arg.begin(), arg.end(), "--help")!=arg.end() ) {
    cout<<"Dumps one and two dimensional datasets of a HDF5 file as a space separated"<<endl;
    cout<<"table."<<endl;
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

  int maxrows=0;
  vector<int>* column=new vector<int>[arg.size()];
  DataSet* dataSet=new DataSet[arg.size()];
  int col=0;
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

    dataSet[i]=file.openDataSet(datasetname);
    DataType datatype=dataSet[i].getDataType();

    DataSpace space=dataSet[i].getSpace();
    int N=space.getSimpleExtentNdims();
    hsize_t* dims=new hsize_t[N];
    space.getSimpleExtentDims(dims);
    int columns;
    if(datatype.getClass()==H5T_COMPOUND)
      columns=dataSet[i].getCompType().getNmembers();
    else
      columns=dims[1];
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
    if(header) {
      cout<<comment<<" File/DataSet: "<<filename<<datasetname<<endl;
      string desc=SimpleAttribute<string>::getData(dataSet[i], "Description");
      if(desc!="") cout<<comment<<"   Description: "<<desc<<endl;
      if(dataSet[i].getDataType().getClass()==H5T_COMPOUND) {
        cout<<comment<<"   Member Label:"<<desc<<endl;
        CompType dataType=dataSet[i].getCompType();
        for(int j=0; j<column[i].size(); j++)
          cout<<comment<<"     "<<setfill('0')<<setw(4)<<col++<<": "<<dataType.getMemberName(column[i][j]-1)<<endl;
      }
      else {
        cout<<comment<<"   Column Label:"<<desc<<endl;
        vector<string> cols=SimpleAttribute<vector<string> >::getData(dataSet[i], "Column Label");
        for(int j=0; j<column[i].size(); j++)
          cout<<comment<<"     "<<setfill('0')<<setw(4)<<col++<<": "<<cols[column[i][j]-1]<<endl;
      }
    }
  }
  delete[]column;
  delete[]dataSet;

/*  DataType* datatype=new DataType[arg.size()];
  vector<int>* column=new vector<int>[arg.size()];
  int maxrows=0;
  int col=1;

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
  vector<Serie2D<CTYPE> > s2d##TYPE; \
  int ks2d##TYPE=0;
# include "../knowntypes.def"
# undef FOREACHKNOWNTYPE
  vector<Serie1D<char*> > s1d;
  int ks1d=0;
# define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
  vector<SimpleDataSet<vector<CTYPE> > > sdsv##TYPE; \
  int ksdsv##TYPE=0;
# include "../knowntypes.def"
# undef FOREACHKNOWNTYPE


    if(header) cout<<comment<<" File/DataSet: "<<filename<<datasetname<<endl;
    if(0);
#   define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
    else if(datatype[k]==H5TYPE && ds.getSpace().getSimpleExtentNdims()==2) { \
      s2d##TYPE.push_back(Serie2D<CTYPE>(file, datasetname)); \
      if(header) dumpHeader(s2d##TYPE[ks2d##TYPE], column[k], col); \
      ks2d##TYPE++; \
    }
#   include "../knowntypes.def"
#   undef FOREACHKNOWNTYPE
    else if(datatype[k].getClass()==H5T_COMPOUND) {
      s1d.push_back(Serie1D<char*>(file, datasetname));
      if(header) dumpHeader(s1d[ks1d], column[k], col);
      ks1d++;
    }
#   define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
    else if(datatype[k]==H5TYPE && ds.getSpace().getSimpleExtentNdims()==1) { \
      sdsv##TYPE.push_back(SimpleDataSet<vector<CTYPE> >(file, datasetname)); \
      if(header) dumpHeader(sdsv##TYPE[ksdsv##TYPE], column[k], col); \
      ksdsv##TYPE++; \
    }
#   include "../knowntypes.def"
#   undef FOREACHKNOWNTYPE
    else cerr<<"Unknown datatype"<<endl;

    file.close();
  }

  for(int r=0; r<maxrows; r++) {
#   define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
    ks2d##TYPE=0;
#   include "../knowntypes.def"
#   undef FOREACHKNOWNTYPE
    ks1d=0;
#   define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
    ksdsv##TYPE=0;
#   include "../knowntypes.def"
#   undef FOREACHKNOWNTYPE

    for(int k=0; k<arg.size(); k++) {
      if(0);
#     define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
      else if(datatype[k]==H5TYPE && ks2d##TYPE<s2d##TYPE.size() && s2d##TYPE[ks2d##TYPE].getSpace().getSimpleExtentNdims()==2) { \
        dumpSubRow(s2d##TYPE[ks2d##TYPE], column[k], r); \
        ks2d##TYPE++; \
      }
#     include "../knowntypes.def"
#     undef FOREACHKNOWNTYPE
      else if(datatype[k].getClass()==H5T_COMPOUND) {
        dumpSubRow(s1d[ks1d], column[k], r);
        ks1d++;
      }
#     define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
      else if(datatype[k]==H5TYPE && ksdsv##TYPE<sdsv##TYPE.size() && sdsv##TYPE[ksdsv##TYPE].getSpace().getSimpleExtentNdims()==1) { \
        dumpSubRow(sdsv##TYPE[ksdsv##TYPE], column[k], r); \
        ksdsv##TYPE++; \
      }
#     include "../knowntypes.def"
#     undef FOREACHKNOWNTYPE
      else cerr<<"Unknown datatype"<<endl;
    }
    cout<<endl;
  }

  delete[]datatype;
  delete[]column;*/
}
