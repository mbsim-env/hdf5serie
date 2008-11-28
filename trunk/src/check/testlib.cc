#include <config.h>
#include <H5Cpp.h>
#include <serie2d.h>
#include <serie1d.h>
#include <simpleattribute.h>
#include <simpledataset.h>
#include <iostream>
//#include <fmatvec.h>

using namespace H5;
using namespace std;
//using namespace fmatvec;

#pragma pack(push)
#pragma pack(1)
struct MyStruct {
  double d;
  float f;
  int i;
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
struct MyStruct2 {
  double d;
  float f;
  string s;
  int i;
};
#pragma pack(pop)

int main() {


  /***** SimpleDataSet *****/
  cout<<"MYDATASET\n";

  { // scalar double
  cout<<"scalar double\n";
  H5File file("test.h5", H5F_ACC_TRUNC);

  SimpleDataSet<double> dsd;
  dsd.create(file, "dsd");
  double d=5.67;
  dsd.write(d);
  dsd.setDescription("testdesc");
  cout<<dsd.read()<<endl;

  SimpleDataSet<double> dsd2;
  double d2=5.671;
  dsd2.write(file, "dsd2", d2);
  cout<<dsd2.read()<<endl;

  double d3=76.24;
  SimpleDataSet<double> dsd3(file, "dsd3", d3);
  cout<<dsd3.read()<<endl;

  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  SimpleDataSet<double> dsd;
  dsd.open(file, "dsd");
  cout<<dsd.read()<<endl;
  cout<<dsd.getDescription()<<endl;
  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  SimpleDataSet<double> dsd;
  cout<<dsd.read(file, "dsd")<<endl;
  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  cout<<SimpleDataSet<double>::getData(file, "dsd")<<endl;
  file.close();
  }
  


  { // scalar string
  cout<<"scalar string\n";
  H5File file("test.h5", H5F_ACC_TRUNC);

  SimpleDataSet<string> dsd;
  dsd.create(file, "dsd");
  string d="sdlfkjsf";
  dsd.write(d);
  dsd.setDescription("testdesc");
  cout<<dsd.read()<<endl;

  SimpleDataSet<string> dsd2;
  string d2="slfjdskhdgfgl";
  dsd2.write(file, "dsd2", d2);
  cout<<dsd2.read()<<endl;

  string d3="sewru";
  SimpleDataSet<string> dsd3(file, "dsd3", d3);
  cout<<dsd3.read()<<endl;

  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  SimpleDataSet<string> dsd;
  dsd.open(file, "dsd");
  cout<<dsd.read()<<endl;
  cout<<dsd.getDescription()<<endl;
  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  SimpleDataSet<string> dsd;
  cout<<dsd.read(file, "dsd")<<endl;
  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  cout<<SimpleDataSet<string>::getData(file, "dsd")<<endl;
  file.close();
  }









  { // vector double
  cout<<"vector double\n";
  H5File file("test.h5", H5F_ACC_TRUNC);

  SimpleDataSet<vector<double> > dsd;
  dsd.create(file, "dsd");
  vector<double> d; d.push_back(5.67); d.push_back(7.34);
  dsd.write(d);
  dsd.setDescription("testdesc");
  vector<double> dout;
  dout=dsd.read();
  for(int i=0; i<dout.size(); i++) cout<<dout[i]<<endl;

  SimpleDataSet<vector<double> > dsd2;
  vector<double> d2; d2.push_back(5.671); d2.push_back(7.341);
  dsd2.write(file, "dsd2", d2);
  vector<double> dout2;
  dout2=dsd2.read();
  for(int i=0; i<dout2.size(); i++) cout<<dout2[i]<<endl;

  vector<double> d3; d3.push_back(5.671); d3.push_back(7.341);
  SimpleDataSet<vector<double> > dsd3(file, "dsd3", d3);
  vector<double> dout3;
  dout3=dsd3.read();
  for(int i=0; i<dout3.size(); i++) cout<<dout3[i]<<endl;

  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  SimpleDataSet<vector<double> > dsd;
  dsd.open(file, "dsd");
  vector<double> dout;
  dout=dsd.read();
  for(int i=0; i<dout.size(); i++) cout<<dout[i]<<endl;
  cout<<dsd.getDescription()<<endl;
  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  SimpleDataSet<vector<double> > dsd;
  vector<double> dout;
  dout=dsd.read(file, "dsd");
  for(int i=0; i<dout.size(); i++) cout<<dout[i]<<endl;
  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  vector<double> dout;
  dout=SimpleDataSet<vector<double> >::getData(file, "dsd");
  for(int i=0; i<dout.size(); i++) cout<<dout[i]<<endl;
  file.close();
  }
  


  { // vector string
  cout<<"vector string\n";
  H5File file("test.h5", H5F_ACC_TRUNC);

  SimpleDataSet<vector<string> > dsd;
  dsd.create(file, "dsd");
  vector<string> d; d.push_back("sdlkfj"); d.push_back("owiuer");
  dsd.write(d);
  dsd.setDescription("testdesc");
  vector<string> dout;
  dout=dsd.read();
  for(int i=0; i<dout.size(); i++) cout<<dout[i]<<endl;

  SimpleDataSet<vector<string> > dsd2;
  vector<string> d2; d2.push_back("xc.vmvcn"); d2.push_back("slkdfj");
  dsd2.write(file, "dsd2", d2);
  vector<string> dout2;
  dout2=dsd2.read();
  for(int i=0; i<dout2.size(); i++) cout<<dout2[i]<<endl;

  vector<string> d3; d3.push_back("eowriu"); d3.push_back("sjoinxdfd");
  SimpleDataSet<vector<string> > dsd3(file, "dsd3", d3);
  vector<string> dout3;
  dout3=dsd3.read();
  for(int i=0; i<dout3.size(); i++) cout<<dout3[i]<<endl;

  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  SimpleDataSet<vector<string> > dsd;
  dsd.open(file, "dsd");
  vector<string> dout;
  dout=dsd.read();
  for(int i=0; i<dout.size(); i++) cout<<dout[i]<<endl;
  cout<<dsd.getDescription()<<endl;
  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  SimpleDataSet<vector<string> > dsd;
  vector<string> dout;
  dout=dsd.read(file, "dsd");
  for(int i=0; i<dout.size(); i++) cout<<dout[i]<<endl;
  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  vector<string> dout;
  dout=SimpleDataSet<vector<string> >::getData(file, "dsd");
  for(int i=0; i<dout.size(); i++) cout<<dout[i]<<endl;
  file.close();
  }
















  /***** Attribute *****/
  cout<<"ATTRIBUTE\n";

  { // scalar double
  cout<<"scalar double\n";
  H5File file("test.h5", H5F_ACC_TRUNC);
  SimpleDataSet<double> data(file, "data", 1.234);

  SimpleAttribute<double> dsd;
  dsd.create(data, "dsd");
  double d=5.67;
  dsd.write(d);
  cout<<dsd.read()<<endl;

  SimpleAttribute<double> dsd2;
  double d2=5.671;
  dsd2.write(data, "dsd2", d2);
  cout<<dsd2.read()<<endl;

  double d3=76.24;
  SimpleAttribute<double> dsd3(data, "dsd3", d3);
  cout<<dsd3.read()<<endl;

  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  DataSet data=file.openDataSet("data");
  SimpleAttribute<double> dsd;
  dsd.open(data, "dsd");
  cout<<dsd.read()<<endl;
  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  DataSet data=file.openDataSet("data");
  SimpleAttribute<double> dsd;
  cout<<dsd.read(data, "dsd")<<endl;
  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  DataSet data=file.openDataSet("data");
  cout<<SimpleAttribute<double>::getData(data, "dsd")<<endl;
  file.close();
  }
  


  { // scalar string
  cout<<"scalar string\n";
  H5File file("test.h5", H5F_ACC_TRUNC);
  SimpleDataSet<double> data(file, "data", 1.234);

  SimpleAttribute<string> dsd;
  dsd.create(data, "dsd");
  string d="sdlfkjsf";
  dsd.write(d);
  cout<<dsd.read()<<endl;

  SimpleAttribute<string> dsd2;
  string d2="slfjdskhdgfgl";
  dsd2.write(data, "dsd2", d2);
  cout<<dsd2.read()<<endl;

  string d3="sewru";
  SimpleAttribute<string> dsd3(data, "dsd3", d3);
  cout<<dsd3.read()<<endl;

  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  DataSet data=file.openDataSet("data");
  SimpleAttribute<string> dsd;
  dsd.open(data, "dsd");
  cout<<dsd.read()<<endl;
  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  DataSet data=file.openDataSet("data");
  SimpleAttribute<string> dsd;
  cout<<dsd.read(data, "dsd")<<endl;
  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  DataSet data=file.openDataSet("data");
  cout<<SimpleAttribute<string>::getData(data, "dsd")<<endl;
  file.close();
  }









  { // vector double
  cout<<"vector double\n";
  H5File file("test.h5", H5F_ACC_TRUNC);
  SimpleDataSet<double> data(file, "data", 1.234);

  SimpleAttribute<vector<double> > dsd;
  dsd.create(data, "dsd", 2);
  vector<double> d; d.push_back(5.67); d.push_back(7.34);
  dsd.write(d);
  vector<double> dout;
  dout=dsd.read();
  for(int i=0; i<dout.size(); i++) cout<<dout[i]<<endl;

  SimpleAttribute<vector<double> > dsd2;
  vector<double> d2; d2.push_back(5.671); d2.push_back(7.341);
  dsd2.write(data, "dsd2", d2);
  vector<double> dout2;
  dout2=dsd2.read();
  for(int i=0; i<dout2.size(); i++) cout<<dout2[i]<<endl;

  vector<double> d3; d3.push_back(5.671); d3.push_back(7.341);
  SimpleAttribute<vector<double> > dsd3(data, "dsd3", d3);
  vector<double> dout3;
  dout3=dsd3.read();
  for(int i=0; i<dout3.size(); i++) cout<<dout3[i]<<endl;

  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  DataSet data=file.openDataSet("data");
  SimpleAttribute<vector<double> > dsd;
  dsd.open(data, "dsd");
  vector<double> dout;
  dout=dsd.read();
  for(int i=0; i<dout.size(); i++) cout<<dout[i]<<endl;
  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  DataSet data=file.openDataSet("data");
  SimpleAttribute<vector<double> > dsd;
  vector<double> dout;
  dout=dsd.read(data, "dsd");
  for(int i=0; i<dout.size(); i++) cout<<dout[i]<<endl;
  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  DataSet data=file.openDataSet("data");
  vector<double> dout;
  dout=SimpleAttribute<vector<double> >::getData(data, "dsd");
  for(int i=0; i<dout.size(); i++) cout<<dout[i]<<endl;
  file.close();
  }
  


  { // vector string
  cout<<"vector string\n";
  H5File file("test.h5", H5F_ACC_TRUNC);
  SimpleDataSet<double> data(file, "data", 1.234);

  SimpleAttribute<vector<string> > dsd;
  dsd.create(data, "dsd", 2);
  vector<string> d; d.push_back("sdlkfj"); d.push_back("owiuer");
  dsd.write(d);
  vector<string> dout;
  dout=dsd.read();
  for(int i=0; i<dout.size(); i++) cout<<dout[i]<<endl;

  SimpleAttribute<vector<string> > dsd2;
  vector<string> d2; d2.push_back("xc.vmvcn"); d2.push_back("slkdfj");
  dsd2.write(data, "dsd2", d2);
  vector<string> dout2;
  dout2=dsd2.read();
  for(int i=0; i<dout2.size(); i++) cout<<dout2[i]<<endl;

  vector<string> d3; d3.push_back("eowriu"); d3.push_back("sjoinxdfd");
  SimpleAttribute<vector<string> > dsd3(data, "dsd3", d3);
  vector<string> dout3;
  dout3=dsd3.read();
  for(int i=0; i<dout3.size(); i++) cout<<dout3[i]<<endl;

  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  DataSet data=file.openDataSet("data");
  SimpleAttribute<vector<string> > dsd;
  dsd.open(data, "dsd");
  vector<string> dout;
  dout=dsd.read();
  for(int i=0; i<dout.size(); i++) cout<<dout[i]<<endl;
  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  DataSet data=file.openDataSet("data");
  SimpleAttribute<vector<string> > dsd;
  vector<string> dout;
  dout=dsd.read(data, "dsd");
  for(int i=0; i<dout.size(); i++) cout<<dout[i]<<endl;
  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  DataSet data=file.openDataSet("data");
  vector<string> dout;
  dout=SimpleAttribute<vector<string> >::getData(data, "dsd");
  for(int i=0; i<dout.size(); i++) cout<<dout[i]<<endl;
  file.close();
  }



  /***** MYTIMESERIE *****/
  cout<<"TIMESERIE\n";
  {
  H5File file("test.h5", H5F_ACC_TRUNC);
  Serie2D<double> ts;
  vector<string> colhead;
  colhead.push_back("col1");
  colhead.push_back("col22");
  colhead.push_back("col333");
  ts.create(file, "timeserie", colhead);
  ts.setDescription("mydesctipsldfk");
  vector<double> data;
  data.push_back(1.2);
  data.push_back(2.3);
  data.push_back(3.4);
  ts.append(data);
  ts.append(data);
  vector<double> out;
  out=ts.getRow(1);
  for(int i=0; i<out.size(); i++) cout<<out[i]<<endl;
  out=ts.getColumn(1);
  for(int i=0; i<out.size(); i++) cout<<out[i]<<endl;
  cout<<ts.getDescription()<<endl;
  vector<string> outhead;
  outhead=ts.getColumnLabel();
  for(int i=0; i<outhead.size(); i++) cout<<outhead[i]<<endl;
  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDWR);
  Serie2D<double> ts;
  ts.open(file, "timeserie");
  cout<<ts.getDescription()<<endl;
  vector<double> data;
  data.push_back(1.2);
  data.push_back(2.3);
  data.push_back(3.4);
  ts.append(data);
  vector<double> out;
  out=ts.getColumn(1);
  for(int i=0; i<out.size(); i++) cout<<out[i]<<endl;
  vector<string> outhead;
  outhead=ts.getColumnLabel();
  for(int i=0; i<outhead.size(); i++) cout<<outhead[i]<<endl;
  file.close();
  }




