#ifndef _SIMPLEATTRIBUTE_H_
#define _SIMPLEATTRIBUTE_H_

#include <H5Cpp.h>
#include <vector>
#include <string>
#include <toh5type.h>
#include <assert.h>

namespace H5 {

  template<class T>
  class SimpleAttribute : public Attribute {
    private:
      DataType memDataType;
    public:
      SimpleAttribute();
      SimpleAttribute(const SimpleAttribute<T>& attribute);
      SimpleAttribute(const DataSet& parent, const std::string& name, bool create=false);
      SimpleAttribute(const DataSet& parent, const std::string& name, const T& data);
      void create(const DataSet& parent, const std::string& name);
      void open(const DataSet& parent, const std::string& name);
      void write(const T& data);
      T read();
      void write(const DataSet& parent, const std::string& name, const T& data);
      T read(const DataSet& parent, const std::string& name);
      static T getData(const DataSet& parent, const std::string& name);
      static T getData(const CommonFG& parent, const std::string& name);
  };

  // a partial specialisation of class SimpleAttribute<T> for T=std::vector<T>
  template<class T>
  class SimpleAttribute<std::vector<T> > : public Attribute {
    private:
      DataType memDataType;
    public:
      SimpleAttribute();
      SimpleAttribute(const SimpleAttribute<std::vector<T> >& attribute);
      SimpleAttribute(const DataSet& parent, const std::string& name, const int count=0);
      SimpleAttribute(const DataSet& parent, const std::string& name, const std::vector<T>& data);
      void create(const DataSet& parent, const std::string& name, const int count);
      void open(const DataSet& parent, const std::string& name);
      void write(const std::vector<T>& data);
      std::vector<T> read();
      void write(const DataSet& parent, const std::string& name, const std::vector<T>& data);
      std::vector<T> read(const DataSet& parent, const std::string& name);
      static std::vector<T> getData(const DataSet& parent, const std::string& name);
      static std::vector<T> getData(const CommonFG& parent, const std::string& name);
  };

  // a partial specialisation of class SimpleAttribute<T> for T=std::vector<std::vector<T> >
  template<class T>
  class SimpleAttribute<std::vector<std::vector<T> > > : public Attribute {
    private:
      DataType memDataType;
    public:
      SimpleAttribute();
      SimpleAttribute(const SimpleAttribute<std::vector<std::vector<T> > >& attribute);
      SimpleAttribute(const DataSet& parent, const std::string& name, const int rows=0, const int columns=0);
      SimpleAttribute(const DataSet& parent, const std::string& name, const std::vector<std::vector<T> >& data);
      void create(const DataSet& parent, const std::string& name, const int rows, const int columns);
      void open(const DataSet& parent, const std::string& name);
      void write(const std::vector<std::vector<T> >& data);
      std::vector<std::vector<T> > read();
      void write(const DataSet& parent, const std::string& name, const std::vector<std::vector<T> >& data);
      std::vector<std::vector<T> > read(const DataSet& parent, const std::string& name);
      static std::vector<std::vector<T> > getData(const DataSet& parent, const std::string& name);
      static std::vector<std::vector<T> > getData(const CommonFG& parent, const std::string& name);
  };

}

#endif
