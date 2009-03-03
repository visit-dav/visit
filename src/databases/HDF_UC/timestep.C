// $Id: timestep.cpp,v 1.3 2007/09/07 18:36:34 lgosink Exp $

#include "timestep.h"        // H5_FQ_Timestep and H5_FQ_Variable
#include "fqindex.h"        // H5_FQ_IndexUnbinned
#include <base_api.h>
#include <vector>            // std::vector
#include <algorithm>
#include <sstream>      // std::ostringstream

/// Constructor
H5_FQ_Variable::H5_FQ_Variable(const H5_FQ_Timestep *tbl, const char *nm)
        : ibis::column(tbl, ibis::OID, nm), h5file_(tbl->getH5Index()) {

    BaseFileInterface::DataType type;
    std::vector<int64_t> dims;
    std::string strnm(nm);

    int64_t timeValue = tbl->getTimeValue();
    h5file_.getVariableInfo(strnm, timeValue, dims, &type);
    
    switch (type) {
        case BaseFileInterface::H5_Int32:
            m_type = ibis::INT; break;
        case BaseFileInterface::H5_Float:
            m_type = ibis::FLOAT; break;
        case BaseFileInterface::H5_Double:
            m_type = ibis::DOUBLE; break;
        case BaseFileInterface::H5_Byte:
            m_type = ibis::BYTE; break;
        case BaseFileInterface::H5_Int64:
            m_type = ibis::LONG; break;
        default:
            ibis::util::logMessage("H5_FQ_Variable", "unknown data type (HDF5) %d",
                                   static_cast<int>(type));
    }
} // H5_FQ_Variable::H5_FQ_Variable

// NOTE: the array is resize to have enough space to store the bytes.  No
// type checking is performed.  This function is correct only if the
// correct type of array is given.
template <typename E>
int H5_FQ_Variable::getValues(array_t<E>& arr) const {
    
    std::vector<int64_t> dims;
    const H5_FQ_Timestep& tbl =
        *reinterpret_cast<const H5_FQ_Timestep*>(partition());
    int64_t tval = tbl.getTimeValue();
    std::string strnm(name());

    BaseFileInterface::DataType type;
    h5file_.getVariableInfo(strnm, tval, dims, &type);
    int64_t size = h5file_.getDatasetSize(dims, type);

    arr.resize(size/sizeof(E));
    h5file_.getData(strnm, tval, arr.begin());

    return 0;
} // H5_FQ_Variable::getValues

// explicit instantiation
template int H5_FQ_Variable::getValues(array_t<int>&) const;
template int H5_FQ_Variable::getValues(array_t<unsigned int>&) const;
//template int H5_FQ_Variable::getValues(array_t<long>&) const;
template int H5_FQ_Variable::getValues(array_t<float>&) const;
template int H5_FQ_Variable::getValues(array_t<double>&) const;

template <typename E>
int H5_FQ_Variable::getPointValues(array_t<E>& arr,
                                   const std::vector<int32_t>& coords) const {
                                   
    std::vector<int64_t> dims;
    const H5_FQ_Timestep& tbl =
        *reinterpret_cast<const H5_FQ_Timestep*>(partition());
    int64_t tval = tbl.getTimeValue();
    std::string strnm(name());

    BaseFileInterface::DataType type;
    h5file_.getVariableInfo(strnm, tval, dims, &type);

    arr.resize(coords.size() / dims.size());
    bool ret = h5file_.getPointData
        (strnm, tval, static_cast<void*>(arr.begin()),
         reinterpret_cast<const std::vector<int32_t>&>(coords));
    if (ret)
        return arr.size();
    else
        return -1;
} // H5_FQ_Variable::getPointValues


