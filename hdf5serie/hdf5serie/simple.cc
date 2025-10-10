// open
template<class T>
HDF5SERIE_CLASS<T>::HDF5SERIE_CLASS(int dummy, HDF5SERIE_PARENTCLASS *parent_, const std::string& name_) : HDF5SERIE_BASECLASS(parent_, name_) {
  memDataTypeID=toH5Type<T>();

  id.reset(HDF5SERIE_H5XOPEN, &HDF5SERIE_H5XCLOSE);
  memDataSpaceID.reset(H5Screate(H5S_SCALAR), &H5Sclose);
}

template<class T>
HDF5SERIE_CLASS<T>::HDF5SERIE_CLASS(HDF5SERIE_PARENTCLASS *parent_, const std::string& name_) : HDF5SERIE_BASECLASS(parent_, name_) {
  memDataTypeID=toH5Type<T>();
  memDataSpaceID.reset(H5Screate(H5S_SCALAR), &H5Sclose);
  #ifdef HDF5SERIE_DATASETTYPE
    ScopedHID propID(H5Pcreate(H5P_DATASET_CREATE), &H5Pclose);
    checkCall(H5Pset_attr_phase_change(propID, 0, 0));
  #endif
  id.reset(HDF5SERIE_H5XCREATE, &HDF5SERIE_H5XCLOSE);
}

template<class T>
HDF5SERIE_CLASS<T>::~HDF5SERIE_CLASS() = default;

template<class T>
void HDF5SERIE_CLASS<T>::close() {
  HDF5SERIE_BASECLASS::close();
  // memDataSpaceID.reset(); do not close this since its not file related (to avoid the need for reopen it in writetemp mode)
  id.reset();
}

template<class T>
void HDF5SERIE_CLASS<T>::enableSWMR() {
  if(file->getType(true) == File::writeWithRename)
    id.reset(HDF5SERIE_H5XOPEN, &HDF5SERIE_H5XCLOSE);
  HDF5SERIE_BASECLASS::enableSWMR();
}

template<class T>
void HDF5SERIE_CLASS<T>::write(const T& data) {
  checkCall(HDF5SERIE_H5XWRITE(&data));
}
template<>
void HDF5SERIE_CLASS<string>::write(const string& data) {
  const char *buf[1]={data.c_str()};
  checkCall(HDF5SERIE_H5XWRITE(buf));
}

template<class T>
T HDF5SERIE_CLASS<T>::read() {
  T ret;
  checkCall(HDF5SERIE_H5XREAD(&ret));
  return ret;
}
template<>
string HDF5SERIE_CLASS<string>::read() {
  VecStr buf(1);
  checkCall(HDF5SERIE_H5XREAD(&buf[0]));
  return buf[0];
}

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
  template class HDF5SERIE_CLASS<CTYPE>;
# include "knowntypes.def"
# undef FOREACHKNOWNTYPE



template<class T>
HDF5SERIE_CLASS<vector<T> >::HDF5SERIE_CLASS(int dummy, HDF5SERIE_PARENTCLASS *parent_, const std::string& name_) : HDF5SERIE_BASECLASS(parent_, name_) {
  id.reset(HDF5SERIE_H5XOPEN, &HDF5SERIE_H5XCLOSE);
  memDataSpaceID.reset(HDF5SERIE_H5XGET_SPACE, &H5Sclose);
  hsize_t dims[1];
  checkCall(H5Sget_simple_extent_dims(memDataSpaceID, dims, nullptr));
  size=dims[0];

  if constexpr(is_same_v<T, string>) {
#ifdef HDF5SERIE_DATASETTYPE
    ScopedHID stringTypeID(H5Dget_type(id), &H5Tclose);
    bool isVarStr = H5Tis_variable_str(stringTypeID) > 0;
    if(!isVarStr) {
      fixedStringTypeID.reset(H5Tcopy(H5T_C_S1), &H5Tclose);
      if(H5Tset_size(fixedStringTypeID, H5Tget_size(stringTypeID))<0)
        throw runtime_error("Internal error: Can not create varaible length string datatype.");
      memDataTypeID=fixedStringTypeID;
    }
    else
      memDataTypeID=toH5Type<T>();
#else
    memDataTypeID=toH5Type<T>();
#endif
  }
  else
    memDataTypeID=toH5Type<T>();
}

