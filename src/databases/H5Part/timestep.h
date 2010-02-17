// $Id: timestep.h,v 1.8 2009-05-05 17:48:27 kewu Exp $
#ifndef _FASTQUERY_TIMESTEP_H
#define _FASTQUERY_TIMESTEP_H

#include <visit-config.h>

#ifdef HAVE_LIBFASTBIT

#include "indexfile.h"  // the underline HDF5 operations
#include <column.h>     // ibis::column
#include <part.h>       // ibis::part
#include <meshQuery.h>  // ibis::meshQuery

/**
   The class H5_FQ_Timestep is a container of the variables (HDF5
   datasets).  It mirrors a FastBit data partition.
*/
class H5_FQ_Timestep : public ibis::part {
public:
    H5_FQ_Timestep(const H5_Index& h5file, const int64_t t);
    virtual ~H5_FQ_Timestep() {releaseAllQueries();};

    /// Return the internal HDF5 handler object.
    H5_Index& getH5Index() const {return h5file_;}
    int64_t getTimeValue() const {return time_;}
    /// Return an reference to an array storing the extend of the
    /// dimensions of the regular mesh that defines the data sets.
    const std::vector<uint32_t>& getMeshDims() const { return shapeSize;}
 
    /// Build indexes for all datasets in the time step.  If opt is nil, it
    /// uses the default option for bitmap indexes, which builds unbinned
    /// indexes.  The second argument in FastBit specifies the number of
    /// threads used to build the indexes, but this implementation always
    /// always uses one thread.
    virtual int buildIndexes(const char* opt=0, int =1);
    /// Build index for the named dataset in the time step.  The optional
    /// argument is the binning option.  The format for binning option is
    /// "<binning xxxx />".  If the option is not specified or the string
    /// is empty, it defaults to "<binning none />".  The details of the
    /// specification is explained in:
    /// http://sdm.lbl.gov/fastbit/doc/indexSpec.html.
    int buildIndex(const char* name, const char *binning=0);
    /// Create indexes for a list of variable names.
    int createIndex(const std::vector<const char*>& names,
                    const char *binning=0);

    /// Select records of interest by specifying range conditions on some
    /// variables.  Returns a 16-character query token.
    const char* createQuery(int nkeys, const char **keys,
                            void *ubounds, void *lbounds);
    /// All query conditions specified in a string.
    const char* createQuery(const char* cond);

    const char* createEqualitySelectionQuery(const char* varname,
                                             const std::vector<double>& ids);

    /// Destroy the query specified by the token.  Should be done to
    /// release internal resources if the query objection is no longer
    /// needed.  Once this function is called, the query token return
    /// should no longer be used because the token was part of the query
    /// object this function destroys.
    void destroyQuery(const char *token);

    /// Return the conditions of the query.  It always return a string
    /// no matter which version of createQuery was called to generate the
    /// query token.
    const char* getQueryConditions(const char* token) const;

    /// Compute the number of hits by directly reading the data values.
    /// Count the number of hits without using any possible indexes.
    int64_t sequentialScan(const char *token) const;
    /// Estimate the maximum number of hits, i.e., the number of
    /// records/cells/points satisfying the specified conditions.
    int64_t estimateNumHits(const char *token);
    /// Fully evaluate the query.  This will return the exact number of
    /// hits.
    int64_t submitQuery(const char *token);
    /// Return the number of records selected.  It will return -1 before
    /// the function submitQuery is called.
    int64_t getNumHits(const char *token) const;

    /// Retrieve the coordinates of the records selected by the query.  The
    /// variable @c coord contains a list of coordinates of each mesh point
    /// that satisfies the specified query conditions.  Let @c d be the
    /// number of dimension of the mesh, the size of @c coord is expected
    /// to be @c d * number-of-hits.  The coordinates of each mesh point is
    /// given in the same order as return by the function @c getMeshDims.
    /// This function return the actual number of point it placed in @c
    /// coord.  It returns a negative number if the @c token is not a known
    /// token.  Make sure to call @c submitQuery before trying to call this
    /// function.
    int64_t getHitLocations(const char *token,
                            std::vector<hsize_t>& coord) const;
                            

    /*
      void get1DHistogram(const char* varName,
      const char* condition,
      int64_t num_bins,
      std::vector<int64_t> &counts) const;
    */

    // ---- required by FastBit to perform candidate check ----
    /// Evaluate query by reading the values from the data file.
    virtual long evaluateRangex(const ibis::qContinuousRange& cmp,
                               const ibis::bitvector& mask,
                               ibis::bitvector& res) const;

    virtual long doScan(const ibis::qRange& cmp,
                        const ibis::bitvector& mask,
                        ibis::bitvector& hits) const;

private:
    // member variables
    H5_Index& h5file_;
    int64_t time_;
    typedef std::map< const char*, ibis::meshQuery*,
                      std::less< const char* > > queryList;
    queryList qlist; // list of querys on this set of variables

    H5_FQ_Timestep(); // no default constructor
    H5_FQ_Timestep(const H5_FQ_Timestep&); // no copy constructor
    const H5_FQ_Timestep& operator=(const H5_FQ_Timestep&); // no assignment

    /// Remove all stored selections.
    int releaseAllQueries();
}; // class H5_FQ_Timestep

/**
   The class H5_FQ_variable is a thin wrapper on ibis::column.  It maps
   each variable to a column of FastBit data partition.
*/
class H5_FQ_Variable : public ibis::column {
public:
    H5_FQ_Variable(const H5_FQ_Timestep* tbl, const char* name);
    virtual ~H5_FQ_Variable() {};

    /// Read all values into an array_t object.  The argument arr must be
    /// an array_t<Type>* with the correct Type.  Currently, the five
    /// supported types mapped to C++ elementary types as follows:
    ///
    /// - H5_Float: array_t<float>*
    /// - H5_Double: array_t<double>*
    /// - H5_Int32: array_t<int32_t>*
    /// - H5_Int64: array_t<int64_t>*
    /// - H5_Byte: array_t<char>*
    ///
    /// It returns 0 to indicate success and a negative number to indicate
    /// error.
    virtual int getValuesArray(void* arr) const;

    /// Read values with specified coordinates.
    template <typename E>
    int getPointValues(array_t<E>& arr,
                       const std::vector<hsize_t>& coords) const;
    virtual array_t<double>*  selectDoubles(const ibis::bitvector& mask) const;
    virtual array_t<float>*   selectFloats(const ibis::bitvector& mask) const;
    virtual array_t<int32_t>* selectInts(const ibis::bitvector& mask) const;
    virtual array_t<int64_t>* selectLongs(const ibis::bitvector& mask) const;
    virtual array_t<char>*    selectBytes(const ibis::bitvector& mask) const;

    /// Return the H5_Index object containing this variable.
    H5_Index& getH5Index() const {return h5file_;}

    virtual void loadIndex(const char*, int) const throw ();
    virtual long indexSize() const;
    virtual double getActualMin() const;
    virtual double getActualMax() const;

protected:
    /// Resolve a continuous range condition on a sorted column.
    virtual int searchSorted(const ibis::qContinuousRange&,
                             ibis::bitvector&) const;
    /// Resolve a discrete range condition on a sorted column.
    virtual int searchSorted(const ibis::qDiscreteRange&,
                             ibis::bitvector&) const;

private:
    // member variables
    H5_Index& h5file_;

    H5_FQ_Variable(); // no default constructor

    /// Attempt to read a bitmap index from the HDF5.  If successful,
    /// return the index read, otherwise return 0 (null pointer).
    ibis::index* readIndex() const;
}; // class H5_FQ_Variable

#endif

#endif
