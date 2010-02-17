// $Id: fqindex.C,v 1.1 2009-05-06 19:14:36 prabhat Exp $

#include <math.h>       // std::log
#include "fqindex.h"

#ifdef HAVE_LIBFASTBIT

/// Constructor.
H5_FQ_IndexUnbinned::H5_FQ_IndexUnbinned(const H5_FQ_Variable* c)
    : ibis::relic(static_cast<ibis::column*>(0)), isNewIndex(false) {
    if (c == 0) {
        ibis::util::logMessage("H5_FQ_IndexUnbinned",
                               "incomplete initialization.  "
                               "The constructor needs "
                               "a valid H5_FQ_Variable pointer");
        return;
    }
    col = c;
    H5_Index& h5file = reinterpret_cast<const H5_FQ_Variable*>(col)->
        getH5Index();
    int64_t tval = reinterpret_cast<const H5_FQ_Timestep*>(c->partition())->
        getTimeValue();
    uint64_t tmp;
    std::string nm(c->name());
    std::string fbkey = "HDF5_FastQuery.";
    fbkey += c->name();
    fbkey += ".forceIndexRebuild";
    bool err = ibis::gParameters().isTrue(fbkey.c_str());
    if (err == false)
        err = h5file.getBitmapKeysLength(nm, tval, &tmp);
    else
        tmp = 0;
    if (err && tmp > 0) {
        readOld(h5file, tval);
    }
    else {
        buildNew();
    }
    if (ibis::gVerbose > 2) { // output a summary of the index
        ibis::util::logger lg;
        print(lg.buffer());
    }
} // H5_FQ_IndexUnbinned::H5_FQ_IndexUnbinned

/// Build a new index in-memory.
void H5_FQ_IndexUnbinned::buildNew() {
    std::string nm(col->name());
    BaseFileInterface::DataType type;
    const H5_FQ_Timestep& tbl =
        *reinterpret_cast<const H5_FQ_Timestep*>(col->partition());

    std::vector<int64_t> dims;
    reinterpret_cast<const H5_FQ_Variable*>(col)->getH5Index().
        getVariableInfo(nm, tbl.getTimeValue(), dims, &type);

    switch (type) {
    case BaseFileInterface::H5_Int32: {
        array_t<int32_t> arr;
        int ierr =
            reinterpret_cast<const H5_FQ_Variable*>(col)->getValuesArray(&arr);
        if (ierr >= 0) {
            construct(arr);
            isNewIndex = true;
        }
        else if (ibis::gVerbose >= 0) {
            col->logWarning("H5_FQ_IndexUnbinned",
                            "getValuesArray failed with error code %d", ierr);
        }
        break;}
    case BaseFileInterface::H5_Int64: {
        array_t<int64_t> arr;
        int ierr =
            reinterpret_cast<const H5_FQ_Variable*>(col)->getValuesArray(&arr);
        if (ierr >= 0) {
            construct(arr);
            isNewIndex = true;
        }
        else if (ibis::gVerbose >= 0) {
            col->logWarning("H5_FQ_IndexUnbinned",
                            "getValuesArray failed with error code %d", ierr);
        }
        break;}
    case BaseFileInterface::H5_Float: {
        array_t<float> arr;
        int ierr =
            reinterpret_cast<const H5_FQ_Variable*>(col)->getValuesArray(&arr);
        if (ierr >= 0) {
            construct(arr);
            isNewIndex = true;
        }
        else if (ibis::gVerbose >= 0) {
            col->logWarning("H5_FQ_IndexUnbinned",
                            "getValuesArray failed with error code %d", ierr);
        }
        break;}
    case BaseFileInterface::H5_Double: {
        array_t<double> arr;
        int ierr =
            reinterpret_cast<const H5_FQ_Variable*>(col)->getValuesArray(&arr);
        if (ierr >= 0) {
            construct(arr);
            isNewIndex = true;
        }
        else if (ibis::gVerbose >= 0) {
            col->logWarning("H5_FQ_IndexUnbinned",
                            "getValuesArray failed with error code %d", ierr);
        }
        break;}
    default: {
        col->logWarning("H5_FQ_IndexUnbinned", "can not generate index for "
                        "data type %d (dataset %s).  It only knows H5_Int32, "
                        "H5_Int64, H5_Float and H5_Double.",
                        static_cast<int>(type), nm.c_str());
        break;}
    }
} // H5_FQ_IndexUnbinned::buildNew