template<class T>
HDF5SERIE_CLASS<vector<T> >::HDF5SERIE_CLASS(HDF5SERIE_PARENTCLASS *parent_, const std::string& name_, int size_, int fixedStrSize, int compression) : HDF5SERIE_BASECLASS(parent_, name_) {
  size=size_;
  if constexpr(is_same_v<T, string>) {
    if(fixedStrSize<0)
      memDataTypeID=toH5Type<T>();
    else {
      fixedStringTypeID.reset(H5Tcopy(H5T_C_S1), &H5Tclose);
      if(H5Tset_size(fixedStringTypeID, fixedStrSize)<0)
        throw runtime_error("Internal error: Can not create varaible length string datatype.");
      memDataTypeID=fixedStringTypeID;
    }
  }
  else {
    if(fixedStrSize>=0)
      throw runtime_error("A fixed string size is only possible with T=string.");
    memDataTypeID=toH5Type<T>();
  }
  hsize_t dims[1];
  dims[0]=size;
  memDataSpaceID.reset(H5Screate_simple(1, dims, nullptr), &H5Sclose);
  #ifdef HDF5SERIE_DATASETTYPE
    ScopedHID propID(H5Pcreate(H5P_DATASET_CREATE), &H5Pclose);
    checkCall(H5Pset_attr_phase_change(propID, 0, 0));
    checkCall(H5Pset_chunk(propID, 1, dims));
    if(compression>0) checkCall(H5Pset_deflate(propID, compression));
  #endif
  id.reset(HDF5SERIE_H5XCREATE, &HDF5SERIE_H5XCLOSE);
}

template<class T>
HDF5SERIE_CLASS<vector<T> >::~HDF5SERIE_CLASS() = default;

template<class T>
void HDF5SERIE_CLASS<vector<T> >::close() {
  HDF5SERIE_BASECLASS::close();
  // memDataSpaceID.reset(); do not close this since its not file related (to avoid the need for reopen it in writetemp mode)
  id.reset();
}

template<class T>
void HDF5SERIE_CLASS<vector<T> >::enableSWMR() {
  if(file->getType(true) == File::writeWithRename)
    id.reset(HDF5SERIE_H5XOPEN, &HDF5SERIE_H5XCLOSE);
  HDF5SERIE_BASECLASS::enableSWMR();
}

template<class T>
void HDF5SERIE_CLASS<vector<T> >::write(const vector<T> &data) {
  if(static_cast<int>(data.size())!=size)
    throw Exception(getPath(), "Size mismatch in write.");
  checkCall(HDF5SERIE_H5XWRITE(&data[0]));
}
template<>
void HDF5SERIE_CLASS<vector<string> >::write(const vector<string>& data) {
  if(static_cast<int>(data.size())!=size)
    throw Exception(getPath(), "the dimension does not match");
  if(fixedStringTypeID<0) {
    VecStr buf(size);
    for(unsigned int i=0; i<static_cast<size_t>(size); i++) {
      auto &e=data[i];
      buf.alloc(i, e.size());
      strcpy(buf[i], e.c_str());
    }
    checkCall(HDF5SERIE_H5XWRITE(&buf[0]));
  }
  else {
    auto fixedStrSize=H5Tget_size(fixedStringTypeID);
    vector<char> buf(fixedStrSize*size, '\0');
    for(int i=0; i<size; i++) {
      auto &e=data[i];
      if(e.size()>fixedStrSize)
        throw Exception(getPath(), "The string to write has length "+to_string(e.size())+
                                   " which is longer than the defined fixed string size of "+to_string(fixedStrSize)+".");
      copy(e.begin(), e.end(), buf.begin()+fixedStrSize*i);
    }
    checkCall(HDF5SERIE_H5XWRITE(&buf[0]));
  }
}

