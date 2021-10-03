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
#include <cassert>
#include <cfenv>
#include <string>
#include <hdf5serie/vectorserie.h>
#include <hdf5serie/simpledataset.h>
#include <hdf5serie/toh5type.h>
#include <iomanip>
#include <limits>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

using namespace H5;
using namespace std;

/** \page h5dumpserie Program h5dumpserie
 * 
 * The program h5dumpserie can be used to dump the content of a HDF5 file
 * created by this library as a ascii table. This ascii table can then be
 * read by programs like gnuplot, ...
 *
 * See the output of "h5dumpserie -h" - which is given (unformated) below - for a detailed description:
 *
 * \verbinclude h5dumpserie.txt
 */

/** \page h5lsserie Program h5lsserie
 *
 * The program h5lsserie can be used to list the content of a HDF5 file.
 * If a dataset has been created by this library then the description
 * attribute of the dataset is also shown.
 *
 * Usage:
 *
 * h5lsserie <filename>/<path>
 */

string comment="#";
string quote="\"";
string complexFormat="%1%+%2%i";
string delim=" ";
string mynan="nan";
int precision=numeric_limits<double>::digits10+1;

void printRow(Dataset *d, int row);

int main(int argc, char* argv[]) {
#ifndef _WIN32
  assert(feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW)!=-1);
#endif

  vector<string> arg;
  for(int i=1; i<argc; i++)
    arg.emplace_back(argv[i]);

  if(arg.empty() ||
     find(arg.begin(), arg.end(), "-h")!=arg.end() ||
     find(arg.begin(), arg.end(), "--help")!=arg.end() ) {
     cout<<
#    include "h5dumpserie.txt"
     ;
    return 0;
  }

  vector<string>::iterator i;

  i=find(arg.begin(), arg.end(), "-c");
  if(i!=arg.end()) {
    comment=*(i+1);
    arg.erase(i, i+2);
  }

  i=find(arg.begin(), arg.end(), "-d");
  if(i!=arg.end()) {
    delim=*(i+1);
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

  i=find(arg.begin(), arg.end(), "-f");
  if(i!=arg.end()) {
    complexFormat=*(i+1);
    arg.erase(i, i+2);
  }

  i=find(arg.begin(), arg.end(), "-n");
  if(i!=arg.end()) {
    mynan=*(i+1);
    arg.erase(i, i+2);
  }

  i=find(arg.begin(), arg.end(), "-p");
  if(i!=arg.end()) {
    precision=boost::lexical_cast<int>(*(i+1));
    arg.erase(i, i+2);
  }

  unsigned int maxrows=0;
  vector<vector<int> > column(arg.size());
  vector<Dataset*> dataSet(arg.size());
  vector<std::shared_ptr<File> > file(arg.size());
  int col=1;
  for(unsigned int k=0; k<arg.size(); k++) {
    string para=arg[k];

    int i;
    i=para.find('/');
    string filename=para.substr(0, i);
    file[k].reset(new File(filename, File::read));

    string dummy=para.substr(i);
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

    dataSet[k]=dynamic_cast<Dataset*>(file[k]->openChildObject(datasetname));
    vector<hsize_t> dims=dataSet[k]->getExtentDims();
    if(dims.empty())
      continue;
    int columns=0;
    if(dims.size()==1)
      columns=1;
    if(dims.size()==2)
      columns=dims[1];
    maxrows=maxrows>dims[0]?maxrows:dims[0];
    while((i=columnname.find(','))>0) {
      string columnstr=columnname.substr(0,i);
      columnname=columnname.substr(i+1);
      i=columnstr.find('-');
      if(i<0)
        column[k].push_back(boost::lexical_cast<int>(columnstr));
      else {
        int begin, end;
        if(columnstr.substr(0,i).empty()) begin=1; else begin=boost::lexical_cast<int>(columnstr.substr(0,i));
        if(columnstr.substr(i+1).empty()) end=columns; else end=boost::lexical_cast<int>(columnstr.substr(i+1));
        if(end>=begin)
          for(int j=begin; j<=end; j++)
            column[k].push_back(j);
        else
          for(int j=begin; j>=end; j--)
            column[k].push_back(j);
      }
    }
    if(header) {
      cout<<comment<<" File/DataSet: "<<filename<<datasetname<<endl;
      if(dataSet[k]->hasChildAttribute("Description")) {
        string desc=dataSet[k]->openChildAttribute<SimpleAttribute<string> >("Description")->read();
        cout<<comment<<"   Description: "<<desc<<endl;
      }

//      if(dataSet[k].getDataType().getClass()==H5T_COMPOUND) {
//        try {
//          CompType dataType=dataSet[k].getCompType();
//          cout<<comment<<"   Member Label:"<<endl;
//          for(unsigned int j=0; j<column[k].size(); j++) {
//            if(dataType.getMemberDataType(column[k][j]-1).getClass()==H5T_ARRAY) {
//              hsize_t dims[1];
//              dataType.getMemberArrayType(column[k][j]-1).getArrayDims(dims);
//              for(unsigned int l=0; l<dims[0]; l++)
//                cout<<comment<<"     "<<setfill('0')<<setw(4)<<col++<<": "<<dataType.getMemberName(column[k][j]-1)<<"("<<l+1<<")"<<endl;
//            }
//            else
//              cout<<comment<<"     "<<setfill('0')<<setw(4)<<col++<<": "<<dataType.getMemberName(column[k][j]-1)<<endl;
//          }
//        }
//        catch(...) {
//          cout<<comment<<"   Member labels are not avaliable."<<endl;
//        }
//      }
//      else {
        if(dataSet[k]->hasChildAttribute("Column Label")) {
          vector<string> cols=dataSet[k]->openChildAttribute<SimpleAttribute<vector<string> > >("Column Label")->read();
          cout<<comment<<"   Column Label:"<<endl;
          for(int j : column[k])
            cout<<comment<<"     "<<setfill('0')<<setw(4)<<col++<<": "<<cols[j-1]<<endl;
        }
        else
          cout<<comment<<"   Column labels are not avaliable."<<endl;
//      }
    }
  }

  cout<<setprecision(precision)<<scientific;
  for(unsigned int row=0; row<maxrows; row++) {
    for(unsigned int k=0; k<arg.size(); k++) {
      // Output mynan for to short datasets
      vector<hsize_t> dims=dataSet[k]->getExtentDims();
      if(row>=dims[0]) {
        for(unsigned int i=0; i<column[k].size(); i++)
          cout<<(k==0&&i==0?"":delim)<<mynan;
        continue;
      }
      
      cout<<(k==0?"":delim);
      printRow(dataSet[k], row);
    }
    cout<<endl;
  }

  return 0;
}