void H5_FQ_IndexUnbinned::readOld(H5_Index& h5file, int64_t tval) {
    uint64_t nobs = 0;
    std::string nm(col->name());
    bool berr = h5file.getBitmapKeysLength(nm, tval, &nobs);
    if (berr == false || nobs == 0) {
        return;
    }

    clear();
    isNewIndex = false;

    // read in the keys (store the resulting values in vals)
    vals.resize(nobs);
    std::vector<int64_t> dims;
    BaseFileInterface::DataType type;
    h5file.getVariableInfo(nm, tval, dims, &type);

    switch (type) {
    case BaseFileInterface::H5_Int32: {
        array_t<int32_t> buf(nobs);
        berr = h5file.getBitmapKeys(nm, tval, buf.begin());
        vals.resize(nobs);
        for (unsigned i = 0; i < nobs; ++ i)
            vals[i] = buf[i];
        break;}
    case BaseFileInterface::H5_Int64: {
        array_t<int64_t> buf(nobs);
        berr = h5file.getBitmapKeys(nm, tval, buf.begin());
        vals.resize(nobs);
        for (unsigned i = 0; i < nobs; ++ i)
            vals[i] = buf[i];
        break;}
    case BaseFileInterface::H5_Float: {
        array_t<float> buf(nobs);
        berr = h5file.getBitmapKeys(nm, tval, buf.begin());
        vals.resize(nobs);
        for (unsigned i = 0; i < nobs; ++ i)
            vals[i] = buf[i];
        break;}
    case BaseFileInterface::H5_Double: {
        vals.resize(nobs);
        berr = h5file.getBitmapKeys(nm, tval, vals.begin());
        break;}
    default: {
        col->logWarning("H5_FQ_IndexUnbinned::readOld",
                        "can not read index for data type %d (dataset %s).  It "
                        "only knows H5_Int32, H5_Int64, H5_Float and H5_Double.",
                        static_cast<int>(type), col->name());
        return;}
    }

    // read in the offsets
    offsets.resize(nobs+1);
    {
        uint32_t *buf = new uint32_t[nobs+1];
        berr = h5file.getBitmapOffsets(nm, tval, buf);
        for (unsigned i = 0; i <= nobs; ++ i)
            offsets[i] = buf[i];
        delete [] buf;
    }

    // activate the first bitmap so that the size information is available
    bits.resize(nobs);
    for (unsigned i = 0; i < nobs; ++ i)
        bits[i] = 0;
    //activate(0);
    if (! dims.empty()) {
        nrows = 1;
        for (unsigned i = 0; i < dims.size(); ++ i)
            nrows *= dims[i];
    }
    else {
        nrows = 0;
    }
    fname = ibis::util::strnewdup(col->name());
} // H5_FQ_IndexUnbinned::readOld

/// Write the index to an active HDF5 file.
void H5_FQ_IndexUnbinned::write(H5_Index& h5file) const {
    // Because the internally stored pointer to H5_Index is a const, the caller
    // must provide another H5_Index without the const qualifier.  Clearly,
    // this H5_Index can be same as the internally stored one.
    const unsigned nobs = bits.size();
    if (nobs == 0) {
        col->logMessage("H5_FQ_IndexUnbinned::write", "no bitmaps to write");
        return;
    }

    if (! isNewIndex) {
        H5_Index& h5file_ = reinterpret_cast<const H5_FQ_Variable*>(col)->
            getH5Index();
        if (&h5file == &h5file_) {
            if (ibis::gVerbose > 3)
                col->logMessage("H5_FQ_IndexUnbinned::write",
                                "no need to write back to the same file");
            return;
        }
        activate(); // activate all bitmaps before writing to another one
    }

    // first figure out the number of words in all serialized bitmaps.
    uint32_t off[nobs+1]; // uint64_t or uint32_t? prefer 32-bit
    off[0] = 0;
    for (unsigned i = 0; i < nobs; ++ i) {
        off[i+1] = off[i];
        const ibis::bitvector* tmp = bits[i];
        if (tmp) {
            const ibis::bitvector::word_t w = tmp->getSerialSize();
            if (w > 1) {
                off[i+1] += w;
            }
        }
    }
    int64_t tval = reinterpret_cast<const H5_FQ_Timestep*>(col->partition())->
        getTimeValue();
    std::string nm(col->name());
    bool berr = h5file.createBitmap(nm, tval, off[nobs]);
    if (! berr) {
        col->logWarning("H5_FQ_IndexUnbinned::write",
                        "unable to create bitmap dataset for writing");
        return;
    }


    // go through the bitmaps to write each one separately
    for (unsigned i = 0; i < nobs; ++ i) {
        if (off[i] < off[i+1]) {
            array_t<ibis::bitvector::word_t> buf;
            bits[i]->write(buf);
            berr = h5file.writeBitmap
                (nm, tval, off[i], off[i+1],
                 reinterpret_cast<uint32_t*>(buf.begin()));
            if (! berr) {
                col->logWarning("H5_FQ_IndexUnbinned::write",
                                "failed to write bitvector %u (%lu words)",
                                i, static_cast<unsigned long>
                                (off[i+1]-off[i]));
                // is there a way to rollback the write operations?
            }
        }
    }


    // write the bitmap offsets
    berr = h5file.setBitmapOffsets(nm, tval, off, nobs+1);

    // write the keys, need return the values to their original type
    if (col->type() == ibis::DOUBLE) {
        berr = h5file.setBitmapKeys(nm, tval,
                                    const_cast<double*>(vals.begin()), nobs);
        if (! berr) {
            col->logWarning("H5_FQ_IndexUnbinned::write",
                            "failed to write the bitmap keys array[%lu])",
                            nobs);
        }
        double range[2];
        range[0] = vals[0];
        range[1] = vals[nobs-1];
        berr = h5file.setActualRange(nm, tval, range);
        berr = h5file.setExpectedRange(nm, tval, range);
    }
 
    else if (col->type() == ibis::FLOAT) {
        array_t<float> tmp(vals.size());
        for (unsigned i = 0; i < vals.size(); ++ i)
            tmp[i] = static_cast<float>(vals[i]);
        berr = h5file.setBitmapKeys(nm, tval, tmp.begin(), nobs);
        float range[2];
        range[0] = static_cast<float>(vals[0]);
        range[1] = static_cast<float>(vals[nobs-1]);
        berr = h5file.setActualRange(nm, tval, range);
        berr = h5file.setExpectedRange(nm, tval, range);
    }
    else if (col->type() == ibis::LONG) {
        array_t<int64_t> tmp(vals.size());
        for (unsigned i = 0; i < vals.size(); ++ i)
            tmp[i] = static_cast<int64_t>(vals[i]);
        berr = h5file.setBitmapKeys(nm, tval, tmp.begin(), nobs);
        int64_t range[2];
        range[0] = static_cast<int64_t>(vals[0]);
        range[1] = static_cast<int64_t>(vals[nobs-1]);
        berr = h5file.setActualRange(nm, tval, range);
        berr = h5file.setExpectedRange(nm, tval, range);
    }
    else if (col->type() == ibis::INT) {
        array_t<int32_t> tmp(vals.size());
        for (unsigned i = 0; i < vals.size(); ++ i)
            tmp[i] = static_cast<int>(vals[i]);
        berr = h5file.setBitmapKeys(nm, tval, tmp.begin(), nobs);
        int range[2];
        range[0] = static_cast<int>(vals[0]);
        range[1] = static_cast<int>(vals[nobs-1]);
        berr = h5file.setActualRange(nm, tval, range);
        berr = h5file.setExpectedRange(nm, tval, range);
    }
    else {
        col->logWarning("H5_FQ_IndexUnbinned::write",
                        "HDF5_FastQuery does not support this data type "
                        "yet (dataset name=%s)", col->name());

    }

} // H5_FQ_IndexUnbinned::write

