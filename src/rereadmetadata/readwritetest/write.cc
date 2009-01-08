#include <config.h>
#include <vectorserie.h>
#include <structserie.h>
#include <iostream>

using namespace std;
using namespace H5;

struct St {
  double d;
  int i;
};

int main() {
  H5File file("test.h5", H5F_ACC_TRUNC);

  Group grp=file.createGroup("mygrp");
  Group grp2=grp.createGroup("mygrp2");
  VectorSerie<double> ts(grp2, "ts", vector<string>(4));
  StructSerie<St> ts1d;
  St s;
  ts1d.registerMember(s, s.d, "mydouble");
  ts1d.registerMember(s, s.i, "myint");
  ts1d.create(grp2, "ts1d");
  vector<double> data(4);
  int i=0;
  while(1) {
    data[0]=i;
    data[1]=i/10;
    data[2]=i/100;
    data[3]=i/1000;
    ts.append(data);
    s.d=9.9;
    s.i=i;
    ts1d.append(s);
    file.flush(H5F_SCOPE_GLOBAL);
    cout<<i<<endl;
    sleep(1);
    i++;
  }

  file.close();
}
