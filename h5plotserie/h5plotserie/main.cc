/*
    h5plotserie - plot the data of a hdf5 file.
    Copyright (C) 2009 Martin Förg

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <QApplication>
#include <QDir>
#include "mainwindow.h"
#include <iostream>
#include <algorithm>

using namespace std;

int main(int argc, char *argv[]) {

  QApplication app(argc, argv);

  vector<string> arg;
  for(int i=1; i<argc; i++)
    arg.push_back(argv[i]);

  vector<string>::iterator i1, i2, i3;
  i1=find(arg.begin(), arg.end(), "-h");
  i2=find(arg.begin(), arg.end(), "--help");
  if(i1!=arg.end() || i2!=arg.end() ) {
    cout<<"h5plotserie - plot the data of a hdf5 file"<<endl
    <<""<<endl
    <<"Copyright (C) 2009 Martin Foerg <mfoerg@users.berlios.de>"<<endl
    <<"This is free software; see the source for copying conditions. There is NO"<<endl
    <<"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."<<endl
    <<""<<endl
    <<"Licensed under the GNU General Public License (GPL)"<<endl
    <<""<<endl
    <<"Usage:" << endl
    <<"  h5plotserie <file.h5>"<<endl
    <<"    -h, --help: Show this help"<<endl;
    if(i1!=arg.end()) arg.erase(i1); else if(i2!=arg.end()) arg.erase(i2);
    return 0;
  }

  if(arg.size() == 0) {
    QDir dir;
    QRegExp filterRE1(".+\\.mbsim\\.h5");
    dir.setFilter(QDir::Files);
    dir.setPath(".");
    QStringList file=dir.entryList();
    for(int j=0; j<file.size(); j++)
      if(filterRE1.exactMatch(file[j]))
	arg.push_back(file[j].toStdString());
  }

  MainWindow *mainwindow = new MainWindow(arg);
  mainwindow->show();
  return app.exec();
}
