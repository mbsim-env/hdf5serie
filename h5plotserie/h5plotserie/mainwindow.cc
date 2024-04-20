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
#include "QMainWindow"
#include "QMenu"
#include "QMenuBar"
#include "QDockWidget"
#include "QStatusBar"
#include "QMessageBox"
#include "QFileDialog"
#include "QFile"
#include "QTextStream"
#include "QTimer"
#include <QSettings>
#include <QDomDocument>
#include <QShortcut>
#include "mainwindow.h"
#include "dataselection.h"
#include "curves.h"
#include "plotarea.h"
#include "set"
#include "hdf5serie/file.h"
#include <boost/dll.hpp>

using namespace std;

MainWindow::MainWindow(const QStringList &arg) {

  QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction("Add h5-File", this, &MainWindow::addH5FileDialog);
  fileMenu->addAction("Save all plot windows", this, &MainWindow::saveAllPlotWindows);
  fileMenu->addAction("Load plot windows", this, &MainWindow::loadPlotWindows);
  fileMenu->addSeparator();
  fileMenu->addAction("Exit", this, &MainWindow::close, QKeySequence::Quit);

  auto *dockMenu=new QMenu("Docks", menuBar());
  menuBar()->addMenu(dockMenu);

  QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction("GUI Help", this, &MainWindow::help);
  helpMenu->addAction("About", this, &MainWindow::about);

  statusBar()->showMessage(tr("Ready"));

  plotArea = new PlotArea(this);
  setCentralWidget(plotArea);
  QSettings settings;
  plotArea->setShowMaximized(settings.value("plotwindow/showmaximized", true).toBool());

  auto *dataSelectionDW=new QDockWidget("Data Selection", this);
  dockMenu->addAction(dataSelectionDW->toggleViewAction());
  dataSelectionDW->setObjectName("dockWidget/dataSelection");
  addDockWidget(Qt::LeftDockWidgetArea,dataSelectionDW);
  dataSelection = new DataSelection(this);
  dataSelectionDW->setWidget(dataSelection);
  dataSelectionDW->setFeatures(dataSelectionDW->features() | QDockWidget::DockWidgetVerticalTitleBar);

  requestFlushTimer=new QTimer(this);
  connect(requestFlushTimer, &QTimer::timeout, dataSelection, &DataSelection::requestFlush);
  requestFlushTimer->start(500);

  auto *curvesDW=new QDockWidget("Curves", this);
  dockMenu->addAction(curvesDW->toggleViewAction());
  curvesDW->setObjectName("dockWidget/curves");
  addDockWidget(Qt::BottomDockWidgetArea,curvesDW);
  curves = new Curves(this);
  curvesDW->setWidget(curves);
  curvesDW->setFeatures(curvesDW->features() | QDockWidget::DockWidgetVerticalTitleBar);

  setWindowTitle(tr("h5Plotserie Improved"));
  setWindowIcon(QIcon((boost::dll::program_location().parent_path().parent_path()/
                      "share"/"h5plotserie"/"icons"/"h5plotserie.svg").string().c_str()));

  if(arg.contains("--maximized"))
    showMaximized();

  QString projectFile;
  QRegExp filterProject(".+\\.mbsh5");
  QDir dir;
  dir.setFilter(QDir::Files);
  for(auto & it : arg) {
    if(it[0]=='-') continue;
    dir.setPath(it);
    if(dir.exists()) {
      QStringList file=dir.entryList();
      for(int j=0; j<file.size(); j++) {
	if(projectFile.isEmpty() and filterProject.exactMatch(file[j]))
	  getDataSelection()->addFile(dir.path()+"/"+file[j]);
      }
      continue;
    }
    if(QFile::exists(it)) {
      if(projectFile.isEmpty())
	getDataSelection()->addFile(it);
      continue;
    }
  }

  // auto exit if everything is finished
  if(arg.contains("--autoExit")) {
    auto timer=new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this, timer](){
      timer->stop();
      if(!close())
        timer->start(100);
    });
    timer->start(100);
  }
}

MainWindow::~MainWindow() {
  if (plotArea) {
    delete plotArea;
    plotArea=nullptr;
  }
  if (dataSelection) {
    delete dataSelection;
    dataSelection=nullptr;
  }
  if(curves) {
    delete curves;
    curves=nullptr;
  }
}

