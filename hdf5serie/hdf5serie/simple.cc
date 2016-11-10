// open
template<class T>
HDF5SERIE_CLASS<T>::HDF5SERIE_CLASS(int dummy, HDF5SERIE_PARENTCLASS *parent_, const std::string& name_) : HDF5SERIE_BASECLASS(parent_, name_) {
  T dummy2;
  memDataTypeID=toH5Type(dummy2);
  open();
}

template<class T>
HDF5SERIE_CLASS<T>::HDF5SERIE_CLASS(HDF5SERIE_PARENTCLASS *parent_, const std::string& name_) : HDF5SERIE_BASECLASS(parent_, name_) {
  T dummy;
  memDataTypeID=toH5Type(dummy);
  memDataSpaceID.reset(H5Screate(H5S_SCALAR), &H5Sclose);
  #ifdef HDF5SERIE_DATASETTYPE
    ScopedHID propID(H5Pcreate(H5P_DATASET_CREATE), &H5Pclose);
    H5Pset_attr_phase_change(propID, 0, 0);
  #endif
  id.reset(HDF5SERIE_H5XCREATE, &HDF5SERIE_H5XCLOSE);
}

template<class T>
HDF5SERIE_CLASS<T>::~HDF5SERIE_CLASS() {
}

template<class T>
void HDF5SERIE_CLASS<T>::close() {
  HDF5SERIE_BASECLASS::close();
  memDataSpaceID.reset();
  id.reset();
}

template<class T>
void HDF5SERIE_CLASS<T>::open() {
  id.reset(HDF5SERIE_H5XOPEN, &HDF5SERIE_H5XCLOSE);
  memDataSpaceID.reset(H5Screate(H5S_SCALAR), &H5Sclose);
  HDF5SERIE_BASECLASS::open();
}

template<class T>
void HDF5SERIE_CLASS<T>::write(const T& data) {
  HDF5SERIE_H5XWRITE(&data);
}
template<>
void HDF5SERIE_CLASS<string>::write(const string& data) {
  const char *buf[1]={data.c_str()};
  HDF5SERIE_H5XWRITE(buf);
}

template<class T>
T HDF5SERIE_CLASS<T>::read() {
  T ret;
  HDF5SERIE_H5XREAD(&ret);
  return ret;
}
template<>
string HDF5SERIE_CLASS<string>::read() {
  VecStr buf(1);
  HDF5SERIE_H5XREAD(&buf[0]);
  return buf[0];
}

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
  template class HDF5SERIE_CLASS<CTYPE>;
# include "knowntypes.def"
# undef FOREACHKNOWNTYPE



template<class T>
HDF5SERIE_CLASS<vector<T> >::HDF5SERIE_CLASS(int dummy, HDF5SERIE_PARENTCLASS *parent_, const std::string& name_) : HDF5SERIE_BASECLASS(parent_, name_) {
  T dummy2;
  memDataTypeID=toH5Type(dummy2);
  open();
}

template<class T>
HDF5SERIE_CLASS<vector<T> >::HDF5SERIE_CLASS(HDF5SERIE_PARENTCLASS *parent_, const std::string& name_, int size_) : HDF5SERIE_BASECLASS(parent_, name_) {
  size=size_;
  T dummy;
  memDataTypeID=toH5Type(dummy);
  hsize_t dims[1];
  dims[0]=size;
  memDataSpaceID.reset(H5Screate_simple(1, dims, NULL), &H5Sclose);
  #ifdef HDF5SERIE_DATASETTYPE
    ScopedHID propID(H5Pcreate(H5P_DATASET_CREATE), &H5Pclose);
    H5Pset_attr_phase_change(propID, 0, 0);
  #endif
  id.reset(HDF5SERIE_H5XCREATE, &HDF5SERIE_H5XCLOSE);
}

template<class T>
HDF5SERIE_CLASS<vector<T> >::~HDF5SERIE_CLASS() {
}

template<class T>
void HDF5SERIE_CLASS<vector<T> >::close() {
  HDF5SERIE_BASECLASS::close();
  memDataSpaceID.reset();
  id.reset();
}

template<class T>
void HDF5SERIE_CLASS<vector<T> >::open() {
  id.reset(HDF5SERIE_H5XOPEN, &HDF5SERIE_H5XCLOSE);
  memDataSpaceID.reset(HDF5SERIE_H5XGET_SPACE, &H5Sclose);
  hsize_t dims[1];
  H5Sget_simple_extent_dims(memDataSpaceID, dims, NULL);
  size=dims[0];
  HDF5SERIE_BASECLASS::open();
}

template<class T>
void HDF5SERIE_CLASS<vector<T> >::write(const vector<T> &data) {
  if(static_cast<int>(data.size())!=size)
    throw Exception(getPath(), "Size mismatch in write.");
  HDF5SERIE_H5XWRITE(&data[0]);
}
template<>
void HDF5SERIE_CLASS<vector<string> >::write(const vector<string>& data) {
  if(static_cast<int>(data.size())!=size)
    throw Exception(getPath(), "the dimension does not match");
  VecStr buf(size);
  for(unsigned int i=0; i<static_cast<size_t>(size); i++) {
    buf[i]=(char*)malloc((data[i].size()+1)*sizeof(char));
    strcpy(buf[i], data[i].c_str());
  }
  HDF5SERIE_H5XWRITE(&buf[0]);
}

