/*
    h5plotserie - plot the data of a hdf5 file.
    Copyright (C) 2010 Markus Schneider

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
#include "mainwindow.h"
#include "dataselection.h"
#include "curves.h"
#include "plotarea.h"
#include "set"
#include "hdf5serie/file.h"
#include <boost/dll.hpp>

using namespace std;

MainWindow::MainWindow(const QStringList &arg) {

  QMenu * fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction("add h5-File", this, &MainWindow::addH5FileDialog);
  fileMenu->addAction("save all plot windows", this, &MainWindow::saveAllPlotWindows);
  fileMenu->addAction("load plot windows", this, &MainWindow::loadPlotWindows);
  fileMenu->addAction("exit", this, &MainWindow::close, QKeySequence::Quit);

  menuBar()->addSeparator();
  QMenu * helpMenu = menuBar()->addMenu(tr("&About"));
  helpMenu->addAction("GUI Help", this, &MainWindow::help);
  helpMenu->addAction("About", this, &MainWindow::about);

  statusBar()->showMessage(tr("Ready"));

  plotArea = new PlotArea(this);
  setCentralWidget(plotArea);

  QDockWidget * dataSelectionDW=new QDockWidget("Data Selection", this);
  dataSelectionDW->setObjectName("dockWidget/dataSelection");
  addDockWidget(Qt::LeftDockWidgetArea,dataSelectionDW);
  dataSelection = new DataSelection(this);
  dataSelectionDW->setWidget(dataSelection);
  dataSelectionDW->setFeatures(QDockWidget::NoDockWidgetFeatures);

  requestFlushTimer=new QTimer(this);
  connect(requestFlushTimer, &QTimer::timeout, dataSelection, &DataSelection::requestFlush);
  requestFlushTimer->start(500);

  QDockWidget *curvesDW=new QDockWidget("Curves", this);
  curvesDW->setObjectName("dockWidget/curves");
  addDockWidget(Qt::LeftDockWidgetArea,curvesDW);
  curves = new Curves(this);
  curvesDW->setWidget(curves);
  curvesDW->setFeatures(QDockWidget::NoDockWidgetFeatures);

  setWindowTitle(tr("h5Plotserie Improved"));
  setWindowIcon(QIcon((boost::dll::program_location().parent_path().parent_path()/
                      "share"/"h5plotserie"/"icons"/"h5plotserie.svg").string().c_str()));

  // auto exit if everything is finished
  if(arg.contains("--autoExit")) {
    auto timer=new QTimer(this);
    connect(timer, &QTimer::timeout, [this, timer](){
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
      "<h2>Actions in data list</h2>"
      "<ul>"
      "  <dt>Left-Click</dt><dd> Plot curve (x = first, y = selected) </dd>"
      "  <dt>Shift+Left-Click</dt><dd> Add curve (x = first, y = selected)</dd>"
      "  <dt>Shift+Left-Click and ONE cell selected</dt><dd> replace selected cell with selected data</dd>"
      "  <dt>Ctrl+Left-Click</dt><dd> Plot curve in new window </dd>"
      "</ul>"
      "<h2>Actions in curve list</h2>"
      "  <dt>Left-Click</dt><dd> Choose current curve </dd>"
      "  <dt>Right-Click</dt><dd> Remove curve </dd>"
      );
}

void MainWindow::about() {
  QMessageBox::about(this, "About h5plotserie",
      "<h1>h5plotserie - plot hdf5 file</h1>"
      "<p>Copyright &copy; Martin Foerg <tt>&lt;martin.o.foerg@googlemail.com&gt;</tt><p/>"
      "<p>Licensed under the General Public License (see file COPYING).</p>"
      "<p>This is free software; see the source for copying conditions.  There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.</p>"
      "<h2>Authors:</h2>"
      "<ul>"
      "  <li>Martin Foerg <tt>&lt;martin.o.foerg@googlemail.com&gt;</tt> </li>"
      "  <li>Markus Friedrich <tt>&lt;friedrich.at.gc@googlemail.com&gt;</tt> </li>"
      "  <li>Markus Schneider <tt>&lt;markus.ms.schneider@googlemail.com&gt;</tt> </li>"
      "</ul>"
      "<h2>This program uses:</h2>"
      "<ul>"
      "  <li>'Qt - A cross-platform application and UI framework' by Nokia from <tt>http://www.qtsoftware.com</tt> (License: GPL/LGPL)</li>"
      "  <li>'Qwt - Qt Widgets for Technical Applications' by Uwe Rathmann from <tt>http://qwt.sourceforge.net</tt> (Licence: Qwt/LGPL)</li>"
      "  <li>'HDF5Serie - A HDF5 Wrapper for Time Series' by Markus Friedrich from <tt>https://www.mbsim-env.de/</tt> (License: LGPL)</li>"
      "  <li>'HDF - Hierarchical Data Format' by The HDF Group from <tt>http://www.hdfgroup.org</tt> (License: NCSA-HDF)</li>"
      "  <li>...</li>"
      "</ul>"
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
  QMainWindow::closeEvent(event);
}

void MainWindow::showEvent(QShowEvent *event) {
  QSettings settings;
  restoreGeometry(settings.value("mainwindow/geometry").toByteArray());
  restoreState(settings.value("mainwindow/state").toByteArray());
  QMainWindow::showEvent(event);
}