  /***** FMATVEC *****/
/*  cout<<"FMATVEC\n";
  {
  H5File file("test.h5", H5F_ACC_TRUNC);
  Vec data("[1.2;2.3;9.9]");
  SimpleDataSet<vector<double> > d(file, "mydata", data);
  Vec out=SimpleDataSet<vector<double> >::getData(file, "mydata");
  file.close();
  cout<<out<<endl;
  }*/

  /***** Attribute vector<vector<double>> *****/
  cout<<"Attribute vector<vector<double>>\n";
  {
  H5File file("test.h5", H5F_ACC_TRUNC);
  SimpleDataSet<double> d(file, "d", 3.56);
  SimpleAttribute<vector<vector<double> > > a(d, "a",2,3);
  vector<double> d1; d1.push_back(1); d1.push_back(2); d1.push_back(3);
  vector<double> d2; d2.push_back(4); d2.push_back(5); d2.push_back(6);
  vector<vector<double> > data; data.push_back(d1); data.push_back(d2);
  a.write(data);
  SimpleAttribute<vector<vector<double> > > ar;
  ar.open(d,"a");
  vector<vector<double> > out;
  out=ar.read();
  for(int r=0; r<out.size(); r++)
    for(int c=0; c<out[r].size(); c++)
      cout<<out[r][c]<<endl;
  file.close();
  }

