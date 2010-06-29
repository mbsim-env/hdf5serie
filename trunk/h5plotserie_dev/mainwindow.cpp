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

#include <QtGui>
#include <QtXml>
#include "mainwindow.h"
#include "dataselection.h"
#include "curves.h"
#include "plotarea.h"

using namespace std;

MainWindow::MainWindow(vector<string>& arg) : QMainWindow() {

  QMenu * fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction("add h5-File", this, SLOT(addH5FileDialog()));
  fileMenu->addAction("save all plot windows", this, SLOT(saveAllPlotWindows()));
  fileMenu->addAction("load plot windows", this, SLOT(loadPlotWindows()));
  menuBar()->addSeparator();
  QMenu * helpMenu = menuBar()->addMenu(tr("&About"));
  helpMenu->addAction("GUI Help", this, SLOT(help()));
  helpMenu->addAction("About", this, SLOT(about()));

  statusBar()->showMessage(tr("Ready"));

  plotArea = new PlotArea(this);
  setCentralWidget(plotArea);

  QDockWidget * dataSelectionDW=new QDockWidget("Data Selection", this);
  addDockWidget(Qt::LeftDockWidgetArea,dataSelectionDW);
  dataSelection = new DataSelection(this);
  dataSelectionDW->setWidget(dataSelection);
  dataSelectionDW->setFeatures(QDockWidget::NoDockWidgetFeatures);

  QDockWidget *curvesDW=new QDockWidget("Curves", this);
  addDockWidget(Qt::LeftDockWidgetArea,curvesDW);
  curves = new Curves(this);
  curvesDW->setWidget(curves);
  curvesDW->setFeatures(QDockWidget::NoDockWidgetFeatures);

  setWindowTitle(tr("h5Plotserie Improved"));

  for(unsigned int i=0; i<arg.size(); i++)
    dataSelection->addFile(arg[i].c_str());
}

MainWindow::~MainWindow() {
  if (plotArea) {
    delete plotArea;
    plotArea=NULL;
  }
  if (dataSelection) {
    delete dataSelection;
    dataSelection=NULL;
  }
  if(curves) {
    delete curves;
    curves=NULL;
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
      "<p>Copyright &copy; Martin Foerg <tt>&lt;mfoerg@user.berlios.de&gt;</tt><p/>"
      "<p>Licensed under the General Public License (see file COPYING).</p>"
      "<p>This is free software; see the source for copying conditions.  There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.</p>"
      "<h2>Authors:</h2>"
      "<ul>"
      "  <li>Martin Foerg <tt>&lt;mfoerg@users.berlios.de&gt;</tt> </li>"
      "  <li>Markus Friedrich <tt>&lt;mafriedrich@users.berlios.de&gt;</tt> </li>"
      "  <li>Markus Schneider <tt>&lt;schneidm@users.berlios.de&gt;</tt> </li>"
      "</ul>"
      "<h2>This program uses:</h2>"
      "<ul>"
      "  <li>'Qt - A cross-platform application and UI framework' by Nokia from <tt>http://www.qtsoftware.com</tt> (License: GPL/LGPL)</li>"
      "  <li>'Qwt - Qt Widgets for Technical Applications' by Uwe Rathmann from <tt>http://qwt.sourceforge.net</tt> (Licence: Qwt/LGPL)</li>"
      "  <li>'HDF5Serie - A HDF5 Wrapper for Time Series' by Markus Friedrich from <tt>http://hdf5serie.berlios.de</tt> (License: LGPL)</li>"
      "  <li>'HDF - Hierarchical Data Format' by The HDF Group from <tt>http://www.hdfgroup.org</tt> (License: NCSA-HDF)</li>"
      "  <li>...</li>"
      "</ul>"
      "<p>A special thanks to all authors of these projects.</p>"
      );
}

void MainWindow::addH5FileDialog() {
  QStringList files=QFileDialog::getOpenFileNames(this, "Open hdf5 files", ".", "hdf5 Files (*.mbsim.h5)");
  for(int i=0; i<files.size(); i++)
    dataSelection->addFile(files[i]);
}

void MainWindow::saveAllPlotWindows() {
  QString fileName=QFileDialog::getSaveFileName(0, "Save plow windows", ".", "xml Files (*.h5Layout.xml)");

  QFile file(fileName+".h5Layout.xml");
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&file);
  out << curves->saveCurves();
  file.close();
}

void MainWindow::loadPlotWindows() {
  QStringList files=QFileDialog::getOpenFileNames(this, "Load saved plot windows", ".", "h5Layout Files (*.h5Layout.xml)");
  for(int i=0; i<files.size(); i++) {
    QDomDocument doc("h5PlotDataset");
    QFile file(files[i]);
    if (!file.open(QIODevice::ReadOnly))
      return;
    if (!doc.setContent(&file)) {
      file.close();
      return;
    }
    curves->loadCurves(&doc);
    file.close();
  }
}
