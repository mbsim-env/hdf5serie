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

#include "mainwindow.h"
#include "treewidgetitem.h"
#include <QApplication>
#include <QDockWidget>
#include <QGridLayout>
#include <QTreeWidget>
#include <QListWidget>
#include <QTableWidget>
#include <QMenuBar>
#include <QFileDialog>
#include <QSplitter>
#include <QMessageBox>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_zoomer.h>
#include <qwt_legend.h>
#include <qwt_legend_item.h>
#include <hdf5serie/simpledataset.h>
#include <hdf5serie/vectorserie.h>
#include <QPen>
#include <QColor>
#include <sstream>
#include <iostream>

using namespace H5;
using namespace std;

MainWindow::MainWindow(vector<string>& arg) {

  QGridLayout* mainlayout = new QGridLayout;

  centralWidget = new QWidget;  
  setCentralWidget(centralWidget);
  centralWidget->setLayout(mainlayout);

  QDockWidget *objectListDW=new QDockWidget("Data",this);
  addDockWidget(Qt::LeftDockWidgetArea,objectListDW);
  QSplitter *splitter = new QSplitter;
  objectListDW->setWidget(splitter);
  treeWidget = new QTreeWidget;
  splitter->addWidget(treeWidget);

  treeWidget->setHeaderHidden(true);
  treeWidget->setColumnCount(1);


  for(unsigned int i=0; i<arg.size(); i++)
    addFile(arg[i].c_str());

  listWidget = new QListWidget;
  splitter->addWidget(listWidget);


  QDockWidget *curveDW=new QDockWidget("Curves",this);
  addDockWidget(Qt::LeftDockWidgetArea,curveDW);
  tableWidget = new QTreeWidget;
  curveDW->setWidget(tableWidget);
  tableWidget->setColumnCount(33);
  QStringList sl;
  sl << "Number" << "x-label" << "y-label" << "x-path" << "y-path";
  tableWidget->setHeaderLabels(sl);

  myPlot = new MyPlot(centralWidget);
  mdiArea = new QMdiArea;
  mainlayout->addWidget(mdiArea, 0, 0);
  mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  mdiArea->addSubWindow(myPlot);
  mdiArea->currentSubWindow()->setWindowTitle(QString("Plot ")+QString::number(mdiArea->subWindowList().size()));

  QwtLegend *legend = new QwtLegend;
  myPlot->insertLegend(legend);

  uint linewidth=1;
  int c0=0;
  int c1=255;
  int c2=128;
  pen.append(QPen(QColor(c0, c0, c1), linewidth)); /*  1 */
  pen.append(QPen(QColor(c1, c0, c0), linewidth)); /*  2 */
  pen.append(QPen(QColor(c0, c1, c0), linewidth)); /*  3 */
  pen.append(QPen(QColor(c0, c0, c0), linewidth)); /*  4 */
  pen.append(QPen(QColor(c0, c0, c2), linewidth)); /*  5 */
  pen.append(QPen(QColor(c2, c0, c0), linewidth)); /*  6 */
  pen.append(QPen(QColor(c0, c2, c0), linewidth)); /*  7 */
  pen.append(QPen(QColor(c2, c0, c1), linewidth)); /*  8 */
  pen.append(QPen(QColor(c0, c2, c1), linewidth)); /*  9 */
  pen.append(QPen(QColor(c1, c2, c0), linewidth)); /* 10 */
  pen.append(QPen(QColor(c1, c0, c2), linewidth)); /* 11 */
  pen.append(QPen(QColor(c0, c2, c2), linewidth)); /* 12 */
  pen.append(QPen(QColor(c2, c2, c0), linewidth)); /* 13 */
  pen.append(QPen(QColor(c2, c2, c1), linewidth)); /* 14 */
  pen.append(QPen(QColor(c1, c2, c2), linewidth)); /* 15 */
  pen.append(QPen(QColor(c2, c2, c2), linewidth)); /* 16 */
  pen.append(QPen(QColor(c0, c0, c1), linewidth, Qt::DotLine)); /* 17 */
  pen.append(QPen(QColor(c1, c0, c0), linewidth, Qt::DotLine)); /* 18 */
  pen.append(QPen(QColor(c0, c1, c0), linewidth, Qt::DotLine)); /* 19 */
  pen.append(QPen(QColor(c0, c0, c0), linewidth, Qt::DotLine)); /* 20 */
  pen.append(QPen(QColor(c0, c0, c2), linewidth, Qt::DotLine)); /* 21 */
  pen.append(QPen(QColor(c2, c0, c0), linewidth, Qt::DotLine)); /* 22 */
  pen.append(QPen(QColor(c0, c2, c0), linewidth, Qt::DotLine)); /* 23 */
  pen.append(QPen(QColor(c2, c0, c1), linewidth, Qt::DotLine)); /* 24 */
  pen.append(QPen(QColor(c0, c2, c1), linewidth, Qt::DotLine)); /* 25 */
  pen.append(QPen(QColor(c1, c2, c0), linewidth, Qt::DotLine)); /* 26 */
  pen.append(QPen(QColor(c1, c0, c2), linewidth, Qt::DotLine)); /* 27 */
  pen.append(QPen(QColor(c0, c2, c2), linewidth, Qt::DotLine)); /* 28 */
  pen.append(QPen(QColor(c2, c2, c0), linewidth, Qt::DotLine)); /* 29 */
  pen.append(QPen(QColor(c2, c2, c1), linewidth, Qt::DotLine)); /* 30 */
  pen.append(QPen(QColor(c1, c2, c2), linewidth, Qt::DotLine)); /* 31 */
  pen.append(QPen(QColor(c2, c2, c2), linewidth, Qt::DotLine)); /* 32 */

  QMenu *fileMenu=new QMenu("File", menuBar());
  QAction *addFileAct=fileMenu->addAction("Open File(s)...", this, SLOT(openFileDialog()));
  menuBar()->addMenu(fileMenu);
  addFileAct=fileMenu->addAction("Close File...", this, SLOT(closeFile()));
  menuBar()->addMenu(fileMenu);
  QMenu *plotMenu=new QMenu("Plot", menuBar());
  QAction *addPlotAct=plotMenu->addAction("Add Plot...", this, SLOT(addPlotWindow()));
  menuBar()->addMenu(plotMenu);

  // help menu
  menuBar()->addSeparator();
  QMenu *helpMenu=new QMenu("Help", menuBar());
  helpMenu->addAction("GUI Help...", this, SLOT(help()));
  helpMenu->addAction("About OpenMBV...", this, SLOT(about()));
  menuBar()->addMenu(helpMenu);

  QObject::connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(updateData(QTreeWidgetItem*,int)));
  QObject::connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(plot(QListWidgetItem*)));
  QObject::connect(tableWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(detachCurve(QTreeWidgetItem*,int)));
  QObject::connect(mdiArea,SIGNAL(subWindowActivated ( QMdiSubWindow *)), this, SLOT(windowChanged(QMdiSubWindow*)));
}