  /***** Attribute vector<vector<string>> *****/
  cout<<"Attribute vector<vector<string>>\n";
  {
  H5File file("test.h5", H5F_ACC_TRUNC);
  SimpleDataSet<double> d(file, "d", 3.56);
  SimpleAttribute<vector<vector<string> > > a(d, "a",2,3);
  vector<string> d1; d1.push_back("a"); d1.push_back("bb"); d1.push_back("ccc");
  vector<string> d2; d2.push_back("d"); d2.push_back("ee"); d2.push_back("fff");
  vector<vector<string> > data; data.push_back(d1); data.push_back(d2);
  a.write(data);
  SimpleAttribute<vector<vector<string> > > ar;
  ar.open(d,"a");
  vector<vector<string> > out;
  out=ar.read();
  for(int r=0; r<out.size(); r++)
    for(int c=0; c<out[r].size(); c++)
      cout<<out[r][c]<<endl;
  file.close();
  }

  /***** DataSet vector<vector<double>> *****/
  cout<<"DataSet vector<vector<double>>\n";
  {
  H5File file("test.h5", H5F_ACC_TRUNC);
  SimpleDataSet<vector<vector<double> > > d(file, "d", true);
  vector<double> d1; d1.push_back(1); d1.push_back(2); d1.push_back(3);
  vector<double> d2; d2.push_back(4); d2.push_back(5); d2.push_back(6);
  vector<vector<double> > data; data.push_back(d1); data.push_back(d2);
  d.write(data);
  SimpleDataSet<vector<vector<double> > > dr;
  dr.open(file,"d");
  vector<vector<double> > out;
  out=dr.read();
  for(int r=0; r<out.size(); r++)
    for(int c=0; c<out[r].size(); c++)
      cout<<out[r][c]<<endl;
  file.close();
  }

