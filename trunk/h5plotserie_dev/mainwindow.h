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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class PlotArea;
class Curves;
class DataSelection;

class MainWindow : public QMainWindow {

  Q_OBJECT

  public:
    MainWindow(std::vector<std::string>& arg);
    ~MainWindow();

    Curves * getCurves() {return curves; }
    PlotArea * getPlotArea() {return plotArea; }
    DataSelection * getDataSelection() {return dataSelection; }

  private slots:
    void about();
    void help();
    void addH5FileDialog();
    void saveAllPlotWindows();
    void loadPlotWindows();

  private:
    PlotArea * plotArea;
    DataSelection * dataSelection;
    Curves * curves;

};

#endif // MAINWINDOW_H

