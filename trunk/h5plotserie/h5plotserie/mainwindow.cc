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
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_zoomer.h>
#include <hdf5serie/simpledataset.h>
#include <hdf5serie/vectorserie.h>
#include <QPen>
#include <QColor>
#include <sstream>

using namespace H5;
using namespace std;

MainWindow::MainWindow(vector<string>& arg) {

  QGridLayout* mainlayout = new QGridLayout;

  centralWidget = new QWidget;  
  setCentralWidget(centralWidget);
  centralWidget->setLayout(mainlayout);

  QDockWidget *objectListDW=new QDockWidget("Objects",this);
  objectListDW->setObjectName("Objects");
  addDockWidget(Qt::LeftDockWidgetArea,objectListDW);
  treeWidget = new QTreeWidget(objectListDW);
  objectListDW->setWidget(treeWidget);
  treeWidget->setHeaderHidden(true);
  treeWidget->setColumnCount(1);

  QDockWidget *dataListDW=new QDockWidget("Data",this);
  dataListDW->setObjectName("Data");
  addDockWidget(Qt::LeftDockWidgetArea,dataListDW);
  listWidget = new QListWidget;
  dataListDW->setWidget(listWidget);

  file = new H5::H5File(arg[0], H5F_ACC_RDONLY);
  QList<QTreeWidgetItem *> items;
  for(unsigned int i=0; i<file->getNumObjs(); i++)  {
    QTreeWidgetItem *item = new TreeWidgetItem(QStringList(file->getObjnameByIdx(i).c_str()));
    H5::Group grp = file->openGroup(file->getObjnameByIdx(i));
    insertChildInTree(grp, item);

    treeWidget->insertTopLevelItem(i,item);
  }

  myPlot = new QwtPlot(QwtText(""), centralWidget);

  curve = new QwtPlotCurve;
  curve->setPen(QPen(Qt::red));
  curve->attach(myPlot);

  mainlayout->addWidget(myPlot, 0, 0);

  zoom = new QwtPlotZoomer(myPlot->canvas());

  QObject::connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(check(QTreeWidgetItem*,int)));
  QObject::connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(plot(QListWidgetItem*)));
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
	stringstream path; 
	getPath(item,path,0);
	path << "/data";
	vs.open(*file,path.str());
	QStringList sl;
	static_cast<TreeWidgetItem*>(child)->setPath(path.str());
	for(unsigned int i=0; i<vs.getColumns(); i++) {
	  sl << vs.getColumnLabel()[i].c_str();
	}
	static_cast<TreeWidgetItem*>(child)->setStringList(sl);
      }
    }
  }
}

void MainWindow::getPath(QTreeWidgetItem* item, stringstream &s, int col) {
  QTreeWidgetItem* parent = item->parent();
  if(parent)
    getPath(parent,s,col);
  s  << "/" << item->text(col).toStdString();
}

void MainWindow::plot(QListWidgetItem* item) {
  string s = static_cast<TreeWidgetItem*>(treeWidget->currentItem())->getPath();
  int col = listWidget->row(item);
  VectorSerie<double> vs;
  vs.open(*file,s);
  vector<double> t = vs.getColumn(0);
  vector<double> q = vs.getColumn(col);
  curve->setData(t.data(),q.data(),t.size());
  myPlot->setTitle(s.c_str());
  myPlot->setAxisTitle(QwtPlot::xBottom,vs.getColumnLabel()[0].c_str());
  myPlot->setAxisTitle(QwtPlot::yLeft,vs.getColumnLabel()[col].c_str());
  myPlot->setAxisAutoScale(QwtPlot::xBottom);
  myPlot->setAxisAutoScale(QwtPlot::yLeft);
  myPlot->replot();
  zoom->setZoomBase();
  vs.close();
}

void MainWindow::check(QTreeWidgetItem* item, int col) {
  listWidget->clear();
  if( item->text(col) == "data") {
    listWidget->addItems(static_cast<TreeWidgetItem*>(item)->getStringList());
  }
}