array_t<double>*   H5_FQ_Variable::selectDoubles(const ibis::bitvector& mask) const {
  array_t<double>* array = new array_t<double>;
  array_t<double> prop;
  uint32_t i = 0;
  uint32_t tot = mask.cnt();
  ibis::horometer timer;
  if (ibis::gVerbose > 3) {
      LOGGER(ibis::gVerbose > 4)
          << "H5_FQ_Variable[" << (thePart->name() ? thePart->name() : "?")
          << "." << name() << "]::selectDoubles starting timer..";
      timer.start();
  }
#ifdef DEBUG
  LOGGER(1) << "DEBUG -- reading " << name() << ", mask.cnt() = " << tot
            << ", mask.bytes() = " << mask.bytes()
            << ", mask.size()*8/pagesize = "
            << mask.size()*8/ibis::fileManager::pageSize()
            << ", read all = "
            << (mask.bytes()/4 > mask.size()/ibis::fileManager::pageSize() ?
                "yes" : "no");
#endif
  if (mask.size() < 1048576 || tot+tot > mask.size() ||
      mask.bytes()/4 > mask.size()/ibis::fileManager::pageSize()) {
      // read all values than extract the ones marked with 1 in mask
      getValues(prop); // retrieving all values of this variable
      array->resize(tot);
      if (tot > prop.size()) tot = prop.size();
      const uint32_t nprop = prop.size();
      ibis::bitvector::indexSet index = mask.firstIndexSet();
      if (nprop >= mask.size()) {
          while (index.nIndices() > 0) {
              const ibis::bitvector::word_t *idx0 = index.indices();
              if (index.isRange()) {
                  for (uint32_t j = *idx0; j<idx0[1]; ++j, ++i) {
                      (*array)[i] = (prop[j]);
                  }
              }
              else {
                  for (uint32_t j = 0; j<index.nIndices(); ++j, ++i) {
                      (*array)[i] = (prop[idx0[j]]);
                  }
              }
              ++ index;
          }
      }
      else {
          while (index.nIndices() > 0) {
              const ibis::bitvector::word_t *idx0 = index.indices();
              if (*idx0 >= nprop) break;
              if (index.isRange()) {
                  for (uint32_t j = *idx0;
                       j<(idx0[1]<=nprop ? idx0[1] : nprop);
                       ++j, ++i) {
                      (*array)[i] = (prop[j]);
                  }
              }
              else {
                  for (uint32_t j = 0; j<index.nIndices(); ++j, ++i) {
                      if (idx0[j] < nprop)
                          (*array)[i] = (prop[idx0[j]]);
                      else
                          break;
                  }
              }
              ++ index;
          }
      }
  }
  else {
      // generate the coordinates and ask HDF5 function to extract them
      std::vector<int32_t> coord;
      coord.reserve(tot);
      for (ibis::bitvector::indexSet ix = mask.firstIndexSet();
           ix.nIndices() > 0; ++ ix) {
          const ibis::bitvector::word_t *ind = ix.indices();
          if (ix.isRange()) {
              for (unsigned int j = ind[0]; j < ind[1]; ++ j)
                coord.push_back(static_cast<int32_t>(j));
          }
          else {
              for (unsigned int j = 0; j < ix.nIndices(); ++j)
                coord.push_back(static_cast<int32_t>(ind[j]));
          }
      }
      i = getPointValues(*array, coord);
  }
  if (i != tot) {
    array->resize(i);
    logWarning("selectDoubles", "expects to retrieve %lu elements "
               "but only got %lu", static_cast<long unsigned>(tot),
               static_cast<long unsigned>(i));
  }
  else if (ibis::gVerbose > 3) {
      timer.stop();
      LOGGER(ibis::gVerbose >= 0)
          << "H5_FQ_Variable[" << (thePart->name() ? thePart->name() : "?")
          << "." << name() << "]::selectDoubles extracted " << tot << " value"
          << (tot > 1 ? "s" : "") << " out of " << mask.size() << " took "
          << timer.CPUTime() << " sec (CPU) and " << timer.realTime()
          << " sec (elapsed) time";
  }
  return array;        
} // H5_FQ_Variable::selectDoubles


void H5_FQ_Variable::loadIndex(const char*) const throw () {
    writeLock lock(this, "loadIndex");
    if (idx == 0 && thePart->nRows() > 0) {
        try { // if an index is not available, create one
            if (ibis::gVerbose > 7)
                logMessage("loadIndex", "loading an index from");
            if (idx == 0) {
#ifdef H5_FQ_ALWAYS_INDEX
                // either read the old index or create one in memory
                idx = new H5_FQ_IndexUnbinned(this);
#else
                // only read index already stored in the associated HDF5 file
                idx = readIndex();
#endif
            }
            if (idx && ibis::gVerbose > 10) {
                ibis::util::ioLock lck;
                //idx->print(std::cout);
            }
        }
        catch (const char *s) {
            logWarning("loadIndex", "ibis::index::ceate(%s) throw "
                       "the following exception\n%s", name(), s);
            idx = 0;
        }
        catch (const std::exception& e) {
            logWarning("loadIndex", "ibis::index::create(%s) failed "
                       "to create a new index -- %s", name(), e.what());
            idx = 0;
        }
        catch (...) {
            logWarning("loadIndex", "ibis::index::create(%s) failed "
                       "to create a new index -- unknown error", name());
            idx = 0;
        }
    }
} // H5_FQ_Variable::loadIndex



long H5_FQ_Variable::indexSize() const{

  const H5_FQ_Timestep *tbl = reinterpret_cast<const H5_FQ_Timestep*>(partition());

  int64_t timeval = tbl->getTimeValue();
  std::string strname(name());
  uint64_t size = 0;
  
  bool berr = h5file_.getBitmapSize(strname, (unsigned int64_t)(timeval), &size);
  
  if (ibis::gVerbose > 3) {
    if (!berr || size<=0) 
      logWarning("indexSize", "failed to determine the bitmap length "
                 "for variable %s in file %s", name(),
                 h5file_.getFileName().c_str());
    
    else 
      logMessage("indexSize", "found bitmap length "
                 "for variable %s in file %s to be %lu", 
                 name(), h5file_.getFileName().c_str(), size);
  }
  return size;
}


/// The called is responsible for deallocating the index returned.
ibis::index* H5_FQ_Variable::readIndex() const {
    const H5_FQ_Timestep *tbl =
        reinterpret_cast<const H5_FQ_Timestep*>(partition());
    int64_t timeval = tbl->getTimeValue();
    std::string strname(name());
    uint64_t nkeys, noffsets;
    bool berr = h5file_.getBitmapKeysLength(strname, (unsigned int64_t)(timeval), &nkeys);
    ibis::index *ret = 0;
    if (!berr || nkeys == 0) {
        if (ibis::gVerbose > 3)
            logWarning("readIndex", "failed to find the bitmap keys length "
                       "for variable %s in file %s", name(),
                       h5file_.getFileName().c_str());
        return ret;
    }

    berr = h5file_.getBitmapOffsetsLength(strname, timeval, &noffsets);
    if (!berr || nkeys == 0) {
        if (ibis::gVerbose > 3)
            logWarning("readIndex", "failed to find the length of the offset "
                       "array for variable %s in file %s", name(),
                       h5file_.getFileName().c_str());
        return ret;
    }

    if (nkeys+1 == noffsets) {
        ret = new H5_FQ_IndexUnbinned(this);
    }
    else if (nkeys == (noffsets-1)*2) {
        ret = new H5_FQ_IndexBinned(this, 0);
    }
    else {
        ibis::util::ioLock lock;
        //std::cerr << "Error -- H5_FQ_Variable::readIndex found nkeys = "
        //          << nkeys << " and noffsets = " << noffsets
        //          << " for variable " << name() << " in "
        //          << h5file_.getFileName() << " do not satisfy "
        //    "nkeys+1==noffsets or nkeys==2*(offsets-1)"
        //          << std::endl;
    }
    return ret;
} // H5_FQ_Variable::readIndex


