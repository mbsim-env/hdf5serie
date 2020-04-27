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

  public:
    PlotArea(QWidget * parent = nullptr);

    void addPlotWindow(const QString &windowTitle);
};

class PlotWindow : public QMdiSubWindow {

  public:
    PlotWindow(QWidget * parent = nullptr);

    void detachPlot();
    void plotDataSet(PlotData pd, int penColor);
    void replotPlot();

    void setPlotGrid(bool grid_=true) {plotGrid=grid_; }

  protected:
    void closeEvent(QCloseEvent * event) override;
  
  private:
    QwtPlot * plot{0};
    QVector<QPen> pen;
    QwtPlotZoomer * zoom{0};
    double xMinValue{0}, yMinValue{0}, xMaxValue{0}, yMaxValue{0};
    bool plotGrid{true};
};

#endif // PLOTAREA_H
