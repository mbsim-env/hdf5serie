/*
   h5plotserie - plot the data of a hdf5 file.
   Copyright (C) 2010 Markus Schneider

  This library is free software; you can redistribute it and/or 
  modify it under the terms of the GNU Lesser General Public 
  License as published by the Free Software Foundation; either 
  version 2.1 of the License, or (at your option) any later version. 
   
  This library is distributed in the hope that it will be useful, 
  but WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
  Lesser General Public License for more details. 
   
  You should have received a copy of the GNU Lesser General Public 
  License along with this library; if not, write to the Free Software 
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
   */

#include <config.h>
#include <cassert>
#include <cfenv>
#include <QApplication>
#include <QDir>
#include <QSettings>
#include "mainwindow.h"
#include "dataselection.h"
#include "curves.h"
#include <iostream>
#ifdef _WIN32
#  include <windows.h>
#else
#  include "qt-unix-signals/sigwatch.h"
#endif

using namespace std;

int main(int argc, char** argv) {
#ifndef _WIN32
//MISSING Qt seems to generate some FPE, hence disabled  assert(feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW)!=-1);
#endif

  QStringList arg;
  for (int i=1; i<argc; i++)
    arg.push_back(argv[i]);

  for(int i=0; i<arg.size(); i++)
    if(arg[i].contains("-h", Qt::CaseSensitive) || arg[i].contains("--help", Qt::CaseSensitive)) {
      cout
        << "h5plotserie - plot the data of a hdf5 file" << endl
        << "" << endl
        << "Copyright (C) 2009 Martin Foerg <martin.o.foerg@googlemail.com>" << endl
        << "This is free software; see the source for copying conditions. There is NO" << endl
        << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl
        << "" << endl
        << "Licensed under the GNU Lesser General Public License (LGPL)" << endl
        << "" << endl
        << "Usage:" << endl
        << "  h5plotserie <file> | <file.h5Layout.xml>" << endl
        << "    -h, --help: Show this help" << endl
        << "    --fullscreen: Start in full screen mode" << endl
        << "    --maximized: Show window maximized on startup." << endl;
      return 0;
    }

  char moduleName[2048];
#ifdef _WIN32
  GetModuleFileName(nullptr, moduleName, sizeof(moduleName));
#else
  size_t s=readlink("/proc/self/exe", moduleName, sizeof(moduleName));
  moduleName[s]=0; // null terminate
#endif
  QCoreApplication::setLibraryPaths(QStringList(QFileInfo(moduleName).absolutePath())); // do not load plugins from buildin defaults

  QApplication app(argc, argv);
#ifndef _WIN32
  UnixSignalWatcher sigwatch;
  sigwatch.watchForSignal(SIGINT);
  sigwatch.watchForSignal(SIGTERM);
  QObject::connect(&sigwatch, &UnixSignalWatcher::unixSignal, &app, &QApplication::quit);
#endif

  // regenerate arg: QApplication removes all arguments known by Qt
  arg.clear();
  for (int i=1; i<argc; i++)
    arg.push_back(argv[i]);

  app.setOrganizationName("mbsim-env");
  app.setApplicationName("h5plotserie");
  app.setOrganizationDomain("www.mbsim-env.de");
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QLocale::setDefault(QLocale::C);
  setlocale(LC_ALL, "C");

  MainWindow mainWindow(arg);
  mainWindow.show();
  if(arg.contains("--fullscreen")) mainWindow.showFullScreen(); // must be done after mainWindow.show()

  return app.exec();
}
