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
//                         avtDatasetOnDemandFilter.C                        //
// ************************************************************************* //

#include <avtDatasetOnDemandFilter.h>

#include <vtkDataSet.h>

#include <avtCommonDataFunctions.h>
#include <avtDataTree.h>
#include <avtExtents.h>
#include <avtOriginatingSource.h>
#include <avtPointSelection.h>

#include <ImproperUseException.h>
#include <IncompatibleDomainListsException.h>
#include <DebugStream.h>


// ****************************************************************************
//  Method: avtDatasetOnDemandFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   March 1, 2008
//
// Modifications:
//   Dave Pugmire, Thu Mar 13 08:44:18 EDT 2008
//   Set default for maxQueueLength.
//
//   Hank Childs, Thu Jun 12 16:08:41 PDT 2008
//   Initialize operatingOnDemand.
//   
//    Dave Pugmire, Mon Jan 26 13:04:56 EST 2009
//    Initialize purgeDSCount.
//
//    Dave Pugmire, Tue Feb  3 11:05:24 EST 2009
//    Initialize loadDSCount.
//
// ****************************************************************************

avtDatasetOnDemandFilter::avtDatasetOnDemandFilter()
{
    maxQueueLength = 2;
    operatingOnDemand = false;
    purgeDSCount = 0;
    loadDSCount = 0;
}


// ****************************************************************************
//  Method: avtDatasetOnDemandFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   March 1, 2008
//
// Modifications:
//   Dave Pugmire, Thu Mar 13 08:44:18 EDT 2008
//   Change domain cache from map to list.
//
// ****************************************************************************

avtDatasetOnDemandFilter::~avtDatasetOnDemandFilter()
{
    while ( ! domainQueue.empty() )
    {
        domainQueue.front().ds->Delete();
        domainQueue.pop_front();
    }
}


// ****************************************************************************
//  Method: avtDatasetOnDemandFilter::GetDomain
//
//  Purpose:
//      Forces a pipeline update to fetch a single domain.
//
//  Programmer: Hank Childs
//  Creation:   March 1, 2008
//
// Modifications:
//
//   Dave Pugmire, Thu Mar 13 08:44:18 EDT 2008
//   Change domain cache from map to list. Purge queue if it grows beyond max len.
//
//   Hank Childs, Thu Jun 12 11:08:55 PDT 2008
//   Tell the contract we are streaming.
//
//    Dave Pugmire, Mon Jan 26 13:04:56 EST 2009
//    Increment purgeDSCount when a DS is purged.
//
//    Dave Pugmire, Tue Feb  3 11:05:24 EST 2009
//    Increment loadDSCount when a DS is loaded.
//
//    Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//    Added support for time/domain.
//
// ****************************************************************************

vtkDataSet *
avtDatasetOnDemandFilter::GetDomain(int domainId,
                                    int timeStep)
{
    debug1<<"avtDatasetOnDemandFilter::GetDomain("<<domainId<<", "<<timeStep<<");"<<endl;
    if ( ! OperatingOnDemand() )
        EXCEPTION0(ImproperUseException);

    if ( domainId < 0 )
        return NULL;

    // See if it is already in the cache.  If so, just return it.
    std::list<DomainCacheEntry>::iterator it;
    for ( it = domainQueue.begin(); it != domainQueue.end(); it++ )
        // Found it. Move it to the front of the list.
        if (it->domainID == domainId &&
            it->timeStep == timeStep)
        {
            DomainCacheEntry entry;
            entry.ds = it->ds;
            entry.domainID = it->domainID;
            entry.timeStep = timeStep;

            //Remove, then move to front.
            domainQueue.erase( it );
            domainQueue.push_front( entry );
            return entry.ds;
        }


    debug5<<"     Update->GetDomain "<<domainId<<" time= "<<timeStep<<endl;
    avtContract_p new_contract = new avtContract(firstContract);
    vector<int> domains;
    domains.push_back(domainId);
    new_contract->GetDataRequest()->GetRestriction()->TurnOnAll();
    new_contract->GetDataRequest()->GetRestriction()->RestrictDomains(domains);
    if (timeStep >= 0)
        new_contract->GetDataRequest()->SetTimestep(timeStep);
    new_contract->SetOnDemandStreaming(true);

    GetInput()->Update(new_contract);
    vtkDataSet *rv = GetInputDataTree()->GetSingleLeaf();

    // Add it to the cache.
    DomainCacheEntry entry;
    entry.domainID = domainId;
    entry.timeStep = timeStep;
    entry.ds = rv;
    rv->Register(NULL);
    loadDSCount++;

    domainQueue.push_front(entry);
    if ( domainQueue.size() > maxQueueLength )
    {
        vtkDataSet *purgeDS = domainQueue.back().ds;
        int purgeDomainID = domainQueue.back().domainID;
        domainQueue.pop_back();
        purgeDS->Delete();
        purgeDSCount++;
    }

    return rv;
}

// ****************************************************************************
//  Method: avtDatasetOnDemandFilter::GetDataAroundPoint
//
//  Purpose:
//      Forces a pipeline update to fetch the data around a point.
//
//  Programmer: Hank Childs
//  Creation:   March 22, 2009
//
// ****************************************************************************

