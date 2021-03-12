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
    Curves(QWidget * parent = nullptr);

    void modifyPlotData(PlotData pd, const QString &mode);
    std::shared_ptr<QDomDocument> saveCurves();
    void initLoadCurve(const QString &fileName);
    void loadCurve(QDomDocument * doc);

  public:
    void plotCurrentTab();
};

class PlotDataTable : public QTableWidget {

  public:
    PlotDataTable(QWidget * parent = nullptr, const QString &name = "");
    
    void clearTable();
    void addDataSet(PlotData pd);
    void replaceDataSet(PlotData pd);
    void savePlot(QDomDocument * doc, QDomElement * tab);
};

#endif // CURVES_H