  /***** DataSet vector<vector<string>> *****/
  cout<<"DataSet vector<vector<string>>\n";
  {
  H5File file("test.h5", H5F_ACC_TRUNC);
  SimpleDataSet<vector<vector<string> > > d(file, "d", true);
  vector<string> d1; d1.push_back("a"); d1.push_back("bb"); d1.push_back("ccc");
  vector<string> d2; d2.push_back("d"); d2.push_back("ee"); d2.push_back("fff");
  vector<vector<string> > data; data.push_back(d1); data.push_back(d2);
  d.write(data);
  SimpleDataSet<vector<vector<string> > > dr;
  dr.open(file,"d");
  vector<vector<string> > out;
  out=dr.read();
  for(int r=0; r<out.size(); r++)
    for(int c=0; c<out[r].size(); c++)
      cout<<out[r][c]<<endl;
  file.close();
  }

  /***** DataSet vector<vector<double>> fmatvec *****/
/*  cout<<"DataSet vector<vector<double>> fmatvec\n";
  {
  H5File file("test.h5", H5F_ACC_TRUNC);
  SimpleDataSet<vector<vector<double> > > d(file, "d");
  Mat data("[1,2,3,9;4,5,6,9]");
  d.write(data);
  SimpleDataSet<vector<vector<double> > > dr;
  dr.open(file,"d");
  Mat out;
  out=dr.read();
  cout<<out<<endl;
  file.close();
  }*/