void MainWindow::addFile(const QString &name) {

  file.append(new H5::H5File(name.toStdString(), H5F_ACC_RDONLY));

  int j = file.size()-1;

  int iofl = name.lastIndexOf("/");
  fileName.append(name.right(name.size()-iofl-1));

  TreeWidgetItem *topitem = new TreeWidgetItem(QStringList(fileName[j]));
  treeWidget->addTopLevelItem(topitem);
  QList<QTreeWidgetItem *> items;
  for(unsigned int i=0; i<file[j]->getNumObjs(); i++)  {
    QTreeWidgetItem *item = new TreeWidgetItem(QStringList(file[j]->getObjnameByIdx(i).c_str()));
    H5::Group grp = file[j]->openGroup(file[j]->getObjnameByIdx(i));
    insertChildInTree(grp, item);

    topitem->addChild(item);
  }
}

void MainWindow::insertChildInTree(H5::Group &grp, QTreeWidgetItem *item) {
  for(unsigned int j=0; j<grp.getNumObjs(); j++) {
    QTreeWidgetItem *child = new TreeWidgetItem(QStringList(grp.getObjnameByIdx(j).c_str()));
    item->addChild(child);
    if(grp.getObjTypeByIdx(j)==0) {
      H5::Group childgrp = grp.openGroup(grp.getObjnameByIdx(j));
      insertChildInTree(childgrp, child);
    }
    else {
      if(grp.getObjnameByIdx(j) == "data") {
	VectorSerie<double> vs;
	QString path; 
	getPath(item,path,0);
	path += "/data";
	static_cast<TreeWidgetItem*>(child)->setPath(path);
      }
    }
  }
}

void MainWindow::getPath(QTreeWidgetItem* item, QString &s, int col) {
  QTreeWidgetItem* parent = item->parent();
  if(parent)
    getPath(parent,s,col);
  s  += "/" + item->text(col);
}