// activate all bitmaps at once by reading all of them into memory.
void H5_FQ_IndexUnbinned::activate() const {
    // assumes that vals, bits and offsets have been initialized properly.
    const unsigned nobs = vals.size();
    if (nobs == 0 || offsets.empty()) return;

    bool berr;
    H5_Index& h5file = reinterpret_cast<const H5_FQ_Variable*>(col)->
        getH5Index();
    std::string nm(col->name());
    const int64_t tval = reinterpret_cast<const H5_FQ_Timestep*>
        (col->partition())->getTimeValue();
    if ((bits.empty() || bits[0] == 0) && str == 0) {
        // read all bitmaps into memory and store the bytes in str
        str = new ibis::fileManager::storage(4*offsets[nobs]);
        berr = h5file.readBitmap(nm, tval, 0, offsets[nobs],
                                 (uint32_t*)(str->begin()));
        if (berr == false) {
            col->logWarning("H5_FQ_IndexUnbinned::activate",
                            "unable to read all bitmaps at once (dataset %s)",
                            col->name());
            delete str;
            str = 0;
        }
    }

    if (str) {
        for (unsigned i = 0; i < nobs; ++ i) {
            if (bits[i] == 0 && offsets[i+1] > offsets[i]) {
                array_t<ibis::bitvector::word_t>
                    buf(str, offsets[i]*sizeof(ibis::bitvector::word_t),
                        offsets[i+1]-offsets[i]);
                bits[i] = new ibis::bitvector(buf);
            }
        }
    }
    else { // need to read one bitmap at a time
        for (unsigned i = 0; i < nobs; ++ i) {
            if (bits[i] == 0 && offsets[i+1] > offsets[i]) {
                array_t<ibis::bitvector::word_t> buf(offsets[i+1]-offsets[i]);
                berr = h5file.readBitmap(nm, tval, offsets[i],
                                         offsets[i+1],
                                         (uint32_t*)(buf.begin()));
                if (berr) {
                    bits[i] = new ibis::bitvector(buf);
                }
                else {
                    col->logWarning("H5_FQ_IndexUnbinned::activate",
                                    "unable to read bitmap %u (dataset %s) at "
                                    "offset %u (size %u)",
                                    i, col->name(),
                                    static_cast<unsigned>(offsets[i]),
                                    static_cast<unsigned>
                                    (offsets[i+1]-offsets[i]));
                }
            }
        }
    }
} // H5_FQ_IndexUnbinned::activat

// activate the ith bitmap
void H5_FQ_IndexUnbinned::activate(uint32_t i) const {
    const unsigned nobs = vals.size();
    std::string nm(col->name());
    if (i >= nobs) return;
    if (bits[i] != 0) return;
    if (bits.size() != nobs) return;
    if (offsets.empty()) return;
    if (offsets[i] >= offsets[i+1]) {
        col->logMessage("H5_FQ_IndexUnbinned::activate", "offsets[%u] (%u) "
                        ">= offsets[%u] (%u)", i, offsets[i], i+1,
                        offsets[i+1]);
        return;
    }

    bool berr;
    const H5_FQ_Timestep *ts =
        reinterpret_cast<const H5_FQ_Timestep*>(col->partition());
    const int64_t tval = ts->getTimeValue();
    H5_Index& h5file = reinterpret_cast<const H5_FQ_Variable*>(col)->
        getH5Index();
    if (i == 0 && (nobs == 1U || (nobs <= 3U && offsets[1]*5/4 >= offsets[nobs])
                   || (nobs > 3 && static_cast<long>
                       (offsets[1]*log(nobs)) >= offsets[nobs]))) {
        // if the first bitmap takes up a majority of the total bytes, read
        // them all
        str = new ibis::fileManager::storage(4*offsets[nobs]);
        berr = h5file.readBitmap(nm, tval, 0, offsets[nobs],
                                 (uint32_t*)(str->begin()));
        if (berr == false) {
            col->logWarning("H5_FQ_IndexUnbinned::activate",
                            "unable to read all bitmaps at once (dataset %s)",
                            col->name());
            delete str;
            str = 0;
        }
    }

    if (str) { // already in memory
        array_t<ibis::bitvector::word_t> buf
            (str, offsets[i]*sizeof(ibis::bitvector::word_t),
             offsets[i+1]-offsets[i]);
        bits[i] = new ibis::bitvector(buf);
    }
    else {
        array_t<ibis::bitvector::word_t> buf(offsets[i+1]-offsets[i]);
        berr = h5file.readBitmap(nm, tval, offsets[i],
                                 offsets[i+1],
                                 (uint32_t*)(buf.begin()));
        if (berr) {
            bits[i] = new ibis::bitvector(buf);
        }
        else {
            col->logWarning("H5_FQ_IndexUnbinned::activate",
                            "unable to read bitmap %u (dataset %s) at "
                            "offset %u (size %u)",
                            i, col->name(), static_cast<unsigned>(offsets[i]),
                            static_cast<unsigned>(offsets[i+1]-offsets[i]));
        }
    }
} // H5_FQ_IndexUnbinned::activate