  cout<<"SERIE1D\n";
  {
  H5File file("test.h5", H5F_ACC_TRUNC);
  Serie1D<MyStruct> s1;
  vector<MemberNameType> nt;
  nt.push_back(MemberNameType("mydouble", "Double"));
  nt.push_back(MemberNameType("myfloat", "Float"));
  nt.push_back(MemberNameType("myint", "Int"));
  s1.create(file, "serie1d", nt);
  MyStruct data; data.d=6.1; data.f=1.2; data.i=4;
  s1.append(data);
  data.d=7.1; data.f=2.2; data.i=5;
  s1.append(data);
  MyStruct out;
  out=s1.getRow(1);
  cout<<out.d<<" "<<out.f<<" "<<out.i<<endl;
  cout<<s1.getMembers()<<endl;
  vector<string> lab=s1.getMemberLabel();
  for(int i=0; i<lab.size(); i++)
    cout<<lab[i]<<endl;
  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_RDONLY);
  Serie1D<MyStruct> s1(file, "serie1d");
  MyStruct out;
  out=s1.getRow(1);
  cout<<out.d<<" "<<out.f<<" "<<out.i<<endl;
  vector<string> lab=s1.getMemberLabel();
  for(int i=0; i<lab.size(); i++)
    cout<<lab[i]<<endl;
  file.close();
  }
  {
  H5File file("test.h5", H5F_ACC_TRUNC);
  Serie1D<MyStruct2> s2;
  vector<MemberNameType> nt2;
  nt2.push_back(MemberNameType("mydouble", "Double"));
  nt2.push_back(MemberNameType("myfloat", "Float"));
  nt2.push_back(MemberNameType("mystring", "String"));
  nt2.push_back(MemberNameType("myint", "Int"));
  s2.create(file, "serie1dstr", nt2);
  MyStruct2 data3; data3.d=6.1; data3.f=1.2; data3.s="teststr"; data3.i=4;
  s2.append(data3);
  data3.d=7.1; data3.f=2.2; data3.s="teststr2"; data3.i=5;
  s2.append(data3);
  MyStruct2 out=s2.getRow(1);
  cout<<out.d<<endl;
  cout<<out.f<<endl;
  cout<<out.s<<endl;
  cout<<out.i<<endl;
  file.close();
  }
  








  {
  H5File file("test.h5", H5F_ACC_TRUNC);
  Serie1D<MyStruct> s1;
  vector<MemberNameType> nt;
  nt.push_back(MemberNameType("mydouble", "Double"));
  nt.push_back(MemberNameType("myfloat", "Float"));
  nt.push_back(MemberNameType("myint", "Int"));
  s1.create(file, "serie1d", nt);
  MyStruct data; data.d=6.1; data.f=1.2; data.i=4;
  s1.append(data);
  data.d=7.1; data.f=2.2; data.i=5;
  s1.append(data);

  Serie2D<double> ts;
  vector<string> colhead;
  colhead.push_back("col1");
  colhead.push_back("col22");
  colhead.push_back("col333");
  ts.create(file, "timeserie", colhead);
  ts.setDescription("mydesctipsldfk");
  vector<double> data2;
  data2.push_back(1.2);
  data2.push_back(2.3);
  data2.push_back(3.4);
  ts.append(data2);
  ts.append(data2);
  ts.append(data2);

  SimpleDataSet<double> dsd;
  dsd.create(file, "dsd");
  double d=5.67;
  dsd.write(d);

  SimpleDataSet<vector<double> > dsdv;
  dsdv.create(file, "dsdv");
  vector<double> dv; dv.push_back(5.67); dv.push_back(7.34);
  dsdv.write(dv);

  SimpleDataSet<vector<vector<double> > > dvv(file, "d", true);
  vector<double> d1; d1.push_back(1); d1.push_back(2); d1.push_back(3);
  vector<double> d2; d2.push_back(4); d2.push_back(5); d2.push_back(6);
  vector<vector<double> > datavv; datavv.push_back(d1); datavv.push_back(d2);
  dvv.write(datavv);

  Serie1D<MyStruct2> s2;
  vector<MemberNameType> nt2;
  nt2.push_back(MemberNameType("mydouble", "Double"));
  nt2.push_back(MemberNameType("myfloat", "Float"));
  nt2.push_back(MemberNameType("mystring", "String"));
  nt2.push_back(MemberNameType("myint", "Int"));
  s2.create(file, "serie1dstr", nt2);
  MyStruct2 data3; data3.d=6.1; data3.f=1.2; data3.s="teststr"; data3.i=4;
  s2.append(data3);
  data3.d=7.1; data3.f=2.2; data3.s="teststr2"; data3.i=5;
  s2.append(data3);
  MyStruct2 out=s2.getRow(1);
  cout<<out.d<<endl;
  cout<<out.f<<endl;
  cout<<out.s<<endl;
  cout<<out.i<<endl;

  file.close();
  }


  return 0;
}
