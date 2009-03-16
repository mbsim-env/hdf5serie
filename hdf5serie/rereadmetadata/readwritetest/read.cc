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
  ts1d.registerMember(s, s.d, "mydouble");
  ts1d.registerMember(s, s.i, "myint");
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
