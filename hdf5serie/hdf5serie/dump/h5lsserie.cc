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

#include <config.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <hdf5serie/file.h>
#include <hdf5serie/simpleattribute.h>
#include <boost/filesystem.hpp>

using namespace std;
using namespace H5;
using namespace boost::filesystem;

void walkH5(const string &indent, const path &filename, const string &path, GroupBase *obj);
void printhelp();
void printDesc(string indent, Object *obj);
void printLabel(string indent, Dataset *d);

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
    std::ifstream f(filename.c_str());
    bool good=f.good();
    f.close();
    if(good)
    {
      File file(filename, File::read);
      walkH5("", filename, "", &file);
    }
  }

  return 0;
}

void walkH5(const string &indent, const path &filename, const string &path, GroupBase *obj) {
  set<string> names=obj->getChildObjectNames();
  for(set<string>::iterator it=names.begin(); it!=names.end(); ++it) {
    string name=*it;
    if(obj->isExternalLink(name) && !f) {
      pair<boost::filesystem::path, string> link=obj->getExternalLink(name);
      cout<<indent<<"* "<<name<<" (External Link: \""<<link.first.string()<<link.second<<"\")"<<endl;
      continue;
    }

    Object *child=obj->openChildObject(name);

    Group *g=dynamic_cast<Group*>(child);
    if(g) {
      // print and walk
      cout<<indent<<"+ "<<name<<endl;
      printDesc(indent, g);
      walkH5(indent+"  ", filename, path+"/"+name, g);
      continue;
    }

    Dataset *d=dynamic_cast<Dataset*>(child);
    if(d) {
      // print
      cout<<indent<<"- "<<name<<" (Path: \""<<filename.string()<<path<<"/"<<name<<"\")"<<endl;
      printLabel(indent, d);
      printDesc(indent, d);
      continue;
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

void printDesc(string indent, Object *obj) {
  if(d==false) return;

  if(obj->hasChildAttribute("Description")) {
    string ret=obj->openChildAttribute<SimpleAttribute<string> >("Description")->read();
    cout<<indent<<"  Description: \""<<ret<<"\""<<endl;
  }
}

void printLabel(string indent, Dataset *d) {
  if(l==false) return;

  if(d->hasChildAttribute("Column Label")) {
    vector<string> ret=d->openChildAttribute<SimpleAttribute<vector<string> > >("Column Label")->read();
    cout<<indent<<"  Column Label: ";
    for(size_t i=0; i<ret.size(); i++)
      cout<<"\""<<ret[i]<<"\""<<(i!=ret.size()-1?",":"")<<" ";
    cout<<endl;
  }
}
