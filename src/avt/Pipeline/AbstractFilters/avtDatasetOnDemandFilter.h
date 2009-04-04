/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                        avtDatasetOnDemandFilter.h                         //
// ************************************************************************* //

#ifndef AVT_DATASET_ON_DEMAND_FILTER_H
#define AVT_DATASET_ON_DEMAND_FILTER_H

#include <pipeline_exports.h>

#include <avtDatasetToDatasetFilter.h>
#include <list>

class  avtExtents;
class vtkVisItCellLocator;

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
// **************************************************************************** 

struct DomainCacheEntry
{
    vtkDataSet *ds;
    int domainID, timeStep;
    vtkVisItCellLocator* cl;

    DomainCacheEntry() : ds(0), timeStep(-1), cl(0) {}
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
// **************************************************************************** 

class PIPELINE_API avtDatasetOnDemandFilter : virtual public 
                                                      avtDatasetToDatasetFilter
{
  public:
                                 avtDatasetOnDemandFilter();
     virtual                    ~avtDatasetOnDemandFilter();

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

protected:
    int                          GetPurgeDSCount() const { return purgeDSCount; }
    int                          GetLoadDSCount() const { return loadDSCount; }

    std::map<unsigned long, int>           domainLoadCount;

  private:
    std::list<DomainCacheEntry>  domainQueue;
    int                          maxQueueLength;
    avtContract_p                firstContract;
    bool                         operatingOnDemand;
    int                          purgeDSCount, loadDSCount;
};


#endif


