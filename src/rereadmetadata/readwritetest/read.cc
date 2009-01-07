#include <config.h>
#include <h5filero.h>
#include <vectorseriero.h>
#include <structseriero.h>
#include <iostream>

using namespace std;
using namespace H5;

struct St {
  double d;
  int i;
};

int main() {
  H5FileRO file;
  file.openFile("test.h5", H5F_ACC_RDONLY);
  GroupRO grp=file.openGroup("mygrp");
  GroupRO grp2=grp.openGroup("mygrp2");
  VectorSerieRO<double> ts;
  ts.open(grp2, "ts");
  StructSerieRO<St> ts1d;
  St s;
  ts1d.insertMember(s, s.d, "mydouble");
  ts1d.insertMember(s, s.i, "myint");
  ts1d.open(grp2, "ts1d");
  vector<double> data;
  while(1) {
    int r=ts.getRows();
    data=ts.getRow(r-1);
    cout<<data[0]<<" "<<data[1]<<" "<<data[2]<<" "<<data[3]<<endl;

    s=ts1d.getRow(r-1);
    cout<<"1D "<<s.d<<" "<<s.i<<endl;
    sleep(1);
    file.reread();
  }
  //ts.closePermanent();
  file.close();
}
