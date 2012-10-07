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

#include "QFile"
#include "QFileInfo"
#include "QHeaderView"
#include "QDomNode"
#include "curves.h"
#include "plotarea.h"
#include "plotdata.h"
#include "mainwindow.h"
#include "dataselection.h"

Curves::Curves(QWidget * parent) : QTabWidget(parent), numberOfWindows(0) {
  setUsesScrollButtons(true);

  QString windowTitle = QString("Plot %1").arg(++numberOfWindows);
  addTab(new PlotDataTable((QWidget*)(this), windowTitle), windowTitle);
  static_cast<MainWindow*>(parent)->getPlotArea()->addPlotWindow(tabText(currentIndex()));
}

void Curves::modifyPlotData(PlotData pd, const QString &mode) {
  if (QString::compare(mode, "add", Qt::CaseSensitive)==0) {
    if (currentIndex()==-1)
      modifyPlotData(pd, "new");
    else {
      QList<QTableWidgetItem*> selected = static_cast<PlotDataTable*>(currentWidget())->selectedItems();
      if (selected.size()==1) {
        QString columnHeader=pd.string(selected[0]->column());
        PlotData pdSave;
        for (int i=0; i<pdSave.numberOfItems(); i++)
          pdSave.setValue(i, static_cast<PlotDataTable*>(currentWidget())->item(selected[0]->row(), i)->text());
        if ((QString::compare(columnHeader, "x-Label", Qt::CaseSensitive)==0) ||
            (QString::compare(columnHeader, "x-Index", Qt::CaseSensitive)==0) ||
            (QString::compare(columnHeader, "x-Path", Qt::CaseSensitive)==0)) {
          pdSave.setValue("x-Label", pd.getValue("y-Label"));
          pdSave.setValue("x-Index", pd.getValue("y-Index"));
          pdSave.setValue("x-Path", pd.getValue("y-Path"));
        }
        else if ((QString::compare(columnHeader, "y-Label", Qt::CaseSensitive)==0) ||
            (QString::compare(columnHeader, "y-Index", Qt::CaseSensitive)==0) ||
            (QString::compare(columnHeader, "y-Path", Qt::CaseSensitive)==0)) {
          pdSave.setValue("y-Label", pd.getValue("y-Label"));
          pdSave.setValue("y-Index", pd.getValue("y-Index"));
          pdSave.setValue("y-Path", pd.getValue("y-Path"));
          pdSave.setValue("gain", "1");
          pdSave.setValue("offset", "0");
        }
        else if ((QString::compare(columnHeader, "y2-Label", Qt::CaseSensitive)==0) ||
            (QString::compare(columnHeader, "y2-Index", Qt::CaseSensitive)==0) ||
            (QString::compare(columnHeader, "y2-Path", Qt::CaseSensitive)==0)) {
          pdSave.setValue("y2-Label", pd.getValue("y-Label"));
          pdSave.setValue("y2-Index", pd.getValue("y-Index"));
          pdSave.setValue("y2-Path", pd.getValue("y-Path"));
          pdSave.setValue("y2gain", "1");
          pdSave.setValue("y2offset", "0");
        }
        static_cast<PlotDataTable*>(currentWidget())->replaceDataSet(pdSave);
      }
      else
        static_cast<PlotDataTable*>(currentWidget())->addDataSet(pd);
    }
  }
  else if (QString::compare(mode, "new", Qt::CaseSensitive)==0) {
    QString windowTitle = QString("Plot %1").arg(++numberOfWindows);
    addTab(new PlotDataTable((QWidget*)(this), windowTitle), windowTitle);
    setCurrentWidget(widget(count()-1));
    static_cast<PlotDataTable*>(currentWidget())->addDataSet(pd);
    static_cast<MainWindow*>(parent()->parent())->getPlotArea()->addPlotWindow(windowTitle);
  }
  else if (QString::compare(mode, "replace", Qt::CaseSensitive)==0) {
    if (currentIndex()==-1)
      modifyPlotData(pd, "new");
    else {
      static_cast<PlotDataTable*>(currentWidget())->clearTable();
      static_cast<PlotDataTable*>(currentWidget())->addDataSet(pd);
    }
  }
  plotCurrentTab();
}

void Curves::refreshAllTabs() {
  int currentTab=currentIndex();
  int numberOfTabs=count();
  for (int i=0; i<numberOfTabs; i++) {
    setCurrentIndex(i);
    plotCurrentTab();
  }
  setCurrentIndex(currentTab);
}

