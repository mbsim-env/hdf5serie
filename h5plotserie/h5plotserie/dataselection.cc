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
#include "QApplication"
#include "QGridLayout"
#include "QLabel"
#include "QLineEdit"
#include "QListWidget"
#include "QFileInfo"
#include <QDomDocument>
#include "dataselection.h"

#include <hdf5serie/vectorserie.h>

#include "treewidgetitem.h"
#include "plotdata.h"
#include "plotarea.h"
#include "curves.h"
#include "mainwindow.h"

using namespace std;

DataSelection::DataSelection(QWidget * parent) : QSplitter(parent) {
  connect(this, &DataSelection::reopenAllSignal, this, &DataSelection::reopenAll);
  connect(this, &DataSelection::refreshFileSignal, this, &DataSelection::refreshFile);

  QWidget* dummy=new QWidget(this);
  addWidget(dummy);

  auto * fileSelection=new QGridLayout(this);
  dummy->setLayout(fileSelection);

  fileBrowser = new QTreeWidget(this);
  fileSelection->addWidget(fileBrowser,1,0,1,2);
  fileBrowser->setHeaderHidden(true);
  fileBrowser->setColumnCount(1);
  dataSelectionFilter=new OpenMBVGUI::AbstractViewFilter(fileBrowser);
  fileSelection->addWidget(dataSelectionFilter, 0,0,1,2);

  QLabel * pathLabel=new QLabel("Path:");
  fileSelection->addWidget(pathLabel,2,0);
  path=new QLineEdit(this);
  fileSelection->addWidget(path,2,1);
  path->setReadOnly(true);

  currentData=new QListWidget(this);
  addWidget(currentData);

  QObject::connect(fileBrowser, &QTreeWidget::itemClicked, this, &DataSelection::selectFromFileBrowser);
  QObject::connect(currentData, &QListWidget::itemClicked, this, &DataSelection::selectFromCurrentData);
  QObject::connect(fileBrowser, &QTreeWidget::currentItemChanged, this, &DataSelection::updatePath);
}

DataSelection::~DataSelection() {
  delete dataSelectionFilter;
  if (fileBrowser) {
    delete fileBrowser;
    fileBrowser=nullptr;
  }
  if (path) {
    delete path;
    path=nullptr;
  }
}

void DataSelection::addFile(const QString &name) {
  fileInfo.append(name);
  file.append(name);
  std::shared_ptr<H5::File> h5f;
  h5f=std::make_shared<H5::File>(file.back().toStdString(), H5::File::read, [this](){
    // close request
    reopenAllSignal();
  }, [this, name](){
    // refresh callback
    refreshFileSignal(name);
  });
  h5File.emplace_back(name.toStdString(), h5f);

  TreeWidgetItem *topitem = new TreeWidgetItem(QStringList(fileInfo.back().fileName()));
  topitem->setToolTip(0, fileInfo.back().absoluteFilePath());
  fileBrowser->addTopLevelItem(topitem);
  list<string> names=h5f->getChildObjectNames();
  for(const auto & name : names) {
    QTreeWidgetItem *item = new TreeWidgetItem(QStringList(name.c_str()));
    auto *grp = h5f->openChildObject<H5::Group>(name);
    insertChildInTree(grp, item);
    topitem->addChild(item);
  }
}

void DataSelection::rebuild(QTreeWidgetItem *item, Node &node) {
  int i;
  for(i=0; i<node.getNumberOfChilds(); i++) {
    if(item->text(0) == node.getChild(i).getName()) {
      item->setSelected(node.getChild(i).isSelected());
      item->setExpanded(node.getChild(i).isExpanded());
      break;
    }
  }
  if(item->isExpanded()) {
    for(int j=0; j<item->childCount(); j++)
      rebuild(item->child(j),node.getChild(i));
  }
  if(item->isSelected()) {
      fileBrowser->setCurrentItem(item);
      selectFromFileBrowser(item,0);
      updatePath(item);
      currentData->setCurrentRow(node.getChild(i).getRow());
  }
}

void DataSelection::save(QTreeWidgetItem *item, Node &node) {
  node.addChild(Node(item->text(0),item->isExpanded(),item->isSelected(),item->isSelected()?currentData->currentRow():0,this));
  for(int i=0; i<item->childCount(); i++)
    save(item->child(i),node.getChild(node.getNumberOfChilds()-1));
}

void DataSelection::reopenAll() {

  Node root("Root",false,false,0,this);
  for(int i=0; i<fileBrowser->topLevelItemCount(); i++)
    save(fileBrowser->topLevelItem(i),root);

  QList<QFileInfo> info = fileInfo;

  auto curves=static_cast<MainWindow*>(parent()->parent())->getCurves();

  // save all opened content
  auto doc=curves->saveCurves();

  // close all
  path->setText("");
  currentData->clear();
  // close plot MIDs
  auto plotArea=static_cast<MainWindow*>(parent()->parent())->getPlotArea();
  for(auto &sw : plotArea->subWindowList())
    delete sw;
  // close "Curves"
  for(int tab=curves->count()-1; tab>=0; --tab)
    delete curves->widget(tab);
  // close "Data Selection"
  for(int idx=fileBrowser->topLevelItemCount(); idx>=0; --idx)
    delete fileBrowser->topLevelItem(idx);
  // close files
  h5File.clear();
  file.clear();
  fileInfo.clear();

  for(int i=0; i<info.size(); i++)
    addFile(info.at(i).absoluteFilePath());


  curves->loadCurve(doc.get());
  if(curves->count()==0) {
    QString windowTitle = QString("Plot 1");
    curves->addTab(new PlotDataTable((QWidget*)(this), windowTitle), windowTitle);
    plotArea->addPlotWindow(curves->tabText(curves->currentIndex()));
  }
  for(int i=0; i<fileBrowser->topLevelItemCount(); i++)
    rebuild(fileBrowser->topLevelItem(i),root);
}