// activate the ith through jth bitmap
void H5_FQ_IndexUnbinned::activate(uint32_t i, uint32_t j) const {
    const unsigned nobs = vals.size();
    std::string nm(col->name());
    if (i >= nobs || i >= j) return; // empty range
    if (bits.size() != nobs) return;
    if (offsets.empty()) return;

    bool berr;
    const H5_FQ_Timestep *ts =
        reinterpret_cast<const H5_FQ_Timestep*>(col->partition());
    const int64_t tval = ts->getTimeValue();
    H5_Index& h5file = reinterpret_cast<const H5_FQ_Variable*>(col)->
        getH5Index();
    if (i == 0 && (nobs == 1U || (nobs <= 3U && offsets[1]*5/4 >= offsets[nobs])
                   || (nobs > 3 && static_cast<long>
                       (offsets[1]*log(nobs)) >= offsets[nobs]))) {
        // if the first bitmap takes up a majority of the total bytes, read
        // them all
        str = new ibis::fileManager::storage(4*offsets[nobs]);
        berr = h5file.readBitmap(nm, tval, 0, offsets[nobs],
                                 (uint32_t*)(str->begin()));
        if (berr == false) {
            col->logWarning("H5_FQ_IndexUnbinned::activate",
                            "unable to read all bitmaps at once (dataset %s)",
                            col->name());
            delete str;
            str = 0;
        }
    }

    if (str) { // already in memory
        while (i < j) {
            if (bits[i] == 0 && offsets[i+1] > offsets[i]) {
                array_t<ibis::bitvector::word_t> buf
                    (str, offsets[i]*sizeof(ibis::bitvector::word_t),
                     offsets[i+1]-offsets[i]);
                bits[i] = new ibis::bitvector(buf);
            }
            ++ i;
        }
    }
    else {
        while (i < j) {
            // skip to next empty bit vector
            while (i < j && bits[i] != 0)
                ++ i;
            // the last bitvector to activate. can not be larger
            // than j
            unsigned aj = (i<j ? i + 1 : j);
            while (aj < j && bits[aj] == 0)
                ++ aj;
            if (offsets[aj] > offsets[i]) {
                // read bitmaps into memory in one shot
                const unsigned start = offsets[i];
                array_t<ibis::bitvector::word_t> buf(offsets[aj]-start);
                berr = h5file.readBitmap(nm, tval,
                                         offsets[i], offsets[aj],
                                         (uint32_t*)(buf.begin()));
                if (berr) {
                    while (i < aj) {
                        if (bits[i] == 0 && offsets[i+1] > offsets[i]) {
                            array_t<ibis::bitvector::word_t>
                                tmp(buf, offsets[i]-start,
                                    offsets[i+1]-offsets[i]);
                            bits[i] = new ibis::bitvector(tmp);
                        }
                        ++ i;
                    }
                }
                else {
                    col->logWarning("H5_FQ_IndexUnbinned::activate",
                                    "unable to read bitmaps [%u, %u) "
                                    "(dataset %s) at offset %u (size %u)",
                                    i, aj, col->name(), start,
                                    static_cast<unsigned>
                                    (offsets[aj]-offsets[i]));
                }
                i = aj;
            } // if (offsets[aj] > offsets[i])
        } // while (i < j)
    }
} // H5_FQ_IndexUnbinned::activate

H5_FQ_IndexBinned::H5_FQ_IndexBinned(const H5_FQ_Variable *c,
                                     const char *binning)
    : ibis::bin(static_cast<ibis::column*>(0), static_cast<const char*>(0)),
      isNewIndex(false) {
    col = c;
    if (c == 0) { // nothing can be done if c == 0
        ibis::util::logMessage("H5_FQ_IndexBinned",
                               "incomplete initialization.  "
                               "The constructor needs "
                               "a valid H5_FQ_Variable pointer");
        return;
    }

    H5_Index& h5file = reinterpret_cast<const H5_FQ_Variable*>(col)->
        getH5Index();
    int64_t tval = reinterpret_cast<const H5_FQ_Timestep*>(c->partition())->
        getTimeValue();
    uint64_t tmp;
    std::string nm(c->name());
    std::string fbkey = "HDF5_FastQuery.";
    fbkey += c->name();
    fbkey += ".forceIndexRebuild";
    bool err = ibis::gParameters().isTrue(fbkey.c_str());
    if (err == false)
        err = h5file.getBitmapKeysLength(nm, tval, &tmp);
    else
        tmp = 0;
    if (err && tmp > 0) {
        // reading an existing index, no use for the binning option
        readOld(h5file, tval);
    }
    else {
        // building a new index, need to pass the binning option to the
        // actual worker function through calling ibis::column::indexSpec
        if (binning != 0 && *binning != 0) {
            // skip leading space
            while (isspace(*binning) && *binning != 0)
                ++ binning;
            if (*binning != 0) // replace the index specificiation
                const_cast<ibis::column*>(col)->indexSpec(binning);
        }
        buildNew();
    }
    if (ibis::gVerbose > 2) { // output a summary of the index
        ibis::util::logger lg;
        print(lg.buffer());
    }
} // constructor