template<class T>
vector<T> HDF5SERIE_CLASS<vector<T> >::read() {
  vector<T> ret(size);
  HDF5SERIE_H5XREAD(&ret[0]);
  return ret;
}
template<>
vector<string> HDF5SERIE_CLASS<vector<string> >::read() {
  VecStr buf(size);
  HDF5SERIE_H5XREAD(&buf[0]);
  vector<string> data;
  for(unsigned int i=0; i<static_cast<size_t>(size); i++)
    data.push_back(buf[i]);
  return data;
}

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
  template class HDF5SERIE_CLASS<vector<CTYPE> >;
# include "knowntypes.def"
# undef FOREACHKNOWNTYPE



template<class T>
HDF5SERIE_CLASS<vector<vector<T> > >::HDF5SERIE_CLASS(int dummy, HDF5SERIE_PARENTCLASS *parent_, const std::string& name_) : HDF5SERIE_BASECLASS(parent_, name_) {
  T dummy2;
  memDataTypeID=toH5Type(dummy2);
  open();
}

template<class T>
HDF5SERIE_CLASS<vector<vector<T> > >::HDF5SERIE_CLASS(HDF5SERIE_PARENTCLASS *parent_, const std::string& name_, int rows_, int cols_) : HDF5SERIE_BASECLASS(parent_, name_) {
  rows=rows_;
  cols=cols_;
  T dummy;
  memDataTypeID=toH5Type(dummy);
  hsize_t dims[2];
  dims[0]=rows;
  dims[1]=cols;
  memDataSpaceID.reset(H5Screate_simple(2, dims, NULL), &H5Sclose);
  #ifdef HDF5SERIE_DATASETTYPE
    ScopedHID propID(H5Pcreate(H5P_DATASET_CREATE), &H5Pclose);
    H5Pset_attr_phase_change(propID, 0, 0);
  #endif
  id.reset(HDF5SERIE_H5XCREATE, &HDF5SERIE_H5XCLOSE);
}

template<class T>
HDF5SERIE_CLASS<vector<vector<T> > >::~HDF5SERIE_CLASS() {
}

template<class T>
void HDF5SERIE_CLASS<vector<vector<T> > >::close() {
  HDF5SERIE_BASECLASS::close();
  memDataSpaceID.reset();
  id.reset();
}

template<class T>
void HDF5SERIE_CLASS<vector<vector<T> > >::open() {
  id.reset(HDF5SERIE_H5XOPEN, &HDF5SERIE_H5XCLOSE);
  memDataSpaceID.reset(HDF5SERIE_H5XGET_SPACE, &H5Sclose);
  hsize_t dims[2];
  H5Sget_simple_extent_dims(memDataSpaceID, dims, NULL);
  rows=dims[0];
  cols=dims[1];
  HDF5SERIE_BASECLASS::open();
}

template<class T>
void HDF5SERIE_CLASS<vector<vector<T> > >::write(const vector<vector<T> > &data) {
  if(static_cast<int>(data.size())!=rows || static_cast<int>(data[0].size())!=cols)
    throw Exception(getPath(), "Size mismatch in write.");
  vector<T> buf(rows*cols);
  int i=0;
  for(typename vector<vector<T> >::const_iterator ir=data.begin(); ir!=data.end(); ++ir)
    for(typename vector<T>::const_iterator ic=ir->begin(); ic!=ir->end(); ++ic, ++i)
      buf[i]=*ic;
  HDF5SERIE_H5XWRITE(&buf[0]);
}
template<>
void HDF5SERIE_CLASS<vector<vector<string> > >::write(const vector<vector<string> >& data) {
  if(static_cast<int>(data.size())!=rows || static_cast<int>(data[0].size())!=cols)
    throw Exception(getPath(), "Size mismatch in write.");
  VecStr buf(rows*cols);
  int i=0;
  for(typename vector<vector<string> >::const_iterator ir=data.begin(); ir!=data.end(); ++ir)
    for(typename vector<string>::const_iterator ic=ir->begin(); ic!=ir->end(); ++ic, ++i) {
      buf[i]=(char*)malloc((ic->size()+1)*sizeof(char));
      strcpy(buf[i], ic->c_str());
    }
  HDF5SERIE_H5XWRITE(&buf[0]);
}

template<class T>
vector<vector<T> > HDF5SERIE_CLASS<vector<vector<T> > >::read() {
  vector<T> buf(rows*cols);
  HDF5SERIE_H5XREAD(&buf[0]);
  vector<vector<T> > ret(rows);
  int r=0, c;
  for(typename vector<vector<T> >::iterator ir=ret.begin(); ir!=ret.end(); ++ir, ++r) {
    ir->resize(cols);
    c=0;
    for(typename vector<T>::iterator ic=ir->begin(); ic!=ir->end(); ++ic, ++c)
      *ic=buf[r*cols+c];
  }
  return ret;
}
template<>
vector<vector<string> > HDF5SERIE_CLASS<vector<vector<string> > >::read() {
  VecStr buf(rows*cols);
  HDF5SERIE_H5XREAD(&buf[0]);
  vector<vector<string> > ret(rows);
  int r=0, c;
  for(typename vector<vector<string> >::iterator ir=ret.begin(); ir!=ret.end(); ++ir, ++r) {
    ir->resize(cols);
    c=0;
    for(typename vector<string>::iterator ic=ir->begin(); ic!=ir->end(); ++ic, ++c)
      *ic=buf[r*cols+c];
  }
  return ret;
}

# define FOREACHKNOWNTYPE(CTYPE, H5TYPE) \
  template class HDF5SERIE_CLASS<vector<vector<CTYPE> > >;
# include "knowntypes.def"
# undef FOREACHKNOWNTYPE
