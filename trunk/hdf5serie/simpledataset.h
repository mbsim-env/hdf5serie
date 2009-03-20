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
 *   mafriedrich@users.berlios.de
 *
 */

#ifndef _HDF5SERIE_SIMPLEDATASET_H_
#define _HDF5SERIE_SIMPLEDATASET_H_

#include <H5Cpp.h>
#include <vector>
#include <string>
#include <hdf5serie/toh5type.h>
#include <hdf5serie/simpleattribute.h>
#include <assert.h>

namespace H5 {

  /** \brief A scalar, vector or matrix dataset
   *
   * A HDF5 dataset of a scalar a vector or a matrix of fixed size of type CTYPE.
   * CTYPE can be one of the folling types.
   * - char
   * - signed char
   * - unsigned char
   * - short
   * - unsigned short
   * - int
   * - unsigned int
   * - long
   * - unsigned long
   * - long long
   * - unsigned long long
   * - float
   * - double
   * - long double
   * - std::string
   *
   * The template type T is simple CTYPE for a scalar, std::vector<CTYPE> for a vector
   * and std::vector<std::vector<CTYPE> > for a matrix.
   *
   * A NOTE if using a vector-matrix-library: See class description of VectorSerie.
   */
  template<class T>
  class SimpleDataSet : public DataSet {
    private:
      DataSpace dataSpace;
      DataType memDataType;
    public:
      /** \brief A stub constructor */
      SimpleDataSet();

      /** \brief Copy constructor */
      SimpleDataSet(const SimpleDataSet<T>& dataset);

      /** \brief Constructor for opening or creating a dataset
       *
       * If \a create is true see create(), if \a create is false see open()
       */
      SimpleDataSet(const CommonFG& parent, const std::string& name, bool create=false);

      /** \brief Constructor for creating and writing a dataset
       *
       * See create() and write().
       */
      SimpleDataSet(const CommonFG& parent, const std::string& name, const T& data);
      
      /** Creating a dataset
       *
       * Creates a dataset named \a name as a child of \a parent.
       */
      void create(const CommonFG& parent, const std::string& name);

      /** \brief Open a dataset
       *
       * Opens the dataset named \a name as a child of position \a parent.
       */
      void open(const CommonFG& parent, const std::string& name);

      /** \brief Write data
       *
       * Writes the data \a data to the HDF5 file
       */
      void write(const T& data);

      /** \brief Read data
       *
       * Read the data from the HDF5 file
       */
      T read();

      /** \brief Creating and write a dataset
       *
       * See create() and write().
       */
      void write(const CommonFG& parent, const std::string& name, const T& data);

      /** \brief Open and read data
       *
       * See open() and read()
       */
      T read(const CommonFG& parent, const std::string& name);

      /** \brief Sets a description for the dataset
       *
       * The value of \a desc is stored as an string attribute named \p Description in the dataset.
       */
      void setDescription(const std::string& description);

      /** \brief Return the description for the dataset
       *
       * Returns the value of the string attribute named \p Description of the dataset.
       */
      std::string getDescription();

      /** Static open and read data.
       *
       * This function can be called statically. See open() and read()
       */
      static T getData(const CommonFG& parent, const std::string& name);
  };

  // a partial specialisation of class SimpleDataSet<T> for T=std::vector<T>
  template<class T>
  class SimpleDataSet<std::vector<T> > : public DataSet {
    private:
      DataType memDataType;
    public:
      SimpleDataSet();
      SimpleDataSet(const SimpleDataSet<std::vector<T> >& dataset);
      SimpleDataSet(const CommonFG& parent, const std::string& name, bool create=false);
      SimpleDataSet(const CommonFG& parent, const std::string& name, const std::vector<T>& data);
      void create(const CommonFG& parent, const std::string& name);
      void open(const CommonFG& parent, const std::string& name);
      void write(const std::vector<T>& data);
      std::vector<T> read();
      void write(const CommonFG& parent, const std::string& name, const std::vector<T>& data);
      std::vector<T> read(const CommonFG& parent, const std::string& name);
      void setDescription(const std::string& description);
      std::string getDescription();
      static std::vector<T>  getData(const CommonFG& parent, const std::string& name);
  };

  // a partial specialisation of class SimpleDataSet<T> for T=std::vector<std::vector<T> >
  template<class T>
  class SimpleDataSet<std::vector<std::vector<T> > > : public DataSet {
    private:
      DataType memDataType;
    public:
      SimpleDataSet();
      SimpleDataSet(const SimpleDataSet<std::vector<std::vector<T> > >& dataset);
      SimpleDataSet(const CommonFG& parent, const std::string& name, bool create=false);
      SimpleDataSet(const CommonFG& parent, const std::string& name, const std::vector<std::vector<T> >& data);
      void create(const CommonFG& parent, const std::string& name);
      void open(const CommonFG& parent, const std::string& name);
      void write(const std::vector<std::vector<T> >& data);
      std::vector<std::vector<T> > read();
      void write(const CommonFG& parent, const std::string& name, const std::vector<std::vector<T> >& data);
      std::vector<std::vector<T> > read(const CommonFG& parent, const std::string& name);
      void setDescription(const std::string& description);
      std::string getDescription();
      static std::vector<std::vector<T> > getData(const CommonFG& parent, const std::string& name);
  };

}

#endif