template<typename T>
class Format {
  public:
    Format(const T& data_) : data(data_) {}
    const T& data;
};
template<typename T>
ostream& operator<<(ostream& os, const Format<T>& format) {
  os<<format.data;
  return os;
}
template<>
ostream& operator<<(ostream& os, const Format<string>& format) {
  os<<quote<<format.data<<quote;
  return os;
}
template<typename T>
ostream& operator<<(ostream& os, const Format<complex<T>>& format) {
  stringstream strR;
  strR<<setprecision(precision)<<scientific;
  strR<<format.data.real();
  stringstream strI;
  strI<<setprecision(precision)<<scientific;
  strI<<format.data.imag();
  os<<boost::format(complexFormat)%strR.str()%strI.str();
  return os;
}

void printRow(Dataset *d, int row) {
# define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
  { \
    VectorSerie<CTYPE> *dd=dynamic_cast<VectorSerie<CTYPE>*>(d); \
    if(dd) { \
      vector<CTYPE> vec(dd->getColumns()); \
      dd->getRow(row, vec); \
      for(size_t i=0; i<vec.size(); ++i) \
        cout<<(i==0?"":delim)<<Format(vec[i]); \
    } \
  }
# include "hdf5serie/knowntypes.def"
# undef FOREACHKNOWNTYPE

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
  { \
    SimpleDataset<vector<CTYPE> > *dd=dynamic_cast<SimpleDataset<vector<CTYPE> >*>(d); \
    if(dd) { \
      vector<CTYPE> vec=dd->read(); \
      cout<<Format(vec[row]); \
    } \
  }
# include "hdf5serie/knowntypes.def"
# undef FOREACHKNOWNTYPE

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
  { \
    SimpleDataset<vector<vector<CTYPE> > > *dd=dynamic_cast<SimpleDataset<vector<vector<CTYPE> > >*>(d); \
    if(dd) { \
      vector<vector<CTYPE> > mat=dd->read(); \
      for(size_t i=0; i<mat[0].size(); ++i) \
        cout<<(i==0?"":delim)<<Format(mat[row][i]); \
    } \
  }
# include "hdf5serie/knowntypes.def"
# undef FOREACHKNOWNTYPE
}
