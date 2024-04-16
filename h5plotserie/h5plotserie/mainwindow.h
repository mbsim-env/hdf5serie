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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class PlotArea;
class Curves;
class DataSelection;

class MainWindow : public QMainWindow {

  public:
    MainWindow(const QStringList &arg);
    ~MainWindow() override;

    Curves* getCurves() { return curves; }
    PlotArea* getPlotArea() { return plotArea; }
    DataSelection* getDataSelection() { return dataSelection; }

  private:
    void about();
    void help();
    void addH5FileDialog();
    void saveAllPlotWindows();
    void loadPlotWindows();

    PlotArea *plotArea;
    DataSelection *dataSelection;
    Curves *curves;
    QTimer *requestFlushTimer;
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

};

#endif // MAINWINDOW_H

