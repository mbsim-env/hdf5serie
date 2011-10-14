/* Copyright (C) 2011 Markus Friedrich
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

#include <iostream>
#include <string.h>
#include <fstream>
#include <H5Cpp.h>
#include <simpleattribute.h>

using namespace std;
using namespace H5;

void walkH5(string indent, string path, H5Object *obj);
void printhelp();
void printDesc(string indent, H5Object *obj);
void printLabel(string indent, H5Object *obj);

bool d=false, l=false, f=false, h=false;

int main(int argc, char *argv[]) {
  for(int i=1; i<argc; i++) {
    if(strcmp(argv[i], "-d")==0) d=true;
    if(strcmp(argv[i], "-l")==0) l=true;
    if(strcmp(argv[i], "-f")==0) f=true;
    if(strcmp(argv[i], "-h")==0 || strcmp(argv[i], "--help")==0) h=true;
  }

  if(h || argc<=1) {
    printhelp();
    return 0;
  }

  for(int i=1; i<argc; i++) {
    string filename=argv[i];
    ifstream f(filename.c_str());
    bool good=f.good();
    f.close();
    if(good)
    {
      H5File *file=new H5File(filename, H5F_ACC_RDONLY);
      walkH5("", filename, (H5Object*)file);
      delete file;
    }
  }

  return 0;
}

void walkH5(string indent, string path, H5Object *obj) {
  for(size_t i=0; i<((Group*)obj)->getNumObjs(); i++) {
    string name=((Group*)obj)->getObjnameByIdx(i);
    Group nextobj;
    string link;
    char *buff;
    DataSet ds;
    int ind;
    switch(((Group*)obj)->getObjTypeByIdx(i)) {

      case H5G_GROUP:
        // print and walk
        cout<<indent<<"+ "<<name<<endl;
        nextobj=((Group*)obj)->openGroup(name);
        printDesc(indent, &nextobj);
        walkH5(indent+"  ", path+"/"+name, &nextobj);
        break;

      case H5G_LINK:
      case H5G_UDLINK:
        // get link name
        H5L_info_t link_buff;
        H5Lget_info(obj->getId(), name.c_str(), &link_buff, H5P_DEFAULT);
        buff=new char[link_buff.u.val_size];
        H5Lget_val(obj->getId(), name.c_str(), buff, link_buff.u.val_size, H5P_DEFAULT);
        const char *filename;
        const char *obj_path;
        H5Lunpack_elink_val(buff, link_buff.u.val_size, NULL, &filename, &obj_path);
        link=path;
        ind=link.find_last_of('/');
        link=ind>=0?link.substr(0,ind):".";
        // print and walk
        cout<<indent<<"* "<<name<<" (External Link: \""<<link<<obj_path<<filename<<"\")"<<endl;
        delete[]buff;
        nextobj=((Group*)obj)->openGroup(name);
        printDesc(indent, &nextobj);
        if(f) walkH5(indent+"  ", path+"/"+name, &nextobj);
        break;

      case H5G_DATASET:
        // print
        cout<<indent<<"- "<<name<<" (Path: \""<<path<<"/data\")"<<endl;
        ds=((Group*)obj)->openDataSet(name);
        printLabel(indent, &ds);
        printDesc(indent, &ds);
        break;

      default:
        break;
    }
  }
}

void printhelp() {
cout<<
"h5lsserie"<<endl<<
""<<endl<<
"Lists the groups and datasets in a hdf5 file."<<endl<<
""<<endl<<
"Copyright (C) 2009 Markus Friedrich <friedrich.at.gc@googlemail.com>"<<endl<<
"This is free software; see the source for copying conditions. There is NO"<<endl<<
"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."<<endl<<
""<<endl<<
"Licensed under the GNU Lesser General Public License (LGPL)"<<endl<<
""<<endl<<
"Usage:"<<endl<<
"  h5lsserie [-d] [-l] [-f] [-h|--help] <file.h5> ..."<<endl<<
"    -h, --help: Show this help"<<endl<<
"    -d:         Show 'Description' attribute"<<endl<<
"    -l:         Show 'Column/Member Label'"<<endl<<
"    -f:         Follow external links"<<endl;
}

void printDesc(string indent, H5Object *obj) {
  if(d==false) return;

  // save and disable c error printing
  H5E_auto2_t func;
  void* client_data;
  Exception::getAutoPrint(func, &client_data);
  Exception::dontPrint();
  string ret;
  // catch error if Attribute is not found
  try {
    ret=SimpleAttribute<string>::getData(*obj, "Description");
  }
  catch(AttributeIException e) {
    ret=string();
  }
  // restore c error printing
  Exception::setAutoPrint(func, client_data);

  if(ret!="")
    cout<<indent<<"  Description: \""<<ret<<"\""<<endl;
}

void printLabel(string indent, H5Object *obj) {
  if(l==false) return;

  // save and disable c error printing
  H5E_auto2_t func;
  void* client_data;
  Exception::getAutoPrint(func, &client_data);
  Exception::dontPrint();
  vector<string> ret;
  // catch error if Attribute is not found
  try {
    ret=SimpleAttribute<vector<string> >::getData(*obj, "Column Label");
  }
  catch(AttributeIException e) {
    ret=vector<string>(0);
  }
  // restore c error printing
  Exception::setAutoPrint(func, client_data);

  if(ret.size()>0) {
    cout<<indent<<"  Column Label: ";
    for(size_t i=0; i<ret.size(); i++)
      cout<<"\""<<ret[i]<<"\" ";
    cout<<endl;
  }
}
