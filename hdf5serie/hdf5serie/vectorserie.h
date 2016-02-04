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

#include <hdf5serie/interface.h>
#include <hdf5serie/file.h>
#include <vector>

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
   * A note when using a vector-matrix-library:
   * It is likly that the data is calculated by a vector-matrix-library. If so,
   * and the vector object (of type T) of the library (e.g. fmatvec) has a size() member function, returning
   * the size of the vector, and operator[](int i) returns for i=0 a reference to the first element and
   * all other elements are store continously after the address of this element, then you
   * can use the vector-object as parameter for append(const DataType &data).
  */
  template<class T>
  class VectorSerie : public Dataset {
    friend class Container<Object, GroupBase>;
    private:
      hid_t memDataTypeID;
      ScopedHID memDataSpaceID;
      hsize_t dims[2];
    protected:
      VectorSerie(int dummy, GroupBase *parent_, const std::string &name_);
      VectorSerie(GroupBase *parent_, const std::string &name_, int cols,
        int compression=File::getDefaultCompression(), int chunkSize=File::getDefaultChunkSize());
      ~VectorSerie();
      void close();
      void open();

    public:
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
      void append(const T data[], size_t size);

      /** Convinience append function.
       * DataType must provide a "size_t size()" member function which returns the number of elements
       * as well as a "T &operator[](int i)" member function which returns a reference to the i-te element.
       * All elements must be lie in order in memory */
      template<class DataType>
      void append(const DataType &data){
        append(&data[0], data.size());
      }

      /** \brief Returns the number of rows in the dataset */
      inline int getRows();

      /** \brief Returns the number of columns(=number of data elements) in the dataset */
      inline unsigned int getColumns();

      /** \brief Returns the data vector at row \a row
       * The first row is 0. The last avaliable row ist getRows()-1.
       * \a data points to an array of \a size elements of type T.
       */
      void getRow(const int row, size_t size, T data[]);

      /** Convinience getRow function.
       * DataType must provide a "size_t size()" member function which returns the number of elements
       * as well as a "T &operator[](int i)" member function which returns a reference to the i-te element.
       * The elements of DataType must be of Type T and must be stored sequencially in memory. */
      template<class DataType>
      void getRow(const int row, DataType &data) {
        getRow(row, data.size(), &data[0]);
      }

      /** Convinience getRow function for backward compatiblity.
       * Returns a copy in form of a std::vector<T>. */
      std::vector<T> getRow(const int row) {
        std::vector<T> data(dims[1]);
        getRow(row, dims[1], &data[0]);
        return data;
      }

      /** \brief Returns the data vector at column \a column
       *
       * The first column is 0. The last avaliable column ist getColumns()-1.
       */
      void getColumn(const int column, size_t size, T data[]);

      /** Convinience getRow function.
       * DataType must provide a "size_t size()" member function which returns the number of elements
       * as well as a "T &operator[](int i)" member function which returns a reference to the i-te element.
       * The elements of DataType must be of Type T and must be stored sequencially in memory. */
      template<class DataType>
      void getColumn(const int column, DataType &data) {
        getColumn(column, data.size(), &data[0]);
      }

      /** Convinience getColumn function for backward compatiblity.
       * Returns a copy in form of a std::vector<T>. */
      std::vector<T> getColumn(const int row) {
        size_t rows=getRows();
        std::vector<T> data(rows);
        getColumn(row, rows, &data[0]);
        return data;
      }

      /** \brief Return the description for the dataset
       *
       * Returns the value of the string attribute named \p Description of the dataset.
       */
      std::string getDescription();

      void setColumnLabel(const std::vector<std::string> &columnLabel);

      /** \brief Returns the column labels
       *
       * Return the value of the string vector attribute named \p Column \p Label of
       * the dataset.
       */
      std::vector<std::string> getColumnLabel();
  };



  // inline definitions

  template<class T>
  int VectorSerie<T>::getRows() {
    ScopedHID fileSpaceID(H5Dget_space(id), &H5Sclose);
    H5Sget_simple_extent_dims(fileSpaceID, dims, NULL);
    return dims[0];
  }

  template<class T>
  unsigned int VectorSerie<T>::getColumns() {
    return dims[1];
  }

}

#endif // _TIMESERIE_H_
