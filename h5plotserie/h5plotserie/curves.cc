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
#include "curves.h"
#include "plotarea.h"
#include "plotdata.h"
#include "mainwindow.h"
#include "dataselection.h"
#include "dialogs.h"
#include <QFile>
#include <QFileInfo>
#include <QHeaderView>
#include <QDomNode>
#include <QApplication>
#include <QMenu>
#include <QShortcut>

Curves::Curves(QWidget *parent) : QTabWidget(parent) {
  setUsesScrollButtons(true);
  connect(new QShortcut(QKeySequence::Delete,this), &QShortcut::activated, this, &Curves::deletePressed);
  auto plotArea = static_cast<MainWindow*>(parent)->getPlotArea();
  connect(this,&QTabWidget::tabBarClicked,this,[=](int i) { plotArea->setActiveSubWindow(plotArea->subWindowList().at(i)); });
  connect(plotArea,&QMdiArea::subWindowActivated,this, [=](QMdiSubWindow* subwindow) { setCurrentIndex(plotArea->subWindowList().indexOf(subwindow)); });
  connect(this, &QTabWidget::tabBarDoubleClicked, this, &Curves::tabBarDoubleClicked);
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
    QString label = count()?"Plot "+QString::number(tabText(count()-1).mid(5).toInt()+1):"Plot 1";
    addTab(new PlotDataTable(this, label), label);
    setCurrentWidget(widget(count()-1));
    static_cast<PlotDataTable*>(currentWidget())->addDataSet(pd);
    static_cast<MainWindow*>(parent()->parent())->getPlotArea()->addPlotWindow(label);
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

void Curves::plotCurrentTab() {
  auto *plotWindow = static_cast<PlotWindow*>(static_cast<PlotArea*>(static_cast<MainWindow*>(parent()->parent())->getPlotArea())->activeSubWindow());
  plotWindow->detachPlot();
  auto *plotDataTable = static_cast<PlotDataTable*>(currentWidget());
  for (int i=0; i<plotDataTable->rowCount(); i++) {
    PlotData pd;
    for (int j=0; j<pd.numberOfItems(); j++)
      pd.setValue(j, plotDataTable->item(i, j)->text());
    plotWindow->plotDataSet(pd, i);
  }
  plotWindow->replotPlot();
}

void Curves::plotAllTabs() {
  auto list = static_cast<PlotArea*>(static_cast<MainWindow*>(parent()->parent())->getPlotArea())->subWindowList();
  for(int i=0; i<count(); i++) {
    auto *plotWindow = static_cast<PlotWindow*>(list.at(i));
    plotWindow->detachPlot();
    auto *plotDataTable = static_cast<PlotDataTable*>(widget(i));
    for (int i=0; i<plotDataTable->rowCount(); i++) {
      PlotData pd;
      for (int j=0; j<pd.numberOfItems(); j++)
	pd.setValue(j, plotDataTable->item(i, j)->text());
      plotWindow->plotDataSet(pd, i);
    }
    plotWindow->replotPlot();
  }
}

std::shared_ptr<QDomDocument> Curves::saveCurves() {
  auto doc=std::make_shared<QDomDocument>("h5PlotDataset");
  QDomElement root = doc->createElement("h5PlotDataset");
  doc->appendChild(root);
  for (int t=0; t<count(); t++) {
    QDomElement tabTag = doc->createElement("tab");
    root.appendChild(tabTag);
    static_cast<PlotDataTable*>(widget(t))->savePlot(doc.get(), &tabTag);
  }
  return doc;
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

void Curves::loadCurve(QDomDocument *doc) {
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

void Curves::removeTab(const QString &name) {
  for(int i=0; i<count(); i++) {
    if(tabText(i)==name) {
      auto *tabWidget = widget(i);
      QTabWidget::removeTab(indexOf(tabWidget));
      delete tabWidget;
    }
  }
}

void Curves::changeTabName(int i, const QString &name) {
  setTabText(i,name);
  static_cast<PlotWindow*>(static_cast<MainWindow*>(parent()->parent())->getPlotArea()->subWindowList().at(i))->setWindowTitle(name);
}

void Curves::deletePressed() {
  if(count()) {
    auto *widget = static_cast<QTableWidget*>(currentWidget());
    int row = widget->currentRow();
    if(row>=0) {
      widget->removeRow(row);
      plotCurrentTab();
    }
  }
}

void Curves::tabBarDoubleClicked(int i) {
  auto *dialog = new LineEditDialog("Edit plot window name", tabText(i), this);
  connect(dialog, &LineEditDialog::accepted, this, [=]() { changeTabName(i,dialog->getText()); });
  dialog->exec();
}


PlotDataTable::PlotDataTable(QWidget *parent, const QString &name) : QTableWidget(parent) {
  setObjectName(name);
  setWindowTitle(name);

  PlotData pd;
  setRowCount(0);
  setColumnCount(pd.numberOfItems());
  for (int i=0; i<pd.numberOfItems(); i++) 
    setHorizontalHeaderItem(i, new QTableWidgetItem(pd.string(i)));

  connect(this, &PlotDataTable::cellChanged, static_cast<Curves*>(parent), &Curves::plotCurrentTab);
  connect(this, &QTableWidget::cellPressed, this, &PlotDataTable::dataSetClicked);

  horizontalHeader()->setSectionsClickable(true);
  horizontalHeader()->installEventFilter(this);
}

void PlotDataTable::addDataSet(PlotData pd) {
  blockSignals(true);
  insertRow(rowCount());
  for (int i=0; i<pd.numberOfItems(); i++)
    setItem(rowCount()-1, i, new QTableWidgetItem(pd.getValue(pd.string(i))));
  resizeColumnsToContents();
  resizeRowsToContents();
  blockSignals(false);
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

void PlotDataTable::savePlot(QDomDocument *doc, QDomElement *tab) {
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

void PlotDataTable::dataSetClicked(int row, int col) {
  if(QApplication::mouseButtons()==Qt::RightButton) {
    QMenu *menu = new QMenu;
    QAction *action=new QAction("Remove curve", menu);
    action->setShortcut(QKeySequence::Delete);
    connect(action,&QAction::triggered,this,[=](){ removeRow(row); static_cast<Curves*>(parent()->parent())->plotCurrentTab(); });
    menu->addAction(action);
    menu->exec(QCursor::pos());
    delete menu;
  }
}
