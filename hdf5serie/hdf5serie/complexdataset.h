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

#ifndef _HDF5SERIE_COMPLEXDATASET_H_
#define _HDF5SERIE_COMPLEXDATASET_H_

#include <vector>
#include <complex>

namespace H5 {

  class Group;

  struct Complex {
    double r{0};
    double i{0};
  };

  void writeComplexDataset(const char *name, Group *group, const std::vector<std::complex<double>> &data);
  void writeComplexDataset(const char *name, Group *group, const std::vector<std::vector<std::complex<double>>> &data);
  std::vector<std::complex<double>> readComplexVector(const char *name, Group *group);
  std::vector<std::vector<std::complex<double>>> readComplexMatrix(const char *name, Group *group);
}

#endif

