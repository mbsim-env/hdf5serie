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

#ifndef TREEWIDGETITEM_H
#define TREEWIDGETITEM_H

#include <QTreeWidgetItem>

class TreeWidgetItem : public QTreeWidgetItem {

  private:
    QString path;
    QStringList list;
    bool searchMatched;
  public:
    TreeWidgetItem ( const QStringList & strings) : QTreeWidgetItem(strings), searchMatched(true) {}
    void setPath(const QString& p) {path = p;}
    void setStringList(QStringList &list_) {list = list_;}
    QStringList& getStringList() {return list;}
    const QString& getPath() const {return path;}
    QString& getPath() {return path;}
    bool getSearchMatched() { return searchMatched; }
    void setSearchMatched(bool m) { searchMatched=m; }
    void updateTextColor() {
      if(searchMatched)
        setForeground(0, QBrush(QApplication::style()->standardPalette().color(QPalette::Active, QPalette::Text)));
      else
        setForeground(0, QBrush(QColor(255,0,0)));
    }
};


#endif
