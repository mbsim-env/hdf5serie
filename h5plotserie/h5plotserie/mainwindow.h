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
    QString xLabel;
    QString yLabel;
    QString xPath;
    QString yPath;

    static bool isNaN(double v) { return v != v; }
  public:

    void setxLabel(QString &string) {xLabel = string;}
    void setyLabel(QString &string) {yLabel = string;}
    void setxPath(QString &string) {xPath = string;}
    void setyPath(QString &string) {yPath = string;}
    QString& getxPath() {return xPath;}
    QString& getyPath() {return yPath;}
    QString& getxLabel() {return xLabel;}
    QString& getyLabel() {return yLabel;}
    const QString& getxPath() const {return xPath;}
    const QString& getyPath() const {return yPath;}
    const QString& getxLabel() const {return xLabel;}
    const QString& getyLabel() const {return yLabel;}

    void draw(QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &yMap, int from, int to) const;
    QwtDoubleRect boundingRect() const;
};

class MainWindow : public QMainWindow {

  Q_OBJECT

  private:
    QWidget *centralWidget;
    QList<H5::H5File*> file;
    QList<QString> fileName;
    QwtPlot *myPlot;
    QVector<MyCurve*> curve;
    QVector<QPen> pen;
    QwtPlotZoomer *zoom;
    QTreeWidget *treeWidget;
    QListWidget *listWidget;
    QTreeWidget *tableWidget;
    QMdiArea *mdiArea;

    void insertChildInTree(H5::Group &grp, QTreeWidgetItem *item);
    void getPath(QTreeWidgetItem* item, QString &s, int col);
    int getTopLevelIndex(QTreeWidgetItem* item);

    void updateTableWidget();
    void addFile(const QString &name);

  public:
    MainWindow(std::vector<std::string>& arg);

  public slots:
    void help();
    void about();
    void plot(QListWidgetItem *item);
    void updateData(QTreeWidgetItem*, int);
    void openContextMenu();
    void addPlotWindow();
    void printPlotWindow();
    //void exportPlot2SVG();
    void openFileDialog();
    void windowChanged(QMdiSubWindow*);
    void closeFile();
    void detachCurve();
};

#endif
