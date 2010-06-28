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

#include <QtGui>
#include "plotarea.h"
#include "plotdata.h"
#include "curves.h"
#include "mainwindow.h"

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_grid.h>

#include <hdf5serie/vectorserie.h>

PlotArea::PlotArea(QWidget * parent) : QMdiArea(parent) {
  setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

void PlotArea::addPlotWindow(const QString &windowTitle) {
  QMdiSubWindow * q = addSubWindow(new PlotWindow(this));
  q->setObjectName(windowTitle);
  q->setWindowTitle(windowTitle);
  q->setAttribute(Qt::WA_DeleteOnClose);
  q->show();
}

PlotWindow::PlotWindow(QWidget * parent) : QMdiSubWindow(parent) {

  plot = new QwtPlot();
  setWidget(plot);

  plot->setCanvasBackground(QColor(255, 255, 255));
  plot->replot();

  zoom = new QwtPlotZoomer(plot->canvas());

  uint linewidth=1;
  pen.append(QPen(Qt::red, linewidth));
  pen.append(QPen(Qt::green, linewidth));
  pen.append(QPen(Qt::blue, linewidth));
  pen.append(QPen(Qt::cyan, linewidth));
  pen.append(QPen(Qt::magenta, linewidth));
  pen.append(QPen(Qt::yellow, linewidth));
  pen.append(QPen(Qt::gray, linewidth));
  pen.append(QPen(Qt::darkRed, linewidth));
  pen.append(QPen(Qt::darkGreen, linewidth));
  pen.append(QPen(Qt::darkBlue, linewidth));
  pen.append(QPen(Qt::darkCyan, linewidth));
  pen.append(QPen(Qt::darkMagenta, linewidth));
  pen.append(QPen(Qt::darkYellow, linewidth));
  pen.append(QPen(Qt::darkGray, linewidth));
}

void PlotWindow::detachPlot() {
  QwtPlotItemList il = plot->itemList();
  for(int i=0; i<il.size(); i++)
    il[i]->detach();
  plot->replot();
  xMinValue=99e99;
  xMaxValue=-99e99;
  yMinValue=99e99;
  yMaxValue=-99e99;
}

void PlotWindow::plotDataSet(PlotData pd, int penColor) {
  H5::H5File* file = new H5::H5File(QString(pd.getValue("Filepath")+"/"+pd.getValue("Filename")).toStdString(), H5F_ACC_RDONLY);
  H5::VectorSerie<double> vs;

  vs.open(*file, pd.getValue("x-Path").toStdString());
  std::vector<double> xVal = vs.getColumn(pd.getValue("x-Index").toInt());
  vs.close();

  vs.open(*file, pd.getValue("y-Path").toStdString());
  std::vector<double> yVal = vs.getColumn(pd.getValue("y-Index").toInt());
  vs.close();

  if (xVal.size()==yVal.size()) {
    const double offset=pd.getValue("offset").toDouble();
    const double gain=pd.getValue("gain").toDouble();
    for (unsigned int i=0; i<xVal.size(); i++) {
      if (!isNaN(xVal[i])) {
        if (xVal[i]<xMinValue)
          xMinValue=xVal[i];
        if (xVal[i]>xMaxValue)
          xMaxValue=xVal[i];
      }
      if (!isNaN(yVal[i])) {
        yVal[i]=gain*(yVal[i]+offset);
        if (yVal[i]<yMinValue)
          yMinValue=yVal[i];
        if (yVal[i]>yMaxValue)
          yMaxValue=yVal[i];
      }
    }
    for (unsigned int i=0; i<xVal.size(); i++) {
      if (isNaN(xVal[i]))
        xVal[i]=.5*(xMinValue+xMaxValue);
      if (isNaN(yVal[i]))
        yVal[i]=.5*(yMinValue+yMaxValue);
    }

    QwtPlotCurve * c = new QwtPlotCurve();
    c->attach(plot);
    while (penColor>pen.size()-1)
      penColor-=pen.size();
    c->setPen(pen[penColor]);
    c->setData(&xVal[0], &yVal[0], xVal.size());
    c->draw(0, xVal.size());
  }
  else {
    QMessageBox msgBox;
    msgBox.setText("Different sizes of x- and y-Vector. I'm going to skip these data.");
    msgBox.exec();
  }
}

void PlotWindow::replotPlot() {
  plot->setAxisAutoScale(QwtPlot::xBottom);
  plot->setAxisAutoScale(QwtPlot::yLeft);

  QwtPlotGrid *grid = new QwtPlotGrid;
  grid->enableXMin(true);
  grid->enableYMin(true);
  grid->setMajPen(QPen(Qt::black, 0, Qt::DotLine));
  grid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
  grid->attach(plot);

  plot->replot();
  //zoom->setZoomBase(QwtDoubleRect(xMinValue, yMaxValue, xMaxValue-xMinValue, yMaxValue-yMinValue));
  zoom->setZoomBase();
}

void PlotWindow::closeEvent(QCloseEvent *) {
  Curves * c = (dynamic_cast<MainWindow*>(parent()->parent()->parent()))->getCurves();
  PlotDataTable * pd=c->findChild<PlotDataTable*>(windowTitle());
  if (pd) {
    int index=c->indexOf(pd);
    pd->close();
    c->removeTab(index);
  }
}