void H5_FQ_IndexBinned::buildNew() {
    std::string nm(col->name());
    BaseFileInterface::DataType type;
    const H5_FQ_Timestep& tbl =
        *reinterpret_cast<const H5_FQ_Timestep*>(col->partition());

    std::vector<int64_t> dims;
    reinterpret_cast<const H5_FQ_Variable*>(col)->getH5Index().
        getVariableInfo(nm, tbl.getTimeValue(), dims, &type);

    switch (type) {
    case BaseFileInterface::H5_Int32: {
        array_t<int32_t> arr;
        int ierr =
            reinterpret_cast<const H5_FQ_Variable*>(col)->getValuesArray(&arr);
        if (ierr >= 0) {
            construct(arr);
            isNewIndex = true;
        }
        else if (ibis::gVerbose >= 0) {
            col->logWarning("H5_FQ_IndexBinned",
                            "getValuesArray failed with error code %d", ierr);
        }
        break;}
    case BaseFileInterface::H5_Int64: {
        array_t<int64_t> arr;
        int ierr = 
            reinterpret_cast<const H5_FQ_Variable*>(col)->getValuesArray(&arr);
        if (ierr >= 0) {
            construct(arr);
            isNewIndex = true;
        }
        else if (ibis::gVerbose >= 0) {
            col->logWarning("H5_FQ_IndexBinned",
                            "getValuesArray failed with error code %d", ierr);
        }
        break;}
    case BaseFileInterface::H5_Float: {
        array_t<float> arr;
        int ierr =
            reinterpret_cast<const H5_FQ_Variable*>(col)->getValuesArray(&arr);
        if (ierr >= 0) {
            construct(arr);
            isNewIndex = true;
        }
        else if (ibis::gVerbose >= 0) {
            col->logWarning("H5_FQ_IndexBinned",
                            "getValuesArray failed with error code %d", ierr);
        }
        break;}
    case BaseFileInterface::H5_Double: {
        array_t<double> arr;
        int ierr = 
            reinterpret_cast<const H5_FQ_Variable*>(col)->getValuesArray(&arr);
        if (ierr >= 0) {
            construct(arr);
            isNewIndex = true;
        }
        else if (ibis::gVerbose >= 0) {
            col->logWarning("H5_FQ_IndexBinned",
                            "getValuesArray failed with error code %d", ierr);
        }
        break;}
    default: {
        col->logWarning("H5_FQ_IndexBinned", "can not generate index for "
                        "data type %d (dataset %s).  It only knows H5_Int32, "
                        "H5_Int64, H5_Float and H5_Double.",
                        static_cast<int>(type), nm.c_str());
        break;}
    }
} // H5_FQ_IndexBinned::buildNew

void H5_FQ_IndexBinned::readOld(H5_Index& h5file, int64_t tval) {
    uint64_t nkeys = 0;
    std::string nm(col->name());
    bool berr = h5file.getBitmapKeysLength(nm, tval, &nkeys);
    if (berr == false || nkeys == 0) {
        return;
    }

    clear();
    isNewIndex = false;
    nobs = nkeys / 2;
    if (nobs + nobs != nkeys) {
        col->logWarning("readOld", "the number of keys (%lu) is expected "
                        "to be an event number, but it is not",
                        static_cast<long unsigned>(nkeys));
        nobs = 0;
        return;
    }

    bounds.resize(nobs);
    maxval.resize(nobs);
    minval.resize(nobs);
    std::vector<int64_t> dims;
    BaseFileInterface::DataType type;
    // read in the keys (store the reading result in a temporary buf)
    h5file.getVariableInfo(nm, tval, dims, &type);

    switch (type) {
    case BaseFileInterface::H5_Int32: {
        array_t<int32_t> buf(nkeys);
        berr = h5file.getBitmapKeys(nm, tval, buf.begin());
        if (! berr) {
            clear();
            return;
        }
        for (unsigned i = 0; i < nobs; ++ i) {
            maxval[i] = buf[i+nobs];
            minval[i] = buf[i];
            if (i > 0)
                bounds[i-1] = ibis::util::compactValue(maxval[i-1], buf[i]);
        }
        bounds[nobs-1] = DBL_MAX;
        break;}
    case BaseFileInterface::H5_Int64: {
        array_t<int64_t> buf(nkeys);
        berr = h5file.getBitmapKeys(nm, tval, buf.begin());
        if (! berr) {
            clear();
            return;
        }
        for (unsigned i = 0; i < nobs; ++ i) {
            maxval[i] = buf[i+nobs];
            minval[i] = buf[i];
            if (i > 0)
                bounds[i-1] = ibis::util::compactValue(maxval[i-1], buf[i]);
        }
        bounds[nobs-1] = DBL_MAX;
        break;}
    case BaseFileInterface::H5_Float: {
        array_t<float> buf(nkeys);
        berr = h5file.getBitmapKeys(nm, tval, buf.begin());
        if (! berr) {
            clear();
            return;
        }
        for (unsigned i = 0; i < nobs; ++ i) {
            maxval[i] = buf[i+nobs];
            minval[i] = buf[i];
            if (i > 0)
                bounds[i-1] = ibis::util::compactValue(maxval[i-1], buf[i]);
        }
        bounds[nobs-1] = DBL_MAX;
        break;}
    case BaseFileInterface::H5_Double: {
        array_t<double> buf(nkeys);
        berr = h5file.getBitmapKeys(nm, tval, buf.begin());
        if (! berr) {
            clear();
            return;
        }
        for (unsigned i = 0; i < nobs; ++ i) {
            maxval[i] = buf[i+nobs];
            minval[i] = buf[i];
            if (i > 0)
                bounds[i-1] = ibis::util::compactValue(maxval[i-1], buf[i]);
        }
        bounds[nobs-1] = DBL_MAX;
        break;}
    default: {
        col->logWarning("H5_FQ_IndexBinned::readOld", "can not read index for "
                        "data type %d (dataset %s).  It only knows H5_Int32, "
                        "H5_Int64, H5_Float and H5_Double.",
                        static_cast<int>(type), col->name());
        return;}
    }

    // read in the offsets
    offsets.resize(nobs+1);
    {
        uint32_t *buf = new uint32_t[nobs+1];
        berr = h5file.getBitmapOffsets(nm, tval, buf);
        for (unsigned i = 0; i <= nobs; ++ i)
            offsets[i] = buf[i];
        delete [] buf;
    }
    if (! berr) {
        col->logWarning("H5_FQ_IndexBinned::readOld",
                        "failed to read the values of bitmap offsets "
                        "(expected number of elements: %u)", nobs);
        clear();
        return;
    }

    // activate the first bitmap so that the size information is available
    bits.resize(nobs);
    for (unsigned i = 0; i < nobs; ++ i)
        bits[i] = 0;
    // activate(0);
    if (! dims.empty()) {
        nrows = 1;
        for (unsigned i = 0; i < dims.size(); ++ i)
            nrows *= dims[i];
    }
    else {
        nrows = 0;
    }
    fname = ibis::util::strnewdup(col->name());
} // H5_FQ_IndexBinned::readOld

