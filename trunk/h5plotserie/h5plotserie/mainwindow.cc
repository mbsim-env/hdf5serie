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
#include <QDateTime>
#include <QKeyEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QPrintDialog>
#include <QSplitter>
#include <QMessageBox>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QPrinter>
//#include <QSvgGenerator>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_zoomer.h>
#include <qwt_legend.h>
#include <qwt_legend_item.h>
#include <hdf5serie/simpledataset.h>
#include <hdf5serie/vectorserie.h>
#include <QPen>
#include <QColor>
#include <QLabel>
#include <QLineEdit>
#include <sstream>
#include <iostream>
#include <hdf5serie/fileserie.h>

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

  QWidget* dummy=new QWidget(this);
  splitter->addWidget(dummy);
  QGridLayout* treeWidgetLO=new QGridLayout(this);
  dummy->setLayout(treeWidgetLO);

  QLabel *filterL=new QLabel("Filter: ");
  treeWidgetLO->addWidget(filterL,0,0);
  filterL->setToolTip("Enter a regular expression to filter the list.");
  filter=new QLineEdit(this);
  filter->setToolTip("Enter a regular expression to filter the list.");
  connect(filter,SIGNAL(returnPressed()),this,SLOT(filterObjectList()));
  treeWidgetLO->addWidget(filter,0,1);

  treeWidget = new QTreeWidget;
  treeWidgetLO->addWidget(treeWidget,1,0,1,2);

  treeWidget->setHeaderHidden(true);
  treeWidget->setColumnCount(1);

  treeWidgetLO->addWidget(new QLabel("Path: "),2,0);
  path=new QLineEdit(this);
  treeWidgetLO->addWidget(path,2,1);
  path->setReadOnly(true);


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
  fileMenu->addAction("Open File(s)...", this, SLOT(openFileDialog()));
  menuBar()->addMenu(fileMenu);
  QMenu *plotMenu=new QMenu("Plot", menuBar());
  plotMenu->addAction("Add Plot...", this, SLOT(addPlotWindow()));
  menuBar()->addMenu(plotMenu);
  plotMenu->addAction("Print Plot...", this, SLOT(printPlotWindow()));
  //QAction *exportPlotAct=plotMenu->addAction("Export Plot...", this, SLOT(exportPlot2SVG()));

  // help menu
  menuBar()->addSeparator();
  QMenu *helpMenu=new QMenu("Help", menuBar());
  helpMenu->addAction("GUI Help...", this, SLOT(help()));
  helpMenu->addAction("About h5plotserie...", this, SLOT(about()));
  menuBar()->addMenu(helpMenu);

  // title
  setWindowTitle("h5plotserie");

  QObject::connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(updateData(QTreeWidgetItem*,int)));
  QObject::connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(plot(QListWidgetItem*)));
  QObject::connect(mdiArea,SIGNAL(subWindowActivated ( QMdiSubWindow *)), this, SLOT(windowChanged(QMdiSubWindow*)));
  QObject::connect(tableWidget,SIGNAL(pressed(QModelIndex)), this, SLOT(openContextMenuTable()));
  QObject::connect(treeWidget,SIGNAL(pressed(QModelIndex)), this, SLOT(openContextMenuTree()));
  QObject::connect(treeWidget,SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(updatePath(QTreeWidgetItem *)));
}

