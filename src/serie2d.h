#ifndef _SERIE2D_H_
#define _SERIE2D_H_

#include <H5Cpp.h>
#include <vector>
#include <string>
#include <assert.h>
#include <simpleattribute.h>

namespace H5 {

  template<class T>
  class Serie2D : public DataSet {
    private:
      DataType memDataType;
      DataSpace memDataSpace;
      hsize_t dims[2];
    public:
      Serie2D();
      Serie2D(const Serie2D<T>& dataset);
      Serie2D(const CommonFG& parent, const std::string& name);
      Serie2D(const CommonFG& parent, const std::string& name, const std::vector<std::string>& columnLabel);
      void create(const CommonFG& parent, const std::string& name, const std::vector<std::string>& columnLabel);
      void open(const CommonFG& parent, const std::string& name);
      void setDescription(const std::string& desc);
      void append(const std::vector<T> &data);
      void operator<<(const std::vector<T>& data);
      inline int getRows();
      inline int getColumns();
      std::vector<T> getRow(const int row);
      std::vector<T> getColumn(const int column);
      std::string getDescription();
      std::vector<std::string> getColumnLabel();

      void extend(const hsize_t* size);
  };



  // inline definitions

  template<class T>
  int Serie2D<T>::getRows() {
    //////////
    return dims[0];
    //////////
    //DataSpace fileDataSpace=getSpace();
    //fileDataSpace.getSimpleExtentDims(dims);
    //return dims[0];
    //////////
  }

  template<class T>
  int Serie2D<T>::getColumns() {
    return dims[1];
  }

}

#endif // _TIMESERIE_H_