void Curves::plotCurrentTab() {
  const QString tabName = tabText(currentIndex());
  PlotWindow * plotWindow = static_cast<MainWindow*>(parent()->parent())->getPlotArea()->findChild<PlotWindow*>(tabName);

  plotWindow->detachPlot();
  PlotDataTable * plotDataTable = static_cast<PlotDataTable*>(currentWidget());
  for (int i=0; i<plotDataTable->rowCount(); i++) {
    PlotData pd;
    for (int j=0; j<pd.numberOfItems(); j++)
      pd.setValue(j, plotDataTable->item(i, j)->text());
    plotWindow->plotDataSet(pd, i);
  }
  plotWindow->replotPlot();
}

QString Curves::saveCurves() {
  QDomDocument doc("h5PlotDataset");
  QDomElement root = doc.createElement("h5PlotDataset");
  doc.appendChild(root);
  for (int t=0; t<count(); t++) {
    QDomElement tabTag = doc.createElement("tab");
    root.appendChild(tabTag);
    static_cast<PlotDataTable*>(widget(t))->savePlot(&doc, &tabTag);
  }
  return doc.toString();
}

void Curves::initLoadCurve(const QString &fileName) {
  QDomDocument doc("h5PlotDataset");
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly))
    return;
  if (!doc.setContent(&file)) {
    file.close();
    return;
  }
  loadCurve(&doc);
  file.close();
}

void Curves::loadCurve(QDomDocument * doc) {
  QDomElement docElem = doc->documentElement();
  QDomNode tabData = docElem.firstChildElement("tab");
  while(!tabData.isNull()) {
    QDomNode dataData = tabData.firstChildElement("data");
    bool newTab=true;
    while(!dataData.isNull()) {
      PlotData pd;
      for (int i=0; i<pd.numberOfItems(); i++) {
        QDomNode colData = dataData.firstChildElement(pd.xmlString(i));
        if (colData.isElement())
          pd.setValue(i, colData.toElement().text());
      }
      
      QList<QFileInfo> * loadedFiles=static_cast<MainWindow*>(parent()->parent())->getDataSelection()->getFileInfo();
      bool alreadyLoaded=false;
      int i=0;
      while (i<loadedFiles->size() && (alreadyLoaded==false)) {
        if (QString::compare(pd.getValue("Filename"), (loadedFiles->at(i)).fileName(), Qt::CaseSensitive)==0)
          alreadyLoaded=true;
        i++;
      }
      if (!alreadyLoaded) {
        if (QFile::exists("./"+pd.getValue("Filename"))) // in actual directory
          static_cast<MainWindow*>(parent()->parent())->getDataSelection()->addFile("./"+pd.getValue("Filename"));
        else // absolut path
          static_cast<MainWindow*>(parent()->parent())->getDataSelection()->addFile(pd.getValue("Filepath")+"/"+pd.getValue("Filename"));
      }
      
      if (newTab) {
        modifyPlotData(pd, "new");
        newTab=false;
      }
      else
        modifyPlotData(pd, "add");
      dataData = dataData.nextSiblingElement("data");
    }
    tabData = tabData.nextSiblingElement("tab");
  }
}

PlotDataTable::PlotDataTable(QWidget * parent, const QString &name) : QTableWidget(parent) {
  setObjectName(name);
  setWindowTitle(name);

  PlotData pd;
  setRowCount(0);
  setColumnCount(pd.numberOfItems());
  for (int i=0; i<pd.numberOfItems(); i++) 
    setHorizontalHeaderItem(i, new QTableWidgetItem(pd.string(i)));

  QObject::connect(this, SIGNAL(currentItemChanged(QTableWidgetItem *, QTableWidgetItem *)), parent, SLOT(plotCurrentTab()));

  horizontalHeader()->setClickable(true);
  horizontalHeader()->installEventFilter(this);
}

void PlotDataTable::addDataSet(PlotData pd) {
  insertRow(rowCount());
  for (int i=0; i<pd.numberOfItems(); i++)
    setItem(rowCount()-1, i, new QTableWidgetItem(pd.getValue(pd.string(i))));
  resizeColumnsToContents();
  resizeRowsToContents();
}

void PlotDataTable::replaceDataSet(PlotData pd) {
  for (int i=0; i<pd.numberOfItems(); i++)
    setItem(currentRow(), i, new QTableWidgetItem(pd.getValue(pd.string(i))));
  resizeColumnsToContents();
  resizeRowsToContents();
}

void PlotDataTable::clearTable() {
  for (int i=rowCount(); i>0; i--)
    removeRow(i-1);
}

void PlotDataTable::savePlot(QDomDocument * doc, QDomElement * tab) {
  for (int r=0; r<rowCount(); r++) {
    QDomElement datasetTag=doc->createElement("data");
    tab->appendChild(datasetTag);
    PlotData pd;
    for (int c=0; c<pd.numberOfItems(); c++) {
      QDomElement dataTag=doc->createElement(pd.xmlString(c));
      datasetTag.appendChild(dataTag);
      QDomText t = doc->createTextNode(item(r, c)->text());
      dataTag.appendChild(t);
    }
  }
}

