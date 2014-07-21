/** \brief A scalar, vector or matrix attribute/dataset
 *
 * A HDF5 attribute/dataset of a scalar a vector or a matrix of fixed size of type CTYPE.
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
class HDF5SERIE_CLASS : public HDF5SERIE_BASECLASS {
  friend class Container<HDF5SERIE_CONTAINERBASECLASS, HDF5SERIE_PARENTCLASS>;
  private:
    hid_t memDataTypeID;
    ScopedHID memDataSpaceID;
  protected:
    /** \brief Constructor for opening or creating a attribute/dataset
     *
     * For a scalar value T=CTYPE: 
     * If \a create is true see create(), if \a create is false see open()
     *
     * For a vector value T=std::vector<CTYPE>:
     * The declaration of this function is 
     * HDF5SERIE_CLASS(const H5Object& parent, const std::string& name, int count=0)
     * If \a count is not 0 see create(), if \a create is 0 see open()
     *
     * For a matrix value T=std::vector<std::vector<CTYPE> >:
     * The declaration of this function is 
     * HDF5SERIE_CLASS(const H5Object& parent, const std::string& name, int rows=0, int columns=0)
     * If \a rows is not 0 and \a column is not 0 see create(), if \a rows and \a columns is 0 see open()
     */
    HDF5SERIE_CLASS(int dummy, HDF5SERIE_PARENTCLASS *parent_, const std::string& name_);
    HDF5SERIE_CLASS(HDF5SERIE_PARENTCLASS *parent_, const std::string& name_);

    ~HDF5SERIE_CLASS();

    void close();
    void open();

  public:
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

    #ifdef HDF5SERIE_DATASETTYPE
      void setDescription(const std::string &desc) {
        SimpleAttribute<std::string> *a=createChildAttribute<SimpleAttribute<std::string> >("Description")();
        a->write(desc);
      }
      std::string getDescription() {
        SimpleAttribute<std::string> *a=openChildAttribute<SimpleAttribute<std::string> >("Description");
        return a->read();
      }
    #endif
};

template<class T>
class HDF5SERIE_CLASS<std::vector<T> > : public HDF5SERIE_BASECLASS {
  friend class Container<HDF5SERIE_CONTAINERBASECLASS, HDF5SERIE_PARENTCLASS>;
  private:
    hid_t memDataTypeID;
    ScopedHID memDataSpaceID;
    int size;
  protected:
    HDF5SERIE_CLASS(int dummy, HDF5SERIE_PARENTCLASS *parent_, const std::string& name_);
    HDF5SERIE_CLASS(HDF5SERIE_PARENTCLASS *parent_, const std::string& name_, int size_);
    ~HDF5SERIE_CLASS();
    void close();
    void open();
  public:
    void write(const std::vector<T>& data);
    std::vector<T> read();
    #ifdef HDF5SERIE_DATASETTYPE
      void setDescription(const std::string &desc) {
        SimpleAttribute<std::string> *a=createChildAttribute<SimpleAttribute<std::string> >("Description")();
        a->write(desc);
      }
      std::string getDescription() {
        SimpleAttribute<std::string> *a=openChildAttribute<SimpleAttribute<std::string> >("Description");
        return a->read();
      }
    #endif
};

template<class T>
class HDF5SERIE_CLASS<std::vector<std::vector<T> > > : public HDF5SERIE_BASECLASS {
  friend class Container<HDF5SERIE_CONTAINERBASECLASS, HDF5SERIE_PARENTCLASS>;
  private:
    hid_t memDataTypeID;
    ScopedHID memDataSpaceID;
    int rows;
    int cols;
  protected:
    HDF5SERIE_CLASS(int dummy, HDF5SERIE_PARENTCLASS *parent_, const std::string& name_);
    HDF5SERIE_CLASS(HDF5SERIE_PARENTCLASS *parent_, const std::string& name_, int rows_, int cols_);
    ~HDF5SERIE_CLASS();
    void close();
    void open();
  public:
    void write(const std::vector<std::vector<T> >& data);
    std::vector<std::vector<T> > read();
    #ifdef HDF5SERIE_DATASETTYPE
      void setDescription(const std::string &desc) {
        SimpleAttribute<std::string> *a=createChildAttribute<SimpleAttribute<std::string> >("Description")();
        a->write(desc);
      }
      std::string getDescription() {
        SimpleAttribute<std::string> *a=openChildAttribute<SimpleAttribute<std::string> >("Description");
        return a->read();
      }
    #endif
};