void MainWindow::addFile(const QString &name) {

  file.append(new H5::H5File(name.toStdString(), H5F_ACC_RDONLY));

  int j = file.size()-1;

  fileInfo.append(name);

  TreeWidgetItem *topitem = new TreeWidgetItem(QStringList(fileInfo[j].fileName()));
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

void MainWindow::detachCurve() {
  if(myPlot) {
    int row = tableWidget->indexOfTopLevelItem(tableWidget->currentItem());
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

void MainWindow::openContextMenuTable() {
  if(QApplication::mouseButtons() & Qt::RightButton) {
    QMenu *menu=new QMenu("Curve Menu");
    QAction *action = new QAction("Detach",this);
    menu->addAction(action);
    connect(action,SIGNAL(triggered(bool)),this,SLOT(detachCurve()));
    menu->exec(QCursor::pos());
    delete menu;
  }
}

void MainWindow::openContextMenuTree() {
  QTreeWidgetItem* item = treeWidget->currentItem();
  updateData(treeWidget->currentItem(),0);
  if(item && item->parent()==0) {
    if(QApplication::mouseButtons() & Qt::RightButton) {
      QAction* action;
      QMenu *menu=new QMenu("Object Menu");
    //  action = new QAction("Reload",this);
    //  menu->addAction(action);
    //  connect(action,SIGNAL(triggered(bool)),this,SLOT(reloadFile()));
      action = new QAction("Unload",this);
      menu->addAction(action);
      connect(action,SIGNAL(triggered(bool)),this,SLOT(closeFile()));
      menu->exec(QCursor::pos());
      delete menu;
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
    // check, whether file has changed
    checkForFileModification();

    QString path = static_cast<TreeWidgetItem*>(treeWidget->currentItem())->getPath();
    int col = listWidget->row(item);
    VectorSerie<double> vs;
    int j = getTopLevelIndex(treeWidget->currentItem());

    vs.open(*file[j],path.toStdString());
    QString fullName = fileInfo[j].fileName()+path;

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
	curve->setxIndex(col);
	curve->setxFileIndex(j);
	curve->setxBasicPath(path);
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
	newcurve->setxBasicPath(path);
	newcurve->setyBasicPath(path);
	newcurve->setxLabel(xlabel);
	newcurve->setyLabel(ylabel);
	newcurve->setxIndex(0);
	newcurve->setyIndex(col);
	newcurve->setxFileIndex(j);
	newcurve->setyFileIndex(j);
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
      newcurve->setxBasicPath(path);
      newcurve->setyBasicPath(path);
      newcurve->setxLabel(xlabel);
      newcurve->setyLabel(ylabel);
      newcurve->setxIndex(0);
      newcurve->setyIndex(col);
      newcurve->setxFileIndex(j);
      newcurve->setyFileIndex(j);
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
      "  <dt>Right-Click</dt><dd> Remove curve </dd>"
      "<h2>Keyboard actions</h2>"
      "  <dt>a</dt><dd> Update current plot window and rescale </dd>"
      "  <dt>e</dt><dd> Update current plot window </dd>"
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
      "</ul>"
      "<h2>This program uses:</h2>"
      "<ul>"
      "  <li>'Qt - A cross-platform application and UI framework' by Nokia from <tt>http://www.qtsoftware.com</tt> (License: GPL/LGPL)</li>"
      "  <li>'Qwt - Qt Widgets for Technical Applications' by Uwe Rathmann from <tt>http://qwt.sourceforge.net</tt> (Licence: Qwt/LGPL)</li>"
      "  <li>'HDF5Serie - A HDF5 Wrapper for Time Series' by Markus Friedrich from <tt>http://code.google.com/p/hdf5serie/</tt> (License: LGPL)</li>"
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

void MainWindow::printPlotWindow() {
    QPrinter printer;
    //printer.setOutputFileName("./plot.ps");

    QString docName = myPlot->title().text();
    cout << docName.toStdString() << endl;
    if ( !docName.isEmpty() ) {
        docName.replace (QRegExp (QString::fromLatin1 ("\n")), tr (" -- "));
        printer.setDocName (docName);
    }

    printer.setCreator("Print Plot");
    printer.setOrientation(QPrinter::Landscape);

    QPrintDialog dialog(&printer);
    if ( dialog.exec() ) {
        QwtPlotPrintFilter filter;
        if ( printer.colorMode() == QPrinter::GrayScale ) {
            filter.setOptions(QwtPlotPrintFilter::PrintAll 
                & ~QwtPlotPrintFilter::CanvasBackground);
        }
        myPlot->print(printer, filter);
    }
}

//void MainWindow::exportPlot2SVG() {
//    QString fileName = "bode.svg";
//
//#ifdef QT_SVG_LIB
//#ifndef QT_NO_FILEDIALOG
//    fileName = QFileDialog::getSaveFileName(
//        this, "Export File Name", QString(),
//        "SVG Documents (*.svg)");
//#endif
//    if ( !fileName.isEmpty() )
//    {
//        QSvgGenerator generator;
//        generator.setFileName(fileName);
//        generator.setSize(QSize(800, 600));
//
//        myPlot->print(generator);
//    }
//#endif
//}

void MainWindow::checkForFileModification() {

  // check, whether a file has changed
  for(int j=0; j< fileInfo.size(); j++) {
    QFileInfo info(fileInfo[j].absoluteFilePath());
    if(info.lastModified().toTime_t() != fileInfo[j].lastModified().toTime_t()) {
      file[j]->close();
      while(info.lastModified().toTime_t() != fileInfo[j].lastModified().toTime_t()) {
	fileInfo[j]=info;
      }
      //QString str = QString("kill -USR2 $(cat ") + fileInfo[j].absolutePath() + QString("/.") + fileInfo[j].fileName() + QString(".pid 2> /dev/null) >& /dev/null");
      //system(str.toStdString().c_str()); 
    //  system("sleep 1");

      file[j]->openFile(fileInfo[j].absoluteFilePath().toStdString(), H5F_ACC_RDONLY);
    }
  }
}

void MainWindow::updatePlotWindow() {

  checkForFileModification();

  QList<QMdiSubWindow *> list = mdiArea->subWindowList();

  QwtPlotItemList il = myPlot->itemList();
  for(int i=0; i<il.size(); i++) {

    MyCurve *curve = static_cast<MyCurve*>(il[i]);
    int jx = curve->getxFileIndex();
    int jy = curve->getyFileIndex();
    VectorSerie<double> vsx;
    VectorSerie<double> vsy;
    vsx.open(*file[jx],curve->getxBasicPath().toStdString());
    vsy.open(*file[jy],curve->getyBasicPath().toStdString());
    vector<double> x = vsx.getColumn(curve->getxIndex());
    vector<double> y = vsy.getColumn(curve->getyIndex());
    curve->setData(&x[0],&y[0],x.size());
    vsx.close();
    vsy.close();
  }
} 

void MainWindow::keyPressEvent(QKeyEvent *event) {
  switch (event->key()) {
    case Qt::Key_A:
      if(myPlot) {
	updatePlotWindow();
	myPlot->setAxisAutoScale(QwtPlot::xBottom);
	myPlot->setAxisAutoScale(QwtPlot::yLeft);
	myPlot->replot();
	static_cast<MyPlot*>(myPlot)->getZoom()->setZoomBase();
      }
      break;
    case Qt::Key_E:
      if(myPlot) {
	updatePlotWindow();
	myPlot->replot();
	static_cast<MyPlot*>(myPlot)->getZoom()->setZoomBase();
      }
      break;
    default:
      QMainWindow::keyPressEvent(event);
  }
}

void MainWindow::closeFile() {
  QTreeWidgetItem* item = treeWidget->currentItem();
  if(item && item->parent()==0) {
    int index = treeWidget->indexOfTopLevelItem(item);
    treeWidget->takeTopLevelItem(index);
    updateTableWidget();
    file[index]->close();
    file.removeAt(index);
    fileInfo.removeAt(index);
  }
}

void MainWindow::updatePath(QTreeWidgetItem *cur) {
  QString str=cur->text(0);
  for(QTreeWidgetItem *item=cur->parent(); item!=0; item=item->parent())
    str=item->text(0)+"/"+str;
  path->setText(str);
};

// MainWindow::filterObjectList(...) and MainWindow::searchObjectList(...) are taken from OpenMBV.
// If changes are made here, please do the same changes in OpenMBV
void MainWindow::filterObjectList() {
  QRegExp filterRegExp(filter->text());
  searchObjectList(treeWidget->invisibleRootItem(), filterRegExp);
}
void MainWindow::searchObjectList(QTreeWidgetItem *item, const QRegExp& filterRegExp) {
  for(int i=0; i<item->childCount(); i++) {
    // search recursive
    searchObjectList(item->child(i), filterRegExp);
    // set color
    QColor c=item->child(i)->foreground(0).color();
    c.setRed(filterRegExp.indexIn(item->child(i)->text(0))<0?255:0);
    item->child(i)->setForeground(0, QBrush(c));
    // if all children and children children are red, collapse
    int count=0;
    for(int j=0; j<item->child(i)->childCount(); j++)
      if((item->child(i)->child(j)->childCount()!=0 && item->child(i)->child(j)->foreground(0).color().red()==255 && 
          item->child(i)->child(j)->isExpanded()==false) ||
         (item->child(i)->child(j)->childCount()==0 && item->child(i)->child(j)->foreground(0).color().red()==255))
        count++;
    item->child(i)->setExpanded(count!=item->child(i)->childCount());
    // hide
    item->child(i)->setHidden(false);
    if((item->child(i)->childCount()!=0 && item->child(i)->foreground(0).color().red()==255 && 
        item->child(i)->isExpanded()==false) ||
       (item->child(i)->childCount()==0 && item->child(i)->foreground(0).color().red()==255)) {
      bool hide=true;
      for(QTreeWidgetItem *it=item; it!=0; it=it->parent())
        if(filterRegExp.indexIn(it->text(0))>=0) { hide=false; break; }
      item->child(i)->setHidden(hide);
    }
  }
}

void MyCurve::draw(QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &yMap, int from, int to) const {

  if (to < 0)
    to = dataSize() - 1;

  size_t first, last = from;
  while ((int)last <= to) {
    first = last;
    while ((int)first <= to && isNaN(data().y(first)))
      ++first;
    last = first;
    while ((int)last <= to && !isNaN(data().y(last)))
      ++last;
    if ((int)first <= to)
      QwtPlotCurve::draw(p, xMap, yMap, first, last - 1);
  }
}

QwtDoubleRect MyCurve::boundingRect() const {

  if (dataSize() <= 0)
    return QwtDoubleRect(1.0, 1.0, -2.0, -2.0); // Empty data.

  size_t first = 0;//, last = dataSize() - 1;
  while ((int)first < dataSize() && isNaN(data().y(first)))
    ++first;

  if ((int)first == dataSize())
    return QwtDoubleRect(1.0, 1.0, -2.0, -2.0); // Empty data.

  double minX, maxX, minY, maxY;
  minX = maxX = x(first);
  minY = maxY = y(first);
  for (size_t i = first + 1; i < (unsigned int)dataSize(); ++i)
  {
    const double xv = x(i);
    if (xv < minX)
      minX = xv;
    if (xv > maxX)
      maxX = xv;
    const double yv = y(i);
    if (yv < minY)
      minY = yv;
    if (yv > maxY)
      maxY = yv;
  }
  return QwtDoubleRect(minX, minY, maxX - minX, maxY - minY);
}


