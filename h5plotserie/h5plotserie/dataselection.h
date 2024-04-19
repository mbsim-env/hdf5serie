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

#ifndef DATASELECTION_H
#define DATASELECTION_H

#include <QSplitter>
#include <boost/filesystem.hpp>
#include <list>
#include "abstractviewfilter.h"
#include "qobjectdefs.h"

namespace H5 {
  class Group;
  class File;
}

class QTreeWidget;
class QTreeWidgetItem;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QFileInfo;

class QWidget;

class DataSelection : public QSplitter {
  Q_OBJECT

  class Node {
    public:
      Node(QString name_, bool expanded_, bool selected_, int row_) : name(std::move(name_)), expanded(expanded_), selected(selected_), row(row_) { }
      void addChild(const Node &node) { child.push_back(node); }
      int getNumberOfChilds() const { return child.size(); }
      Node& getChild(int i) { return child[i]; }
      const QString& getName() const { return name; }
      bool isSelected() const { return selected; }
      bool isExpanded() const { return expanded; }
      int getRow() const { return row; }
    private:
      QString name;
      bool expanded{false};
      bool selected{false};
      int row;
      std::vector<Node> child;
  };

  public:
    DataSelection(QWidget * parent = nullptr);
    ~DataSelection() override;
    
    void addFile(const QString &name);
    void rebuild(QTreeWidgetItem *item, Node &node);
    void save(QTreeWidgetItem *item, Node &node);
    void reopenAll();
    void refreshFile(const QString &name);
    QList<QFileInfo> * getFileInfo() { return &fileInfo; }
    std::shared_ptr<H5::File> getH5File(const boost::filesystem::path &p) const;
    void requestFlush();
    void expandToDepth(int depth);
    void keyPressed(int key);

  private:
    void selectFromFileBrowser(QTreeWidgetItem* item, int col);
    void selectFromCurrentData(QListWidgetItem *item, const QString &mode); //=plot
    void updatePath(QTreeWidgetItem *);

    OpenMBVGUI::AbstractViewFilter *dataSelectionFilter;
    QTreeWidget *fileBrowser; // treeWidget
    QLineEdit *path;
    QListWidget *currentData; // listWidget;

    void insertChildInTree(H5::Group *grp, QTreeWidgetItem *item);
    void getPath(QTreeWidgetItem* item, QString &s, int col);
    int getTopLevelIndex(QTreeWidgetItem* item);

    void expandToDepth(QTreeWidgetItem *item, int depth);

    void currentItemClicked(QTreeWidgetItem *item);
    void currentDataClicked(QListWidgetItem *item);

    QList<QString> file;
    QList<QFileInfo> fileInfo;

    std::list<std::pair<boost::filesystem::path, std::shared_ptr<H5::File>>> h5File;

  Q_SIGNALS:
    void reopenAllSignal();
    void refreshFileSignal(const QString &name);
};

#endif // DATASELECTION_H