int MainWindow::getTopLevelIndex(QTreeWidgetItem* item) {
  QTreeWidgetItem* parent = item->parent();
  if(parent)
    return getTopLevelIndex(parent);
  else // item is TopLevelItem
    return treeWidget->indexOfTopLevelItem(item); 
}

void MainWindow::plot(QListWidgetItem* item) {
  if(myPlot) {
    QString path = static_cast<TreeWidgetItem*>(treeWidget->currentItem())->getPath();
    int col = listWidget->row(item);
    VectorSerie<double> vs;
    int j = getTopLevelIndex(treeWidget->currentItem());
    vs.open(*file[j],path.toStdString());
    QString fullName = fileName[j]+path;

    vector<double> t = vs.getColumn(0);
    vector<double> q = vs.getColumn(col);

    QString xlabel = QString::fromStdString(vs.getColumnLabel()[0]);
    QString ylabel = QString::fromStdString(vs.getColumnLabel()[col]);

    int k=0;
    if(QApplication::keyboardModifiers() & Qt::ControlModifier) {
      if(tableWidget->currentItem()) {
	QwtPlotItemList il = myPlot->itemList();
	int row = tableWidget->currentIndex().row();

	MyCurve *curve = static_cast<MyCurve*>(il[row]);
	QwtData* data = curve->data().copy();
	QwtArrayData* adata = dynamic_cast<QwtArrayData*>(data);
	curve->setData(&q[0],&adata->yData()[0],q.size());
	curve->setxPath(fullName);
	curve->setyPath(curve->getyPath());
	curve->setxLabel(ylabel);
	curve->setyLabel(curve->getyLabel());
	static_cast<QwtLegendItem*>(myPlot->legend()->find(curve))->setText(QwtText(QString::number(row+1)));
      }
    }
    else if(QApplication::keyboardModifiers() & Qt::ShiftModifier) {
      k = myPlot->itemList().size();
      if(k<pen.size()) {
	MyCurve* newcurve = new MyCurve;
	newcurve->attach(myPlot);
	newcurve->setPen(pen[k]);
	newcurve->setData(&t[0],&q[0],t.size());
	newcurve->setxPath(fullName);
	newcurve->setyPath(fullName);
	newcurve->setxLabel(xlabel);
	newcurve->setyLabel(ylabel);
	static_cast<QwtLegendItem*>(myPlot->legend()->find(newcurve))->setText(QwtText(QString::number(k+1)));
      }
      else {
	cout << "Error" << endl;
      }
    } else
    {
      QwtPlotItemList il = myPlot->itemList();
      for(int i=0; i<il.size(); i++)
	il[i]->detach();

      MyCurve* newcurve = new MyCurve;
      newcurve->attach(myPlot);
      newcurve->setPen(pen[0]);
      newcurve->setData(&t[0],&q[0],t.size());
      newcurve->setxPath(fullName);
      newcurve->setyPath(fullName);
      newcurve->setxLabel(xlabel);
      newcurve->setyLabel(ylabel);
      //myPlot->setAxisTitle(QwtPlot::xBottom,xlabel.c_str());
      //myPlot->setAxisTitle(QwtPlot::yLeft,ylabel.c_str());
      static_cast<QwtLegendItem*>(myPlot->legend()->find(newcurve))->setText(QwtText(QString::number(1)));
    }
    //  myPlot->setTitle(path.c_str());
    myPlot->setAxisAutoScale(QwtPlot::xBottom);
    myPlot->setAxisAutoScale(QwtPlot::yLeft);


    myPlot->replot();

    static_cast<MyPlot*>(myPlot)->getZoom()->setZoomBase();
    vs.close();
    updateTableWidget();
  }
}

void MainWindow::updateTableWidget() {
  tableWidget->clear();
  QStringList sl;
  if(myPlot) {
    QwtPlotItemList il = myPlot->itemList();

    for(int i=0; i<il.size(); i++) {
      sl.clear();
      MyCurve *item = static_cast<MyCurve*>(il[i]);
      sl << QString::number(i+1) << item->getxLabel() << item->getyLabel() << item->getxPath() << item->getyPath();
      QTreeWidgetItem *newItem = new QTreeWidgetItem(sl);
      tableWidget->addTopLevelItem(newItem);
    }
  }
}

void MainWindow::updateData(QTreeWidgetItem* item, int col) {
  listWidget->clear();
  if( item->text(col) == "data") {
    QString path = static_cast<TreeWidgetItem*>(item)->getPath();
    VectorSerie<double> vs;
    int j = getTopLevelIndex(item);
    vs.open(*file[j],path.toStdString());
    QStringList sl;
    for(unsigned int i=0; i<vs.getColumns(); i++) {
      sl << vs.getColumnLabel()[i].c_str();
    }
    listWidget->addItems(sl);
  }
}