void DataSelection::refreshFile(const QString &name) {
  auto it=find_if(h5File.begin(), h5File.end(), [&name](const std::pair<boost::filesystem::path, std::shared_ptr<H5::File>> &f){
    return f.first==name.toStdString();
  });
  if(it==h5File.end())
    return;
  auto h5f=it->second;
  h5f->refresh();
  auto curves=static_cast<MainWindow*>(parent()->parent())->getCurves();
  if(curves) curves->plotAllTabs();
}

shared_ptr<H5::File> DataSelection::getH5File(const boost::filesystem::path &p) const {
  auto it=find_if(h5File.begin(), h5File.end(), [&p](const decltype(h5File)::value_type& a){
    return boost::filesystem::equivalent(a.first, p);
  });
  if(it==h5File.end())
    throw runtime_error("Cannot find "+p.string()+" in h5File.");
  return it->second;
}

void DataSelection::insertChildInTree(H5::Group *grp, QTreeWidgetItem *item) {
  list<string> names=grp->getChildObjectNames();
  for(const auto & name : names) {
    QTreeWidgetItem *child = new TreeWidgetItem(QStringList(name.c_str()));
    item->addChild(child);
    auto *g=dynamic_cast<H5::Group*>(grp->openChildObject(name));
    if(g)
      insertChildInTree(g, child);
    else {
      if(name == "data") {
        QString path; 
        getPath(item,path,0);
        path += "/data";
        static_cast<TreeWidgetItem*>(child)->setPath(path);
//	std::shared_ptr<H5::File> h5f=getH5File(file[0].toStdString());
//	H5::VectorSerie<double> *vs = h5f->openChildObject<H5::VectorSerie<double> >(path.toStdString());
//	size_t rows=vs->getRows();
//	std::vector<double> xVal(rows);
//	vs->getColumn(0, xVal);
//	cout << xVal[0] << " " << xVal[1] << endl;
      }
    }
  }
}

void DataSelection::getPath(QTreeWidgetItem* item, QString &s, int col) {
  QTreeWidgetItem* parentWidget = item->parent();
  if(parentWidget)
    getPath(parentWidget, s, col);
  s  += "/" + item->text(col);
}

void DataSelection::selectFromFileBrowser(QTreeWidgetItem* item, int col) {
  currentData->clear();
  if( item->text(col) == "data") {
    QString path = static_cast<TreeWidgetItem*>(item)->getPath();
    int j = getTopLevelIndex(item);
    std::shared_ptr<H5::File> h5f=getH5File(file[j].toStdString());
    auto *vs=h5f->openChildObject<H5::VectorSerie<double> >(path.toStdString());
    QStringList sl;
    for(unsigned int i=0; i<vs->getColumns(); i++)
      sl << vs->getColumnLabel()[i].c_str();
    currentData->addItems(sl);
  }
}

int DataSelection::getTopLevelIndex(QTreeWidgetItem* item) {
  QTreeWidgetItem* parentWidget = item->parent();
  if(parentWidget)
    return getTopLevelIndex(parentWidget);
  else // item is TopLevelItem
    return fileBrowser->indexOfTopLevelItem(item); 
}

void DataSelection::selectFromCurrentData(QListWidgetItem* item) {
  QString path = static_cast<TreeWidgetItem*>(fileBrowser->currentItem())->getPath();
  int col = currentData->row(item);
  int j = getTopLevelIndex(fileBrowser->currentItem());
  std::shared_ptr<H5::File> h5f=getH5File(file[j].toStdString());
  auto *vs=h5f->openChildObject<H5::VectorSerie<double> >(path.toStdString());

  PlotData pd;
  pd.setValue("Filepath", fileInfo[j].absolutePath());
  pd.setValue("Filename", fileInfo[j].fileName());
  pd.setValue("x-Label", QString::fromStdString(vs->getColumnLabel()[0]));
  pd.setValue("y-Label", QString::fromStdString(vs->getColumnLabel()[col]));
  pd.setValue("offset", "0");
  pd.setValue("gain", "1");
  pd.setValue("x-Path", path);
  pd.setValue("y-Path", path);
  pd.setValue("x-Index", QString("%1").arg(0));
  pd.setValue("y-Index", QString("%1").arg(col));

  if (QApplication::keyboardModifiers() & Qt::ShiftModifier)
    static_cast<MainWindow*>(parent()->parent())->getCurves()->modifyPlotData(pd, "add");
  else if (QApplication::keyboardModifiers() & Qt::ControlModifier)
    static_cast<MainWindow*>(parent()->parent())->getCurves()->modifyPlotData(pd, "new");
  else
    static_cast<MainWindow*>(parent()->parent())->getCurves()->modifyPlotData(pd, "replace");
}

void DataSelection::updatePath(QTreeWidgetItem *cur) {
  QString str=cur->text(0);
  for (QTreeWidgetItem *item=cur->parent(); item!=nullptr; item=item->parent())
    str=item->text(0)+"/"+str;
  path->setText(str);
};

void DataSelection::requestFlush() {
  for(auto h5f : h5File)
    h5f.second->requestFlush();
}
