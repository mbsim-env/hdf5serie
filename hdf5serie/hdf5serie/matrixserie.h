/* Copyright (C) 2009 Markus Friedrich
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
 * Contact:
 *   friedrich.at.gc@googlemail.com
 *
 */

#ifndef _HDF5SERIE_MATRIXSERIE_H_
#define _HDF5SERIE_MATRIXSERIE_H_

#include <H5Cpp.h>
#include <vector>
#include <string>
#include <assert.h>
#include <hdf5serie/simpleattribute.h>
#include <hdf5serie/fileserie.h>

namespace H5 {


   
  /** \brief Serie of matrices.
   *
   * A HDF5 dataset for reading and writing a serie of data matrices.
   * The type of the elements of the matrix (template type T) can be of:
   *   - char
   *   - signed char
   *   - unsigned char
   *   - short
   *   - unsigned short
   *   - int
   *   - unsigned int
   *   - long
   *   - unsigned long
   *   - long long
   *   - unsigned long long
   *   - float
   *   - double
   *   - long double
   *   - std::string
   *
   * The data is stored as a 3D array in the HDF5 file. The first dimension is the serie.
   *
   * A Note when using a vector-matrix-library:
   * It is likly that the data in calculated by using a vector-matrix-library. If so,
   * and the vector object (of type T) of the library (e.g. fmatvec from http://code.google.com/p/fmatvec/) has a cast-operator
   * to std::vector<T> and a constructor with a single parameter of type
   * std::vector<T>, then you can use the vector-object wherever a object of type
   * std::vector<T> is needed.
   * The same applies to a matrix-object for std::vector<std::vector<T> >.
  */
  template<class T>
  class MatrixSerie : public DataSet {
    private:
      DataType memDataType;
      DataSpace memDataSpace;
      hsize_t dims[3];
    public:
      /** \brief A stub constructor
       *
       * Creates a empty object.
      */
      MatrixSerie();

      /** \brief Copy constructor */
      MatrixSerie(const MatrixSerie<T>& dataset);

      /** \brief Constructor for opening a dataset
       *
       * see open()
       */
      MatrixSerie(const CommonFG& parent, const std::string& name);

      /** \brief Dataset creating constructor
       *
       * see create()
      */
      MatrixSerie(const CommonFG& parent, const std::string& name, const int rows, const int cols, int compression=FileSerie::getDefaultCompression(), int chunkSize=FileSerie::getDefaultChunkSize());

      /** \brief Creating a dataset
       *
       * Creates a dataset named \a name as a child of position \a parent.
       * By default the dataset is compressed using deflate (gzip) with compression level
       * FileSerie::defaultCompression. Use \a compression to adjuste the compression level [1-9] or 0 to disable compression.
      */
      void create(const CommonFG& parent, const std::string& name, const int rows, const int cols, int compression=FileSerie::getDefaultCompression(), int chunksize=FileSerie::getDefaultChunkSize());

      /** \brief Open a dataset
       *
       * Opens the dataset named \a name as a child of position \a parent.
       */
      void open(const CommonFG& parent, const std::string& name);

      /** \brief Sets a description for the dataset
       *
       * The value of \a desc is stored as an string attribute named \p Description in the dataset.
       */
      void setDescription(const std::string& desc);

      /** \brief Append a matrix
       *
       * Appends the matrix \a data at the end of the dataset.
       * The fist dimension of the HDF5 array will be incremented by this operation.
       */
      void append(const std::vector<std::vector<T> > &matrix);

      /** \brief Returns the number of matrices in the dataset */
      inline unsigned int getNumberOfMatrices();

      /** \brief Returns the number of rows of the matrix */
      inline unsigned int getRows();

      /** \brief Returns the number of columns of the matrix */
      inline unsigned int getColumns();

      /** \brief Returns the matrix at position \a number
       *
       * The first number is 0. The last avaliable number/position is getNumberOfMatrices()-1.
       */
      std::vector<std::vector<T> > getMatrix(const int number);

      /** \brief Return the description for the dataset
       *
       * Returns the value of the string attribute named \p Description of the dataset.
       */
      std::string getDescription();

      void extend(const hsize_t* size);
  };



  // inline definitions

  template<class T>
  unsigned int MatrixSerie<T>::getNumberOfMatrices() {
    return dims[0];
  }

  template<class T>
  unsigned int MatrixSerie<T>::getRows() {
    // get current dims from dataspace (maybe another (single-)writer process has increased the number of rows)
    DataSpace fileDataSpace=getSpace();
    fileDataSpace.getSimpleExtentDims(dims);
    // return current value
    return dims[1];
  }

  template<class T>
  unsigned int MatrixSerie<T>::getColumns() {
    return dims[2];
  }

}

#endif
