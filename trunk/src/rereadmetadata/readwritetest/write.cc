#include <config.h>
#include <serie2d.h>
#include <iostream>

using namespace std;
using namespace H5;

int main() {
  H5File file("test.h5", H5F_ACC_TRUNC);

  Serie2D<double> ts(file, "ts", vector<string>(4));
  vector<double> data(4);
  int i=0;
  while(1) {
    data[0]=i;
    data[1]=i/10;
    data[2]=i/100;
    data[3]=i/1000;
    ts.append(data);
    file.flush(H5F_SCOPE_GLOBAL);
    cout<<i<<endl;
    sleep(1);
    i++;
  }

  file.close();
}