/// Constructor
H5_FQ_Timestep::H5_FQ_Timestep(const H5_Index& h5file, const int64_t t) :
        ibis::part(static_cast<const char*>(0), static_cast<const char*>(0)),
        h5file_(const_cast<H5_Index&>(h5file)), time_(t) {
    std::vector<std::string> names;

    h5file_.getVariableNames(names);

    for (unsigned i = 0; i < names.size(); ++ i) {
        H5_FQ_Variable *var = new H5_FQ_Variable(this, names[i].c_str());
        columns[var->name()] = var;
        if (i == 0) { // record the dimension of the array and the total size
            std::vector<int64_t> dims;

            BaseFileInterface::DataType type;
            h5file_.getVariableInfo(names[i], time_, dims, &type);

            nEvents = 1;
            shapeSize.resize(dims.size());
            for (unsigned j = 0; j < dims.size(); ++ j) {
                shapeSize[j] = static_cast<unsigned>(dims[j]);
                nEvents *= dims[j];
            }
        }
    }

    if (nEvents > 0 && ! columns.empty()) {// define a name for the table
        std::ostringstream ostr;
        ostr << h5file_.getFileName();
        ostr << '.' << time_;
        m_name = ibis::util::strnewdup(ostr.str().c_str());
        ostr << " -- time step " << time_ << " in file "
             << h5file_.getFileName();
        m_desc = ibis::util::strnewdup(ostr.str().c_str());
        amask.set(1, nEvents); // every record is valid

        // TODO: need to allocate a cleaner object, actually define a new
        // cleaner class!
    }
} // H5_FQ_Timestep::H5_FQ_Timestep


int H5_FQ_Timestep::createIndex(const std::vector<const char *>& names,
                                const char *binning) {
    if (names.empty())
        return buildIndexes();

    bool binned = false;
    if (binning != 0 && *binning != 0) {
        binned = (strncmp(binning, "<binning null", 13) != 0 &&
                  strncmp(binning, "<binning none", 13) != 0 &&
                  strncmp(binning, "<binning no ", 12) != 0 &&
                  strncmp(binning, "unbinned", 8) != 0);
    }

    ibis::horometer timer;
    timer.start();
    int cnt = 0;
    for (unsigned i = 0; i < names.size(); ++ i) {
        const char* name = names[i];

        ibis::part::columnList::const_iterator it = columns.find(name);
        if (it == columns.end()) {
            logWarning("createIndex", "unable to locate a column named %s",
                       name);
            continue;
        }

        bool isnew = false;
        if (binned) {
            H5_FQ_IndexBinned
                tmp(reinterpret_cast<const H5_FQ_Variable*>((*it).second),
                    binning);
            isnew = tmp.newIndex();
            if (isnew) {
                ++ cnt;
                tmp.write(h5file_);
                if (ibis::gVerbose > 5) {
                    // print some information about the index
                    ibis::util::ioLock lock;
                    //tmp.print(std::cout);
                }
            }
        }
        else {
            H5_FQ_IndexUnbinned
                tmp(reinterpret_cast<const H5_FQ_Variable*>((*it).second));
            isnew = tmp.newIndex();
            if (isnew) {
                ++ cnt;
                tmp.write(h5file_);
                if (ibis::gVerbose > 5) {
                    // print some information about the index
                    ibis::util::ioLock lock;
                    //tmp.print(std::cout);
                }
            }
        }
    }
    if (ibis::gVerbose > 1) {
        timer.stop();
        logMessage("createIndex", "creating index for %d variable%s took "
                   "%G sec CPU time and %G sec elapsed time",
                   cnt, (cnt>1?"s":""),
                   timer.CPUTime(), timer.realTime());
    }
    return cnt;
} // H5_FQ_Timestep::createIndex

int H5_FQ_Timestep::buildIndexes() {
    ibis::horometer timer;
    timer.start();
    int cnt = 0;
    for (ibis::part::columnList::const_iterator it = columns.begin();
         it != columns.end(); ++ it) {
        ibis::horometer tm;
        tm.start();
        // call the constructor to either read an index or build a new one
        H5_FQ_IndexUnbinned
            tmp(reinterpret_cast<const H5_FQ_Variable*>((*it).second));
        if (tmp.newIndex()) { // has not be written to this file, write it
            ++ cnt;
            tmp.write(h5file_);
            if (ibis::gVerbose > 1) {
                tm.stop();
                logMessage("buildIndexes", "building index for variable %s "
                           "took %G sec CPU time and %G sec elapsed time",
                           (*it).first, tm.CPUTime(), tm.realTime());
            }
        }
        else if (ibis::gVerbose > 1) {
            tm.stop();
            logMessage("buildIndexes", "reading index for variable %s "
                       "took %G sec CPU time and %G sec elapsed time",
                       (*it).first, tm.realTime(), tm.CPUTime());
        }
    }
    if (ibis::gVerbose > 0) {
        timer.stop();
        unsigned nrd = columns.size() - cnt;
        logMessage("buildIndexes", "building %d index%s and reading %u "
                   "index%s used %G sec CPU time and %G sec elapsed time",
                   cnt, (cnt>1?"es":""), nrd, (nrd>1?"es":""),
                   timer.CPUTime(), timer.realTime());
    }
    return cnt;
} // H5_FQ_Timestep::buildIndexes

