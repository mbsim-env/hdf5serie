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

#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QMainWindow>
#include <qwt_plot_curve.h>
#include <qwt_plot.h>

namespace H5 {
  class H5File;
  class Group;
}
namespace sdt {
  class stringstream;
}
class QwtPlot;
//class QwtPlotCurve;
class QTreeWidget;
class QListWidget;
class QTableWidget;
class QTreeWidgetItem;
class QListWidgetItem;
class QwtPlotZoomer;
class QCheckBox;
class QMdiArea;
class QMdiSubWindow;

class MyPlot : public QwtPlot {
  private:
    QwtPlotZoomer *zoom;
  public:
    MyPlot(QWidget *p);
    ~MyPlot();
    void setZoomer(QwtPlotZoomer* zoom_) {zoom = zoom_;}
    QwtPlotZoomer* getZoom() {return zoom;}
};

class MyCurve : public QwtPlotCurve {
  private:
    std::string xLabel;
    std::string yLabel;
    std::string xPath;
    std::string yPath;
  public:
    void setxLabel(std::string &string) {xLabel = string;}
    void setyLabel(std::string &string) {yLabel = string;}
    void setxPath(std::string &string) {xPath = string;}
    void setyPath(std::string &string) {yPath = string;}
    std::string& getxPath() {return xPath;}
    std::string& getyPath() {return yPath;}
    std::string& getxLabel() {return xLabel;}
    std::string& getyLabel() {return yLabel;}
    const std::string& getxPath() const {return xPath;}
    const std::string& getyPath() const {return yPath;}
    const std::string& getxLabel() const {return xLabel;}
    const std::string& getyLabel() const {return yLabel;}
};

class MainWindow : public QMainWindow {

  Q_OBJECT

  private:
    QWidget *centralWidget;
    H5::H5File * file;
    QwtPlot *myPlot;
    std::vector<MyCurve*> curve;
    std::vector<QPen> pen;
    QwtPlotZoomer *zoom;
    QTreeWidget *treeWidget;
    QListWidget *listWidget;
    QTreeWidget *tableWidget;
    QMdiArea *mdiArea;

    void insertChildInTree(H5::Group &grp, QTreeWidgetItem *item);
    void getPath(QTreeWidgetItem* item, std::stringstream &s, int col);

    void updateTableWidget();

  public:
    MainWindow(std::vector<std::string>& arg);

    public slots:
      void help();
    void about();
    void plot(QListWidgetItem *item);
    void updateData(QTreeWidgetItem*, int);
    void detachCurve(QTreeWidgetItem*, int);
    void addPlotWindow();
    void windowChanged(QMdiSubWindow*);
};

#endif
