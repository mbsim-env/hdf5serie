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

#ifndef _HDF5SERIE_VECTORSERIE_H_
#define _HDF5SERIE_VECTORSERIE_H_

#include <H5Cpp.h>
#include <vector>
#include <string>
#include <assert.h>
#include <hdf5serie/simpleattribute.h>
#include <hdf5serie/fileserie.h>

namespace H5 {


   
  /** \brief Serie of vectors.
   *
   * A HDF5 dataset for reading and writing a serie of data vectors.
   * The type of the elements of the vector (template type T) can be of:
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
   * The data is stored as a 2D array in the HDF5 file. Each row is onw data vector.
   *
   * A Note when using a vector-matrix-library:
   * It is likly that the data in calculated by using a vector-matrix-library. If so,
   * and the vector object (of type T) of the library (e.g. fmatvec from http://code.google.com/p/fmatvec/) has a cast-operator
   * to std::vector<T> and a constructor with a single parameter of type
   * std::vector<T>, then you can use the vector-object wherever a object of type
   * std::vector<T> is needed. E.g. in append(const std::vector<T>) or the return value of getRow(int r).
   * The same applies to a matrix-object for std::vector<std::vector<T> >.
  */
  template<class T>
  class VectorSerie : public DataSet {
    private:
      DataType memDataType;
      DataSpace memDataSpace;
      hsize_t dims[2];
    public:
      /** \brief A stub constructor
       *
       * Creates a empty object.
      */
      VectorSerie();

      /** \brief Copy constructor */
      VectorSerie(const VectorSerie<T>& dataset);

      /** \brief Constructor for opening a dataset
       *
       * see open()
       */
      VectorSerie(const CommonFG& parent, const std::string& name);

      /** \brief Dataset creating constructor
       *
       * see create()
      */
      VectorSerie(const CommonFG& parent, const std::string& name, const std::vector<std::string>& columnLabel, int compression=FileSerie::getDefaultCompression(), int chunkSize=FileSerie::getDefaultChunkSize());

      /** \brief Creating a dataset
       *
       * Creates a dataset named \a name as a child of position \a parent.
       * By default the dataset is compressed using deflate (gzip) with compression level
       * FileSerie::defaultCompression. Use \a compression to adjuste the compression level [1-9] or 0 to disable compression.
       * Each element of the data vector (columns in the HDF5 file) must be given 
       * a description label using the parameter \a columnLabel. The column labels are
       * stored as a string vector attribute named \p Column \p Label in the dataset.
      */
      void create(const CommonFG& parent, const std::string& name, const std::vector<std::string>& columnLabel, int compression=FileSerie::getDefaultCompression(), int chunkSize=FileSerie::getDefaultChunkSize());

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

      /** \brief Append a data vector
       *
       * Appends the data vector \a data at the end of the dataset.
       * The number of rows of the HDF5 array will be incremented by this operation.
       */
      void append(const std::vector<T> &data);

      /** \brief Returns the number of rows in the dataset */
      inline int getRows();

      /** \brief Returns the number of columns(=number of data elements) in the dataset */
      inline unsigned int getColumns();

      /** \brief Returns the data vector at row \a row
       *
       * The first row is 0. The last avaliable row ist getRows()-1.
       */
      std::vector<T> getRow(const int row);

      /** \brief Returns the data vector at column \a column
       *
       * The first column is 0. The last avaliable column ist getColumns()-1.
       */
      std::vector<T> getColumn(const int column);

      /** \brief Return the description for the dataset
       *
       * Returns the value of the string attribute named \p Description of the dataset.
       */
      std::string getDescription();

      /** \brief Returns the column labels
       *
       * Return the value of the string vector attribute named \p Column \p Label of
       * the dataset.
       */
      std::vector<std::string> getColumnLabel();

      void extend(const hsize_t* size);
  };



  // inline definitions

  template<class T>
  int VectorSerie<T>::getRows() {
    return dims[0];
  }

  template<class T>
  unsigned int VectorSerie<T>::getColumns() {
    return dims[1];
  }

}

#endif // _TIMESERIE_H_
