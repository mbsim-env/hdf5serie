// replace this file (boost preprocessor iteration) with c++11 code if we use c++11

// iteration for function "T* operator()(...)"
#if BOOST_PP_ITERATION_DEPTH()==1

  #if BOOST_PP_ITERATION()>0
  template<
    // call iteration for "typename P1 p1, typename P2 p2, ..."
    #define BOOST_PP_ITERATION_PARAMS_2 (4, (1, BOOST_PP_ITERATION(), "hdf5serie/interface_creatoroperator_iter.h", 1))
    #include BOOST_PP_ITERATE()
  >
  #endif
  T* operator ()(
      #if BOOST_PP_ITERATION()>0
        // call iteration for "P1 p1, P2 p2, ..."
        #define BOOST_PP_ITERATION_PARAMS_2 (4, (1, BOOST_PP_ITERATION(), "hdf5serie/interface_creatoroperator_iter.h", 2))
        #include BOOST_PP_ITERATE()
      #endif
    ) {
    std::pair<typename std::map<std::string, Child*>::iterator, bool> ret=childs.insert(std::pair<std::string, Child*>(name, NULL));
    if(!ret.second)
      throw Exception(self->getPath(), "A element of name "+name+" already exists.");
    try {
      T* r=new T(static_cast<Self*>(self), name
        #if BOOST_PP_ITERATION()>0
          ,
          // call iteration for "p1, p2, ..."
          #define BOOST_PP_ITERATION_PARAMS_2 (4, (1, BOOST_PP_ITERATION(), "hdf5serie/interface_creatoroperator_iter.h", 3))
          #include BOOST_PP_ITERATE()
        #endif
      );
      ret.first->second=r;
      return r;
    }
    catch(...) {
      childs.erase(name);
      throw;
    }
  }

// iteration for "typename P1 p1, typename P2 p2, ..."
#elif BOOST_PP_ITERATION_DEPTH()==2 && BOOST_PP_ITERATION_FLAGS()==1

   typename BOOST_PP_CAT(P, BOOST_PP_ITERATION())
   #if BOOST_PP_ITERATION()!=BOOST_PP_ITERATION_FINISH()
   ,
   #endif

// iteration for "P1 p1, P2 p2, ..."
#elif BOOST_PP_ITERATION_DEPTH()==2 && BOOST_PP_ITERATION_FLAGS()==2

   BOOST_PP_CAT(P, BOOST_PP_ITERATION()) BOOST_PP_CAT(p, BOOST_PP_ITERATION())
   #if BOOST_PP_ITERATION()!=BOOST_PP_ITERATION_FINISH()
   ,
   #endif

// iteration for "p1, p2, ..."
#elif BOOST_PP_ITERATION_DEPTH()==2 && BOOST_PP_ITERATION_FLAGS()==3

   BOOST_PP_CAT(p, BOOST_PP_ITERATION())
   #if BOOST_PP_ITERATION()!=BOOST_PP_ITERATION_FINISH()
   ,
   #endif

#endif