/// Write the index to an active HDF5 file.
void H5_FQ_IndexBinned::write(H5_Index& h5file) const {
    // Because the internally stored pointer to H5_Index is a const, the caller
    // must provide another H5_Index without the const qualifier.  Clearly,
    // this H5_Index can be same as the internally stored one.
    if (nobs == 0) {
        col->logMessage("H5_FQ_IndexBinned::write", "no bitmaps to write");
        return;
    }

    if (! isNewIndex) { // an existing index
        H5_Index& h5file_ = reinterpret_cast<const H5_FQ_Variable*>(col)->
            getH5Index();
        if (&h5file == &h5file_) {
            if (ibis::gVerbose > 3)
                col->logMessage("H5_FQ_IndexBinned::write",
                                "no need to write back to the same file");
            return;
        }
        activate(); // activate all bitmaps before writing to another one
    }

    // first figure out the number of words in all serialized bitmaps.
    uint32_t off[nobs+1]; // uint64_t or uint32_t? prefer 32-bit
    off[0] = 0;
    for (unsigned i = 0; i < nobs; ++ i) {
        off[i+1] = off[i];
        const ibis::bitvector* tmp = bits[i];
        if (tmp) {
            const ibis::bitvector::word_t w = tmp->getSerialSize();
            if (w > 1) {
                off[i+1] += w;
            }
        }
    }
    int64_t tval = reinterpret_cast<const H5_FQ_Timestep*>(col->partition())->
        getTimeValue();
    std::string nm(col->name());
    bool berr = h5file.createBitmap(nm, tval, off[nobs]);
    if (! berr) {
        col->logWarning("H5_FQ_IndexBinned::write",
                        "failed to create bitmap dataset for writing");
        return;
    }

    // go through the bitmaps to write each one separately
    for (unsigned i = 0; i < nobs; ++ i) {
        if (off[i] < off[i+1]) {
            array_t<ibis::bitvector::word_t> buf;
            bits[i]->write(buf);
            berr = h5file.writeBitmap
                (nm, tval, off[i], off[i+1],
                 reinterpret_cast<uint32_t*>(buf.begin()));
            if (! berr) {
                col->logWarning("H5_FQ_IndexBinned::write",
                                "failed to write bitvector %u (%lu words)",
                                i, static_cast<unsigned long>
                                (off[i+1]-off[i]));
                // is there a way to rollback the write operations?
            }
        }
    }

    // write the bitmap offsets
    berr = h5file.setBitmapOffsets(nm, tval, off, nobs+1);
    if (! berr) {
        col->logWarning("H5_FQ_IndexBinned::write",
                        "failed to write the offset array (%lu elements)",
                        static_cast<unsigned long>(nobs+1));
    }

    // write the keys, need return the min/max values to their original type
    const long unsigned int nkeys = nobs + nobs;
    if (col->type() == ibis::DOUBLE) {
        array_t<double> buf(nkeys);
        for (unsigned i = 0; i < nobs; ++ i) {
            buf[i] = minval[i];
            buf[i+nobs] = maxval[i];
        }
        berr = h5file.setBitmapKeys(nm, tval, buf.begin(), nkeys);
        if (! berr) {
            col->logWarning("H5_FQ_IndexBinned::write",
                            "failed to write the bitmap keys array[%lu])",
                            nkeys);
        }
        buf[0] = getMin();
        buf[1] = getMax();
        berr = h5file.setActualRange(nm, tval, buf.begin());
        berr = h5file.setExpectedRange(nm, tval, buf.begin());

        if (! berr) {
            col->logWarning("H5_FQ_IndexBinned::write",
                            "failed to write the expected range)",
                            nkeys);
        }
    }
    else if (col->type() == ibis::FLOAT) {
        array_t<float> buf(nkeys);
        for (unsigned i = 0; i < nobs; ++ i) {
            buf[i] = static_cast<float>(minval[i]);
            buf[i+nobs] = static_cast<float>(maxval[i]);
        }
        berr = h5file.setBitmapKeys(nm, tval, buf.begin(), nkeys);
        if (! berr) {
            col->logWarning("H5_FQ_IndexBinned::write",
                            "failed to write the bitmap keys array[%lu])",
                            nkeys);
        }
        buf[0] = static_cast<float>(getMin());
        buf[1] = static_cast<float>(getMax());
        berr = h5file.setActualRange(nm, tval, buf.begin());
        berr = h5file.setExpectedRange(nm, tval, buf.begin());
    }
    else if (col->type() == ibis::LONG) {
        array_t<int64_t> buf(nkeys);
        for (unsigned i = 0; i < nobs; ++ i) {
            buf[i] = static_cast<int64_t>(minval[i]);
            buf[i+nobs] = static_cast<int64_t>(maxval[i]);
        }
        berr = h5file.setBitmapKeys(nm, tval, buf.begin(), nkeys);
        if (! berr) {
            col->logWarning("H5_FQ_IndexBinned::write",
                            "failed to write the bitmap keys array[%lu])",
                            nkeys);
        }
        buf[0] = static_cast<int64_t>(getMin());
        buf[1] = static_cast<int64_t>(getMax());
        berr = h5file.setActualRange(nm, tval, buf.begin());
        berr = h5file.setExpectedRange(nm, tval, buf.begin());
    }
    else if (col->type() == ibis::INT) {
        array_t<int32_t> buf(nkeys);
        for (unsigned i = 0; i < nobs; ++ i) {
            buf[i] = static_cast<int>(minval[i]);
            buf[i+nobs] = static_cast<int>(maxval[i]);
        }
        berr = h5file.setBitmapKeys(nm, tval, buf.begin(), nkeys);
        if (! berr) {
            col->logWarning("H5_FQ_IndexBinned::write",
                            "failed to write the bitmap keys array[%lu])",
                            nkeys);
        }
        buf[0] = static_cast<int>(getMin());
        buf[1] = static_cast<int>(getMax());
        berr = h5file.setActualRange(nm, tval, buf.begin());
        berr = h5file.setExpectedRange(nm, tval, buf.begin());
    }
    else {
        col->logWarning("H5_FQ_IndexBinned::write",
                        "HDF5_FastQuery does not support this data type "
                        "yet (dataset name=%s)", col->name());
    }
} // H5_FQ_IndexBinned::write

