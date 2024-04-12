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
#include "QApplication"
#include "QGridLayout"
#include "QLabel"
#include "QLineEdit"
#include "QListWidget"
#include "QFileInfo"
#include "QSettings"
#include <QDomDocument>
#include <QMenu>
#include "dataselection.h"

#include <hdf5serie/vectorserie.h>
#include <hdf5serie/simpleattribute.h>

#include "treewidgetitem.h"
#include "plotdata.h"
#include "plotarea.h"
#include "curves.h"
#include "mainwindow.h"

using namespace std;

DataSelection::DataSelection(QWidget * parent) : QSplitter(parent) {
  QSettings settings;
  connect(this, &DataSelection::reopenAllSignal, this, &DataSelection::reopenAll);
  connect(this, &DataSelection::refreshFileSignal, this, &DataSelection::refreshFile);

  auto* dummy=new QWidget(this);
  addWidget(dummy);

  auto * fileSelection=new QGridLayout(this);
  dummy->setLayout(fileSelection);

  // filter settings
  OpenMBVGUI::AbstractViewFilter::setFilterType(static_cast<OpenMBVGUI::AbstractViewFilter::FilterType>(settings.value("mainwindow/filter/type", 0).toInt()));
  OpenMBVGUI::AbstractViewFilter::setCaseSensitive(settings.value("mainwindow/filter/casesensitivity", false).toBool());
  connect(OpenMBVGUI::AbstractViewFilter::staticObject(), &OpenMBVGUI::AbstractViewFilterStatic::optionsChanged, [](){
    QSettings settings;
    settings.setValue("mainwindow/filter/type", static_cast<int>(OpenMBVGUI::AbstractViewFilter::getFilterType()));
    settings.setValue("mainwindow/filter/casesensitivity", OpenMBVGUI::AbstractViewFilter::getCaseSensitive());
  });

  fileBrowser = new QTreeWidget(this);
  fileSelection->addWidget(fileBrowser,1,0,1,2);
  fileBrowser->setHeaderHidden(true);
  fileBrowser->setColumnCount(1);
  dataSelectionFilter=new OpenMBVGUI::AbstractViewFilter(fileBrowser);
  fileSelection->addWidget(dataSelectionFilter, 0,0,1,2);

  auto * pathLabel=new QLabel("Path:");
  fileSelection->addWidget(pathLabel,2,0);
  path=new QLineEdit(this);
  fileSelection->addWidget(path,2,1);
  path->setReadOnly(true);

  currentData=new QListWidget(this);
  addWidget(currentData);

  QObject::connect(fileBrowser, &QTreeWidget::itemClicked, this, &DataSelection::selectFromFileBrowser);
  QObject::connect(fileBrowser, &QTreeWidget::currentItemChanged, this, &DataSelection::updatePath);
  QObject::connect(fileBrowser, &QTreeWidget::itemPressed, this, &DataSelection::currentItemClicked);
  QObject::connect(currentData, &QListWidget::itemPressed, this, &DataSelection::currentDataClicked);

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

  auto *topitem = new TreeWidgetItem(QStringList(fileInfo.back().fileName()));
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
  node.addChild(Node(item->text(0),item->isExpanded(),item->isSelected(),item->isSelected()?currentData->currentRow():0));
  for(int i=0; i<item->childCount(); i++)
    save(item->child(i),node.getChild(node.getNumberOfChilds()-1));
}