int H5_FQ_Timestep::buildIndex(const char *name, const char *binning) {
    ibis::part::columnList::const_iterator it = columns.find(name);
    if (it == columns.end()) {
        logWarning("buildIndex", "unable to locate a column named %s",
                   name);
        return 0;
    }

    bool binned = false;
    if (binning != 0 && *binning != 0) {
        binned = (strncmp(binning, "<binning null", 13) != 0 &&
                  strncmp(binning, "<binning none", 13) != 0 &&
                  strncmp(binning, "<binning no ", 12) != 0 &&
                  strncmp(binning, "unbinned", 8) != 0);
    }

    bool isnew = false;
    ibis::horometer timer;
    timer.start();
    if (binned) {
        H5_FQ_IndexBinned
            tmp(reinterpret_cast<const H5_FQ_Variable*>((*it).second),
                binning);
        isnew = tmp.newIndex();
        if (isnew) {
            tmp.write(h5file_);
            if (ibis::gVerbose > 5) { // print some information about the index
                ibis::util::ioLock lock;
                //tmp.print(std::cout);
            }
        }
    }
    else {
        H5_FQ_IndexUnbinned
            tmp(reinterpret_cast<const H5_FQ_Variable*>((*it).second));
        isnew = tmp.newIndex();
        if (isnew) {
            tmp.write(h5file_);
            if (ibis::gVerbose > 5) { // print some information about the index
                ibis::util::ioLock lock;
                //tmp.print(std::cout);
            }
        }
    }

    if (ibis::gVerbose > 1) {
        timer.stop();
        logMessage("buildIndex", "%s index for %s took %G sec CPU time "
                   "and %G sec elapsed time", (isnew?"building":"reading"),
                   name, timer.CPUTime(), timer.realTime());
    }
    return (isnew?1:0);
} // H5_FQ_Timestep::buildIndex

int H5_FQ_Timestep::releaseAllQueries() {
    // need to an exclusive lock to avoid interfering with destroyQuery
    mutexLock mtx(this, "releaseAllQueries");
    int ret = 0;
    { // A read lock must be used to delete the queries.  The queries might
      // hold read locks on this table.
        readLock lock(this, "releaseAllQueries");
        ret = static_cast<int>(qlist.size());
        queryList::iterator it;
        for (it = qlist.begin(); it != qlist.end(); ++ it) {
            delete (*it).second;
        }
    }
    { // A write lock is needed only for manipulation of qlist.
        writeLock lock(this, "releaseAllQueries");
        qlist.clear();
    }
    if (ibis::gVerbose > 3)
        logMessage("releaseAllQueries", "removed %d quer%s", ret,
                   (ret>1?"ies":"y"));
    return ret;
} // H5_FQ_Timestep::releaseAllQueries

const char *H5_FQ_Timestep::createQuery(int nkeys, const char **keys,
                                        void *ubounds, void *lbounds) {
    const char *token = 0;
    if (nkeys <= 0) return token;

    unsigned offset = 0;
    std::ostringstream ostr;
    for (int i = 0; i < nkeys; ++ i) {
        columnList::const_iterator it = columns.find(keys[i]);
        if (it == columns.end()) {
            logWarning("createQuery", "\"%s\" is not a know dataset "
                       "name.  Skip corresponding range condition",
                       keys[i]);
            continue;
        }

        if (! ostr.str().empty())
            ostr << " AND ";

        void *utmp = reinterpret_cast<char*>(lbounds)+offset;
        void *ltmp = reinterpret_cast<char*>(ubounds)+offset;
        unsigned elmsize = 0;
        switch ((*it).second->type()) {
            case ibis::DOUBLE: {
                double ud, ld; // need extra memcpy because of potential
                // alignment problem
                elmsize = sizeof(double);
                memcpy(&ld, ltmp, elmsize);
                memcpy(&ud, utmp, elmsize);
                ostr << ld << "<=" << keys[i] << "<=" << ud;
                break;}
            case ibis::LONG: {
                int64_t ud, ld; // need extra memcpy because of potential
                // alignment problem
                elmsize = sizeof(int64_t);
                memcpy(&ld, ltmp, elmsize);
                memcpy(&ud, utmp, elmsize);
                ostr << ld << "<=" << keys[i] << "<=" << ud;
                break;}
            case ibis::FLOAT:
                ostr << *(reinterpret_cast<float*>(ltmp))
                     << "<=" << keys[i] << "<="
                     << *(reinterpret_cast<float*>(utmp));
                elmsize = sizeof(float);
                break;
            case ibis::INT:
                ostr << *(reinterpret_cast<int*>(ltmp))
                     << "<=" << keys[i] << "<="
                     << *(reinterpret_cast<int*>(utmp));
                elmsize = sizeof(int);
                break;
            default:
                logWarning("createQuery", "do NOT yet this FastBit column "
                           "type (%u) yet",
                           static_cast<unsigned>((*it).second->type()));
                break;
        }
        offset += elmsize;
    }

    if (ostr.str().empty()) {
        logWarning("createQuery", "no valid range condition specified");
        return token;
    }

    ibis::meshQuery *sel = new ibis::meshQuery(ibis::util::userName(), this);
    sel->setWhereClause(ostr.str().c_str());
    //sel->evaluate(); // evaluate the query to determine hits
    token = sel->id();
    { // need a write lock to modify the query list
        writeLock lock(this, "createQuery");
        qlist[token] = sel;
    }
    return token;
} // H5_FQ_Timestep::createQuery

