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

#ifndef PLOTDATA_H
#define PLOTDATA_H

#include "QString"

#include "iostream"

struct dataset {
  QString string;
  QString xmlString;
  QString value;
};

class PlotData {

  public:
    PlotData() {
      dataset d;

      d.string = "Filename";
      d.xmlString = "filename";
      d.value = "";
      plotdata.push_back(d);

      d.string = "x-Label";
      d.xmlString = "xLabel";
      d.value = "";
      plotdata.push_back(d);

      d.string = "x-Index";
      d.xmlString = "xIndex";
      d.value = "";
      plotdata.push_back(d);

      d.string = "y-Label";
      d.xmlString = "yLabel";
      d.value = "";
      plotdata.push_back(d);

      d.string = "y-Index";
      d.xmlString = "yIndex";
      d.value = "";
      plotdata.push_back(d);

      d.string = "offset";
      d.xmlString = "offset";
      d.value = "";
      plotdata.push_back(d);

      d.string = "gain";
      d.xmlString = "gain";
      d.value = "";
      plotdata.push_back(d);

      d.string = "x-Path";
      d.xmlString = "xPath";
      d.value = "";
      plotdata.push_back(d);

      d.string = "y-Path";
      d.xmlString = "yPath";
      d.value = "";
      plotdata.push_back(d);

      d.string = "y2-Path";
      d.xmlString = "y2Path";
      d.value = "";
      plotdata.push_back(d);

      d.string = "y2-Label";
      d.xmlString = "y2Label";
      d.value = "";
      plotdata.push_back(d);

      d.string = "y2-Index";
      d.xmlString = "y2Index";
      d.value = "";
      plotdata.push_back(d);

      d.string = "y2offset";
      d.xmlString = "y2offset";
      d.value = "";
      plotdata.push_back(d);

      d.string = "y2gain";
      d.xmlString = "y2gain";
      d.value = "";
      plotdata.push_back(d);

      d.string = "Filepath";
      d.xmlString = "filepath";
      d.value = "";
      plotdata.push_back(d);
    }

    QString string(int i) {return plotdata[i].string; }
    QString xmlString(int i) {return plotdata[i].xmlString; }
    
    void setValue(const QString& property, const QString& v) {plotdata[searchIndex(property)].value=v; }
    void setValue(int i, const QString& v) {plotdata[i].value=v; }
    
    QString getValue(const QString& property) {return plotdata[searchIndex(property)].value; }
    QString getValue(int i) {return plotdata[i].value; }

    void showValues() {
      for (int i=0; i<numberOfItems(); i++)
        std::cout << plotdata[i].string.toStdString() << ": " << plotdata[i].value.toStdString() << std::endl;
    }

    int numberOfItems() {return plotdata.size(); }

  private:
    std::vector<dataset> plotdata;

    int searchIndex(const QString& property) {
      int i=0;
      while (QString::compare(property, plotdata[i].string, Qt::CaseSensitive)!=0 && i<numberOfItems())
        i++;
      return i;
    }
};

#endif // PLOTDATA_H