void DataSelection::reopenAll() {

  Node root("Root",false,false,0);
  for(int i=0; i<fileBrowser->topLevelItemCount(); i++)
    save(fileBrowser->topLevelItem(i),root);

  QList<QFileInfo> info = fileInfo;

  // close all
  path->setText("");
  currentData->clear();
  for(int idx=fileBrowser->topLevelItemCount(); idx>=0; --idx)
    delete fileBrowser->topLevelItem(idx);
  // close files
  h5File.clear();
  file.clear();
  fileInfo.clear();

  for(const auto & i : info)
    addFile(i.absoluteFilePath());

  for(int i=0; i<fileBrowser->topLevelItemCount(); i++)
    rebuild(fileBrowser->topLevelItem(i),root);

  static_cast<MainWindow*>(parent()->parent())->getCurves()->plotAllTabs();
  dataSelectionFilter->applyFilter();
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
      QString path; 
      getPath(item,path,0);
      path += QString("/") + name.c_str();
      H5::ElementType et;
      hid_t t;
      grp->openChildObject(path.toStdString(), &et, &t);
      if(et==H5::vectorSerie && H5Tequal(t, H5T_NATIVE_DOUBLE)) {
        static_cast<TreeWidgetItem*>(child)->setPath(path);
        static_cast<TreeWidgetItem*>(child)->setIsVectorSerieDouble(true);
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
  if( static_cast<TreeWidgetItem*>(item)->getIsVectorSerieDouble()) {
    QString path = static_cast<TreeWidgetItem*>(item)->getPath();
    int j = getTopLevelIndex(item);
    std::shared_ptr<H5::File> h5f=getH5File(file[j].toStdString());
    auto *vs=h5f->openChildObject<H5::VectorSerie<double> >(path.toStdString());
    QStringList sl;
    if(vs->hasChildAttribute("Column Label")) {
      auto ret=vs->openChildAttribute<H5::SimpleAttribute<vector<string> > >("Column Label")->read();
      for(auto &i : ret)
        sl << i.c_str();
    }
    else
      for(size_t i=1; i<=vs->getColumns(); ++i)
        sl << QString("Column %1").arg(i);
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

void DataSelection::selectFromCurrentData(QListWidgetItem* item, const QString &mode) {
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

  static_cast<MainWindow*>(parent()->parent())->getCurves()->modifyPlotData(pd, mode);
}

void DataSelection::updatePath(QTreeWidgetItem *cur) {
  QString str=cur->text(0);
  for (QTreeWidgetItem *item=cur->parent(); item!=nullptr; item=item->parent())
    str=item->text(0)+"/"+str;
  path->setText(str);
};

void DataSelection::requestFlush() {
  for(const auto& h5f : h5File)
    h5f.second->requestFlush();
}

void DataSelection::expandToDepth(int depth) {
  expandToDepth(fileBrowser->currentItem(), depth);
}

void DataSelection::expandToDepth(QTreeWidgetItem *item, int depth) {
  item->setExpanded(depth>-1);
  for(int i=0; i<item->childCount(); i++) {
    if(depth>0) {
      if(item->child(i)->childCount()>0) item->child(i)->setExpanded(true);
    }
    else {
      if(item->child(i)->childCount()>0) item->child(i)->setExpanded(false);
    }
    expandToDepth(item->child(i), depth-1);
  }
}

void DataSelection::currentItemClicked(QTreeWidgetItem *item) {
  if(QApplication::mouseButtons()==Qt::RightButton) {
    QMenu *menu = new QMenu;
//    auto iconPath(boost::dll::program_location().parent_path().parent_path()/"share"/"mbsimgui"/"icons");
    auto *action = new QAction("Expand to depth 0", this);
    action->setShortcut(QKeySequence("0"));
    connect(action,&QAction::triggered,this,[=](){ expandToDepth(-1); });
    menu->addAction(action);
    action = new QAction("Expand to depth 1", this);
    action->setShortcut(QKeySequence("1"));
    connect(action,&QAction::triggered,this,[=](){ expandToDepth(0); });
    menu->addAction(action);
    action = new QAction("Expand to depth 2", this);
    action->setShortcut(QKeySequence("2"));
    connect(action,&QAction::triggered,this,[=](){ expandToDepth(1); });
    menu->addAction(action);
    action = new QAction("Expand to depth 3", this);
    action->setShortcut(QKeySequence("3"));
    connect(action,&QAction::triggered,this,[=](){ expandToDepth(2); });
    menu->addAction(action);
    action = new QAction("Expand to depth 4", this);
    action->setShortcut(QKeySequence("4"));
    connect(action,&QAction::triggered,this,[=](){ expandToDepth(3); });
    menu->addAction(action);
    action = new QAction("Expand to depth 5", this);
    action->setShortcut(QKeySequence("5"));
    connect(action,&QAction::triggered,this,[=](){ expandToDepth(4); });
    menu->addAction(action);
    action = new QAction("Expand all", this);
    action->setShortcut(QKeySequence("Shift++"));
    connect(action,&QAction::triggered,this,[=](){ expandToDepth(1000); });
    menu->addAction(action);
    action = new QAction("Collapse all", this);
    action->setShortcut(QKeySequence("Shift+-"));
    connect(action,&QAction::triggered,this,[=](){ expandToDepth(-1); });
    menu->addAction(action);
    menu->exec(QCursor::pos());
    delete menu;
  }
}

void DataSelection::currentDataClicked(QListWidgetItem *item) {
  if(QApplication::mouseButtons()==Qt::LeftButton) {
    if (QApplication::keyboardModifiers() & Qt::ShiftModifier)
      selectFromCurrentData(item,"add");
    else if (QApplication::keyboardModifiers() & Qt::ControlModifier)
      selectFromCurrentData(item,"new");
    else
      selectFromCurrentData(item,"replace");
  }
  if(QApplication::mouseButtons()==Qt::RightButton) {
    QMenu *menu = new QMenu;
    QAction *action=new QAction("Open in new window", menu);
    connect(action,&QAction::triggered,this,[=](){ selectFromCurrentData(item,"new"); });
    menu->addAction(action);
    action=new QAction("Add to current window", menu);
    connect(action,&QAction::triggered,this,[=](){ selectFromCurrentData(item,"add"); });
    menu->addAction(action);
    action=new QAction("Replace in current window", menu);
    connect(action,&QAction::triggered,this,[=](){ selectFromCurrentData(item,"replace"); });
    menu->addAction(action);
    menu->exec(QCursor::pos());
    delete menu;
  }
}