void MainWindow::help() {
  QMessageBox::information(this, "h5plotserie - GUI Help", 
      "<h1>GUI Help</h1>"
      "<h2>Actions in model tree</h2>"
      "<ul>"
      "  <dt>Left-Click</dt><dd>Select data set</dd>"
      "  <dt>Right-Click</dt><dd>Open context menu</dd>"
      "</ul>"
      "<h2>Actions in data list</h2>"
      "<ul>"
      "  <dt>Left-Click</dt><dd>Replace curve in current window (x = first, y = selected)</dd>"
      "  <dt>Shift+Left-Click</dt><dd>Add curve to current window (x = first, y = selected)</dd>"
      "  <dt>Ctrl+Left-Click</dt><dd>Plot curve in new window (x = first, y = selected)</dd>"
      "  <dt>Shift+Left-Click</dt><dd>Replace selected cell with selected data</dd>"
      "  <dt>Right-Click</dt><dd>Open context menu</dd>"
      "</ul>"
      "<h2>Actions in curve list</h2>"
      "<ul>"
      "  <dt>Left-Click</dt><dd>Select cell</dd>"
      "  <dt>Right-Click</dt><dd>Open context menu</dd>"
      "</ul>"
      );
}

void MainWindow::about() {
  QMessageBox::about(this, "About h5plotserie",
      "<h1>h5plotserie - plot hdf5 file</h1>"
      "<p>Copyright &copy; Martin Foerg <tt>&lt;martin.o.foerg@googlemail.com&gt;</tt><p/>"
      "<p>Licensed under the Lesser General Public License (see file COPYING).</p>"
      "<p>This is free software; see the source for copying conditions.  There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.</p>"
      "<h2>Authors:</h2>"
      "<ul>"
      "  <li>Martin Foerg <tt>&lt;martin.o.foerg@googlemail.com&gt;</tt> </li>"
      "  <li>Markus Friedrich <tt>&lt;friedrich.at.gc@googlemail.com&gt;</tt> </li>"
      "  <li>Markus Schneider <tt>&lt;markus.ms.schneider@googlemail.com&gt;</tt> </li>"
      "</ul>"
      "<h2>Dependencies:</h2>"
      "<pre>"
#include "../NOTICE"
      "</pre>"
      "<p>A special thanks to all authors of these projects.</p>"
      );
}

void MainWindow::addH5FileDialog() {
  QStringList files=QFileDialog::getOpenFileNames(this, "Open hdf5 files", ".", "Known Files (*.h5 *.mbsh5 *.ombvh5);;MBSim hdf5 Files (*.mbsh5);;OpenMBV hdf5 Files (*.ombvh5);;hdf5 Files (*.h5);;All Files (*.*)");
  for(int i=0; i<files.size(); i++)
    dataSelection->addFile(files[i]);
}

void MainWindow::saveAllPlotWindows() {
  QString fileName=QFileDialog::getSaveFileName(nullptr, "Save plow windows", ".", "xml Files (*.h5Layout.xml)");

  QFile file(fileName+".h5Layout.xml");
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&file);
  out << curves->saveCurves()->toString();
  file.close();
}

void MainWindow::loadPlotWindows() {
  QStringList files=QFileDialog::getOpenFileNames(this, "Load saved plot windows", ".", "h5Layout Files (*.h5Layout.xml)");
  for (int i=0; i<files.size(); i++)
    curves->initLoadCurve(files[i]);
}

void MainWindow::closeEvent(QCloseEvent *event) {
  QSettings settings;
  settings.setValue("mainwindow/geometry", saveGeometry());
  settings.setValue("mainwindow/state", saveState());
  auto c = plotArea->currentSubWindow();
  if(c) settings.setValue("plotwindow/showmaximized", (c->windowState()&Qt::WindowMaximized)==Qt::WindowMaximized);
  QMainWindow::closeEvent(event);
}

void MainWindow::showEvent(QShowEvent *event) {
  QSettings settings;
  restoreGeometry(settings.value("mainwindow/geometry").toByteArray());
  restoreState(settings.value("mainwindow/state").toByteArray());
  QMainWindow::showEvent(event);
}
