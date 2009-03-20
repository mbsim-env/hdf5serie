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
 *   mafriedrich@users.berlios.de
 *
 */

#include <config.h>
#include <fstream>
#include <hdf5serie/vectorserie.h>
#include <hdf5serie/structserie.h>
#include <hdf5serie/simpledataset.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#ifdef HAVE_ANSICSIGNAL
#  include <signal.h>
#endif

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
 * h5lsserie <filename.h5>
 */

string comment="#";
string quote="\"";
string delim=" ";
string nan="nan";

int string2int(string str) {
  stringstream sstr(str);
  int i;
  sstr>>i;
  return i;
}

int main(int argc, char* argv[]) {
  vector<string> arg;
  for(int i=1; i<argc; i++)
    arg.push_back(argv[i]);

  if(arg.size()==0 ||
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

  i=find(arg.begin(), arg.end(), "-n");
  if(i!=arg.end()) {
    nan=*(i+1);
    arg.erase(i, i+2);
  }

  int maxrows=0;
  vector<int>* column=new vector<int>[arg.size()];
  DataSet* dataSet=new DataSet[arg.size()];
  int col=1;
  for(int k=0; k<arg.size(); k++) {
    string para=arg[k];

    int i;
    i=para.find(".h5/");
    string filename=para.substr(0, i+3);
#ifdef HAVE_ANSICSIGNAL
    ifstream lockFile(("."+filename).c_str());
    if(lockFile.good() && ! lockFile.eof()) {
      pid_t pid;
      lockFile>>pid;
      lockFile.close();
      kill(pid, SIGUSR2);
    }
#endif
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

    dataSet[k]=file.openDataSet(datasetname);
    DataType datatype=dataSet[k].getDataType();

    DataSpace space=dataSet[k].getSpace();
    int N=space.getSimpleExtentNdims();
    hsize_t* dims=new hsize_t[N];
    space.getSimpleExtentDims(dims);
    int columns;
    if(datatype.getClass()==H5T_COMPOUND)
      columns=dataSet[k].getCompType().getNmembers();
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
      try {
        H5E_auto2_t func;
        void *client_data;
        Exception::getAutoPrint(func, &client_data);
        Exception::dontPrint();
        string desc=SimpleAttribute<string>::getData(dataSet[k], "Description");
        if(desc!="") cout<<comment<<"   Description: "<<desc<<endl;
        Exception::setAutoPrint(func, client_data);
      } catch(...) {}
      if(dataSet[k].getDataType().getClass()==H5T_COMPOUND) {
        cout<<comment<<"   Member Label:"<<endl;
        CompType dataType=dataSet[k].getCompType();
        for(int j=0; j<column[k].size(); j++) {
          if(dataType.getMemberDataType(column[k][j]-1).getClass()==H5T_ARRAY) {
            hsize_t dims[1];
            dataType.getMemberArrayType(column[k][j]-1).getArrayDims(dims);
            for(int l=0; l<dims[0]; l++)
              cout<<comment<<"     "<<setfill('0')<<setw(4)<<col++<<": "<<dataType.getMemberName(column[k][j]-1)<<"("<<l+1<<")"<<endl;
          }
          else
            cout<<comment<<"     "<<setfill('0')<<setw(4)<<col++<<": "<<dataType.getMemberName(column[k][j]-1)<<endl;
        }
      }
      else {
        cout<<comment<<"   Column Label:"<<endl;
        vector<string> cols=SimpleAttribute<vector<string> >::getData(dataSet[k], "Column Label");
        for(int j=0; j<column[k].size(); j++)
          cout<<comment<<"     "<<setfill('0')<<setw(4)<<col++<<": "<<cols[column[k][j]-1]<<endl;
      }
    }
  }

  cout<<setprecision(17);
  for(int row=0; row<maxrows; row++) {
    for(int k=0; k<arg.size(); k++) {
      // Output nan for to short datasets
      DataSpace space=dataSet[k].getSpace();
      int N=space.getSimpleExtentNdims();
      hsize_t* dims=new hsize_t[N];
      space.getSimpleExtentDims(dims);
      if(row>=dims[0]) {
        for(int i=0; i<column[k].size(); i++)
          cout<<(k==0&&i==0?"":delim)<<nan;
        continue;
      }
      delete[]dims;
      
      if(dataSet[k].getDataType().getClass()==H5T_COMPOUND) {
        hsize_t dims[]={1};
        DataSpace memDataSpace(1, dims);
        DataSpace fileDataSpace=dataSet[k].getSpace();
        hsize_t start[]={row}, count[]={1};
        fileDataSpace.selectHyperslab(H5S_SELECT_SET, count, start);
        CompType memDataType=dataSet[k].getCompType();//////////
        char* buf=new char[memDataType.getSize()];
        dataSet[k].read(buf, memDataType, memDataSpace, fileDataSpace);
        for(int i=0; i<column[k].size(); i++) {
          DataType memberDataType=memDataType.getMemberDataType(column[k][i]-1);
          int memberOffset=memDataType.getMemberOffset(column[k][i]-1);
#         define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
          if(memberDataType==H5TYPE) \
            cout<<(k==0&&i==0?"":delim)<<*(CTYPE*)(buf+memberOffset);
#         include "knownpodtypes.def"
#         undef FOREACHKNOWNTYPE
          if(memberDataType==StrType(PredType::C_S1, H5T_VARIABLE)) {
            char* str=*(char**)(buf+memberOffset);
            cout<<(k==0&&i==0?"":delim)<<quote<<str<<quote;
            free(str);
          }
          if(memberDataType.getClass()==H5T_ARRAY) {
            hsize_t dims[1];
            memDataType.getMemberArrayType(column[k][i]-1).getArrayDims(dims);
#           define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
            if(memberDataType==ArrayType(H5TYPE,1,dims)) \
              for(int l=0; l<dims[0]; l++) \
                cout<<(k==0&&i==0&&l==0?"":delim)<<*(CTYPE*)(buf+memberOffset+sizeof(CTYPE)*l);
#           include "knownpodtypes.def"
#           undef FOREACHKNOWNTYPE
            if(memberDataType==ArrayType(StrType(PredType::C_S1, H5T_VARIABLE),1,dims))
              for(int l=0; l<dims[0]; l++) {
                char* str=*(char**)(buf+memberOffset+sizeof(char*)*l);
                cout<<(k==0&&i==0&&l==0?"":delim)<<quote<<str<<quote;
                free(str);
              }
          }
        }
        delete[]buf;
      }
      else {
        hsize_t dims[2];
        DataSpace fileDataSpace=dataSet[k].getSpace();
        fileDataSpace.getSimpleExtentDims(dims);
        hsize_t start[]={row,0}, count[]={1,dims[1]};
        fileDataSpace.selectHyperslab(H5S_SELECT_SET, count, start);
        DataSpace memDataSpace(2, count);
        DataType memDataType=dataSet[k].getDataType();//////////
        char* buf=new char[memDataType.getSize()*dims[1]];
        dataSet[k].read(buf, memDataType, memDataSpace, fileDataSpace);
        for(int i=0; i<column[k].size(); i++) {
#         define FOREACHKNOWNTYPE(CTYPE, H5TYPE, TYPE) \
          if(memDataType==H5TYPE) \
            cout<<(k==0&&i==0?"":delim)<<*(CTYPE*)(buf+sizeof(CTYPE)*(column[k][i]-1));
#         include "knownpodtypes.def"
#         undef FOREACHKNOWNTYPE
          if(memDataType==StrType(PredType::C_S1, H5T_VARIABLE)) {
            char* str=*(char**)(buf+sizeof(char*)*(column[k][i]-1));
            cout<<(k==0&&i==0?"":delim)<<quote<<str<<quote;
            free(str);
          }
        }
        delete[]buf;
      }
    }
    cout<<endl;
  }

  delete[]column;
  delete[]dataSet;
}
