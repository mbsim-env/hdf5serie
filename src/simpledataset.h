#ifndef _SIMPLEDATASET_H_
#define _SIMPLEDATASET_H_

#include <H5Cpp.h>
#include <vector>
#include <string>
#include <toh5type.h>
#include <simpleattribute.h>
#include <assert.h>

namespace H5 {

  template<class T>
  class SimpleDataSet : public DataSet {
    private:
      DataSpace dataSpace;
      DataType memDataType;
    public:
      SimpleDataSet();
      SimpleDataSet(const SimpleDataSet<T>& dataset);
      SimpleDataSet(const CommonFG& parent, const std::string& name, bool create=false);
      SimpleDataSet(const CommonFG& parent, const std::string& name, const T& data);
      void create(const CommonFG& parent, const std::string& name);
      void open(const CommonFG& parent, const std::string& name);
      void write(const T& data);
      T read();
      void write(const CommonFG& parent, const std::string& name, const T& data);
      T read(const CommonFG& parent, const std::string& name);
      void setDescription(const std::string& description);
      std::string getDescription();
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