template<class T>
vector<T> HDF5SERIE_CLASS<vector<T> >::read() {
  vector<T> ret(size);
  checkCall(HDF5SERIE_H5XREAD(&ret[0]));
  return ret;
}
template<>
vector<string> HDF5SERIE_CLASS<vector<string> >::read() {
  vector<string> data;
  if(fixedStringTypeID<0) {
    VecStr buf(size);
    checkCall(HDF5SERIE_H5XREAD(&buf[0]));
    for(unsigned int i=0; i<static_cast<size_t>(size); i++)
      data.emplace_back(buf[i]);
  }
  else {
    auto fixedStrSize=H5Tget_size(fixedStringTypeID);
    vector<char> buf(fixedStrSize*size);
    checkCall(HDF5SERIE_H5XREAD(&buf[0]));
    for(unsigned int i=0; i<static_cast<size_t>(size); i++) {
      char *start=&buf[i*fixedStrSize];
      data.emplace_back(start, strnlen(start, fixedStrSize));
    }
  }
  return data;
}

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
  template class HDF5SERIE_CLASS<vector<CTYPE> >;
# include "knowntypes.def"
# undef FOREACHKNOWNTYPE



template<class T>
HDF5SERIE_CLASS<vector<vector<T> > >::HDF5SERIE_CLASS(int dummy, HDF5SERIE_PARENTCLASS *parent_, const std::string& name_) : HDF5SERIE_BASECLASS(parent_, name_) {
  id.reset(HDF5SERIE_H5XOPEN, &HDF5SERIE_H5XCLOSE);
  memDataSpaceID.reset(HDF5SERIE_H5XGET_SPACE, &H5Sclose);
  hsize_t dims[2];
  checkCall(H5Sget_simple_extent_dims(memDataSpaceID, dims, nullptr));
  rows=dims[0];
  cols=dims[1];

  if constexpr(is_same_v<T, string>) {
#ifdef HDF5SERIE_DATASETTYPE
    ScopedHID stringTypeID(H5Dget_type(id), &H5Tclose);
    if(H5Tis_variable_str(stringTypeID) == 0) {
      fixedStringTypeID.reset(H5Tcopy(H5T_C_S1), &H5Tclose);
      if(H5Tset_size(fixedStringTypeID, H5Tget_size(stringTypeID))<0)
        throw runtime_error("Internal error: Can not create varaible length string datatype.");
      memDataTypeID=fixedStringTypeID;
    }
    else
      memDataTypeID=toH5Type<T>();
#else
    memDataTypeID=toH5Type<T>();
#endif
  }
  else
    memDataTypeID=toH5Type<T>();
}

template<class T>
HDF5SERIE_CLASS<vector<vector<T> > >::HDF5SERIE_CLASS(HDF5SERIE_PARENTCLASS *parent_, const std::string& name_, int rows_, int cols_, int fixedStrSize, int compression) : HDF5SERIE_BASECLASS(parent_, name_) {
  rows=rows_;
  cols=cols_;
  if constexpr(is_same_v<T, string>) {
    if(fixedStrSize<0)
      memDataTypeID=toH5Type<T>();
    else {
      fixedStringTypeID.reset(H5Tcopy(H5T_C_S1), &H5Tclose);
      if(H5Tset_size(fixedStringTypeID, fixedStrSize)<0)
        throw runtime_error("Internal error: Can not create varaible length string datatype.");
      memDataTypeID=fixedStringTypeID;
    }
  }
  else {
    if(fixedStrSize>=0)
      throw runtime_error("A fixed string size is only possible with T=string.");
    memDataTypeID=toH5Type<T>();
  }
  hsize_t dims[2];
  dims[0]=rows;
  dims[1]=cols;
  memDataSpaceID.reset(H5Screate_simple(2, dims, nullptr), &H5Sclose);
  #ifdef HDF5SERIE_DATASETTYPE
    ScopedHID propID(H5Pcreate(H5P_DATASET_CREATE), &H5Pclose);
    checkCall(H5Pset_attr_phase_change(propID, 0, 0));
    checkCall(H5Pset_chunk(propID, 2, dims));
    if(compression>0) checkCall(H5Pset_deflate(propID, compression));
  #endif
  id.reset(HDF5SERIE_H5XCREATE, &HDF5SERIE_H5XCLOSE);
}

template<class T>
HDF5SERIE_CLASS<vector<vector<T> > >::~HDF5SERIE_CLASS() = default;

template<class T>
void HDF5SERIE_CLASS<vector<vector<T> > >::close() {
  HDF5SERIE_BASECLASS::close();
  // memDataSpaceID.reset(); do not close this since its not file related (to avoid the need for reopen it in writetemp mode)
  id.reset();
}

