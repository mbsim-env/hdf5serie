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

#ifndef CURVES_H
#define CURVES_H

#include <QTabWidget>
#include <QTableWidget>
#include <set>
#include <memory>

class QDomDocument;
class QDomElement;
class PlotData;
class PlotDataTable;

class Curves : public QTabWidget {
  
  public:
    Curves(QWidget *parent=nullptr);

    void modifyPlotData(PlotData pd, const QString &mode);
    std::shared_ptr<QDomDocument> saveCurves();
    void initLoadCurve(const QString &fileName);
    void loadCurve(QDomDocument *doc);
    void removeTab(const QString &name);
    void deletePressed();

  public:
    void plotCurrentTab();
    void plotAllTabs();
};

class PlotDataTable : public QTableWidget {

  public:
    PlotDataTable(QWidget *parent=nullptr, const QString &name="");
    
    void clearTable();
    void addDataSet(PlotData pd);
    void replaceDataSet(PlotData pd);
    void savePlot(QDomDocument *doc, QDomElement *tab);
    void dataSetClicked(int row, int col);
};

#endif // CURVES_H
