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

int main() {
#ifndef _WIN32
  assert(feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW)!=-1);
#endif


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
  file.enableSWMR();
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
  file.enableSWMR();
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
  file.enableSWMR();
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
  file.enableSWMR();
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
  file.enableSWMR();
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
  file.enableSWMR();
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
  file.enableSWMR();
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
  file.enableSWMR();
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
  file.enableSWMR();
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
  file.enableSWMR();
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
  file.enableSWMR();
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
  file.enableSWMR();
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
