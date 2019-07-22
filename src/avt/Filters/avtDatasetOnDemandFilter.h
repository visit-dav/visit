// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtDatasetOnDemandFilter.h                         //
// ************************************************************************* //

#ifndef AVT_DATASET_ON_DEMAND_FILTER_H
#define AVT_DATASET_ON_DEMAND_FILTER_H

#include <filters_exports.h>

#include <avtDatasetToDataObjectFilter.h>
#include <list>

#include <vtkDataSet.h>
#include <vtkVisItCellLocator.h>

class  avtExtents;

// ****************************************************************************
//  Class: avtDatasetOnDemandFilter
//
//  Purpose:
//     Helper class for list of entries 
//
//  Programmer: Hank Childs
//  Creation:   March 1, 2008
//
//  Modifications:
//    Gunther H. Weber, Fri Apr  3 17:32:29 PDT 2009
//    Added visitCellLocator entry and a constructor that ensures that it
//    is set to 0 (because if the value is different from 0 the destructor
//    of avtDatasetOnDemandFilter will call Delete() on it.
//
//    Hank Childs, Sat Apr 11 23:26:01 CDT 2009
//    Overhaul how reference counting is done to fix a memory leak with caching.
//
// **************************************************************************** 

struct DomainCacheEntry
{
    vtkDataSet *ds;
    int domainID, timeStep;
    vtkVisItCellLocator* cl;

    DomainCacheEntry() : ds(0), timeStep(-1), cl(0) {}
    ~DomainCacheEntry() 
           { if (ds != NULL) ds->Delete(); if (cl!= NULL) cl->Delete(); };
    DomainCacheEntry(const DomainCacheEntry &dce) 
           { ds = dce.ds; if (ds!=NULL) ds->Register(NULL);
             cl = dce.cl; if (cl!=NULL) cl->Register(NULL);
             domainID = dce.domainID; timeStep = dce.timeStep; };
    DomainCacheEntry       &operator=(const DomainCacheEntry &dce) 
           {  if (ds != NULL) ds->Delete(); 
              if (cl != NULL) cl->Delete(); 
              ds = dce.ds; if (ds!=NULL) ds->Register(NULL);
              cl = dce.cl; if (cl!=NULL) cl->Register(NULL);
              domainID = dce.domainID; timeStep = dce.timeStep; 
              return *this; 
           };
};


// ****************************************************************************
//  Class: avtDatasetOnDemandFilter
//
//  Purpose:
//      A derived type of avtDatasetToDatasetFilter.  This will pull in chunks
//      of a larger data set on demand.
//
//  Programmer: Hank Childs
//  Creation:   March 1, 2008
//
//  Modifications:
//
//    Hank Childs, Sun Mar  9 07:47:05 PST 2008
//    Add UpdateDataObjectInfo.
//
//    Dave Pugmire, Thu Mar 13 08:44:18 EDT 2008
//    Added DomainLoaded query function.
//    Added ability to control the number of cached domains.
//
//    Hank Childs, Thu Jun 12 15:39:13 PDT 2008
//    Added support for preventing on demand operation.
//
//    Dave Pugmire, Mon Jan 26 13:04:56 EST 2009
//    Added purgeDSCount and access function.
//
//    Dave Pugmire, Tue Feb  3 11:05:24 EST 2009
//    Added loadDSCount and access function
//
//    Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//    Added support for time/domain.
//
//    Hank Childs, Sun Mar 22 13:31:08 CDT 2009
//    Add support for getting data around a point.
//
//    Dave Pugmire, Wed Mar 25 09:15:23 EDT 2009
//    Add domain caching for point decomposed domains.
//
//    Dave Pugmire, Sat Mar 28 09:42:15 EDT 2009
//    Counter to keep track of how many times a domain is loaded.
//
//    Gunther H. Weber, Fri Apr  3 17:40:05 PDT 2009
//    Removed map from domain id to cell locator since we currently
//    use the same domain id for all point based data load operations.
//
//    Hank Childs, Mon Apr 20 08:40:07 PDT 2009
//    Change inheritance so that output can be non-datasets.
//
//    David Camp, Tue May  3 06:51:37 PDT 2011
//    Added virtual function PurgeDomain() to let the PICs filter delete the
//    cell locator and any other resoures to do this a dataset.
//
// **************************************************************************** 

class AVTFILTERS_API avtDatasetOnDemandFilter : virtual public 
                                                   avtDatasetToDataObjectFilter
{
  public:
                                 avtDatasetOnDemandFilter();
     virtual                    ~avtDatasetOnDemandFilter();

     void                        CopyCache(avtDatasetOnDemandFilter *);

  protected:
    virtual bool                 DomainLoaded(int, int) const;
    void                         GetLoadedDomains(std::vector<std::vector<int> > &domains);
    void                         SetMaxQueueLength(int len) { maxQueueLength = len; }
    virtual vtkDataSet           *GetDomain(int, int);
    virtual vtkDataSet           *GetDataAroundPoint(double, double, double, int);
    avtContract_p                ModifyContract(avtContract_p);
    virtual void                 UpdateDataObjectInfo(void);

    // Note: this method only produces valid results *after* ModifyContract
    // is called.
    virtual bool                 OperatingOnDemand(void) const { return operatingOnDemand; }
    virtual bool                 CheckOnDemandViability(void) { return true; }

    // This needs to do something else......
    void                         SetOperatingOnDemand( bool v ) { operatingOnDemand = v; }

    virtual void                 PurgeDomain( const int domain, const int timeStep ) = 0;
    void                         EmptyQueue();

protected:
    int                          GetPurgeDSCount() const { return purgeDSCount; }
    int                          GetLoadDSCount() const { return loadDSCount; }

    std::map<unsigned long, int>           domainLoadCount;

  private:
    std::list<DomainCacheEntry>  domainQueue;
    int                          maxQueueLength;
    avtContract_p                firstContract;
    avtContract_p                lastUsedContract;
    bool                         operatingOnDemand;
    int                          purgeDSCount, loadDSCount;
};

#endif

