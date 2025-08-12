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

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  undef __STRICT_ANSI__ // to define _controlfp which is not part of ANSI and hence not defined in mingw
#  include <cfloat>
#  define __STRICT_ANSI__
#endif
#include <config.h>
#include <cassert>
#include <cfenv>
#include <hdf5serie/vectorserie.h>
#include <hdf5serie/simpleattribute.h>
#include <hdf5serie/simpledataset.h>
#include <iostream>
#include <fmatvec/fmatvec.h>

using namespace H5;
using namespace std;

//struct MyStruct {
//  double d;
//  vector<int> v;
//  vector<string> vs;
//  float f;
//  string s;
//  int i;
//};

int worker(File::FileAccess writeType, bool callEnableSWMR);

int main() {
#ifdef _WIN32
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);
  _controlfp(~(_EM_ZERODIVIDE | _EM_INVALID | _EM_OVERFLOW), _MCW_EM);
#else
  assert(feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW)!=-1);
#endif

  int ret=0;
  ret += worker(File::write, true);
  ret += worker(File::write, false);
  ret += worker(File::writeWithRename, true);
  ret += worker(File::writeWithRename, false);

  return ret;
}

int worker(File::FileAccess writeType, bool callEnableSWMR) {
  cout<<"Running with writeType = "<<writeType<<"\n";

  /***** SimpleDataset *****/
  cout<<"MYDATASET\n";

  { // scalar double
  cout<<"scalar double\n";
  File file("test.h5", writeType);

  SimpleDataset<double> *dsd=file.createChildObject<SimpleDataset<double> >("dsd")();
  double d=5.67;
  dsd->write(d);
  dsd->setDescription("testdesc");
  cout<<dsd->read()<<endl;
  if(callEnableSWMR)
    file.enableSWMR();
  }
  {
  File file("test.h5", File::read);
  auto *dsd=file.openChildObject<SimpleDataset<double> >("dsd");
  cout<<dsd->read()<<endl;
  cout<<dsd->getDescription()<<endl;
  }
  


  { // scalar string
  cout<<"scalar string\n";
  File file("test.h5", writeType);

  SimpleDataset<string> *dsd=file.createChildObject<SimpleDataset<string> >("dsd")();
  string d="sdlfkjsf";
  dsd->write(d);
  dsd->setDescription("testdesc");
  cout<<dsd->read()<<endl;
  if(callEnableSWMR)
    file.enableSWMR();
  }
  {
  File file("test.h5", File::read);
  auto *dsd=file.openChildObject<SimpleDataset<string> >("dsd");
  cout<<dsd->read()<<endl;
  cout<<dsd->getDescription()<<endl;
  }



  { // scalar complex<double>
  cout<<"scalar complex<double>\n";
  File file("test.h5", writeType);

  SimpleDataset<complex<double>> *dsd=file.createChildObject<SimpleDataset<complex<double>> >("dsd")();
  complex<double> d{3.5,7.3};
  dsd->write(d);
  dsd->setDescription("testdesc");
  cout<<dsd->read()<<endl;
  if(callEnableSWMR)
    file.enableSWMR();
  }
  {
  File file("test.h5", File::read);
  auto *dsd=file.openChildObject<SimpleDataset<complex<double>> >("dsd");
  cout<<dsd->read()<<endl;
  cout<<dsd->getDescription()<<endl;
  }









  { // vector double
  cout<<"vector double\n";
  File file("test.h5", writeType);

  SimpleDataset<vector<double> > *dsd=file.createChildObject<SimpleDataset<vector<double> > >("dsd")(2);
  vector<double> d; d.push_back(5.67); d.push_back(7.34);
  dsd->write(d);
  dsd->setDescription("testdesc");
  vector<double> dout;
  dout=dsd->read();
  for(double d : dout) cout<<d<<endl;
  if(callEnableSWMR)
    file.enableSWMR();
  }
  {
  File file("test.h5", File::read);
  auto *dsd=file.openChildObject<SimpleDataset<vector<double> > >("dsd");
  vector<double> dout;
  dout=dsd->read();
  for(double d : dout) cout<<d<<endl;
  cout<<dsd->getDescription()<<endl;
  }
  


  { // vector string
  cout<<"vector string\n";
  File file("test.h5", writeType);

  SimpleDataset<vector<string> > *dsd=file.createChildObject<SimpleDataset<vector<string> > >("dsd")(2);
  vector<string> d; d.emplace_back("sdlkfj"); d.emplace_back("owiuer");
  dsd->write(d);
  dsd->setDescription("testdesc");
  vector<string> dout;
  dout=dsd->read();
  for(auto & d : dout) cout<<d<<endl;
  if(callEnableSWMR)
    file.enableSWMR();
  }
  {
  File file("test.h5", File::read);
  auto *dsd=file.openChildObject<SimpleDataset<vector<string> > >("dsd");
  vector<string> dout;
  dout=dsd->read();
  for(auto & d : dout) cout<<d<<endl;
  cout<<dsd->getDescription()<<endl;
  }



  { // vector complex<double>
  cout<<"vector complex<double>\n";
  File file("test.h5", writeType);

  SimpleDataset<vector<complex<double>> > *dsd=file.createChildObject<SimpleDataset<vector<complex<double>> > >("dsd")(2);
  vector<complex<double>> d; d.emplace_back(3.6,7.5); d.emplace_back(7.3,4.7);
  dsd->write(d);
  dsd->setDescription("testdesc");
  vector<complex<double>> dout;
  dout=dsd->read();
  for(auto & d : dout) cout<<d<<endl;
  if(callEnableSWMR)
    file.enableSWMR();
  }
  {
  File file("test.h5", File::read);
  auto *dsd=file.openChildObject<SimpleDataset<vector<complex<double>> > >("dsd");
  vector<complex<double>> dout;
  dout=dsd->read();
  for(auto & d : dout) cout<<d<<endl;
  cout<<dsd->getDescription()<<endl;
  }
















  /***** Attribute *****/
  cout<<"ATTRIBUTE\n";

  { // scalar double
  cout<<"scalar double\n";
  File file("test.h5", writeType);
  SimpleDataset<double> *data=file.createChildObject<SimpleDataset<double> >("data")();

  SimpleAttribute<double> *dsd=data->createChildAttribute<SimpleAttribute<double> >("dsd")();
  double d=5.67;
  dsd->write(d);
  cout<<dsd->read()<<endl;
  if(callEnableSWMR)
    file.enableSWMR();
  }
  {
  File file("test.h5", File::read);
  Dataset *data=file.openChildObject<SimpleDataset<double> >("data");
  auto *dsd=data->openChildAttribute<SimpleAttribute<double> >("dsd");
  cout<<dsd->read()<<endl;
  }
  


  { // scalar string
  cout<<"scalar string\n";
  File file("test.h5", writeType);
  SimpleDataset<double> *data=file.createChildObject<SimpleDataset<double> >("data")();

  SimpleAttribute<string> *dsd=data->createChildAttribute<SimpleAttribute<string> >("dsd")();
  string d="sdlfkjsf";
  dsd->write(d);
  cout<<dsd->read()<<endl;
  if(callEnableSWMR)
    file.enableSWMR();
  }
  {
  File file("test.h5", File::read);
  Dataset *data=file.openChildObject<SimpleDataset<double> >("data");
  auto *dsd=data->openChildAttribute<SimpleAttribute<string> >("dsd");
  cout<<dsd->read()<<endl;
  }



  { // scalar complex<double>
  cout<<"scalar complex<double>\n";
  File file("test.h5", writeType);
  SimpleDataset<double> *data=file.createChildObject<SimpleDataset<double> >("data")();

  SimpleAttribute<complex<double>> *dsd=data->createChildAttribute<SimpleAttribute<complex<double>> >("dsd")();
  complex<double> d{3.5,8.2};
  dsd->write(d);
  cout<<dsd->read()<<endl;
  if(callEnableSWMR)
    file.enableSWMR();
  }
  {
  File file("test.h5", File::read);
  Dataset *data=file.openChildObject<SimpleDataset<double> >("data");
  auto *dsd=data->openChildAttribute<SimpleAttribute<complex<double>> >("dsd");
  cout<<dsd->read()<<endl;
  }









  { // vector double
  cout<<"vector double\n";
  File file("test.h5", writeType);
  SimpleDataset<double> *data=file.createChildObject<SimpleDataset<double> >("data")();

  constexpr int size=10000;
  SimpleAttribute<vector<double> > *dsd=data->createChildAttribute<SimpleAttribute<vector<double> > >("dsd")(size);
  vector<double> d;
  d.reserve(size);
  for(int i=0; i<size; ++i)
    d.push_back(2*i);
  dsd->write(d);
  vector<double> dout;
  dout=dsd->read();
  for(unsigned int i=0; i<min(dout.size(), static_cast<size_t>(20)); i++) cout<<dout[i]<<endl;
  if(callEnableSWMR)
    file.enableSWMR();
  }
  {
  File file("test.h5", File::read);
  Dataset *data=file.openChildObject<SimpleDataset<double> >("data");
  auto *dsd=data->openChildAttribute<SimpleAttribute<vector<double> > >("dsd");
  vector<double> dout;
  dout=dsd->read();
  for(unsigned int i=0; i<min(dout.size(), static_cast<size_t>(20)); i++) cout<<dout[i]<<endl;
  }
  


  { // vector string
  cout<<"vector string\n";
  File file("test.h5", writeType);
  SimpleDataset<double> *data=file.createChildObject<SimpleDataset<double> >("data")();

  // test a large attribute (>64K). This should automatically switch to dense attribute storge
  constexpr int size=5000;
  SimpleAttribute<vector<string> > *dsd=data->createChildAttribute<SimpleAttribute<vector<string> > >("dsd")(size);
  vector<string> d;
  d.reserve(size);
  for(int i=0; i<size; ++i)
    d.push_back("large attribute test "+to_string(i));
  dsd->write(d);
  vector<string> dout;
  dout=dsd->read();
  for(unsigned int i=0; i<min(dout.size(), static_cast<size_t>(20)); i++) cout<<dout[i]<<endl;
  if(callEnableSWMR)
    file.enableSWMR();
  }
  {
  File file("test.h5", File::read);
  Dataset *data=file.openChildObject<SimpleDataset<double> >("data");
  auto *dsd=data->openChildAttribute<SimpleAttribute<vector<string> > >("dsd");
  vector<string> dout;
  dout=dsd->read();
  for(unsigned int i=0; i<min(dout.size(), static_cast<size_t>(20)); i++) cout<<dout[i]<<endl;
  }



  { // vector complex<double>
  cout<<"vector complex<double>\n";
  File file("test.h5", writeType);
  SimpleDataset<double> *data=file.createChildObject<SimpleDataset<double> >("data")();

  // test a large attribute (>64K). This should automatically switch to dense attribute storge
  constexpr int size=5000;
  SimpleAttribute<vector<complex<double>> > *dsd=data->createChildAttribute<SimpleAttribute<vector<complex<double>> > >("dsd")(size);
  vector<complex<double>> d;
  d.reserve(size);
  for(int i=0; i<size; ++i)
    d.emplace_back(3.5+i,2.5+i);
  dsd->write(d);
  vector<complex<double>> dout;
  dout=dsd->read();
  for(unsigned int i=0; i<min(dout.size(), static_cast<size_t>(20)); i++) cout<<dout[i]<<endl;
  if(callEnableSWMR)
    file.enableSWMR();
  }
  {
  File file("test.h5", File::read);
  Dataset *data=file.openChildObject<SimpleDataset<double> >("data");
  auto *dsd=data->openChildAttribute<SimpleAttribute<vector<complex<double>> > >("dsd");
  vector<complex<double>> dout;
  dout=dsd->read();
  for(unsigned int i=0; i<min(dout.size(), static_cast<size_t>(20)); i++) cout<<dout[i]<<endl;
  }



  /***** MYTIMESERIE *****/
  cout<<"TIMESERIE\n";
  auto write = [writeType, callEnableSWMR](const string &filename) {
    File file(filename, writeType);
    VectorSerie<double> *ts=file.createChildObject<VectorSerie<double> >("timeserie")(3);
    vector<string> colhead;
    colhead.emplace_back("col1");
    colhead.emplace_back("col22");
    colhead.emplace_back("col333");
    ts->setColumnLabel(colhead);
    ts->setDescription("mydesctipsldfk");
    vector<double> data(3);
    for(int i=0; i<15; ++i) {
      data[0]=1.2+i;
      data[1]=2.3+i;
      data[2]=3.4+i;
      ts->append(data);
    }
    vector<double> out;
    out=ts->getRow(1);
    ts->getRow(2, out);
    for(double d : out) cout<<d<<endl;
    out=ts->getColumn(1);
    for(double d : out) cout<<d<<endl;
    cout<<ts->getDescription()<<endl;
    vector<string> outhead;
    outhead=ts->getColumnLabel();
    for(auto & d : outhead) cout<<d<<endl;
    VectorSerie<string> *tsStr=file.createChildObject<VectorSerie<string> >("timeserieStr")(3);
    vector<string> dataStr;
    dataStr.emplace_back("a");
    dataStr.emplace_back("b");
    dataStr.emplace_back("c");
    tsStr->append(dataStr);
    VectorSerie<complex<double>> *tsComplex=file.createChildObject<VectorSerie<complex<double>> >("timeserieComplex")(3);
    vector<complex<double>> dataComplex;
    dataComplex.emplace_back(3.4,7.2);
    dataComplex.emplace_back(4.4,8.2);
    dataComplex.emplace_back(5.4,9.2);
    tsComplex->append(dataComplex);
    if(callEnableSWMR)
      file.enableSWMR();
  };
  write("test2d.h5");
  {
  File file("test2d.h5", File::read);
  auto *ts=file.openChildObject<VectorSerie<double> >("timeserie");
  cout<<ts->getDescription()<<endl;
  vector<double> out;
  out=ts->getColumn(1);
  for(double d : out) cout<<d<<endl;
  vector<string> outhead;
  outhead=ts->getColumnLabel();
  for(auto & d : outhead) cout<<d<<endl;
  }
  {
  File::setDefaultCacheSize(6);
  write("test2dcache.h5");
  File::setDefaultCacheSize(0);
  }
  {
  File file("test2dcache.h5", File::read);
  auto *ts=file.openChildObject<VectorSerie<double> >("timeserie");
  if(ts->getRows()!=15) {
    cerr<<"Wrong h5 file content"<<endl;
    return 1;
  }
  auto row=ts->getRow(0);
  if(row[0]!=1.2 || row[1]!=2.3 || row[2]!=3.4) {
    cerr<<"Wrong h5 file content"<<endl;
    return 1;
  }
  row=ts->getRow(14);
  if(row[0]!=1.2+14 || row[1]!=2.3+14 || row[2]!=3.4+14) {
    cerr<<"Wrong h5 file content"<<endl;
    return 1;
  }
  }




//  /***** FMATVEC *****/
//  cout<<"FMATVEC\n";
//  {
//  H5File file("test.h5", H5F_ACC_TRUNC);
//  Vec data("[1.2;2.3;9.9]");
//  SimpleDataset<vector<double> > d(file, "mydata", data);
//  Vec out=SimpleDataset<vector<double> >::getData(file, "mydata");
//  file.close();
//  cout<<out<<endl;
//  }

  /***** Attribute vector<vector<double>> *****/
  cout<<"Attribute vector<vector<double>>\n";
  {
  File file("test.h5", writeType);
  SimpleDataset<double> *d=file.createChildObject<SimpleDataset<double> >("d")();
  d->write(3.56);
  SimpleAttribute<vector<vector<double> > > *a=d->createChildAttribute<SimpleAttribute<vector<vector<double> > > >("a")(2,3);
  vector<double> d1; d1.push_back(1); d1.push_back(2); d1.push_back(3);
  vector<double> d2; d2.push_back(4); d2.push_back(5); d2.push_back(6);
  vector<vector<double> > data; data.push_back(d1); data.push_back(d2);
  a->write(data);
  vector<vector<double> > out;
  out=a->read();
  for(auto & r : out)
    for(double c : r)
      cout<<c<<endl;
  if(callEnableSWMR)
    file.enableSWMR();
  }

  /***** Attribute vector<vector<string>> *****/
  cout<<"Attribute vector<vector<string>>\n";
  {
  File file("test.h5", writeType);
  SimpleDataset<double> *d=file.createChildObject<SimpleDataset<double> >("d")();
  SimpleAttribute<vector<vector<string> > > *a=d->createChildAttribute<SimpleAttribute<vector<vector<string> > > >("a")(2,3);
  vector<string> d1; d1.emplace_back("a"); d1.emplace_back("bb"); d1.emplace_back("ccc");
  vector<string> d2; d2.emplace_back("d"); d2.emplace_back("ee"); d2.emplace_back("fff");
  vector<vector<string> > data; data.push_back(d1); data.push_back(d2);
  a->write(data);
  vector<vector<string> > out;
  out=a->read();
  for(auto & r : out)
    for(auto & c : r)
      cout<<c<<endl;
  if(callEnableSWMR)
    file.enableSWMR();
  }

  /***** Attribute vector<vector<complex<double>>> *****/
  cout<<"Attribute vector<vector<complex<double>>>\n";
  {
  File file("test.h5", writeType);
  SimpleDataset<double> *d=file.createChildObject<SimpleDataset<double> >("d")();
  SimpleAttribute<vector<vector<complex<double>> > > *a=d->createChildAttribute<SimpleAttribute<vector<vector<complex<double>> > > >("a")(2,3);
  vector<complex<double>> d1; d1.emplace_back(2.3,3.5); d1.emplace_back(2.3,3.5); d1.emplace_back(2.3,3.5);
  vector<complex<double>> d2; d2.emplace_back(2.3,3.5); d2.emplace_back(2.3,3.5); d2.emplace_back(2.3,3.5);
  vector<vector<complex<double>> > data; data.push_back(d1); data.push_back(d2);
  a->write(data);
  vector<vector<complex<double>> > out;
  out=a->read();
  for(auto & r : out)
    for(auto & c : r)
      cout<<c<<endl;
  if(callEnableSWMR)
    file.enableSWMR();
  }

  /***** Dataset vector<vector<double>> *****/
  cout<<"Dataset vector<vector<double>>\n";
  {
  File file("test.h5", writeType);
  SimpleDataset<vector<vector<double> > > *d=file.createChildObject<SimpleDataset<vector<vector<double> > > >("d")(2, 3);
  vector<double> d1; d1.push_back(1); d1.push_back(2); d1.push_back(3);
  vector<double> d2; d2.push_back(4); d2.push_back(5); d2.push_back(6);
  vector<vector<double> > data; data.push_back(d1); data.push_back(d2);
  d->write(data);
  vector<vector<double> > out;
  out=d->read();
  for(auto & r : out)
    for(double c : r)
      cout<<c<<endl;
  if(callEnableSWMR)
    file.enableSWMR();
  }

  /***** Dataset vector<vector<string>> *****/
  cout<<"Dataset vector<vector<string>>\n";
  {
  File file("test.h5", writeType);
  SimpleDataset<vector<vector<string> > > *d=file.createChildObject<SimpleDataset<vector<vector<string> > > >("d")(2, 3);
  vector<string> d1; d1.emplace_back("a"); d1.emplace_back("bb"); d1.emplace_back("ccc");
  vector<string> d2; d2.emplace_back("d"); d2.emplace_back("ee"); d2.emplace_back("fff");
  vector<vector<string> > data; data.push_back(d1); data.push_back(d2);
  d->write(data);
  vector<vector<string> > out;
  out=d->read();
  for(auto & r : out)
    for(auto & c : r)
      cout<<c<<endl;
  if(callEnableSWMR)
    file.enableSWMR();
  }

  /***** Dataset vector<vector<complex<double>>> *****/
  cout<<"Dataset vector<vector<complex<double>>>\n";
  {
  File file("test.h5", writeType);
  SimpleDataset<vector<vector<complex<double>> > > *d=file.createChildObject<SimpleDataset<vector<vector<complex<double>> > > >("d")(2, 3);
  vector<complex<double>> d1; d1.emplace_back(3.3,6.3); d1.emplace_back(3.3,6.3); d1.emplace_back(3.3,6.3);
  vector<complex<double>> d2; d2.emplace_back(3.3,6.3); d2.emplace_back(3.3,6.3); d2.emplace_back(3.3,6.3);
  vector<vector<complex<double>> > data; data.push_back(d1); data.push_back(d2);
  d->write(data);
  vector<vector<complex<double>> > out;
  out=d->read();
  for(auto & r : out)
    for(auto & c : r)
      cout<<c<<endl;
  if(callEnableSWMR)
    file.enableSWMR();
  }

//  /***** Dataset vector<vector<double>> fmatvec *****/
//  cout<<"Dataset vector<vector<double>> fmatvec\n";
//  {
//  H5File file("test.h5", H5F_ACC_TRUNC);
//  SimpleDataset<vector<vector<double> > > d(file, "d");
//  Mat data("[1,2,3,9;4,5,6,9]");
//  d.write(data);
//  SimpleDataset<vector<vector<double> > > dr;
//  dr.open(file,"d");
//  Mat out;
//  out=dr.read();
//  cout<<out<<endl;
//  file.close();
//  }

  return 0;
}
