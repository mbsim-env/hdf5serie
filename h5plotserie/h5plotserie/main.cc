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

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif
#include <config.h>
#include <clocale>
#include <cassert>
#include <cfenv>
#include <QApplication>
#include <QDir>
#include <QSettings>
#include "mainwindow.h"
#include "dataselection.h"
#include "curves.h"
#include <fmatvec/atom.h>
#include <iostream>
#include <boost/filesystem.hpp>
#include "set_current_path.h"
#ifndef _WIN32
#  include "qt-unix-signals/sigwatch.h"
#endif

using namespace std;

namespace {
  #ifndef NDEBUG
    void myQtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
  #endif
}

int main(int argc, char** argv) {
#ifdef _WIN32
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);
#endif
  setlocale(LC_ALL, "C");
  QLocale::setDefault(QLocale::C);

  QStringList arg;
  for (int i=1; i<argc; i++)
    arg.push_back(argv[i]);

  // current directory and adapt paths
  boost::filesystem::path dirFile;
  if(!arg.empty())
    dirFile=(--arg.end())->toStdString();
  boost::filesystem::path newCurrentPath;
  if(auto i=std::find(arg.begin(), arg.end(), "--CC"); !dirFile.empty() && i!=arg.end()) {
    if(boost::filesystem::is_directory(dirFile))
      newCurrentPath=dirFile;
    else
      newCurrentPath=dirFile.parent_path();
    arg.erase(i);
  }
  if(auto i=std::find(arg.begin(), arg.end(), "-C"); i!=arg.end()) {
    auto i2=i; i2++;
    if(boost::filesystem::is_directory(i2->toStdString()))
      newCurrentPath=i2->toStdString();
    else
      newCurrentPath=boost::filesystem::path(i2->toStdString()).parent_path();
    arg.erase(i);
    arg.erase(i2);
  }
  SetCurrentPath currentPath(newCurrentPath);
  for(auto i=arg.rbegin(); i!=arg.rend(); ++i)
    if(currentPath.existsInOrg(i->toStdString()))
      *i=currentPath.adaptPath(i->toStdString()).string().c_str();

  auto i=find(arg.begin(), arg.end(), "-v");
  auto i2=find(arg.begin(), arg.end(), "--verbose");
  if(i==arg.end() && i2==arg.end())
    fmatvec::Atom::setCurrentMessageStream(fmatvec::Atom::Info, std::make_shared<bool>(false));

  for(int i=0; i<arg.size(); i++)
    if(arg[i].toLower()=="-h" || arg[i].toLower()=="--help") {
      cout
        << "h5plotserie - plot the data of a hdf5 file" << endl
        << "" << endl
        << "Copyright (C) 2009 Martin Foerg <martin.o.foerg@googlemail.com>" << endl
        << "This is free software; see the source for copying conditions. There is NO" << endl
        << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl
        << "" << endl
        << "Licensed under the GNU Lesser General Public License (LGPL)" << endl
        << "" << endl
        << "Usage: h5plotserie [-h|--help] [-v|--verbose]" << endl
	<< "                   [--fullscreen] [--maximized]" << endl
	<< "                   [<dir>] [<file>]" << endl
        << "-h, --help:        Show this help." << endl
        << "-v, --verbose:     Print infromational messages to stdout." << endl
        << "--fullscreen:      Start in full screen mode." << endl
        << "--maximized:       Show window maximized on startup." << endl
        <<"-C <dir/file>       Change current to dir to <dir>/dir of <file> first."<<endl
        <<"                    All arguments are still relative to the original current dir."<<endl
        <<"--CC                Change current dir to dir of <mbsimprjfile> first."<<endl
        <<"                    All arguments are still relative to the original current dir."<<endl
        << "<dir>              Open all *.mbsh5 file in dir." << endl
        <<"                    <dir> and <file> must be the last arguments."<<endl
        << "<file>             Open <file> (*.mbsh5)." << endl
        <<"                    <dir> and <file> must be the last arguments."<<endl;

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

  auto argSaved=arg; // save arguments (QApplication removes all arguments known by Qt)
  QApplication app(argc, argv);
#ifndef NDEBUG
  qInstallMessageHandler(myQtMessageHandler);
#endif
  arg=argSaved; // restore arguments
#ifndef _WIN32
  UnixSignalWatcher sigwatch;
  sigwatch.watchForSignal(SIGHUP);
  sigwatch.watchForSignal(SIGINT);
  sigwatch.watchForSignal(SIGTERM);
  QObject::connect(&sigwatch, &UnixSignalWatcher::unixSignal, &app, &QApplication::quit);
#endif

  app.setOrganizationName("mbsim-env");
  app.setApplicationName("h5plotserie");
  app.setOrganizationDomain("www.mbsim-env.de");
  QSettings::setDefaultFormat(QSettings::IniFormat);

  MainWindow mainWindow(arg);
  mainWindow.show();
  if(arg.contains("--fullscreen")) mainWindow.showFullScreen(); // must be done after mainWindow.show()

  return app.exec();
}

namespace {
#ifndef NDEBUG
  void myQtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    static map<QtMsgType, string> typeStr {
      {QtDebugMsg,    "Debug"},
      {QtWarningMsg,  "Warning"},
      {QtCriticalMsg, "Critical"},
      {QtFatalMsg,    "Fatal"},
      {QtInfoMsg,     "Info"},
    };
    cerr<<(context.file?context.file:"<nofile>")<<":"<<context.line<<": "<<(context.function?context.function:"<nofunc>")<<": "<<(context.category?context.category:"<nocategory>")
        <<": "<<typeStr[type]<<": "<<msg.toStdString()<<endl;
    cerr.flush();
    switch(type) {
      case QtDebugMsg:
      case QtInfoMsg:
        break;
      case QtWarningMsg:
      case QtCriticalMsg:
      case QtFatalMsg:
        std::abort();
        break;
    }
  }
#endif
}