const char *H5_FQ_Timestep::createQuery(const char *cond) {
    const char *token = 0;
    if (cond == 0) return token;

    ibis::meshQuery *sel = new ibis::meshQuery(ibis::util::userName(), this);
    sel->setWhereClause(cond);
    //sel->evaluate(); // evaluate the query to determine hits
    token = sel->id();
    { // need a write lock to modify the query list
        writeLock lock(this, "createQuery");
        qlist[token] = sel;
    }
    return token;
} // H5_FQ_Timestep::createQuery


const char *H5_FQ_Timestep::createEqualitySelectionQuery(const char *varname,
                                                         const std::vector<double>& ids) {
  const char *token = 0;
  if (varname == 0) return token;
  
  ibis::meshQuery *sel = new ibis::meshQuery(ibis::util::userName(), this);
  
  // FastBit expects the list of doubles to be sorted
  std::vector<double> sorted_ids;
  for (unsigned int i=0; i<ids.size(); i++)
    sorted_ids.push_back(ids[i]);
  sort(sorted_ids.begin(), sorted_ids.end());
  
  ibis::qDiscreteRange* dRange = new ibis::qDiscreteRange(varname, sorted_ids);
  
  sorted_ids.clear();

  sel->setWhereClause(dRange);
  
  token = sel->id();
  {
    writeLock lock(this, "createQuery");
    qlist[token] = sel;
  }
  return token;
} 


void H5_FQ_Timestep::destroyQuery(const char *token) {
    // need an exclusive lock to prevent variable it being modified between
    // the read and write lock.
    mutexLock mtx(this, "destroyQuery");
    queryList::iterator it = qlist.end();
    { // read lock for operations on the query, can not use a write lock
      // because the query object might hold a read lock on the table.
        readLock lock(this, "destroyQuery");
        it = qlist.find(token);
        if (it != qlist.end())
            delete (*it).second;
    }
    { // write lock for the operation on qlist
        writeLock lock(this, "destroyQuery");
        qlist.erase(it);
    }
} // H5_FQ_Timestep::destroyQuery

const char *H5_FQ_Timestep::getQueryConditions(const char *token) const {
    const char *cnd = 0;
    readLock lock(this, "getQueryConditions");
    queryList::const_iterator it = qlist.find(token);
    if (it != qlist.end())
        cnd = (*it).second->getWhereClause();
    return cnd;
} // H5_FQ_Timestep::getQueryConditions

int64_t H5_FQ_Timestep::getNumHits(const char *token) const {

    int64_t nhits = -1;
    readLock lock(this, "getNumHits");
    queryList::const_iterator it = qlist.find(token);
    if (it != qlist.end() &&
        ibis::query::FULL_EVALUATE == (*it).second->getState())
        nhits = (*it).second->getNumHits();
    return nhits;
} // H5_FQ_Timestep::getNumHits

int64_t H5_FQ_Timestep::sequentialScan(const char *token) const {
    int64_t nhits = -1;
    readLock lock(this, "estimateNumHits");
    queryList::const_iterator it = qlist.find(token);
    if (it != qlist.end()) {
      /*
        ibis::bitvector *hits = (*it).second->sequentialScan();
        if (hits != 0) {
        nhits = hits->cnt();
        delete hits;
        }
        else {
        nhits = 0;
        }
      */
      
      ibis::bitvector hits;
      nhits = (*it).second->sequentialScan(hits);
     
      if (nhits<0)
        nhits = 0;
      
    }
    return nhits;
} // H5_FQ_Timestep::sequentialScan

int64_t H5_FQ_Timestep::estimateNumHits(const char *token) {
    int64_t nest = -1;
    readLock lock(this, "estimateNumHits");
    queryList::iterator it = qlist.find(token);
    if (it != qlist.end()) {
        ibis::query::QUERY_STATE qst = (*it).second->getState();
        switch (qst) {
            default:
                logWarning("estimateNumHits", "query not full specified");
                break;
            case ibis::query::SET_PREDICATE:
            case ibis::query::SPECIFIED:
            case ibis::query::SET_RIDS:
                (*it).second->estimate();
            case ibis::query::QUICK_ESTIMATE:
            case ibis::query::FULL_EVALUATE:
                nest = (*it).second->getMaxNumHits();
                break;
        }
    }
    return nest;
} // H5_FQ_Timestep::estimateNumHits