template<class T>
void HDF5SERIE_CLASS<vector<vector<T> > >::enableSWMR() {
  if(file->getType(true) == File::writeWithRename)
    id.reset(HDF5SERIE_H5XOPEN, &HDF5SERIE_H5XCLOSE);
  HDF5SERIE_BASECLASS::enableSWMR();
}

template<class T>
void HDF5SERIE_CLASS<vector<vector<T> > >::write(const vector<vector<T> > &data) {
  if(static_cast<int>(data.size())!=rows || static_cast<int>(data[0].size())!=cols)
    throw Exception(getPath(), "Size mismatch in write.");
  vector<T> buf(rows*cols);
  int i=0;
  for(auto ir=data.begin(); ir!=data.end(); ++ir)
    for(auto ic=ir->begin(); ic!=ir->end(); ++ic, ++i)
      buf[i]=*ic;
  checkCall(HDF5SERIE_H5XWRITE(&buf[0]));
}
template<>
void HDF5SERIE_CLASS<vector<vector<string> > >::write(const vector<vector<string> >& data) {
  if(static_cast<int>(data.size())!=rows || static_cast<int>(data[0].size())!=cols)
    throw Exception(getPath(), "Size mismatch in write.");
  if(fixedStringTypeID<0) {
    VecStr buf(rows*cols);
    int i=0;
    for(const auto & ir : data)
      for(auto ic=ir.begin(); ic!=ir.end(); ++ic, ++i) {
        buf.alloc(i, ic->size());
        strcpy(buf[i], ic->c_str());
      }
    checkCall(HDF5SERIE_H5XWRITE(&buf[0]));
  }
  else {
    auto fixedStrSize=H5Tget_size(fixedStringTypeID);
    vector<char> buf(fixedStrSize*rows*cols, '\0');
    for(int r=0; r<rows; r++)
      for(int c=0; c<cols; c++) {
        auto &e=data[r][c];
        if(e.size()>fixedStrSize)
          throw Exception(getPath(), "The string to write has length "+to_string(e.size())+
                                     " which is longer than the defined fixed string size of "+to_string(fixedStrSize)+".");
        copy(e.begin(), e.end(), buf.begin()+fixedStrSize*(cols*r+c));
      }
    checkCall(HDF5SERIE_H5XWRITE(&buf[0]));
  }
}

template<class T>
vector<vector<T> > HDF5SERIE_CLASS<vector<vector<T> > >::read() {
  vector<T> buf(rows*cols);
  checkCall(HDF5SERIE_H5XREAD(&buf[0]));
  vector<vector<T> > ret(rows);
  int r=0, c;
  for(auto ir=ret.begin(); ir!=ret.end(); ++ir, ++r) {
    ir->resize(cols);
    c=0;
    for(auto ic=ir->begin(); ic!=ir->end(); ++ic, ++c)
      *ic=buf[r*cols+c];
  }
  return ret;
}
template<>
vector<vector<string> > HDF5SERIE_CLASS<vector<vector<string> > >::read() {
  vector<vector<string> > ret(rows);
  if(fixedStringTypeID<0) {
    VecStr buf(rows*cols);
    checkCall(HDF5SERIE_H5XREAD(&buf[0]));
    int r=0, c;
    for(auto ir=ret.begin(); ir!=ret.end(); ++ir, ++r) {
      ir->resize(cols);
      c=0;
      for(auto ic=ir->begin(); ic!=ir->end(); ++ic, ++c)
        *ic=buf[r*cols+c];
    }
  }
  else {
    auto fixedStrSize=H5Tget_size(fixedStringTypeID);
    vector<char> buf(fixedStrSize*rows*cols);
    checkCall(HDF5SERIE_H5XREAD(&buf[0]));
    for(int r=0; r<rows; ++r) {
      ret[r].resize(cols);
      for(int c=0; c<cols; ++c) {
        char *start=&buf[fixedStrSize*(r*cols+c)];
        ret[r][c]=string(start, strnlen(start, fixedStrSize));
      }
    }
  }
  return ret;
}

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
  template class HDF5SERIE_CLASS<vector<vector<CTYPE> > >;
# include "knowntypes.def"
# undef FOREACHKNOWNTYPE
