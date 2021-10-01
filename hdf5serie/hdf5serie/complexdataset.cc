/* Copyright (C) 2021 MBSim Development Team
 *
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version. 
 *  
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * Lesser General Public License for more details. 
 *  
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Contact: martin.o.foerg@gmail.com
 */

#include "hdf5serie/complexdataset.h"
#include "hdf5serie/file.h"
#include "hdf5serie/simpledataset.h"

using namespace std;

namespace H5 {

  void writeComplexDataset(const char *name, Group *group, const vector<complex<double>> &data) {
    vector<Complex> buf(data.size());
    int i=0;
    for(auto ir=data.begin(); ir!=data.end(); ++ir, ++i) {
      buf[i].r=ir->real();
      buf[i].i=ir->imag();
    }
    hsize_t dimsf[1];
    dimsf[0] = data.size();
    auto dataspace = H5Screate_simple(1, dimsf, NULL);
    auto datatype = H5Tcreate (H5T_COMPOUND, sizeof(Complex));
    H5Tinsert(datatype, "real", HOFFSET(Complex, r), H5T_NATIVE_DOUBLE);
    H5Tinsert(datatype, "imag", HOFFSET(Complex, i), H5T_NATIVE_DOUBLE);
    ScopedHID propID(H5Pcreate(H5P_DATASET_CREATE), &H5Pclose);
    ScopedHID apl(H5Pcreate(H5P_DATASET_ACCESS), &H5Pclose);
    auto dataset = H5Dcreate2(group->getID(), name, datatype, dataspace, H5P_DEFAULT, propID, apl);
    H5Dwrite(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, &buf[0]);
    H5Sclose(dataspace);
    H5Tclose(datatype);
    H5Dclose(dataset);
  }

  void writeComplexDataset(const char *name, Group *group, const vector<vector<complex<double>>> &data) {
    vector<Complex> buf(data.size()*(data.size()?data[0].size():0));
    int i=0;
    for(auto ir=data.begin(); ir!=data.end(); ++ir) {
      for(auto ic=ir->begin(); ic!=ir->end(); ++ic, ++i) {
	buf[i].r=ic->real();
	buf[i].i=ic->imag();
      }
    }
    hsize_t dimsf[2];
    dimsf[0] = data.size();
    dimsf[1] = data.size()?data[0].size():0;
    auto dataspace = H5Screate_simple(2, dimsf, NULL);
    auto datatype = H5Tcreate (H5T_COMPOUND, sizeof(Complex));
    H5Tinsert(datatype, "real", HOFFSET(Complex, r), H5T_NATIVE_DOUBLE);
    H5Tinsert(datatype, "imag", HOFFSET(Complex, i), H5T_NATIVE_DOUBLE);
    ScopedHID propID(H5Pcreate(H5P_DATASET_CREATE), &H5Pclose);
    ScopedHID apl(H5Pcreate(H5P_DATASET_ACCESS), &H5Pclose);
    auto dataset = H5Dcreate2(group->getID(), name, datatype, dataspace, H5P_DEFAULT, propID, apl);
    H5Dwrite(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, &buf[0]);
    H5Sclose(dataspace);
    H5Tclose(datatype);
    H5Dclose(dataset);
  }

  vector<complex<double>> readComplexVector(const char *name, Group *group) {
    auto dataset = H5Dopen(group->getID(), name, H5P_DEFAULT);
    hsize_t dims[1];
    auto dataspace = H5Dget_space(dataset);
    H5Sget_simple_extent_dims(dataspace, dims, nullptr);
    size_t size=dims[0];
    vector<Complex> buf(size);
    auto datatype = H5Tcreate(H5T_COMPOUND, sizeof(Complex));
    H5Tinsert(datatype, "real", HOFFSET(Complex, r), H5T_NATIVE_DOUBLE);
    H5Tinsert(datatype, "imag", HOFFSET(Complex, i), H5T_NATIVE_DOUBLE);
    H5Dread(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, &buf[0]);
    vector<complex<double>> data(size);
    for(size_t i=0; i<size; i++)
      data[i] = complex<double>(buf[i].r, buf[i].i);
    H5Dclose(dataset);
    return data;
  }

  vector<vector<complex<double>>> readComplexMatrix(const char *name, Group *group) {
    auto dataset = H5Dopen(group->getID(), name, H5P_DEFAULT);
    hsize_t dims[2];
    auto dataspace = H5Dget_space(dataset);
    H5Sget_simple_extent_dims(dataspace, dims, nullptr);
    size_t rows=dims[0];
    size_t cols=dims[1];
    vector<Complex> buf(rows*cols);
    auto datatype = H5Tcreate(H5T_COMPOUND, sizeof(Complex));
    H5Tinsert(datatype, "real", HOFFSET(Complex, r), H5T_NATIVE_DOUBLE);
    H5Tinsert(datatype, "imag", HOFFSET(Complex, i), H5T_NATIVE_DOUBLE);
    H5Dread(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, &buf[0]);
    vector<vector<complex<double>>> data(rows,vector<complex<double>>(cols));
    int k=0;
    for(size_t i=0; i<rows; i++) {
      for(size_t j=0; j<cols; j++, k++)
	data[i][j] = complex<double>(buf[k].r, buf[k].i);
    }
    H5Dclose(dataset);
    return data;
  }

}