int64_t H5_FQ_Timestep::submitQuery(const char *token) {
    int64_t nhits = -1;
    readLock lock(this, "submitQuery");
    queryList::const_iterator it = qlist.find(token);
    if (it != qlist.end()) {
        ibis::query::QUERY_STATE qst = (*it).second->getState();
        switch (qst) {
            default:
                logWarning("submitQuery", "query not fully specified");
                break;
            case ibis::query::QUICK_ESTIMATE:
            case ibis::query::SET_PREDICATE:
            case ibis::query::SPECIFIED:
            case ibis::query::SET_RIDS:
                (*it).second->evaluate();
            case ibis::query::FULL_EVALUATE:
                nhits = (*it).second->getNumHits();
                break;
        }
    }
    return nhits;
} // H5_FQ_Timestep::submitQuery

#if defined(BOUNDING_BOXES_TO_LOCATIONS)
int64_t H5_FQ_Timestep::getHitLocations(const char *token,
                                        std::vector<int32_t>& coord) const {
    coord.clear();
    int64_t nhits = -1;
    readLock lock(this, "getHitLocations");
    queryList::const_iterator it = qlist.find(token);
    if (it != qlist.end()) {
        std::vector< std::vector<unsigned> > boxes;
        nhits = (*it).second->getHitsAsRanges(boxes);
        if (boxes.size() <= 0 || nhits < 0) {
            if (nhits > 0)
                nhits = 0;
            return nhits;
        }

        // convert to coordinates of points
        const unsigned ndim = getMeshDims().size();
        coord.reserve(ndim * nhits); // make sure there is enough space
        switch (ndim) {
            default: {
                for (nhits = 0; static_cast<size_t>(nhits) < boxes.size();
                     ++ nhits) {
                    const std::vector<unsigned>& box = boxes[nhits];
                    std::vector<int32_t> pnt(ndim);
                    bool more = true;
                    for (unsigned i = 0; i < ndim && more; ++ i) {
                        pnt[i] = box[i+i];
                        more = (box[i+i] < box[i+i+1]);
                    }
                    while (more) {
                        for (unsigned i = 0; i < ndim; ++ i)
                            coord.push_back(pnt[i]);
                        ++ pnt.back();
                        if (pnt.back() >= box.back()) {
                            int i = ndim - 1;
                            pnt[i] = box[i+i]; // reset to left-most position
                            for (-- i; i >= 0 && more; -- i) {
                                ++ pnt[i];
                                if (pnt[i] < box[i+i+1]) {
                                    more = false;
                                }
                                else {
                                    pnt[i] = box[i+i];
                                }
                            }
                            more = ! more;
                        }
                    }
                } // for (nhits = 0;...
                break;}
            case 3: {
                for (nhits = 0; static_cast<size_t>(nhits) < boxes.size();
                     ++ nhits) {
                    const std::vector<unsigned>& box = boxes[nhits];
                    int32_t pnt[3];
                    bool more = true;
                    pnt[0] = box[0];
                    pnt[1] = box[2];
                    pnt[2] = box[4];
                    more = (box[0] < box[1] && box[2] < box[3] && box[4] < box[5]);
                    while (more) {
                        coord.push_back(pnt[0]);
                        coord.push_back(pnt[1]);
                        coord.push_back(pnt[2]);
                        ++ pnt[2];
                        if (pnt[2] >= box[5]) {
                            pnt[2] = box[4]; // reset to left-most position
                            ++ pnt[1];
                            if (pnt[1] >= box[3]) {
                                pnt[1] = box[2];
                                ++ pnt[0];
                                more = (pnt[0] < box[1]);
                            }
                        }
                    }
                } // for (nhits = 0;...
                break;}
            case 2: {
                for (nhits = 0; static_cast<size_t>(nhits) < boxes.size();
                     ++ nhits) {
                    const std::vector<unsigned>& box = boxes[nhits];
                    int32_t pnt[2];
                    bool more = true;
                    pnt[0] = box[0];
                    pnt[1] = box[2];
                    more = (box[0] < box[1] && box[2] < box[3]);
                    while (more) {
                        coord.push_back(pnt[0]);
                        coord.push_back(pnt[1]);
                        ++ pnt[1];
                        if (pnt[1] >= box[3]) {
                            pnt[1] = box[2];
                            ++ pnt[0];
                            more = (pnt[0] < box[1]);
                        }
                    }
                } // for (nhits = 0;...
                break;}
            case 1: {
                for (nhits = 0; static_cast<size_t>(nhits) < boxes.size();
                     ++ nhits) {
                    const std::vector<unsigned>& box = boxes[nhits];
                    for (uint32_t i = box[0]; i < box[1]; ++ i)
                        coord.push_back(i);
                } // for (nhits = 0;...
                break;}
            case 0: {
                logWarning("getHitLocations", "a mesh can not have 0-dimension");
                break;}
        }
    }
    return nhits;
} // H5_FQ_Timestep::getHitLocations
#else
int64_t H5_FQ_Timestep::getHitLocations(const char *token,
                                        std::vector<int32_t>& coord) const {

    coord.clear();
    int64_t nhits = -1;
    readLock lock(this, "getHitLocations");
    queryList::const_iterator it = qlist.find(token);
    if (it != qlist.end()) {
        // need to copy the coordinates
        std::vector<uint32_t> tmp;

        nhits = ibis::meshQuery::bitvectorToCoordinates
            (*((*it).second->getHitVector()), getMeshDims(), tmp);
        coord.reserve(tmp.size());
        for (unsigned i = 0; i < tmp.size(); ++ i)
            coord.push_back(tmp[i]);
    }
    return nhits;
} // H5_FQ_Timestep::getHitLocations
#endif

