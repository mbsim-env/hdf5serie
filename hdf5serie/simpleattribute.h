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

#ifndef _SIMPLEATTRIBUTE_H_
#define _SIMPLEATTRIBUTE_H_

#include <H5Cpp.h>
#include <vector>
#include <string>
#include <hdf5serie/toh5type.h>
#include <assert.h>

namespace H5 {

  /** \brief A scalar, vector or matrix attribute
   *
   * A HDF5 attribute of a scalar a vector or a matrix of fixed size of type CTYPE.
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
  class SimpleAttribute : public Attribute {
    private:
      DataType memDataType;
    public:
      /** \brief A stub constructor */
      SimpleAttribute();

      /** \brief Copy constructor */
      SimpleAttribute(const SimpleAttribute<T>& attribute);

      /** \brief Constructor for opening or creating a attribute
       *
       * For a scalar value T=CTYPE: 
       * If \a create is true see create(), if \a create is false see open()
       *
       * For a vector value T=std::vector<CTYPE>:
       * The declaration of this function is 
       * SimpleAttribute(const H5Object& parent, const std::string& name, int count=0)
       * If \a count is not 0 see create(), if \a create is 0 see open()
       *
       * For a matrix value T=std::vector<std::vector<CTYPE> >:
       * The declaration of this function is 
       * SimpleAttribute(const H5Object& parent, const std::string& name, int rows=0, int columns=0)
       * If \a rows is not 0 and \a column is not 0 see create(), if \a rows and \a columns is 0 see open()
       */
      SimpleAttribute(const H5Object& parent, const std::string& name, bool create=false);

      /** \brief Constructor for creating and writing a attribute
       *
       * See create() and write().
       */
      SimpleAttribute(const H5Object& parent, const std::string& name, const T& data);

      /** Creating a attribute
       *
       * For a scalar value T=CTYPE: 
       * Creates a scalar attribute named \a name as a child of \a parent.
       *
       * For a vector value T=std::vector<CTYPE>:
       * The declaration of this function is 
       * void create(const H5Object& parent, const std::string& name, int count);
       * Creates a vector attribute of size \a count named \a name as a child of \a parent.
       *
       * For a matrix value T=std::vector<std::vector<CTYPE> >:
       * The declaration of this function is 
       * void create(const H5Object& parent, const std::string& name, int rows, int columns);
       * Creates a matrix attribute of size \a count x \a columns named \a name as a child of \a parent.
       */
      void create(const H5Object& parent, const std::string& name);

      /** \brief Open a attribute
       *
       * Opens the attribute named \a name as a child of position \a parent.
       */
      void open(const H5Object& parent, const std::string& name);

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

      /** \brief Creating and write a attribute
       *
       * See create() and write().
       */
      void write(const H5Object& parent, const std::string& name, const T& data);

      /** \brief Open and read data
       *
       * See open() and read()
       */
      T read(const H5Object& parent, const std::string& name);

      /** Static open and read data.
       *
       * This function can be called statically. See open() and read()
       */
      static T getData(const H5Object& parent, const std::string& name);

      /** Static write data.
       *
       * This function can be called statically. See write()
       */
      static void setData(const H5Object& parent, const std::string& name, const T& data);
  };

  // a partial specialisation of class SimpleAttribute<T> for T=std::vector<T>
  template<class T>
  class SimpleAttribute<std::vector<T> > : public Attribute {
    private:
      DataType memDataType;
    public:
      SimpleAttribute();
      SimpleAttribute(const SimpleAttribute<std::vector<T> >& attribute);
      SimpleAttribute(const H5Object& parent, const std::string& name, const int count=0);
      SimpleAttribute(const H5Object& parent, const std::string& name, const std::vector<T>& data);
      void create(const H5Object& parent, const std::string& name, const int count);
      void open(const H5Object& parent, const std::string& name);
      void write(const std::vector<T>& data);
      std::vector<T> read();
      void write(const H5Object& parent, const std::string& name, const std::vector<T>& data);
      std::vector<T> read(const H5Object& parent, const std::string& name);
      static std::vector<T> getData(const H5Object& parent, const std::string& name);
      static void setData(const H5Object& parent, const std::string& name, const std::vector<T>& data);
  };

  // a partial specialisation of class SimpleAttribute<T> for T=std::vector<std::vector<T> >
  template<class T>
  class SimpleAttribute<std::vector<std::vector<T> > > : public Attribute {
    private:
      DataType memDataType;
    public:
      SimpleAttribute();
      SimpleAttribute(const SimpleAttribute<std::vector<std::vector<T> > >& attribute);
      SimpleAttribute(const H5Object& parent, const std::string& name, const int rows=0, const int columns=0);
      SimpleAttribute(const H5Object& parent, const std::string& name, const std::vector<std::vector<T> >& data);
      void create(const H5Object& parent, const std::string& name, const int rows, const int columns);
      void open(const H5Object& parent, const std::string& name);
      void write(const std::vector<std::vector<T> >& data);
      std::vector<std::vector<T> > read();
      void write(const H5Object& parent, const std::string& name, const std::vector<std::vector<T> >& data);
      std::vector<std::vector<T> > read(const H5Object& parent, const std::string& name);
      static std::vector<std::vector<T> > getData(const H5Object& parent, const std::string& name);
      static void setData(const H5Object& parent, const std::string& name, const std::vector<std::vector<T> >& data);
  };

}

#endif