// activate all bitmaps at once by reading all of them into memory.
void H5_FQ_IndexBinned::activate() const {
    // assumes that bounds, minval, maxval, bits and offsets have been
    // initialized properly.
    if (nobs == 0 || offsets.empty()) return;
    ibis::util::timer mytimer("H5_FQ_IndexBinned::activate()", 3);

    bool berr;
    H5_Index& h5file = reinterpret_cast<const H5_FQ_Variable*>(col)->
        getH5Index();
    std::string nm(col->name());
    const int64_t tval = reinterpret_cast<const H5_FQ_Timestep*>
        (col->partition())->getTimeValue();
    if ((bits.empty() || bits[0] == 0) && str == 0) {
        // read all bitmaps into memory and store the bytes in str
        str = new ibis::fileManager::storage(4*offsets[nobs]);
        berr = h5file.readBitmap(nm, tval, 0, offsets[nobs],
                                 (uint32_t*)(str->begin()));
        if (berr == false) {
            col->logWarning("H5_FQ_IndexBinned::activate",
                            "unable to read all bitmaps at once (dataset %s)",
                            col->name());
            delete str;
            str = 0;
        }
    }

    if (str) {
        for (unsigned i = 0; i < nobs; ++ i) {
            if (bits[i] == 0 && offsets[i+1] > offsets[i]) {
                array_t<ibis::bitvector::word_t>
                    buf(str, offsets[i]*sizeof(ibis::bitvector::word_t),
                        offsets[i+1]-offsets[i]);
                bits[i] = new ibis::bitvector(buf);
            }
        }
    }
    else { // need to read one bitmap at a time
        for (unsigned i = 0; i < nobs; ++ i) {
            if (bits[i] == 0 && offsets[i+1] > offsets[i]) {
                array_t<ibis::bitvector::word_t> buf(offsets[i+1]-offsets[i]);
                berr = h5file.readBitmap(nm, tval,
                                         offsets[i],
                                         offsets[i+1],
                                         (uint32_t*)(buf.begin()));
                if (berr) {
                    bits[i] = new ibis::bitvector(buf);
                }
                else {
                    col->logWarning("H5_FQ_IndexBinned::activate",
                                    "unable to read bitmap %u (dataset %s) at "
                                    "offset %u (size %u)",
                                    i, col->name(),
                                    static_cast<unsigned>(offsets[i]),
                                    static_cast<unsigned>
                                    (offsets[i+1]-offsets[i]));
                }
            }
        }
    }
} // H5_FQ_IndexBinned::activat