void MainWindow::detachCurve(QTreeWidgetItem* item, int col) {
  if(myPlot) {
    int row = tableWidget->indexOfTopLevelItem(item);
    QwtPlotItemList il = myPlot->itemList();
    il[row]->detach();
    delete tableWidget->takeTopLevelItem(row);
    il = myPlot->itemList();
    for(int i=0; i<il.size(); i++) {
      MyCurve *curve = static_cast<MyCurve*>(il[i]);
      curve->setPen(pen[i]);
      static_cast<QwtLegendItem*>(myPlot->legend()->find(curve))->setText(QwtText(QString::number(i+1)));
      tableWidget->topLevelItem(i)->setText(0,QString::number(i+1));
    }
    myPlot->replot();
  }
}

void MainWindow::addPlotWindow() {
  QwtPlot *newplot = new MyPlot(centralWidget);
  mdiArea->addSubWindow(newplot);
  newplot->show();
  mdiArea->currentSubWindow()->setWindowTitle(QString("Plot ")+QString::number(mdiArea->subWindowList().size()));
  QwtLegend *legend = new QwtLegend;
  newplot->insertLegend(legend);
}

void MainWindow::windowChanged(QMdiSubWindow* window) {
  if(window) {
    myPlot = static_cast<QwtPlot*>(window->widget());
    updateTableWidget();
  } 
  else if(mdiArea->currentSubWindow()) {
    myPlot = static_cast<QwtPlot*>(mdiArea->currentSubWindow()->widget());
    updateTableWidget();
  } 
  else {
    myPlot = 0;
  }
}

void MainWindow::help() {
  QMessageBox::information(this, "h5plotserie - GUI Help", 
      "<h1>GUI Help</h1>"
      "<h2>Actions in data list</h2>"
      "<ul>"
      "  <dt>Left-Click</dt><dd> Plot curve (x = first, y = selected) </dd>"
      "  <dt>Shift+Left-Click</dt><dd> Add curve (x = first, y = selected)</dd>"
      "  <dt>Ctrl+Left-Click</dt><dd> Change x-axis of current curve (x = selected, y = old) </dd>"
      "</ul>"
      "<h2>Actions in curve list</h2>"
      "  <dt>Left-Click</dt><dd> Choose current curve </dd>"
      "  <dt>Left-DoubleClick</dt><dd> Remove curve </dd>"
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
      "</ul>"
      "<h2>This program uses:</h2>"
      "<ul>"
      "  <li>'Qt - A cross-platform application and UI framework' by Nokia from <tt>http://www.qtsoftware.com</tt> (License: GPL/LGPL)</li>"
      "  <li>'Qwt - Qt Widgets for Technical Applications' by Uwe Rathmann from <tt>http://qwt.sourceforge.net</tt> (Licence: Qwt/LGPL)</li>"
      "  <li>'HDF5Serie - A HDF5 Wrapper for Time Series' by Markus Friedrich from <tt>http://hdf5serie.berlios.de</tt> (License: LGPL)</li>"
      "  <li>'HDF - Hierarchical Data Format' by The HDF Group from <tt>http://www.hdfgroup.org</tt> (License: NCSA-HDF)</li>"
      "  <li>...</li>"
      "</ul>"
      "<p>A special thanks to all authors of this projects.</p>"
      );
}

MyPlot::MyPlot(QWidget *p) : QwtPlot(p) {
  zoom = new QwtPlotZoomer(this->canvas());
  setCanvasBackground(QColor(255, 255, 255)); // background color of plotting area
}

MyPlot::~MyPlot() {
  delete zoom;
  zoom = 0;
}

void MainWindow::openFileDialog() {
  QStringList files=QFileDialog::getOpenFileNames(0, "OpenMBV Files", ".",
      "hdf5 Files (*.mbsim.h5)");
  for(int i=0; i<files.size(); i++)
    addFile(files[i]);
}

void MainWindow::closeFile() {
  QTreeWidgetItem* item = treeWidget->currentItem();
  if(item && item->parent()==0) {
    int index = treeWidget->indexOfTopLevelItem(item);
    treeWidget->takeTopLevelItem(index);
    updateTableWidget();
    file[index]->close();
    file.removeAt(index);
    fileName.removeAt(index);
  }
}
