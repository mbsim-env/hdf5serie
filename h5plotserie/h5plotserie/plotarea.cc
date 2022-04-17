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

#include "config.h"
#include "QMessageBox"
#include "plotarea.h"
#include "plotdata.h"
#include "curves.h"
#include "mainwindow.h"
#include "dataselection.h"

#include <QStack>

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
  q->showMaximized();
}

PlotWindow::PlotWindow(QWidget * parent) : QMdiSubWindow(parent) {

  plot = new QwtPlot();
  setWidget(plot);

  plot->setCanvasBackground(QColor(255, 255, 255));

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
  for(auto & i : il)
    i->detach();
  plot->replot();
  xMinValue=99e99;
  xMaxValue=-99e99;
  yMinValue=99e99;
  yMaxValue=-99e99;
}

void PlotWindow::plotDataSet(PlotData pd, int penColor) {
  DataSelection *dataSelection=static_cast<MainWindow*>(parent()->parent()->parent())->getDataSelection();
  std::shared_ptr<H5::File> h5file=dataSelection->getH5File(QString(pd.getValue("Filepath")+"/"+pd.getValue("Filename")).toStdString());

  H5::VectorSerie<double> *vs;
  try {
    vs=h5file->openChildObject<H5::VectorSerie<double> >(pd.getValue("x-Path").toStdString());
  }
  catch(...) {
    return;
  }
  size_t rows=vs->getRows();
  std::vector<double> xVal(rows);
  vs->getColumn(pd.getValue("x-Index").toInt(), xVal);

  try {
    vs=h5file->openChildObject<H5::VectorSerie<double> >(pd.getValue("y-Path").toStdString());
  }
  catch(...) {
    return;
  }
  std::vector<double> yVal(rows);
  vs->getColumn(pd.getValue("y-Index").toInt(), yVal);

  std::vector<double> y2Val(rows);
  bool useY2=false;
  if (pd.getValue("y2-Path").length()>0) {
    try {
      vs=h5file->openChildObject<H5::VectorSerie<double> >(pd.getValue("y2-Path").toStdString());
    }
    catch(...) {
      return;
    }
    vs->getColumn(pd.getValue("y2-Index").toInt(), y2Val);
    useY2=true;
  }

  if (xVal.size()==yVal.size()) {

    for (double i : xVal)
      if (!std::isnan(i)) { // xValue
        if (i<xMinValue)
          xMinValue=i;
        if (i>xMaxValue)
          xMaxValue=i;
      }

    if (useY2) {
      if ((yVal.size()==y2Val.size())) {
        const double y2offset=pd.getValue("y2offset").toDouble();
        const double y2gain=pd.getValue("y2gain").toDouble();
        for (double & i : y2Val)
          if (!std::isnan(i)) // y2Value
            i=y2gain*(i+y2offset);
      }
      else {
        useY2=false;
        QMessageBox msgBox;
        msgBox.setText("Different sizes of y- and y2-Vector. I'm going to skip y2 data.");
        msgBox.exec();
      }
    }

    const double offset=pd.getValue("offset").toDouble();
    const double gain=pd.getValue("gain").toDouble();
    for (unsigned int i=0; i<yVal.size(); i++)
      if (!std::isnan(yVal[i])) { // yValue
        yVal[i]=gain*(yVal[i]+offset);
        if (useY2)
          yVal[i]+=y2Val[i];
        if (yVal[i]<yMinValue)
          yMinValue=yVal[i];
        if (yVal[i]>yMaxValue)
          yMaxValue=yVal[i];
      }

    for (unsigned int i=0; i<xVal.size(); i++) {
      if (std::isnan(xVal[i]))
        xVal[i]=.5*(xMinValue+xMaxValue);
      if (std::isnan(yVal[i]))
        yVal[i]=.5*(yMinValue+yMaxValue);
    }

    QwtPlotCurve * c = new QwtPlotCurve();
    c->attach(plot);
    while (penColor>pen.size()-1)
      penColor-=pen.size();
    c->setPen(pen[penColor]);
    c->setSamples(&xVal[0], &yVal[0], xVal.size());
  }
  else {
    QMessageBox msgBox;
    msgBox.setText("Different sizes of x- and y-Vector. I'm going to skip these data.");
    msgBox.exec();
  }
}

void PlotWindow::replotPlot() {
  QStack<QRectF> stack = zoom->zoomStack();
  int index = zoom->zoomRectIndex();
  plot->setAxisAutoScale(QwtPlot::xBottom);
  plot->setAxisAutoScale(QwtPlot::yLeft);
  if (plotGrid) {
    auto *grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->enableYMin(true);
    grid->setMajorPen(QPen(Qt::black, 0, Qt::DotLine));
    grid->setMinorPen(QPen(Qt::gray, 0 , Qt::DotLine));
    grid->attach(plot);
  }
  zoom->setZoomBase();
  stack[0] = zoom->zoomStack()[0];
  zoom->setZoomStack(stack,index);
}

void PlotWindow::closeEvent(QCloseEvent *) {
  Curves * c = (static_cast<MainWindow*>(parent()->parent()->parent()))->getCurves();
  auto * pd=c->findChild<PlotDataTable*>(windowTitle());
  if (pd) {
    int index=c->indexOf(pd);
    pd->close();
    c->removeTab(index);
  }
}
