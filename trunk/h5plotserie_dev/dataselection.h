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

#ifndef DATASELECTION_H
#define DATASELECTION_H

#include <QSplitter>

namespace H5 {
  class H5File;
  class Group;
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

  public:
    DataSelection(QWidget * parent = 0);
    ~DataSelection();
    
    void addFile(const QString &fileName);
    QList<QFileInfo> * getFileInfo() {return &fileInfo; }

  //public slots:

  private slots:
    void selectFromFileBrowser(QTreeWidgetItem* item, int col);
    void selectFromCurrentData(QListWidgetItem *item); //=plot
    void filterObjectList();
    void searchObjectList(QTreeWidgetItem *item, const QRegExp& filterRegExp);
    void updatePath(QTreeWidgetItem *);

  private:
    QTreeWidget *fileBrowser; // treeWidget
    QLineEdit * filter;
    QLineEdit * path;
    QListWidget * currentData; //=listWidget;

    void insertChildInTree(H5::Group &grp, QTreeWidgetItem *item);
    void getPath(QTreeWidgetItem* item, QString &s, int col);
    int getTopLevelIndex(QTreeWidgetItem* item);

    QList<H5::H5File*> file;
    QList<QFileInfo> fileInfo;
};

#endif // DATASELECTION_H
