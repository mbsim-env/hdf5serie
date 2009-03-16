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