// This implemenation overrides the default one that relys on the
// vertically partitioned data files.
long H5_FQ_Timestep::evaluateRangex(const ibis::qContinuousRange& cmp,
                                   const ibis::bitvector& mask,
                                   ibis::bitvector& hits) const {
    std::string colname(cmp.colName());
    columnList::const_iterator it = columns.find(cmp.colName());
    if (it == columns.end()) {
        logWarning("evaluateRange", "unable to find named column %s "
                   "in the data table", cmp.colName());
        return -1;
    }
    int ierr = 0;
    const float mulfactor = 2;
    switch ((*it).second->type()) {
        //case ibis::column::KEY:
        case ibis::UINT: {
            array_t<unsigned int> arr;
            if (mask.cnt() * mulfactor >= mask.size()) {
                // get all values
                ierr = reinterpret_cast<const H5_FQ_Variable*>((*it).second)
                    ->getValues(arr);
            }
            else { // get only the selected values
                std::vector<uint32_t> tmp;
                std::vector<int32_t> coords;
                ibis::meshQuery::bitvectorToCoordinates(mask, getMeshShape(),
                                                        tmp);
                coords.reserve(tmp.size());
                for (unsigned i = 0; i < tmp.size(); ++ i)
                    coords.push_back(tmp[i]);
                ierr = reinterpret_cast<const H5_FQ_Variable*>((*it).second)
                    ->getPointValues(arr, coords);
            }
            if (ierr >= 0)
                doCompare(arr, mask, hits, cmp);
            else
                hits.set(0, mask.size());
            break;
        }
        case ibis::INT: {
            array_t<int> arr;
            if (mask.cnt() * mulfactor >= mask.size()) {
                // get all values
                ierr = reinterpret_cast<const H5_FQ_Variable*>((*it).second)
                    ->getValues(arr);
            }
            else { // get only the selected values
                std::vector<uint32_t> tmp;
                std::vector<int32_t> coords;
                ibis::meshQuery::bitvectorToCoordinates(mask, getMeshShape(),
                                                        tmp);
                coords.reserve(tmp.size());
                for (unsigned i = 0; i < tmp.size(); ++ i)
                    coords.push_back(tmp[i]);
                ierr = reinterpret_cast<const H5_FQ_Variable*>((*it).second)
                    ->getPointValues(arr, coords);
            }
            if (ierr >= 0)
                doCompare(arr, mask, hits, cmp);
            else
                hits.set(0, mask.size());
            break;
        }
        case ibis::LONG: {
            array_t<int64_t> arr;
            if (mask.cnt() * mulfactor >= mask.size()) {
                // get all values
                ierr = reinterpret_cast<const H5_FQ_Variable*>((*it).second)
                    ->getValues(arr);
            }
            else { // get only the selected values
                std::vector<uint32_t> tmp;
                std::vector<int32_t> coords;
                ibis::meshQuery::bitvectorToCoordinates(mask, getMeshShape(),
                                                        tmp);
                coords.reserve(tmp.size());
                for (unsigned i = 0; i < tmp.size(); ++ i)
                    coords.push_back(tmp[i]);
                ierr = reinterpret_cast<const H5_FQ_Variable*>((*it).second)
                    ->getPointValues(arr, coords);
            }
            if (ierr >= 0)
                doCompare(arr, mask, hits, cmp);
            else
                hits.set(0, mask.size());
            break;
        }
        case ibis::FLOAT: {
            array_t<float> arr;
            if (mask.cnt() * mulfactor >= mask.size()) {
                // get all values
                ierr = reinterpret_cast<const H5_FQ_Variable*>((*it).second)
                    ->getValues(arr);
            }
            else { // get only the selected values
                std::vector<uint32_t> tmp;
                std::vector<int32_t> coords;
                ibis::meshQuery::bitvectorToCoordinates(mask, getMeshShape(),
                                                        tmp);
                coords.reserve(tmp.size());
                for (unsigned i = 0; i < tmp.size(); ++ i)
                    coords.push_back(tmp[i]);
                ierr = reinterpret_cast<const H5_FQ_Variable*>((*it).second)
                    ->getPointValues(arr, coords);
            }
            if (ierr >= 0)
                doCompare(arr, mask, hits, cmp);
            else
                hits.set(0, mask.size());
            break;
        }
        case ibis::DOUBLE: {
            array_t<double> arr;
            if (mask.cnt() * mulfactor >= mask.size()) {
                // get all values
                ierr = reinterpret_cast<const H5_FQ_Variable*>((*it).second)
                    ->getValues(arr);
            }
            else { // get only the selected values
                std::vector<uint32_t> tmp;
                std::vector<int32_t> coords;
                ibis::meshQuery::bitvectorToCoordinates(mask, getMeshShape(),
                                                        tmp);
                coords.reserve(tmp.size());
                for (unsigned i = 0; i < tmp.size(); ++ i)
                    coords.push_back(tmp[i]);
                ierr = reinterpret_cast<const H5_FQ_Variable*>((*it).second)
                    ->getPointValues(arr, coords);
            }
            if (ierr >= 0)
                doCompare(arr, mask, hits, cmp);
            else
                hits.set(0, mask.size());
            break;
        }
        default: {
            logWarning("evaluateRange", "unable to process column (name=%s) "
                       "type %d", cmp.colName(),
                       static_cast<int>((*it).second->type()));
            hits.set(0, mask.size());
        }
    }
    return hits.cnt();
} // H5_FQ_Timestep::evaluateRangex

