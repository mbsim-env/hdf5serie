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
#include <hdf5serie/vectorserie.h>
//#include <hdf5serie/matrixserie.h>
//#include <hdf5serie/structserie.h>
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

int main() {


  /***** SimpleDataset *****/
  cout<<"MYDATASET\n";

  { // scalar double
  cout<<"scalar double\n";
  File file("test.h5", File::write);

  SimpleDataset<double> *dsd=file.createChildObject<SimpleDataset<double> >("dsd")();
  double d=5.67;
  dsd->write(d);
  dsd->setDescription("testdesc");
  cout<<dsd->read()<<endl;
  file.reopenAsSWMR();
  }
  {
  File file("test.h5", File::read);
  SimpleDataset<double> *dsd=file.openChildObject<SimpleDataset<double> >("dsd");
  cout<<dsd->read()<<endl;
  cout<<dsd->getDescription()<<endl;
  }
  


  { // scalar string
  cout<<"scalar string\n";
  File file("test.h5", File::write);

  SimpleDataset<string> *dsd=file.createChildObject<SimpleDataset<string> >("dsd")();
  string d="sdlfkjsf";
  dsd->write(d);
  dsd->setDescription("testdesc");
  cout<<dsd->read()<<endl;
  file.reopenAsSWMR();
  }
  {
  File file("test.h5", File::read);
  SimpleDataset<string> *dsd=file.openChildObject<SimpleDataset<string> >("dsd");
  cout<<dsd->read()<<endl;
  cout<<dsd->getDescription()<<endl;
  }









  { // vector double
  cout<<"vector double\n";
  File file("test.h5", File::write);

  SimpleDataset<vector<double> > *dsd=file.createChildObject<SimpleDataset<vector<double> > >("dsd")(2);
  vector<double> d; d.push_back(5.67); d.push_back(7.34);
  dsd->write(d);
  dsd->setDescription("testdesc");
  vector<double> dout;
  dout=dsd->read();
  for(unsigned int i=0; i<dout.size(); i++) cout<<dout[i]<<endl;
  file.reopenAsSWMR();
  }
  {
  File file("test.h5", File::read);
  SimpleDataset<vector<double> > *dsd=file.openChildObject<SimpleDataset<vector<double> > >("dsd");
  vector<double> dout;
  dout=dsd->read();
  for(unsigned int i=0; i<dout.size(); i++) cout<<dout[i]<<endl;
  cout<<dsd->getDescription()<<endl;
  }
  


  { // vector string
  cout<<"vector string\n";
  File file("test.h5", File::write);

  SimpleDataset<vector<string> > *dsd=file.createChildObject<SimpleDataset<vector<string> > >("dsd")(2);
  vector<string> d; d.push_back("sdlkfj"); d.push_back("owiuer");
  dsd->write(d);
  dsd->setDescription("testdesc");
  vector<string> dout;
  dout=dsd->read();
  for(unsigned int i=0; i<dout.size(); i++) cout<<dout[i]<<endl;
  file.reopenAsSWMR();
  }
  {
  File file("test.h5", File::read);
  SimpleDataset<vector<string> > *dsd=file.openChildObject<SimpleDataset<vector<string> > >("dsd");
  vector<string> dout;
  dout=dsd->read();
  for(unsigned int i=0; i<dout.size(); i++) cout<<dout[i]<<endl;
  cout<<dsd->getDescription()<<endl;
  }
















  /***** Attribute *****/
  cout<<"ATTRIBUTE\n";

  { // scalar double
  cout<<"scalar double\n";
  File file("test.h5", File::write);
  SimpleDataset<double> *data=file.createChildObject<SimpleDataset<double> >("data")();

  SimpleAttribute<double> *dsd=data->createChildAttribute<SimpleAttribute<double> >("dsd")();
  double d=5.67;
  dsd->write(d);
  cout<<dsd->read()<<endl;
  file.reopenAsSWMR();
  }
  {
  File file("test.h5", File::read);
  Dataset *data=file.openChildObject<SimpleDataset<double> >("data");
  SimpleAttribute<double> *dsd=data->openChildAttribute<SimpleAttribute<double> >("dsd");
  cout<<dsd->read()<<endl;
  }
  


  { // scalar string
  cout<<"scalar string\n";
  File file("test.h5", File::write);
  SimpleDataset<double> *data=file.createChildObject<SimpleDataset<double> >("data")();

  SimpleAttribute<string> *dsd=data->createChildAttribute<SimpleAttribute<string> >("dsd")();
  string d="sdlfkjsf";
  dsd->write(d);
  cout<<dsd->read()<<endl;
  file.reopenAsSWMR();
  }
  {
  File file("test.h5", File::read);
  Dataset *data=file.openChildObject<SimpleDataset<double> >("data");
  SimpleAttribute<string> *dsd=data->openChildAttribute<SimpleAttribute<string> >("dsd");
  cout<<dsd->read()<<endl;
  }









  { // vector double
  cout<<"vector double\n";
  File file("test.h5", File::write);
  SimpleDataset<double> *data=file.createChildObject<SimpleDataset<double> >("data")();

  constexpr int size=10000;
  SimpleAttribute<vector<double> > *dsd=data->createChildAttribute<SimpleAttribute<vector<double> > >("dsd")(size);
  vector<double> d;
  for(int i=0; i<size; ++i)
    d.push_back(2*i);
  dsd->write(d);
  vector<double> dout;
  dout=dsd->read();
  for(unsigned int i=0; i<min(dout.size(), static_cast<size_t>(20)); i++) cout<<dout[i]<<endl;
  file.reopenAsSWMR();
  }
  {
  File file("test.h5", File::read);
  Dataset *data=file.openChildObject<SimpleDataset<double> >("data");
  SimpleAttribute<vector<double> > *dsd=data->openChildAttribute<SimpleAttribute<vector<double> > >("dsd");
  vector<double> dout;
  dout=dsd->read();
  for(unsigned int i=0; i<min(dout.size(), static_cast<size_t>(20)); i++) cout<<dout[i]<<endl;
  }
  


  { // vector string
  cout<<"vector string\n";
  File file("test.h5", File::write);
  SimpleDataset<double> *data=file.createChildObject<SimpleDataset<double> >("data")();

  // test a large attribute (>64K). This should automatically switch to dense attribute storge
  constexpr int size=5000;
  SimpleAttribute<vector<string> > *dsd=data->createChildAttribute<SimpleAttribute<vector<string> > >("dsd")(size);
  vector<string> d;
  for(int i=0; i<size; ++i)
    d.push_back("large attribute test "+to_string(i));
  dsd->write(d);
  vector<string> dout;
  dout=dsd->read();
  for(unsigned int i=0; i<min(dout.size(), static_cast<size_t>(20)); i++) cout<<dout[i]<<endl;
  file.reopenAsSWMR();
  }
  {
  File file("test.h5", File::read);
  Dataset *data=file.openChildObject<SimpleDataset<double> >("data");
  SimpleAttribute<vector<string> > *dsd=data->openChildAttribute<SimpleAttribute<vector<string> > >("dsd");
  vector<string> dout;
  dout=dsd->read();
  for(unsigned int i=0; i<min(dout.size(), static_cast<size_t>(20)); i++) cout<<dout[i]<<endl;
  }



  /***** MYTIMESERIE *****/
  cout<<"TIMESERIE\n";
  {
  File file("test2d.h5", File::write);
  VectorSerie<double> *ts=file.createChildObject<VectorSerie<double> >("timeserie")(3);
  vector<string> colhead;
  colhead.push_back("col1");
  colhead.push_back("col22");
  colhead.push_back("col333");
  ts->setColumnLabel(colhead);
  ts->setDescription("mydesctipsldfk");
  vector<double> data;
  data.push_back(1.2);
  data.push_back(2.3);
  data.push_back(3.4);
  ts->append(data);
  ts->append(data);
//  fmatvec::Vec v(3);
//  ts->append(v);
  vector<double> out;
  out=ts->getRow(1);
  ts->getRow(2, out);
  for(unsigned int i=0; i<out.size(); i++) cout<<out[i]<<endl;
  out=ts->getColumn(1);
  for(unsigned int i=0; i<out.size(); i++) cout<<out[i]<<endl;
  cout<<ts->getDescription()<<endl;
  vector<string> outhead;
  outhead=ts->getColumnLabel();
  for(unsigned int i=0; i<outhead.size(); i++) cout<<outhead[i]<<endl;
  VectorSerie<string> *tsStr=file.createChildObject<VectorSerie<string> >("timeserieStr")(3);
  vector<string> dataStr;
  dataStr.push_back("a");
  dataStr.push_back("b");
  dataStr.push_back("c");
  tsStr->append(dataStr);
  file.reopenAsSWMR();
  }
  {
  File file("test2d.h5", File::read);
  VectorSerie<double> *ts=file.openChildObject<VectorSerie<double> >("timeserie");
  cout<<ts->getDescription()<<endl;
  vector<double> out;
  out=ts->getColumn(1);
  for(unsigned int i=0; i<out.size(); i++) cout<<out[i]<<endl;
  vector<string> outhead;
  outhead=ts->getColumnLabel();
  for(unsigned int i=0; i<outhead.size(); i++) cout<<outhead[i]<<endl;
  }




//  /***** MYMATRIXSERIE *****/
//  cout<<"MATRIXSERIE\n";
//  {
//  H5File file("testmat.h5", H5F_ACC_TRUNC);
//  MatrixSerie<double> ts;
//  ts.create(file, "matserie", 4, 5);
//  ts.setDescription("mydesctipsldfk");
//  vector<vector<double> > mat;
//  vector<double> row;
//  row.push_back(1.2);
//  row.push_back(2.3);
//  row.push_back(3.4);
//  row.push_back(3.4);
//  row.push_back(3.4);
//  mat.push_back(row);
//  mat.push_back(row);
//  mat.push_back(row);
//  mat.push_back(row);
//  ts.append(mat);
//  ts.append(mat);
//  vector<vector<double> > out;
//  out=ts.getMatrix(0);
//  for(unsigned int r=0; r<out.size(); r++) { for(unsigned int c=0; c<out[r].size(); c++) cout<<out[r][c]<<" "; cout<<endl; }
//  out=ts.getMatrix(1);
//  for(unsigned int r=0; r<out.size(); r++) { for(unsigned int c=0; c<out[r].size(); c++) cout<<out[r][c]<<" "; cout<<endl; }
//  cout<<ts.getDescription()<<endl;
//  }
//  {
//  H5File file("testmat.h5", H5F_ACC_RDWR);
//  MatrixSerie<double> ts;
//  ts.open(file, "matserie");
//  cout<<ts.getDescription()<<endl;
//  vector<vector<double> > mat;
//  vector<double> row;
//  row.push_back(1.2);
//  row.push_back(2.3);
//  row.push_back(3.4);
//  row.push_back(3.4);
//  row.push_back(3.4);
//  mat.push_back(row);
//  mat.push_back(row);
//  mat.push_back(row);
//  mat.push_back(row);
//  ts.append(mat);
//  cout<<ts.getNumberOfMatrices()<<endl;
//  }




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
  File file("test.h5", File::write);
  SimpleDataset<double> *d=file.createChildObject<SimpleDataset<double> >("d")();
  d->write(3.56);
  SimpleAttribute<vector<vector<double> > > *a=d->createChildAttribute<SimpleAttribute<vector<vector<double> > > >("a")(2,3);
  vector<double> d1; d1.push_back(1); d1.push_back(2); d1.push_back(3);
  vector<double> d2; d2.push_back(4); d2.push_back(5); d2.push_back(6);
  vector<vector<double> > data; data.push_back(d1); data.push_back(d2);
  a->write(data);
  vector<vector<double> > out;
  out=a->read();
  for(unsigned int r=0; r<out.size(); r++)
    for(unsigned int c=0; c<out[r].size(); c++)
      cout<<out[r][c]<<endl;
  file.reopenAsSWMR();
  }

  /***** Attribute vector<vector<string>> *****/
  cout<<"Attribute vector<vector<string>>\n";
  {
  File file("test.h5", File::write);
  SimpleDataset<double> *d=file.createChildObject<SimpleDataset<double> >("d")();
  SimpleAttribute<vector<vector<string> > > *a=d->createChildAttribute<SimpleAttribute<vector<vector<string> > > >("a")(2,3);
  vector<string> d1; d1.push_back("a"); d1.push_back("bb"); d1.push_back("ccc");
  vector<string> d2; d2.push_back("d"); d2.push_back("ee"); d2.push_back("fff");
  vector<vector<string> > data; data.push_back(d1); data.push_back(d2);
  a->write(data);
  vector<vector<string> > out;
  out=a->read();
  for(unsigned int r=0; r<out.size(); r++)
    for(unsigned int c=0; c<out[r].size(); c++)
      cout<<out[r][c]<<endl;
  file.reopenAsSWMR();
  }

  /***** Dataset vector<vector<double>> *****/
  cout<<"Dataset vector<vector<double>>\n";
  {
  File file("test.h5", File::write);
  SimpleDataset<vector<vector<double> > > *d=file.createChildObject<SimpleDataset<vector<vector<double> > > >("d")(2, 3);
  vector<double> d1; d1.push_back(1); d1.push_back(2); d1.push_back(3);
  vector<double> d2; d2.push_back(4); d2.push_back(5); d2.push_back(6);
  vector<vector<double> > data; data.push_back(d1); data.push_back(d2);
  d->write(data);
  vector<vector<double> > out;
  out=d->read();
  for(unsigned int r=0; r<out.size(); r++)
    for(unsigned int c=0; c<out[r].size(); c++)
      cout<<out[r][c]<<endl;
  file.reopenAsSWMR();
  }

  /***** Dataset vector<vector<string>> *****/
  cout<<"Dataset vector<vector<string>>\n";
  {
  File file("test.h5", File::write);
  SimpleDataset<vector<vector<string> > > *d=file.createChildObject<SimpleDataset<vector<vector<string> > > >("d")(2, 3);
  vector<string> d1; d1.push_back("a"); d1.push_back("bb"); d1.push_back("ccc");
  vector<string> d2; d2.push_back("d"); d2.push_back("ee"); d2.push_back("fff");
  vector<vector<string> > data; data.push_back(d1); data.push_back(d2);
  d->write(data);
  vector<vector<string> > out;
  out=d->read();
  for(unsigned int r=0; r<out.size(); r++)
    for(unsigned int c=0; c<out[r].size(); c++)
      cout<<out[r][c]<<endl;
  file.reopenAsSWMR();
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


//  cout<<"SERIE1D\n";
//  {
//  H5File file("testcompound.h5", H5F_ACC_TRUNC);
//  StructSerie<MyStruct> s1;
//  MyStruct data;
//  s1.registerMember(data, data.d, "mydouble");
//  s1.registerMember(data, data.v, 3, "myvector");
//  s1.registerMember(data, data.vs, 3, "myvectorstring");
//  s1.registerMember(data, data.f, "myfloat");
//  s1.registerMember(data, data.s, "mystring");
//  s1.registerMember(data, data.i, "myint");
//  s1.create(file, "serie1d");
//  s1.setDescription("myserie1ddesc");
//  cout<<s1.getDescription()<<endl;
//  cout<<s1.getRows()<<endl;
//  cout<<s1.getMembers()<<endl;
//  vector<string> lab;
//  lab=s1.getMemberLabel();
//  for(unsigned int i=0; i<lab.size(); i++)
//    cout<<lab[i]<<endl;
//  vector<int> vv; vv.push_back(1111); vv.push_back(2222); vv.push_back(3333);
//  vector<string> vvstr; vvstr.push_back("str1"); vvstr.push_back("str2"); vvstr.push_back("str3");
//  data.d=6.1; data.f=1.2; data.s="teststr"; data.v=vv; data.vs=vvstr; data.i=4;
//  s1.append(data);
//  data.d=7.1; data.f=2.2; data.s="teststr2"; data.v=vv; data.vs=vvstr; data.i=5;
//  s1.append(data);
//  file.close();
//  }
//  {
//  H5File file("testcompound.h5", H5F_ACC_RDONLY);
//  StructSerie<MyStruct> s1;
//  MyStruct data;
//  s1.registerMember(data, data.d, "mydouble");
//  s1.registerMember(data, data.v, 3, "myvector");
//  s1.registerMember(data, data.vs, 3, "myvectorstring");
//  s1.registerMember(data, data.f, "myfloat");
//  s1.registerMember(data, data.s, "mystring");
//  s1.registerMember(data, data.i, "myint");
//  s1.open(file, "serie1d");
//  MyStruct out;
//  out=s1.getRow(0);
//  cout<<out.d<<endl;
//  for(unsigned int i=0; i<out.v.size(); i++) 
//    cout<<out.v[i]<<endl;
//  for(unsigned int i=0; i<out.vs.size(); i++) 
//    cout<<out.vs[i]<<endl;
//  cout<<out.f<<endl;
//  cout<<out.s<<endl;
//  cout<<out.i<<endl;
//  file.close();
//  }


  return 0;
}