vtkDataSet *
avtDatasetOnDemandFilter::GetDataAroundPoint(double X, double Y, double Z,
                                             int timeStep)
{
    debug1<<"avtDatasetOnDemandFilter::GetDataAroundPoint("<<X<<", "<<Y<<", "<<Z<<", "<<timeStep<<");"<<endl;
    if ( ! OperatingOnDemand() )
        EXCEPTION0(ImproperUseException);

    debug5<<"     Update->GetDataAroundPoint, time= "<<timeStep<<endl;
    avtContract_p new_contract = new avtContract(firstContract);
    new_contract->GetDataRequest()->GetRestriction()->TurnOnAll();
    avtPointSelection *ptsel = new avtPointSelection;
    double p[3] = { X, Y, Z };
    ptsel->SetPoint(p);

    // data selection will be deleted by contract.
    new_contract->GetDataRequest()->AddDataSelection(ptsel);

    if (timeStep >= 0)
        new_contract->GetDataRequest()->SetTimestep(timeStep);
    new_contract->SetOnDemandStreaming(true);

    GetInput()->Update(new_contract);
    vtkDataSet *rv = GetInputDataTree()->GetSingleLeaf();

    return rv;
}

// ****************************************************************************
//  Method: avtDatasetOnDemandFilter::DomainLoaded
//
//  Purpose:
//      Query if domain is loaded.
//
//  Programmer: Dave Pugmire
//  Creation:   March 13, 2008
//
//  Modifications:
//
//    Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//    Added support for time/domain.
//
// ****************************************************************************
bool
avtDatasetOnDemandFilter::DomainLoaded(int domainID, int timeStep) const
{
    if ( ! OperatingOnDemand() )
        EXCEPTION0(ImproperUseException);

    if ( domainID >= 0 )
    {
        std::list<DomainCacheEntry>::const_iterator it;
        for (it = domainQueue.begin(); it != domainQueue.end(); it++)
            if (it->domainID == domainID &&
                it->timeStep == timeStep)
                return true;
    }
    return false;
}

// ****************************************************************************
//  Method: avtDatasetOnDemandFilter::GetLoadedDomains
//
//  Purpose:
//      Return a list of domains that are loaded.
//
//  Programmer: Dave Pugmire
//  Creation:   March 19, 2008
//
//  Modifications:
//
//    Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//    Added support for time/domain.
//
// ****************************************************************************
void
avtDatasetOnDemandFilter::GetLoadedDomains(std::vector<std::vector<int> > &domains)
{
    debug1<<"avtDatasetOnDemandFilter::GetLoadedDomains()\n";
    if ( ! OperatingOnDemand() )
        EXCEPTION0(ImproperUseException);


    domains.resize(0);
    std::list<DomainCacheEntry>::const_iterator it;
    for ( it = domainQueue.begin(); it != domainQueue.end(); it++ )
    {
        vector<int> dom(2);
        dom[0] = it->domainID;
        dom[1] = it->timeStep;
        domains.push_back(dom);
    }
}


// ****************************************************************************
//  Method: avtDatasetOnDemandFilter::ModifyContract
//
//  Purpose:
//      Modifies the contract that steers pipeline execution.
//
//  Programmer: Hank Childs
//  Creation:   March 1, 2008
//
//    Hank Childs, Thu Jun 12 16:12:04 PDT 2008
//    Add support for where on-demand is not possible.
//
// ****************************************************************************

avtContract_p
avtDatasetOnDemandFilter::ModifyContract(avtContract_p in_contract)
{
    firstContract = in_contract;

    // There is a serious architectural problem here.  The CanDoStreaming
    // method will interrogate the contract to see if ghost data is needed.
    // The contract at this point may believe that it is not needed ... but
    // someone upstream may decide that it is.  We could somehow try to defer
    // the decision, but this filter wants to turn off all of the domains, 
    // which may change the decisions other filters make.  
    //
    // One way out would be to always request ghost data, but that would 
    // preclude a lot of on demand processing, which defeats the purpose.
    //
    // I'm putting my head in the sand right now.  If you are running into
    // a problem, have your derived type of filter re-define ModifyContract
    // and always request ghost data.
    operatingOnDemand = 
            GetInput()->GetOriginatingSource()->CanDoStreaming(in_contract)
         && CheckOnDemandViability();

    avtContract_p out_contract = new avtContract(in_contract);

    if (operatingOnDemand)
    {
        // Turn off the processing of all data
        // ... send in an empty domains list
        vector<int> domains;
        out_contract->GetDataRequest()->GetRestriction()
                                                   ->RestrictDomains(domains);
    }

    return out_contract;
}


// ****************************************************************************
//  Method: avtDatasetOnDemandFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Tells the data object info that we are no longer streaming.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2008
//
// ****************************************************************************

void
avtDatasetOnDemandFilter::UpdateDataObjectInfo(void)
{
    // We won't be streaming on demand after this filter executes.
    avtDataValidity &out_atts = GetOutput()->GetInfo().GetValidity();
    out_atts.SetWhetherStreaming(false);
}


