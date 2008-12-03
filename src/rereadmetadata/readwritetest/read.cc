#include <config.h>
#include <h5rereadmetadata.h>
#include <serie2d.h>
#include <iostream>

using namespace std;
using namespace H5;

int main() {
  ////FileAccPropList fileAccProp;
  ////fileAccProp.setCache(0, 0, 0, 0);
  ////hsize_t z=0;
  ////fileAccProp.setMetaBlockSize(z);
  ////fileAccProp.setSieveBufSize(0);
  ////H5File file("test.h5", H5F_ACC_RDONLY, FileCreatPropList::DEFAULT, fileAccProp);
  //H5File file("test.h5", H5F_ACC_RDONLY);

  //Serie2D<double> ts(file, "ts");
  //vector<double> data;
  //while(1) {
  //  int r=ts.getRows();
  //  data=ts.getRow(r-1);
  //  cout<<data[0]<<" "<<data[1]<<" "<<data[2]<<" "<<data[3]<<endl;
  //  sleep(1);
  //}
  //
  //file.close();



  H5FileRO file;
  file.openFile("test.h5", H5F_ACC_RDONLY);
  GroupRO grp=file.openGroup("mygrp");
  Serie2DRO<double> ts;
  ts.open(grp, "ts");
  vector<double> data;
  while(1) {
    int r=ts.getRows();
    data=ts.getRow(r-1);
    cout<<data[0]<<" "<<data[1]<<" "<<data[2]<<" "<<data[3]<<endl;
    sleep(1);
    file.rereadMetadata();
  }
  ts.closePermanent();
  file.close();
}
