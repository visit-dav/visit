// $Id: fqindex.h,v 1.1 2007/09/06 16:54:14 kurts Exp $
#ifndef _FASTQUERY_INDEX_H
#define _FASTQUERY_INDEX_H
#include "indexfile.h"        // the underline HDF5 operations
#include "timestep.h"        // H5_FQ_Variable and H5_FQ_Timestep
#include <index.h>        // FastBit abstract index class
#include <irelic.h>        // FastBit unbinned index classes
#include <ibin.h>        // FastBit binned index classes

/**
   This file defines the data structure of the index classes specialized
   for the HDF5 files.  They are required for reading from and writing to
   HDF5 files.  Currently, only two specializations are implemented,
   H5_FQ_IndexUnbinned and H5_FQ_IndexBinned.  The first of which inherents
   from ibis::relic, the basic bitmap index that indexes every distinct
   value.  The second version implements the binned version of the bitmap
   index, which potentially requires less storage but may require access of
   the raw data to answer some queries.
*/
class H5_FQ_IndexUnbinned : public ibis::relic {
public:
    H5_FQ_IndexUnbinned(const H5_FQ_Variable* c);
    ~H5_FQ_IndexUnbinned() {};

    /// Returns true is this index was build from user data rather than
    /// read from a HDF5 file.
    bool newIndex() const {return isNewIndex;}

    /// Write the content of the index to 
    void write(H5_Index& h5file) const;


protected:
    virtual void activate() const;
    virtual void activate(uint32_t i) const;
    virtual void activate(uint32_t i, uint32_t j) const;

    virtual void clear() {ibis::relic::clear(); isNewIndex=false;}

private:
    bool isNewIndex;

    /// Build a new bitmap index.
    void buildNew();
    /// Read an index from the specified file.
    void readOld(H5_Index& h5file, int64_t tval);

    H5_FQ_IndexUnbinned(); // no default constructor
    H5_FQ_IndexUnbinned(const H5_FQ_IndexUnbinned&); // no copy constructor
    const H5_FQ_IndexUnbinned& operator=(const H5_FQ_IndexUnbinned&);

}; // class H5_FQ_IndexUnbinned


/**
   Bitmap indices with bins.
*/
class H5_FQ_IndexBinned : public ibis::bin {
public:
    H5_FQ_IndexBinned(const H5_FQ_Variable* c, const char *binning);
    ~H5_FQ_IndexBinned() {};

    /// Returns true is this index was build from user data rather than
    /// read from a HDF5 file.
    bool newIndex() const {return isNewIndex;}

    /// Write the content of the index to 
    void write(H5_Index& h5file) const;


protected:
    virtual void activate() const;
    virtual void activate(uint32_t i) const;
    virtual void activate(uint32_t i, uint32_t j) const;
    virtual void clear() {ibis::bin::clear(); isNewIndex=false;}

private:
    bool isNewIndex;

    /// Build a new bitmap index.
    void buildNew();
    /// Read an index from the specified file.
    void readOld(H5_Index& h5file, int64_t tval);

    H5_FQ_IndexBinned(); // no default constructor
    H5_FQ_IndexBinned(const H5_FQ_IndexBinned&); // no copy constructor
    const H5_FQ_IndexBinned& operator=(const H5_FQ_IndexBinned&);

}; // class H5_FQ_IndexBinned
#endif