// This implemenation overrides the default one that relys on the
// vertically partitioned data files.
long H5_FQ_Timestep::doScan(const ibis::qRange& cmp,
                            const ibis::bitvector& mask,
                            ibis::bitvector& hits) const {
    std::string colname(cmp.colName());
    columnList::const_iterator it = columns.find(cmp.colName());
    if (it == columns.end()) {
        logWarning("doScan", "unable to find named column %s "
                   "in the data table", cmp.colName());
        return -1;
    }
    int ierr = 0;
    const float mulfactor = 2;
    switch ((*it).second->type()) {
        //case ibis::column::KEY:
        case ibis::UINT: {
            array_t<unsigned int> arr;
            if (mask.cnt() * mulfactor >= mask.size()) {
                // get all values
                ierr = reinterpret_cast<const H5_FQ_Variable*>((*it).second)
                    ->getValues(arr);
            }
            else { // get only the selected values
                std::vector<uint32_t> tmp;
                std::vector<int32_t> coords;
                ibis::meshQuery::bitvectorToCoordinates(mask, getMeshShape(),
                                                        tmp);
                coords.reserve(tmp.size());
                for (unsigned i = 0; i < tmp.size(); ++ i)
                    coords.push_back(tmp[i]);
                ierr = reinterpret_cast<const H5_FQ_Variable*>((*it).second)
                    ->getPointValues(arr, coords);
            }
            if (ierr >= 0)
                doCompare(arr, mask, hits, cmp);
            else
                hits.set(0, mask.size());
            break;
        }
        case ibis::INT: {
            array_t<int> arr;
            if (mask.cnt() * mulfactor >= mask.size()) {
                // get all values
                ierr = reinterpret_cast<const H5_FQ_Variable*>((*it).second)
                    ->getValues(arr);
            }
            else { // get only the selected values
                std::vector<uint32_t> tmp;
                std::vector<int32_t> coords;
                ibis::meshQuery::bitvectorToCoordinates(mask, getMeshShape(),
                                                        tmp);
                coords.reserve(tmp.size());
                for (unsigned i = 0; i < tmp.size(); ++ i)
                    coords.push_back(tmp[i]);
                ierr = reinterpret_cast<const H5_FQ_Variable*>((*it).second)
                    ->getPointValues(arr, coords);
            }
            if (ierr >= 0)
                doCompare(arr, mask, hits, cmp);
            else
                hits.set(0, mask.size());
            break;
        }
        case ibis::LONG: {
            array_t<int64_t> arr;
            if (mask.cnt() * mulfactor >= mask.size()) {
                // get all values
                ierr = reinterpret_cast<const H5_FQ_Variable*>((*it).second)
                    ->getValues(arr);
            }
            else { // get only the selected values
                std::vector<uint32_t> tmp;
                std::vector<int32_t> coords;
                ibis::meshQuery::bitvectorToCoordinates(mask, getMeshShape(),
                                                        tmp);
                coords.reserve(tmp.size());
                for (unsigned i = 0; i < tmp.size(); ++ i)
                    coords.push_back(tmp[i]);
                ierr = reinterpret_cast<const H5_FQ_Variable*>((*it).second)
                    ->getPointValues(arr, coords);
            }
            if (ierr >= 0)
                doCompare(arr, mask, hits, cmp);
            else
                hits.set(0, mask.size());
            break;
        }
        case ibis::FLOAT: {
            array_t<float> arr;
            if (mask.cnt() * mulfactor >= mask.size()) {
                // get all values
                ierr = reinterpret_cast<const H5_FQ_Variable*>((*it).second)
                    ->getValues(arr);
            }
            else { // get only the selected values
                std::vector<uint32_t> tmp;
                std::vector<int32_t> coords;
                ibis::meshQuery::bitvectorToCoordinates(mask, getMeshShape(),
                                                        tmp);
                coords.reserve(tmp.size());
                for (unsigned i = 0; i < tmp.size(); ++ i)
                    coords.push_back(tmp[i]);
                ierr = reinterpret_cast<const H5_FQ_Variable*>((*it).second)
                    ->getPointValues(arr, coords);
            }
            if (ierr >= 0)
                doCompare(arr, mask, hits, cmp);
            else
                hits.set(0, mask.size());
            break;
        }
        case ibis::DOUBLE: {
            array_t<double> arr;
            if (mask.cnt() * mulfactor >= mask.size()) {
                // get all values
                ierr = reinterpret_cast<const H5_FQ_Variable*>((*it).second)
                    ->getValues(arr);
            }
            else { // get only the selected values
                std::vector<uint32_t> tmp;
                std::vector<int32_t> coords;
                ibis::meshQuery::bitvectorToCoordinates(mask, getMeshShape(),
                                                        tmp);
                coords.reserve(tmp.size());
                for (unsigned i = 0; i < tmp.size(); ++ i)
                    coords.push_back(tmp[i]);
                ierr = reinterpret_cast<const H5_FQ_Variable*>((*it).second)
                    ->getPointValues(arr, coords);
            }
            if (ierr >= 0)
                doCompare(arr, mask, hits, cmp);
            else
                hits.set(0, mask.size());
            break;
        }
        default: {
            logWarning("doScan", "unable to process column (name=%s) "
                       "type %d", cmp.colName(),
                       static_cast<int>((*it).second->type()));
            hits.set(0, mask.size());
        }
    }
    return hits.cnt();
} // H5_FQ_Timestep::doScan