// activate the ith bitmap
void H5_FQ_IndexBinned::activate(uint32_t i) const {
    std::string nm(col->name());
    if (i >= nobs) return;
    if (bits[i] != 0) return;
    if (bits.size() != nobs) return;
    if (offsets.empty()) return;
    std::string evt = "H5_FQ_IndexBinned::activate";
    if (ibis::gVerbose > 2) {
        std::ostringstream oss;
        oss << "(" << i << ")";
        evt += oss.str();
    }
    if (offsets[i] >= offsets[i+1]) {
        col->logMessage(evt.c_str(), "offsets[%u] (%u) "
                        ">= offsets[%u] (%u)", i, offsets[i], i+1,
                        offsets[i+1]);
        return;
    }
    ibis::util::timer mytimer(evt.c_str(), 3);

    bool berr;
    const H5_FQ_Timestep *ts =
        reinterpret_cast<const H5_FQ_Timestep*>(col->partition());
    const int64_t tval = ts->getTimeValue();
    H5_Index& h5file = reinterpret_cast<const H5_FQ_Variable*>(col)->
        getH5Index();
    if (i == 0U && (nobs == 1U ||
                    (nobs <= 3U && offsets[1]*5/4 >= offsets[nobs]) ||
                    (nobs > 3 && static_cast<long>
                     (offsets[1]*log(nobs)) >= offsets[nobs]))) {
        // if the first bitmap takes up a majority of the total bytes, read
        // them all
        str = new ibis::fileManager::storage(4*offsets[nobs]);
        berr = h5file.readBitmap(nm, tval, 0, offsets[nobs],
                                 (uint32_t*)(str->begin()));
        if (berr == false) {
            col->logWarning(evt.c_str(),
                            "unable to read all bitmaps at once (dataset %s)",
                            col->name());
            delete str;
            str = 0;
        }
    }

    if (str) { // already in memory
        array_t<ibis::bitvector::word_t> buf
            (str, offsets[i]*sizeof(ibis::bitvector::word_t),
             offsets[i+1]-offsets[i]);
        bits[i] = new ibis::bitvector(buf);
    }
    else { // read the specified bitmap based on the offsets array
        array_t<ibis::bitvector::word_t> buf(offsets[i+1]-offsets[i]);
        berr = h5file.readBitmap(nm, tval, offsets[i],
                                 offsets[i+1],
                                 (uint32_t*)(buf.begin()));
        if (berr) {
            bits[i] = new ibis::bitvector(buf);
        }
        else {
            col->logWarning(evt.c_str(),
                            "unable to read bitmap %u (dataset %s) at "
                            "offset %u (size %u)",
                            i, col->name(), static_cast<unsigned>(offsets[i]),
                            static_cast<unsigned>(offsets[i+1]-offsets[i]));
        }
    }
} // H5_FQ_IndexBinned::activate

// activate the ith through jth bitmap
void H5_FQ_IndexBinned::activate(uint32_t i, uint32_t j) const {
    std::string nm(col->name());
    if (i >= nobs || i >= j) return; // empty range
    if (bits.size() != nobs) return;
    if (offsets.empty()) return;
    std::string evt = "H5_FQ_IndexBinned::activate";
    if (ibis::gVerbose > 2) {
        std::ostringstream oss;
        oss << "(" << i << ", " << j << ")";
        evt += oss.str();
    }
    ibis::util::timer mytimer(evt.c_str(), 3);

    bool berr;
    const H5_FQ_Timestep *ts =
        reinterpret_cast<const H5_FQ_Timestep*>(col->partition());
    const int64_t tval = ts->getTimeValue();
    H5_Index& h5file = reinterpret_cast<const H5_FQ_Variable*>(col)->
        getH5Index();
    if (i == 0U && (nobs == 1U ||
                    (nobs <= 3U && offsets[1]*5/4 >= offsets[nobs]) ||
                    (nobs > 3 && static_cast<long>
                     (offsets[1]*log(nobs)) >= offsets[nobs]))) {
        // if the first bitmap takes up a majority of the total bytes, read
        // them all
        str = new ibis::fileManager::storage(4*offsets[nobs]);
        berr = h5file.readBitmap(nm, tval, 0, offsets[nobs],
                                 (uint32_t*)(str->begin()));
        if (berr == false) {
            col->logWarning("H5_FQ_IndexBinned::activate",
                            "unable to read all bitmaps at once (dataset %s)",
                            col->name());
            delete str;
            str = 0;
        }
    }

    if (str) { // already in memory
        while (i < j) {
            if (bits[i] == 0 && offsets[i+1] > offsets[i]) {
                array_t<ibis::bitvector::word_t> buf
                    (str, offsets[i]*sizeof(ibis::bitvector::word_t),
                     offsets[i+1]-offsets[i]);
                bits[i] = new ibis::bitvector(buf);
            }
            ++ i;
        }
    }
    else {
        while (i < j) {
            // skip to next empty bit vector
            while (i < j && bits[i] != 0)
                ++ i;
            // the last bitvector to activate. can not be larger
            // than j
            unsigned aj = (i<j ? i + 1 : j);
            while (aj < j && bits[aj] == 0)
                ++ aj;
            if (offsets[aj] > offsets[i]) {
                // read bitmaps into memory in one shot
                const unsigned start = offsets[i];
                array_t<ibis::bitvector::word_t> buf(offsets[aj]-start);
                berr = h5file.readBitmap(nm, tval,
                                         offsets[i], offsets[aj],
                                         (uint32_t*)(buf.begin()));
                if (berr) {
                    while (i < aj) {
                        if (bits[i] == 0 && offsets[i+1] > offsets[i]) {
                            array_t<ibis::bitvector::word_t>
                                tmp(buf, offsets[i]-start,
                                    offsets[i+1]-offsets[i]);
                            bits[i] = new ibis::bitvector(tmp);
                        }
                        ++ i;
                    }
                }
                else {
                    col->logWarning(evt.c_str(),
                                    "unable to read bitmaps [%u, %u) "
                                    "(dataset %s) at offset %u (size %u)",
                                    i, aj, col->name(), start,
                                    static_cast<unsigned>
                                    (offsets[aj]-offsets[i]));
                }
                i = aj;
            } // if (offsets[aj] > offsets[i])
        } // while (i < j)
    }
} // H5_FQ_IndexBinned::activate

#endif
