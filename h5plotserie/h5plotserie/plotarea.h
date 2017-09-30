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

#ifndef PLOTAREA_H
#define PLOTAREA_H

#include <QMdiArea>
#include <QMdiSubWindow>
#include "qvector.h"
#include "qpen.h"

class QCloseEvent;

class PlotWindow;
class PlotData;
class QwtPlot;
class QwtPlotZoomer;

class PlotArea : public QMdiArea {

  Q_OBJECT

  public:
    PlotArea(QWidget * parent = 0);

    void addPlotWindow(const QString &windowTitle);

  private slots:

  private:
};

class PlotWindow : public QMdiSubWindow {

  Q_OBJECT

  public:
    PlotWindow(QWidget * parent = 0);

    void detachPlot();
    void plotDataSet(PlotData pd, int penColor);
    void replotPlot();

    void setPlotGrid(bool grid_=true) {plotGrid=grid_; }

  protected:
    void closeEvent(QCloseEvent * event);
  
  private:
    QwtPlot * plot;
    QVector<QPen> pen;
    QwtPlotZoomer * zoom;
    double xMinValue, yMinValue, xMaxValue, yMaxValue;
    bool plotGrid;
};

#endif // PLOTAREA_H
