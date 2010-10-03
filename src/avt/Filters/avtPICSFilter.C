/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                             avtPICSFilter.C                               //
// ************************************************************************* //

/**
TODO:
Sort: Bias sorting to "my domain". ICs on this domain will be coming to you.

Consider the leaveDomains ICs and the balancing at the same time.

 **/

#include <avtPICSFilter.h>
#include "avtSerialICAlgorithm.h"
#include "avtParDomICAlgorithm.h"
#include "avtMasterSlaveICAlgorithm.h"
#include <math.h>
#include <visitstream.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkLineSource.h>
#include <vtkPlaneSource.h>
#include <vtkPlane.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkPointSource.h>
#include <vtkVisItStreamLine.h>
#include <vtkGlyph3D.h>

#include <avtCallback.h>
#include <avtCellLocatorClassic.h>
#include <avtCellLocatorBIH.h>
#include <avtCellLocatorRect.h>
#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtDataset.h>
#include <avtDataTree.h>
#include <avtDatasetExaminer.h>
#include <avtExtents.h>
#include <avtIVPVTKField.h>
#include <avtIVPVTKTimeVaryingField.h>
#include <avtIVPDopri5.h>
#include <avtIVPAdamsBashforth.h>
#include <avtIVPM3DC1Integrator.h>
#include <avtIVPM3DC1Field.h>
#include <avtIntervalTree.h>
#include <avtMetaData.h>
#include <avtParallel.h>
#include <avtVector.h>

#include <DebugStream.h>
#include <MemStream.h>
#include <TimingsManager.h>
#include <InvalidFilesException.h>
#include <Expression.h>
#include <ExpressionList.h>
#include <ParsingExprList.h>
#include <VisItStreamUtil.h>
#include <snprintf.h>

#ifdef PARALLEL
#include <time.h> // needed for nanosleep
#include <mpi.h>
#endif

// ****************************************************************************
//  Method: avtPICSFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2010
//
//  Modifications:
//
//    Hank Childs, Fri Oct  1 20:43:34 PDT 2010
//    Initialize absTolIsFraction.
//
// ****************************************************************************

avtPICSFilter::avtPICSFilter()
{
    doPathlines = false;
    seedTimeStep0 = 0;
    seedTime0 = 0.0;
    pathlineNextTimeVar = "__pathlineNextTimeVar__";
    pathlineVar = "";
    avtICAlgorithm *icAlgo = NULL;

    maxStepLength = 0.;
    terminationType = avtIntegralCurve::TERMINATE_TIME;
    termination = 100.;
    integrationDirection = VTK_INTEGRATE_FORWARD;
    integrationType = STREAMLINE_INTEGRATE_DORMAND_PRINCE;
    relTol = 1e-7;
    absTol = 0;
    absTolIsFraction = false;
    intervalTree = NULL;
    specifyPoint = false;
    solver = NULL;
    dataSpatialDimension = 3;

    InitialIOTime = 0.0;
    InitialDomLoads = 0;
    activeTimeStep = -1;
    MaxID = 0;
}


// ****************************************************************************
//  Method: avtPICSFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2010
//
//  Modifications:
//
//   Christoph Garth, Fri Jul 9 12:53:11 PDF 2010
//   Replace vtkVisItCellLocator by avtCellLocator.
// 
// ****************************************************************************

avtPICSFilter::~avtPICSFilter()
{
    std::map<DomainType, avtCellLocator*>::const_iterator it;

    for ( it = domainToCellLocatorMap.begin(); it != domainToCellLocatorMap.end(); it++ )
    {
        if (it->second)
            delete it->second;
    }
}


// ****************************************************************************
//  Method: avtPICSFilter::ComputeRankList
//
//  Purpose:
//      Computes the processor for each domain.
//
//  Programmer: Dave Pugmire
//  Creation:   June 23, 2008
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
// ****************************************************************************

void
avtPICSFilter::ComputeRankList(const vector<int> &domList, 
                                     vector<int> &ranks, vector<int> &doms)
{
    ranks.resize(0);
    
    vector<int> r;
    for (int i = 0; i < domList.size(); i++)
    {
        int dom = domList[i];
        DomainType d(dom, 0);
        // TODO: Should this be DomainType d(dom, activeTimeStep); instead?
        int proc = DomainToRank(d);
        r.push_back(proc);
    }

    //Filter out any duplicates....
    for (int i = 0; i < r.size(); i++)
    {
        bool addIt = true;
        for (int j = 0; j < ranks.size(); j++)
        {
            if (ranks[j] == r[i])
            {
                addIt = false;
                break;
            }
        }
        
        if (addIt)
        {
            ranks.push_back(r[i]);
            doms.push_back(domList[i]);
        }
    }
}


// ****************************************************************************
//  Method: avtPICSFilter::SetDomain
//
//  Purpose:
//      Sets the list of possible domains a seed point may lie in.
//
//  Programmer: Dave Pugmire
//  Creation:   June 23, 2008
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Hank Childs, Fri Apr 10 23:31:22 CDT 2009
//   Put if statements in front of debug's.  The generation of strings to
//   output to debug was doubling the total integration time.
//
//   Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//   Changed interface to DebugStream to obtain current debug level.
//   
//   Hank Childs, Thu Jun  3 10:22:16 PDT 2010
//   Use new name "GetCurrentLocation".
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
// ****************************************************************************

void
avtPICSFilter::SetDomain(avtIntegralCurve *ic)
{
    avtVector endPt;
    ic->CurrentLocation(endPt);
    double t = ic->CurrentTime();

    double xyz[3] = { endPt.x, endPt.y, endPt.z };

    int timeStep = GetTimeStep( t );

    ic->seedPtDomainList.clear();

    std::vector<int> doms;

    intervalTree->GetElementsListFromRange( xyz, xyz, doms );

    // if (DebugStream::Level5())
    //     debug5<<"SetDomain(): pt= "<<endPt<<" T= "<<t<<" step= "<<timeStep<<endl;

    for( int i=0; i<doms.size(); i++ )
        ic->seedPtDomainList.push_back( DomainType( doms[i], timeStep ) );

    ic->domain = DomainType(-1,0);
    // 1 domain, easy.
    if (ic->seedPtDomainList.size() == 1)
        ic->domain = ic->seedPtDomainList[0];

    // Point in multiple domains. See if we can shorten the list by 
    //looking at "my" domains.
    else if (ic->seedPtDomainList.size() > 1)
    {
        // See if the point is contained in a domain owned by "me".
        vector<DomainType> newDomList;
        bool foundOwner = false;
        for (int i = 0; i < ic->seedPtDomainList.size(); i++)
        {
            DomainType dom = ic->seedPtDomainList[i];
            if (OwnDomain(dom))
            {
                // If point is inside domain, we are done.
                if (PointInDomain(endPt, dom))
                {
                    ic->seedPtDomainList.resize(0);
                    ic->seedPtDomainList.push_back(dom);
                    foundOwner = true;
                    break;
                }
            }
            else
                newDomList.push_back(dom);
        }

        // Update the list in ic.
        if (!foundOwner)
        {
            ic->seedPtDomainList.resize(0);
            for (int i = 0; i < newDomList.size(); i++)
                ic->seedPtDomainList.push_back(newDomList[i]);
        }
    }
    
    if (ic->seedPtDomainList.size() == 1)
        ic->domain = ic->seedPtDomainList[0];
    if (DebugStream::Level5())
        debug5<<"SetDomain: "<<ic->domain<<endl;
    /*
    debug1<<"::SetDomain() pt=["<<endPt.xyz[0]<<" "<<endPt.xyz[1]
          <<" "<<endPt.xyz[2]<<"] in domains: ";
    for (int i = 0; i < ic->seedPtDomainList.size(); i++)
        debug1<<ic->seedPtDomainList[i]<<", ";
    debug1<<endl;
    */
}


// ****************************************************************************
//  Method: avtPICSFilter::GetDomain
//
//  Purpose:
//      Get the VTK domain that contains the streamline.  The way we "get" 
//      depends on our execution mode.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Dave Pugmire, Mon Mar 16 15:05:14 EDT 2009
//   Make DomainType a const reference.
//
//   Hank Childs, Sun Mar 22 13:31:08 CDT 2009
//   Add support for getting the "domain" by using a point.
//
//   Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//   Changed interface to DebugStream to obtain current debug level.
// ****************************************************************************

vtkDataSet *
avtPICSFilter::GetDomain(const DomainType &domain,
                               double X, double Y, double Z)
{
    if (DebugStream::Level5())
        debug5<<"avtPICSFilter::GetDomain("<<domain<<" "<<X<<" "<<Y<<" "<<Z<<") = ";
    vtkDataSet *ds = NULL;

//    if (DebugStream::Level5())
//        debug5<<"OperatingOnDemand() = "<<OperatingOnDemand()<<endl;

    if (OperatingOnDemand())
    {
        if (specifyPoint)
        {
            ds = avtDatasetOnDemandFilter::GetDataAroundPoint(X,Y,Z,
                                                              domain.timeStep);
        }
        else
        {
            ds = avtDatasetOnDemandFilter::GetDomain(domain.domain,
                                                     domain.timeStep);
        }
    }
    else
    {
        /*
        if (domain.timeStep != curTimeSlice)
        {
          if (DebugStream::Level5())
            debug5<<"::GetDomain()  Loading: "<<domain<<endl;
            avtContract_p new_contract = new avtContract(lastContract);
            new_contract->GetDataRequest()->SetTimestep(domain.timeStep);
            GetInput()->Update(new_contract);
            GetAllDatasetsArgs ds_list;
            bool dummy = false;
            GetInputDataTree()->Traverse(CGetAllDatasets, (void*)&ds_list, dummy);

            dataSets.resize(numDomains,NULL);
            for (int i = 0; i < ds_list.domains.size(); i++)
            {
                vtkDataSet *ds = ds_list.datasets[i];
                ds->Register(NULL);
                dataSets[ ds_list.domains[i] ] = ds;
            }

            curTimeSlice = domain.timeStep;
        }
        */
        ds = dataSets[domain.domain];
    }
    
    if (DebugStream::Level5())
        debug5<<ds<<endl;

    return ds;
}

// ****************************************************************************
//  Method: avtPICSFilter::GetTimeStep
//
//  Purpose:
//      Determine the time step from a t value.
//
//  Programmer: Dave Pugmire
//  Creation:   March 4, 2009
//
//  Modifications:
//    Gunther H. Weber, Thu Apr  2 10:59:47 PDT 2009
//    Return activeTimeStep obtained from contract instead of 0 when doing
//    streamlines.
//
//    Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//    Changed interface to DebugStream to obtain current debug level.
// ****************************************************************************

int
avtPICSFilter::GetTimeStep(double &t) const
{
    if (doPathlines)
    {
        for (int i = 0; i < domainTimeIntervals.size(); i++)
        {
            if (DebugStream::Level5())
                debug5<<" T= "<<t<<" in ["<<domainTimeIntervals[i][0]<<", "<<domainTimeIntervals[i][1]<<"] ?"<<endl;
            if (t >= domainTimeIntervals[i][0] &&
                t < (domainTimeIntervals[i][1]))
            {
                return i;
            }
        }
        //EXCEPTION0(ImproperUseException);
        return -1;
    }
    else
    {
        return activeTimeStep;
    }
}

// ****************************************************************************
//  Method: avtPICSFilter::DomainLoaded
//
//  Purpose:
//      Report back as to whether the domain can be loaded.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
// ****************************************************************************

bool
avtPICSFilter::DomainLoaded(DomainType &domain) const
{
    //debug1<< "avtPICSFilter::DomainLoaded("<<domain<<");\n";
#ifdef PARALLEL
    if (OperatingOnDemand())
        return avtDatasetOnDemandFilter::DomainLoaded(domain.domain, domain.timeStep);
    return PAR_Rank() == domainToRank[domain.domain];
#endif
    
    return true;
}


// ****************************************************************************
// Method: avtPICSFilter::SetMaxStepLength
//
// Purpose: 
//   Sets the filter's integration maximum step length.
//
// Arguments:
//   len : The new step length.
//
// Programmer: Christoph Garth
// Creation:   Mon Feb 25 16:14:44 PST 2008
//
// Modifications:
//
// ****************************************************************************

void
avtPICSFilter::SetMaxStepLength(double len)
{
    maxStepLength = len;
}


// ****************************************************************************
// Method: avtPICSFilter::SetIntegrationType
//
// Purpose: 
//   Sets the filter's integration type.
//
// Arguments:
//   type : The type of integrator to use.
//
// Programmer: Dave Pugmire
// Creation:   Thu Jul 31 15:28:31 EDT 2008
//
// Modifications:
//
// ****************************************************************************

void
avtPICSFilter::SetIntegrationType(int type)
{
    integrationType = type;
}


// ****************************************************************************
// Method: avtPICSFilter::SetTermination
//
// Purpose: 
//   Sets the termination criteria
//
// Arguments:
//   type : Type of termination.
//   term : When to terminate.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 12:57:25 PDT 2002
//
// Modifications:
//   Brad Whitlock, Tue Jan 4 08:58:59 PDT 2005
//   Removed code to set the max time for the filter.
//
//   Christoph Garth, Mon Feb 25 17:12:49 PST 2008
//   Port to new streamline infrastructure
//
//   Dave Pugmire, Fri Jul 11 14:12:49 EST 2008
//   Changed name to SetTermination and added the terminationType argument.
//
//   Dave Pugmire, Wed Aug 6 15:16:23 EST 2008
//   Add accurate distance calculate option.
//
//   Dave Pugmire, Tue Aug 19 17:13:04EST 2008
//   Remove accurate distance calculate option.
//
//   Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//   Added termination by number of steps.
//   
// ****************************************************************************

void
avtPICSFilter::SetTermination(int type, double term)
{
    terminationType = avtIntegralCurve::TERMINATE_TIME;
    
    if (type == STREAMLINE_TERMINATE_DISTANCE)
        terminationType = avtIntegralCurve::TERMINATE_DISTANCE;
    else if (type == STREAMLINE_TERMINATE_TIME)
        terminationType = avtIntegralCurve::TERMINATE_TIME;
    else if (type == STREAMLINE_TERMINATE_STEPS)
        terminationType = avtIntegralCurve::TERMINATE_STEPS;
    else if (type == STREAMLINE_TERMINATE_INTERSECTIONS)
        terminationType = avtIntegralCurve::TERMINATE_INTERSECTIONS;

    termination = term;
}


// ****************************************************************************
// Method: avtPICSFilter::SetPathlines
//
// Purpose: 
//   Turns pathlines on and off.
//
// Arguments:
//   algo : Type of algorithm
//   maxCnt : maximum number of integral curves to process before distributing.
//
// Programmer: Dave Pugmire
// Creation:   Thu Mar  5 09:51:00 EST 2009
//
// Modifications:
//
//   Dave Pugmire, Tue Mar 31 17:01:17 EDT 2009
//   Initialize seedTime0.
//
// ****************************************************************************

void
avtPICSFilter::SetPathlines(bool pathlines, double time0)
{
    doPathlines = pathlines;
    seedTime0 = time0;
}


// ****************************************************************************
// Method: avtPICSFilter::SetStreamlineAlgorithm
//
// Purpose: 
//   Sets the streamline algorithm and parameters.
//
// Arguments:
//   algo : Type of algorithm
//   maxCnt : maximum number of integral curves to process before distributing.
//
// Programmer: Dave Pugmire
// Creation:   Thu Jul 31 12:46:32 EDT 2008
//
// Modifications:
//
//   Dave Pugmire, Thu Feb  5 12:23:33 EST 2009
//   Add workGroupSize for masterSlave algorithm.
//
// ****************************************************************************

void
avtPICSFilter::SetStreamlineAlgorithm(int algo,
                                      int maxCnt,
                                      int domCache,
                                      int workGrpSz)
{
    method = algo;
    maxCount = maxCnt;
    cacheQLen = domCache;
    workGroupSz = workGrpSz;
}


// ****************************************************************************
// Method: avtPICSFilter::SetTolerances
//
// Purpose: 
//   Sets the filter's integration tolerances
//
// Arguments:
//   reltol : The new relative tolerance.
//   abstol : The new absolute tolerance.
//   isBBox : Is abstol a fraction of the bounding box.
//
// Programmer: Christoph Garth
// Creation:   Mon Feb 25 16:14:44 PST 2008
//
// Modifications:
//
//   Hank Childs, Fri Oct  1 20:35:21 PDT 2010
//   Add option for absTol that is a fraction of the bbox.
//
// ****************************************************************************

void
avtPICSFilter::SetTolerances(double reltol, double abstol, bool isFraction)
{
    relTol = reltol;
    absTol = abstol;
    absTolIsFraction = isFraction;
}


// ****************************************************************************
// Method: avtPICSFilter::SetIntegrationDirection
//
// Purpose: 
//   Sets the streamline integration direction
//
// Arguments:
//   dir : The new direction
//
// Programmer: Dave Pugmire
// Creation:   Thu Nov 15 12:09:08 EST 2007
//
// Modifications:
//   
//   Hank Childs, Sat Jun  5 16:06:26 PDT 2010
//   Renamed to SetIntegrationDirection.
//
// ****************************************************************************

void
avtPICSFilter::SetIntegrationDirection(int dir)
{
    integrationDirection = dir;
}


// ****************************************************************************
//  Method: avtPICSFilter::CheckOnDemandViability
//
//  Purpose:
//      Checks to see if on demand processing is viable.  Some generic checks
//      are made by the base class.  This check is to see if interval trees
//      are available, as interval trees are important to this module to do
//      on demand processing.
//
//  Programmer: Hank Childs
//  Creation:   June 16, 2008
//
//  Modifications:
//   Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//   Don't use on demand if user has not requested it.
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Reverse the logic to check for on demand.
//
//   Hank Childs, Fri Mar 12 12:25:11 PST 2010
//   Don't use the interval tree if another filter has invalidated it
//   (i.e. displace, reflect)
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Fix bug with variable scoping.
//
// ****************************************************************************

bool
avtPICSFilter::CheckOnDemandViability(void)
{
    // If we don't want on demand, don't provide it.
    if (method == STREAMLINE_PARALLEL_STATIC_DOMAINS)
    {
        debug1 << "avtPICSFilter::CheckOnDemandViability(): = " << 0 <<endl;
        return false;
    }
    
    bool val = false;
    if (GetInput()->GetInfo().GetValidity().GetSpatialMetaDataPreserved())
    {
        avtIntervalTree *it = GetMetaData()->GetSpatialExtents();
        val = (it == NULL ? val : true);
    }
    debug1 << "avtPICSFilter::CheckOnDemandViability(): = " << val <<endl;
    return val;
}


// ****************************************************************************
//  Method: avtPICSFilter::Execute
//
//  Purpose:
//      Calculates a streamline.
//
//  Programmer: Hank Childs
//  Creation:   March 4, 2008
//
//  Modifications:
//
//    Hank Childs, Thu Jun 12 11:49:10 PDT 2008
//    Make our own copy of the interval tree to make sure it doesn't get 
//    deleted out from underneath us.
//
//    Hank Childs, Mon Jun 16 12:19:20 PDT 2008
//    Calculate a new interval tree when in non-on-demand mode.
//
//    Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//    In serial mode, set the cacheQLen to be the total number of domains.
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Add MasterSlave method.
//
//   Dave Pugmire, Mon Feb 23 13:38:49 EST 2009
//   Initialize the initial domain load count and timer.
//
//   Dave Pugmire, Tue Aug 18 09:10:49 EDT 2009
//   Add ability to restart integration of streamlines.
//
//   Dave Pugmire, Thu Dec  3 13:28:08 EST 2009
//   New methods for seedpoint generation.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//   
//  Dave Pugmire, Tue Jul 13 09:24:57 EDT 2010
//  Move icAlgo cleanup from Execute() to PostExecute(). The poincare plot
//  analysis was using IC data after Execute() had been called.
//  
// ****************************************************************************

void
avtPICSFilter::Execute(void)
{
    Initialize();
    vector<avtIntegralCurve *> ics;
    GetIntegralCurvesFromInitialSeeds(ics);
    numSeedPts = ics.size();

    SetMaxQueueLength(cacheQLen);

#ifdef PARALLEL
    if (method == STREAMLINE_LOAD_ONDEMAND)
        icAlgo = new avtSerialICAlgorithm(this);
    else if (method == STREAMLINE_PARALLEL_STATIC_DOMAINS)
        icAlgo = new avtParDomICAlgorithm(this, maxCount);
    else if (method == STREAMLINE_MASTER_SLAVE)
    {
        icAlgo = avtMasterSlaveICAlgorithm::Create(this,
                                                   maxCount,
                                                   PAR_Rank(),
                                                   PAR_Size(),
                                                   workGroupSz);
    }
#else
    icAlgo = new avtSerialICAlgorithm(this);
#endif

    InitialIOTime = visitTimer->LookupTimer("Reading dataset");
    
    icAlgo->Initialize(ics);
    icAlgo->Execute();

    while (ContinueExecute())
    {
        icAlgo->ResetIntegralCurvesForContinueExecute();
        icAlgo->Execute();
    }
}


// ****************************************************************************
//  Function: AlgorithmToString
//
//  Purpose:
//      Gets the name of an algorithm
//
//  Programmer: Hank Childs
//  Creation:   September 29, 2010
//
// ****************************************************************************

const char *
AlgorithmToString(int algo)
{
    if (algo == STREAMLINE_PARALLEL_STATIC_DOMAINS)
    {
        static const char *s = "Parallel Static Domains";
        return s;
    }
    if (algo == STREAMLINE_MASTER_SLAVE)
    {
        static const char *s = "Master Slave";
        return s;
    }
    if (algo == STREAMLINE_LOAD_ONDEMAND)
    {
        static const char *s = "Load On-Demand";
        return s;
    }
    if (algo == STREAMLINE_VISIT_SELECTS)
    {
        static const char *s = "VisIt Selects Best Algo";
        return s;
    }

    static const char *s = "Unknown Algorithm";
    return s;
}


// ****************************************************************************
//  Method: avtPICSFilter::Initialize
//
//  Modifications:
//
//   Hank Childs, Mon Jul 21 13:09:13 PDT 2008
//   Remove the "area code" from the initialization so it will compile on
//   my box.
//
//   Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//   Add dataSpatialDimension
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Add statusMsgSz.
//
//   Hank Childs, Tue Jan 20 13:06:33 CST 2009
//   Add support for file formats that do their own domain decomposition.
//
//   Dave Pugmire, Mon Feb 23 13:38:49 EST 2009
//   Initialize the initial domain load count and timer.
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Hank Childs, Mon Mar 23 11:02:55 CDT 2009
//   Add handling for the case where we load data on demand using point
//   selections.
//
//   Dave Pugmire, Tue Mar 31 17:01:17 EDT 2009
//   Set seedTimeStep0 from input time value.
//
//   Dave Pugmire, Thu Apr  2 10:59:42 EDT 2009
//   Properly bound seedTime0 search.
//
//   Gunther H. Weber, Fri Apr  3 16:01:48 PDT 2009
//   Initialize seedTimeStep0 even when streamlines are computed since
//   otherwise seed points get created for the wrong time step. 
//
//   Gunther H. Weber, Mon Apr  6 19:19:31 PDT 2009
//   Initialize seedTime0 for streamline mode. 
//
//   Hank Childs, Fri Apr 10 23:31:22 CDT 2009
//   Put if statements in front of debug's.  The generation of strings to
//   output to debug was doubling the total integration time.
//
//   Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//   Changed interface to DebugStream to obtain current debug level.
//   
//   Hank Childs, Thu Feb 18 13:01:31 PST 2010
//   Only set seedTime0 to the simulation time for pathlines and not 
//   streamlines.
//
//   Dave Pugmire, Mon Jun 14 14:16:57 EDT 2010
//   Allow serial algorithm to be run in parallel on single domain datasets.
//
//   Hank Childs, Thu Sep  2 10:50:05 PDT 2010
//   Deal with case where domain IDs are not unique.
//
//   Hank Childs, Sun Sep 19 11:04:32 PDT 2010
//   Parallel support for case where domain IDs are not unique.
//
//   Hank Childs, Wed Sep 29 19:25:06 PDT 2010
//   Add support for the "VisIt Selects" algorithm.
//
// ****************************************************************************

void
avtPICSFilter::Initialize()
{
    dataSpatialDimension = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
    std::string db = GetInput()->GetInfo().GetAttributes().GetFullDBName();
    ref_ptr<avtDatabase> dbp = avtCallback::GetDatabase(db, 0, NULL);
    if (*dbp == NULL)
      EXCEPTION1(InvalidFilesException, db.c_str());
    avtDatabaseMetaData *md = dbp->GetMetaData(0);

    if (doPathlines)
    {
        if (md->AreAllTimesAccurateAndValid() != true)
        {
            avtCallback::IssueWarning("Pathlines - The time data does not appear to be accurate and valid. Will continue.");
        }
    }

    // Get/Compute the interval tree.
    avtIntervalTree *it_tmp = GetMetaData()->GetSpatialExtents();

    bool dontUseIntervalTree = false;
    if (GetInput()->GetInfo().GetAttributes().GetDynamicDomainDecomposition())
    {
        // The reader returns an interval tree with one domain (for everything).
        // This is not what we want.  So forget about this one, as we will be 
        // better off calculating one.
        dontUseIntervalTree = true;
    }
    if (it_tmp == NULL || dontUseIntervalTree)
    {
        if (OperatingOnDemand())
        {
            if (GetInput()->GetInfo().GetAttributes().GetDynamicDomainDecomposition())
            {
                // We are going to assume that the format that operates on
                // demand can accept hints about where the data lies and return
                // that data.
                // (This was previously an exception, so we haven't taken too
                //  far of a step backwards with this assumption.)
                debug1 << "This file format reader does dynamic decomposition." << endl;
                debug1 << "We are assuming it can handle hints about what data "
                       << "to read." << endl;
                specifyPoint = true;

                // Use the dummy interval tree, so we have something that fits
                // the existing interface.
                // Make a copy so it doesn't get deleted out from underneath us.
                intervalTree = new avtIntervalTree(it_tmp);
            }
            else
            {
                // It should be there, or else we would have precluded 
                // OnDemand processing in the method CheckOnDemandViability.
                // Basically, this should never happen, so throw an exception.
                EXCEPTION0(ImproperUseException);
            }
        }
        else 
        {
            bool dataIsReplicated = GetInput()->GetInfo().GetAttributes().
                                             DataIsReplicatedOnAllProcessors();
            bool performCalculationsOverAllProcs = true;
            if (dataIsReplicated)
                performCalculationsOverAllProcs = false;
            GetTypedInput()->RenumberDomainIDs(performCalculationsOverAllProcs);
            intervalTree = GetTypedInput()->CalculateSpatialIntervalTree(
                                               performCalculationsOverAllProcs);
        }
    }
    else
    {
        // Make a copy so it doesn't get deleted out from underneath us.
        intervalTree = new avtIntervalTree(it_tmp);
    }

    //Set domain/ds info.
    numDomains = intervalTree->GetNLeaves();
    domainToRank.resize(numDomains,0);
    dataSets.resize(numDomains,NULL);

#ifdef PARALLEL
    int rank = PAR_Rank();
    int nProcs = PAR_Size();
#endif
    
    // Assign domains to processors, if needed.
    // For load on demand, just give some reasonable default 
    // domainToRank mapping for now.
    if (OperatingOnDemand())
    {
#ifdef PARALLEL
        int amountPer = numDomains / nProcs;
        int oneExtraUntil = numDomains % nProcs;
        int lastDomain = 0;
    
        for (int p = 0; p < nProcs; p++)
        {
            int extra = (p < oneExtraUntil ? 1 : 0);
            int num = amountPer + extra;
            for (int i = 0; i < num; i++)
                domainToRank[lastDomain+i] = p;
            lastDomain += num;
        }
#endif
    }
    else
    {
        // See what I have.
        GetAllDatasetsArgs ds_list;
        bool dummy = false;
        GetInputDataTree()->Traverse(CGetAllDatasets, (void*)&ds_list, dummy);

        // Set and communicate all the domains.
#ifdef PARALLEL
        if (numDomains > 1)
        {
            vector<int> myDoms;
            myDoms.resize(numDomains, 0);
            for (int i = 0; i < ds_list.domains.size(); i++)
                myDoms[ ds_list.domains[i] ] = rank;
            SumIntArrayAcrossAllProcessors(&myDoms[0],&domainToRank[0],numDomains);
            debug5<<"numdomains= "<<numDomains<<" myDoms[0]= "<<myDoms[0]<<endl;
        }
        else
            domainToRank[0] = rank;
#endif
        for (int i = 0; i < ds_list.domains.size(); i++)
        {
            vtkDataSet *ds = ds_list.datasets[i];
            ds->Register(NULL);
            dataSets[ ds_list.domains[i] ] = ds;
        }
        InitialDomLoads = ds_list.domains.size();
    }

#ifdef PARALLEL
    // If not operating on demand, the method *has* to be parallel static domains.
    int actualMethod = method;
    if (actualMethod == STREAMLINE_VISIT_SELECTS)
        actualMethod = STREAMLINE_MASTER_SLAVE;
    
    if ( ! OperatingOnDemand() )
    {
        debug1 << "Can only use parallel static domains because we can't operate on demand" << endl;
        actualMethod = STREAMLINE_PARALLEL_STATIC_DOMAINS;
    }

    // Parallel and one domains, use the serial algorithm.
    if (numDomains == 1)
    {
        debug1 << "Forcing load-on-demand since there is only one domain." << endl;
        actualMethod = STREAMLINE_LOAD_ONDEMAND;
    }

    if ((method != STREAMLINE_VISIT_SELECTS) && (method != actualMethod))
    {
        char str[1024];
        SNPRINTF(str, 1024, "Warning: you selected the algorithm \"%s\", but VisIt decided "
                            "it could not use that algorithm and instead used \"%s\".\n",
                         AlgorithmToString(method), AlgorithmToString(actualMethod));
        avtCallback::IssueWarning(str);
    }
    method = actualMethod;
#else
    // for serial, it's all load on demand.
    method = STREAMLINE_LOAD_ONDEMAND;
#endif

    if (DebugStream::Level5())
    {
        debug5<< "Domain/Data setup:\n";
        for (int i = 0; i < numDomains; i++)
            debug5<<i<<": rank= "<< domainToRank[i]<<" ds= "<<dataSets[i]<<endl;
    }

    // Some methods need random number generator.
    srand(2776724);

    numTimeSteps = 1;
    if (doPathlines)
    {
        if (DebugStream::Level5())
            debug5<<"Times: [";
        for (int i = 0; i < md->GetTimes().size()-1; i++)
        {
            vector<double> intv(2);
            intv[0] = md->GetTimes()[i];
            intv[1] = md->GetTimes()[i+1];

            if (intv[0] >= intv[1])
            {
                avtCallback::IssueWarning("Pathlines - Found two adjacent steps that are not inceasing or equal in time. Setting the time difference to 1. This change will most likely affect the results.");

                intv[0] = (double)i;
                intv[1] = (double)i+1;
            }

            domainTimeIntervals.push_back(intv);
            if (DebugStream::Level5())
                debug5<<" ("<<intv[0]<<", "<<intv[1]<<")";
        }
        if (DebugStream::Level5())
            debug5<<"]"<<endl;
        
        numTimeSteps = domainTimeIntervals.size();
        if (numTimeSteps == 1)
            doPathlines = false;

#if 0
        seedTimeStep0 = activeTimeStep;
        seedTime0 = md->GetTimes()[activeTimeStep];
#else
        if (doPathlines)
        {
            seedTimeStep0 = -1;
            for (int i = 0; i < domainTimeIntervals.size(); i++)
                if (seedTime0 >= domainTimeIntervals[i][0] &&
                    seedTime0 < domainTimeIntervals[i][1])
                {
                    seedTimeStep0 = i;
                    break;
                }
            
            if (seedTimeStep0 == -1)
                EXCEPTION1(ImproperUseException, "Invalid pathline time value.");
        }
#endif
    }
    else //if (!doPathlines)
    {

//      ARS - commented as the doPathlines check below as will always fail
//      because this scope is the !doPathline.

        // We need to set seedTimeStep0 even for non-time varying since it is used
        // as time for the seeds.
//         if (doPathlines)
//             seedTime0 = md->GetTimes()[activeTimeStep];
//         else
        seedTime0 = 0.;
        seedTimeStep0 = activeTimeStep;
    }
}


// ****************************************************************************
//  Method: avtPICSFilter::InitializeLocators
//
//  Purpose:
//      Initializes the locators.  Note that some locators don't need to be
//      initialized ... this really only makes sense for parallel static
//      domains.  For PSL, if we don't initialize, then we end up serializing
//      the initialization, as each processor busywaits and then initializes
//      when they actually get something to do.
//
//  Programmer: Hank Childs
//  Creation:   February 19, 2010
//
//  Modifications:
//
//   Christoph Garth, Fri Jul 9 12:53:11 PDF 2010
//   Replace vtkVisItCellLocator by avtCellLocator.
// 
// ****************************************************************************

void
avtPICSFilter::InitializeLocators(void)
{
    if (doPathlines || OperatingOnDemand() || specifyPoint)
        return;  // maybe this makes sense; haven't thought about it

    int t1 = visitTimer->StartTimer();
    for (int i = 0 ; i < numDomains ; i++)
    {
        DomainType dom;
        dom.domain = i;
        dom.timeStep = seedTimeStep0;
        if (OwnDomain(dom))
        {
            std::map<DomainType,avtCellLocator*>::iterator cli = 
                domainToCellLocatorMap.find( dom );

            if( cli == domainToCellLocatorMap.end() )
            {
                vtkDataSet *ds = GetDomain(dom, 0,0,0);
                SetupLocator(dom, ds);
            } 
        }
    }
    visitTimer->StopTimer(t1, "Initializing locators");
}

// ****************************************************************************
//  Method: avtPICSFilter::ReleaseData
//
//  Purpose:
//      Release data.
//
//  Programmer: David Camp
//  Creation:   August 25, 2010
//
// ****************************************************************************

void
avtPICSFilter::ReleaseData(void)
{
    avtDatasetOnDemandFilter::ReleaseData();
    avtDatasetToDatasetFilter::ReleaseData();

    for (int i = 0; i < dataSets.size(); i++)
    {
        if(dataSets[i])
            dataSets[i]->UnRegister(NULL);
    }
}


// ****************************************************************************
//  Method: avtPICSFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Copies data attributes from the input to the output.
//
//  Programmer: Hank Childs
//  Creation:   September 19, 2010
//
// ****************************************************************************

void
avtPICSFilter::UpdateDataObjectInfo(void)
{
    avtDatasetToDatasetFilter::UpdateDataObjectInfo();
    avtDatasetOnDemandFilter::UpdateDataObjectInfo();

    GetOutput()->GetInfo().GetAttributes().SetDataIsReplicatedOnAllProcessors(true);
}


// ****************************************************************************
//  Method: avtPICSFilter::SetupLocator
//
//  Purpose:
//      Sets up a locator for a specific domain.
//
//  Programmer: Hank Childs
//  Creation:   February 19, 2010
//
//  Modifications:
//
//   Christoph Garth, Fri Jul 9 12:53:11 PDF 2010
//   Replace vtkVisItCellLocator by avtCellLocator. Return cached locator
//   or create one for this domain.
// 
// ****************************************************************************

avtCellLocator *
avtPICSFilter::SetupLocator( const DomainType &domain, vtkDataSet *ds )
{
    avtCellLocator *locator = NULL;

    std::map<DomainType, avtCellLocator*>::iterator it = 
        domainToCellLocatorMap.find( domain );
    
    if( it == domainToCellLocatorMap.end() )
    {
        int timer = visitTimer->StartTimer();

        switch( ds->GetDataObjectType() )
        {
        case VTK_RECTILINEAR_GRID:
            locator = new avtCellLocatorRect( ds );
            break;
        default:
            locator = new avtCellLocatorBIH( ds );
            break;
        }

        domainToCellLocatorMap[domain] = locator;

        visitTimer->StopTimer( timer, "SetupLocator" );
    }
    else
        locator = it->second;

    return locator;
}

// ****************************************************************************
//  Method: avtPICSFilter::GetFieldForDomain
//
//  Purpose:
//      Construct a field instance for a specific domain.
//
//  Programmer: Christoph Garth
//  Creation:   July 13, 2010
//
// ****************************************************************************

avtIVPField* 
avtPICSFilter::GetFieldForDomain( const DomainType &domain, vtkDataSet *ds )
{
    avtCellLocator *locator = SetupLocator( domain, ds );

    // if (doPathlines)
    //     return new avtIVPVTKTimeVaryingField field(velocity1, t1, t2);
    // else
    if (integrationType == STREAMLINE_INTEGRATE_M3D_C1_INTEGRATOR)
        return new avtIVPM3DC1Field( ds, locator );
    else
        return new avtIVPVTKField( ds, locator );
}

// ****************************************************************************
//  Method: avtPICSFilter::PointInDomain
//
//  Purpose:
//      Determine if a point lies in a domain.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//   Add dataSpatialDimension and optimization for reclinear grids.
//
//   Dave Pugmire,Thu Dec 18 13:24:23 EST 2008
//   Fix to rectilinear optimization. If there are ghost zones, need to do the
//   full check. Otherwise, points in ghost zones are reported as inside the
//   domain.
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Dave Pugmire, Mon Mar 23 18:33:10 EDT 2009
//   Make changes for point decomposed domain databases.
//
//   Hank Childs, Tue Mar 31 12:43:05 CDT 2009
//   Early return for 0 cells.
//
//   Hank Childs, Fri Apr  3 13:51:30 CDT 2009
//   Fixed a problem where on demand with point-based lookups could not
//   support multiple seedpoints.
//
//   Hank Childs, Fri Apr 10 23:31:22 CDT 2009
//   Put if statements in front of debug's.  The generation of strings to
//   output to debug was doubling the total integration time.
//
//   Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//   Changed interface to DebugStream to obtain current debug level.
//
//   Hank Childs, Fri Feb 19 17:47:04 CST 2010
//   Use a separate routine to generate a cell locator.
//
//   Dave Pugmire, Tue Mar 23 11:11:11 EDT 2010
//   Make sure we ignore ghost zones with using cell locator.
//
//   Christoph Garth, Fri Jul 9 12:53:11 PDF 2010
//   Replace vtkVisItCellLocator by avtCellLocator
//
// ****************************************************************************

bool
avtPICSFilter::PointInDomain(avtVector &pt, DomainType &domain)
{
    int t1 = visitTimer->StartTimer();

    if (DebugStream::Level5())
        debug5<< "avtPICSFilter::PointInDomain("<<pt<<", dom= "<<domain<<") = ";

    vtkDataSet *ds = GetDomain(domain, pt.x, pt.y, pt.z);

    if (ds == NULL)
    {
        debug5<<"Get DS failed for domain= "<<domain<<endl;
        EXCEPTION0(ImproperUseException);
        return false;
    }

    if (ds->GetNumberOfCells() == 0)
        return false;

    // If it's rectilinear, we can do bbox test...
    if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        double bbox[6];
        intervalTree->GetElementExtents(domain.domain, bbox);
        if (pt.x < bbox[0] || pt.x > bbox[1] ||
            pt.y < bbox[2] || pt.y > bbox[3])
        {
            if (DebugStream::Level5()) debug5<<"FALSE bboxXY"<<endl;
            return false;
        }
        
        if(dataSpatialDimension == 3 &&
           (pt.z < bbox[4] || pt.z > bbox[5]))
        {
            if (DebugStream::Level5()) debug5<<"FALSE bboxZ"<<endl;
            return false;
        }

        //If we don't have ghost zones, then we can rest assured that the
        //point is in this domain. For ghost zones, we have to check cells.
        if (ds->GetCellData()->GetArray("avtGhostZones") == NULL)
        {
            if (DebugStream::Level5()) debug5<<"TRUE noGhosts"<<endl;
            return true;
        }
    }

    // check if we have a locator
    std::map<DomainType,avtCellLocator*>::iterator cli = 
        domainToCellLocatorMap.find( domain );

    if( cli != domainToCellLocatorMap.end() && specifyPoint )
    {
        double bbox[6];
        cli->second->GetDataSet()->GetBounds(bbox);

        if (pt.x < bbox[0] || pt.x > bbox[1] || pt.y < bbox[2] || pt.y > bbox[3] ||
            pt.z < bbox[4] || pt.z > bbox[5])
        {
            // We are getting data in a point based way and the point changed
            // and now we have a new "domain 0".  Remove the locator for the
            // old one.
            delete cli->second;
            domainToCellLocatorMap.erase( domain );
        }
    }

    avtCellLocator* locator = SetupLocator( domain, ds );
    
    vtkIdType cell = locator->FindCell( &pt.x, NULL );

    if( cell != -1 )
    {
        // check if this is perchance a ghost cell; 
        // if it is, we do not want this domain

        if( vtkDataArray* ghosts = ds->GetCellData()->GetArray("avtGhostZones") )
        {
            if( ghosts->GetComponent( cell, 0 ) )
                cell = -1;
        }
    }

    if (DebugStream::Level5())
        debug5 << "avtPICSFilter::PointInDomain( " << pt << " ) returns " 
               << (cell == -1 ? "false" : "true") << endl;

    visitTimer->StopTimer( t1, "PointInDomain" );
    return cell != -1;
}


// ****************************************************************************
//  Method: avtPICSFilter::OwnDomain
//
//  Purpose:
//      Reports whether or not this processor owns a given domain.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
// ****************************************************************************

bool
avtPICSFilter::OwnDomain(DomainType &domain)
{
#ifdef PARALLEL
    if (OperatingOnDemand())
        return true;
    return PAR_Rank() == DomainToRank(domain);
#else
    return true;
#endif
}


// ****************************************************************************
//  Method: avtPICSFilter::ComputeDomainToRankMapping
//
//  Purpose:
//      Compute a mapping of which domains live on which processors.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Hank Childs, Fri Apr 10 23:31:22 CDT 2009
//   Put if statements in front of debug's.  The generation of strings to
//   output to debug was doubling the total integration time.
//
//   Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//   Changed interface to DebugStream to obtain current debug level.
// ****************************************************************************

void
avtPICSFilter::ComputeDomainToRankMapping()
{
#if 0
    domainToRank.resize(numDomains,0);
    dataSets.resize(numDomains,NULL);
    
    // Compute a balanced layout of domains to ranks.
    if (OperatingOnDemand())
    {
#ifdef PARALLEL
        int amountPer = numDomains / nProcs;
        int oneExtraUntil = numDomains % nProcs;
        int lastDomain = 0;
    
        for (int p = 0; p < nProcs; p++)
        {
            int extra = (p < oneExtraUntil ? 1 : 0);
            int num = amountPer + extra;
            for (int i = 0; i < num; i++)
                domainToRank[lastDomain+i] = p;
            lastDomain += num;
        }
#endif
    }
    
    // See what the pipeline has already established.
    else
    {
        GetAllDatasetsArgs ds_list;
        bool dummy = false;
        GetInputDataTree()->Traverse(CGetAllDatasets, (void*)&ds_list, dummy);
        vector<int> myDoms;
        myDoms.resize(numDomains, 0);
        for (int i = 0; i < ds_list.domains.size(); i++)
        {
            myDoms[ ds_list.domains[i] ] = 0;
#ifdef PARALLEL
            myDoms[ ds_list.domains[i] ] = rank;
#endif
            dataSets[ ds_list.domains[i] ] = ds_list.datasets[i];
        }

#ifdef PARALLEL
        if (DebugStream::Level5())
            debug5<<"Sum across all procs\n";
        SumIntArrayAcrossAllProcessors(&myDoms[0], &domainToRank[0], numDomains);
#endif

        for (int i = 0; i < numDomains; i++)
        {
            if (DebugStream::Level5())
                debug5<<"dom: "<<i<<": rank= "<<domainToRank[i]<<" ds= "<<dataSets[i] << endl;
        }
    }

    for (int i = 0; i < numDomains; i++)
        if (DebugStream::Level5())
            debug5<<i<<": rank= "<< domainToRank[i]<<endl;

#endif
}

// ****************************************************************************
//  Modifications:
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change ".size() == 0" test with empty, as empty has much better 
//   performance.
//
// ****************************************************************************

int
avtPICSFilter::DomainToRank(DomainType &domain)
{
    // First time through, compute the mapping.
    if (domainToRank.empty())
        ComputeDomainToRankMapping();

    if (domain.domain < 0 || domain.domain >= domainToRank.size())
        EXCEPTION1(ImproperUseException, "Domain out of range.");
    
    //debug1<<"avtPICSFilter::DomainToRank("<<domain<<") = "<<domainToRank[domain]<<endl;

    if (doPathlines && domain.timeStep != 0)
        EXCEPTION1(ImproperUseException, "Fix DomainToRank for time slices.");

    return domainToRank[domain.domain];
}

// ****************************************************************************
//  Method: avtPICSFilter::IntegrateDomain
//
//  Purpose:
//      Do an integration inside a domain.
//
//  Programmer: Dave Pugmire
//  Creation:   March 4, 2008
//
//  Modifications:
//
//    Hank Childs, Thu Jun 12 15:06:08 PDT 2008
//    Detect whether or not we have ghost data.
//
//   Dave Pugmire, Wed Aug 6 15:16:23 EST 2008
//   Add accurate distance calculate option.
//
//   Dave Pugmire, Tue Aug 19 17:13:04EST 2008
//   Remove accurate distance calculate option.
//
//   Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//   Added termination by number of steps. Cleanup of other term types. 
//
//   Dave Pugmire (on behalf of Hank Childs), Tue Feb 24 09:39:17 EST 2009
//   Initial implemenation of pathlines.
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Dave Pugmire, Tue Mar 31 17:01:17 EDT 2009
//   Fix memory leak.
//
//   Hank Childs, Thu Apr  2 17:58:09 CDT 2009
//   Do our own interpolation.  The previous one we used was too buggy for ugrids.
//
//   Hank Childs, Mon Apr  6 19:05:08 PDT 2009
//   Change the estimation of the extents to be the size of the current 
//   domain (not the whole problem).  This will make the leap size better.
//
//   Hank Childs, Tue Apr  7 08:52:59 CDT 2009
//   Use a single vtkVisItInterpolatedVelocity for pathlines, which means
//   that cell locations are done once, not twice.
//
//   Hank Childs, Fri Apr 10 23:10:06 CDT 2009
//   Correctly tell avtStreamline the end time.  It was giving correct
//   results before, but it was doing many iterations to determine the end
//   time, when it was possible to just specify the end time.
//
//   Hank Childs, Sun Apr 12 17:32:31 PDT 2009
//   Fix problem with streamlines not getting communicated to the right
//   processor in parallel.
//
//   Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//   Changed interface to DebugStream to obtain current debug level.
//
//   Dave Pugmire, Mon Jun 8 2009, 11:44:01 EDT 2009
//   Added color by secondary variable. Remove vorticity/ghostzones flags.
//
//   Dave Pugmire, Tue Aug 11 10:25:45 EDT 2009
//   Add new termination criterion: Number of intersections with an object.
//
//   Dave Pugmire, Tue Aug 18 09:10:49 EDT 2009
//   Add ability to restart integration of streamlines.
//
//   Dave Pugmire, Tue Nov  3 09:15:41 EST 2009
//   Bug fix. Out-of-bounds SLs were being set to terminated.
//
//   Hank Childs, Sat Feb 20 05:12:45 PST 2010
//   Send the SL filter's instance of a locator to the interpolated velocity
//   field.
//
//   Dave Pugmire, Tue Feb 23 09:42:25 EST 2010
//   Use domainToCellLocatorMap.find() instead of [] accessor. It will actually
//   add an entry for the key if doesn't already exist.
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change ".size() == 0" test with empty, as empty has much better 
//   performance.
//
//   Dave Pugmire, Tue Mar 23 11:11:11 EDT 2010
//   Moved zone-to-node centering to the streamline plot.
//
//   Dave Pugmire, Wed May 26 13:48:24 EDT 2010
//   New return type from avtPICS::Advance()
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Christoph Garth, Thu Aug 5 16:38:21 PDT 2010
//   Moved avtIVPField construction to GetFieldForDomain function
//
// ****************************************************************************

void
avtPICSFilter::IntegrateDomain(avtIntegralCurve *ic,
                                     vtkDataSet *ds,
                                     double *extents,
                                     int maxSteps )
{
    int t0 = visitTimer->StartTimer();
    
    avtDataAttributes &a = GetInput()->GetInfo().GetAttributes();

    if (DebugStream::Level4())
        debug4<<"avtPICSFilter::IntegrateDom(dom= "<<ic->domain<<")"<<endl;

    if( ic->status == avtIntegralCurve::STATUS_OK )
    {
        avtIVPField* field = GetFieldForDomain( ic->domain, ds );

        ic->Advance( field );
        
        delete field;
    }

    //Particle exited this domain.
    if( ic->status == avtIntegralCurve::STATUS_OK )
    {
        DomainType oldDomain = ic->domain;
        SetDomain(ic);
        int domCnt = ic->seedPtDomainList.size();
        
        //If we land in none, or the same domain, we're done.
        if (domCnt == 0 || (domCnt == 1 && ic->domain == oldDomain))
        {
            if( DebugStream::Level5() )
                debug5 << "avtPICSFilter::IntegrateDomain(): same domain, finishing\n";

            ic->status = avtIntegralCurve::STATUS_FINISHED;
        }
    }
    
    if (DebugStream::Level4())
        debug4<<"avtPICSFilter::IntegrateDomain(): status = " << ic->status << endl;

    visitTimer->StopTimer(t0, "IntegrateDomain");
}


// ****************************************************************************
//  Method: avtPICSFilter::IntegrateCurves
//
//  Purpose:
//      The toplevel routine that actually integrates a streamline.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//   Pass domain extents into integration for ghost zone handling.
//
//   Hank Childs, Tue Aug 19 14:41:44 PDT 2008
//   Make sure we initialize the bounds, especially if we are in 2D.
//
//   Dave Pugmire, Mon Mar 23 18:33:10 EDT 2009
//   Make changes for point decomposed domain databases.
//
//   Hank Childs, Fri Apr 10 23:31:22 CDT 2009
//   Put if statements in front of debug's.  The generation of strings to
//   output to debug was doubling the total integration time.
//
//   Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//   Changed interface to DebugStream to obtain current debug level.
//   
//   Hank Childs, Thu Jun  3 10:22:16 PDT 2010
//   Use new name "GetCurrentLocation".
//
//   Hank Childs, Fri Jun  4 05:13:49 PDT 2010
//   Remove call for collecting statistics that was a no-op.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
// ****************************************************************************

void
avtPICSFilter::AdvectParticle( avtIntegralCurve *ic,
                               int maxSteps )
{
    if( ic->status != avtIntegralCurve::STATUS_OK )
    {
        debug4 << "avtPICSFilter::AdvectParticle(): curve status is " << ic->status
               << ", returning\n";
        return;
    }

    int t1 = visitTimer->StartTimer();
    
    //Get the required domain.
    avtVector pt;
    ic->CurrentLocation( pt );

    vtkDataSet *ds = GetDomain( ic->domain, pt.x, pt.y, pt.z );

    debug4 << "avtPICSFilter::AdvectParticle(" << pt << " " << ic->domain << ")\n";

    if( ds == NULL )
    {
        // no matching domain? then the curve is done
        ic->status = avtIntegralCurve::STATUS_FINISHED;
    }
    else
    {
        double extents[6] = { 0., 0., 0., 0., 0., 0. };
        intervalTree->GetElementExtents( ic->domain.domain, extents );

        IntegrateDomain(ic, ds, extents, maxSteps);



        //IC exited this domain.
        if( ic->status == avtIntegralCurve::STATUS_OK )
        {
            if( DebugStream::Level5() )
                debug5<<"OOB: call set domain\n";
            SetDomain( ic );
        }
        else
        {
            if (DebugStream::Level5()) debug5<<"Terminate!\n";
        }
    }
    
    if(DebugStream::Level4())
        debug4 << "AdvectParticle DONE: status = "<<ic->status<<" doms= "<<ic->seedPtDomainList<<endl;

    visitTimer->StopTimer(t1, "AdvectParticle");
}

// ****************************************************************************
// Method: avtPICSFilter::SetZToZero
//
// Purpose: 
//   Zero out the Z coordinates.
//
// Arguments:
//   pd : An input polydata dataset.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan 3 10:42:42 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtPICSFilter::SetZToZero(vtkPolyData *pd) const
{
    vtkPoints *pts = pd->GetPoints();
    if(pts != 0)
    {
        if (pts->GetDataType() == VTK_FLOAT)
        {
            float *p = (float*)pts->GetVoidPointer(0);
            for(int i = 0; i < pts->GetNumberOfPoints(); ++i)
            {
                p[3*i+2] = 0.f;
            }
        }
        if (pts->GetDataType() == VTK_DOUBLE)
        {
            double *p = (double*)pts->GetVoidPointer(0);
            for(int i = 0; i < pts->GetNumberOfPoints(); ++i)
            {
                p[3*i+2] = 0.;
            }
        }
    }
}


// ****************************************************************************
//  Method: avtPICSFilter::GetLengthScale
//
//  Purpose:
//      Gets the length scale of the volume.
//
//  Programmer: Hank Childs
//  Creation:   October 1, 2010
//
// ****************************************************************************

double
avtPICSFilter::GetLengthScale(void)
{
    double bbox[6];
    bool   gotBounds = false;
    if (GetInput()->GetInfo().GetValidity().GetSpatialMetaDataPreserved())
    {
        avtIntervalTree *it = GetMetaData()->GetSpatialExtents();
        if (it != NULL)
        {
            it->GetExtents(bbox);
            gotBounds = true;
        }
    }

    if (!gotBounds)
    {
        GetSpatialExtents(bbox);
    }

    double vol = 1;
    int    numDims = 0;
    if (bbox[1] > bbox[0])
    {
        vol *= (bbox[1]-bbox[0]);
        numDims++;
    }
    if (bbox[3] > bbox[2])
    {
        vol *= (bbox[3]-bbox[2]);
        numDims++;
    }
    if (bbox[5] > bbox[4])
    {
        vol *= (bbox[5]-bbox[4]);
        numDims++;
    }

    double length = pow(vol, 1.0/numDims);
    return length;
}


// ****************************************************************************
//  Method: avtPICSFilter::PreExecute
//
//  Purpose:
//      Get the current spatial extents if necessary.
//
//  Programmer: Hank Childs
//  Creation:   March 3, 2007
//
//  Modifications:
//
//    Dave Pugmire, Tue Aug 12 13:44:10 EDT 2008
//    Moved the box extents code to the seed point generation function.
//
//    Hank Childs, Fri Oct  1 20:43:34 PDT 2010
//    Add an option for absTol to be a fraction of the bbox.
//
// ****************************************************************************

void
avtPICSFilter::PreExecute(void)
{
    avtDatasetOnDemandFilter::PreExecute();

    double absTolToUse = absTol;
    if (absTolIsFraction)
    {
        double l = GetLengthScale();
        absTolToUse = l*absTol;
    }
    // Create the solver. --Get from user prefs.
    if (integrationType == STREAMLINE_INTEGRATE_DORMAND_PRINCE)
    {
        solver = new avtIVPDopri5;
        solver->SetMaximumStepSize(maxStepLength);
        solver->SetTolerances(relTol, absTolToUse);
    }
    else if (integrationType == STREAMLINE_INTEGRATE_ADAMS_BASHFORTH)
    {
        solver = new avtIVPAdamsBashforth;
        solver->SetMaximumStepSize(maxStepLength);
        solver->SetTolerances(relTol, absTolToUse);
    }
    else if (integrationType == STREAMLINE_INTEGRATE_M3D_C1_INTEGRATOR)
    {
        solver = new avtIVPM3DC1Integrator;
        solver->SetMaximumStepSize(maxStepLength);
        solver->SetTolerances(relTol, absTolToUse);
    }
}


// ****************************************************************************
//  Method: avtPICSFilter::PostExecute
//
//  Purpose:
//      Delete the solver.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2010
//
//  Modifications:
//
//  Dave Pugmire, Tue Jul 13 09:24:57 EDT 2010
//  Move icAlgo cleanup from Execute() to PostExecute(). The poincare plot
//  analysis was using IC data after Execute() had been called.
//
// ****************************************************************************

void
avtPICSFilter::PostExecute(void)
{
    avtDatasetOnDemandFilter::PostExecute();

    if (solver)
    {
        delete solver;
        solver = NULL;
    }

    delete intervalTree;
    intervalTree = NULL;

    icAlgo->PostExecute();
    delete icAlgo;
    icAlgo = NULL;
}


typedef struct
{
    avtVector pt;
    int domain, id;
} seedPtDomain;

static int comparePtDom(const void *a, const void *b)
{
    seedPtDomain *pdA = (seedPtDomain *)a, *pdB = (seedPtDomain *)b;
    
    if (pdA->domain < pdB->domain)
        return -1;
    else if (pdA->domain > pdB->domain)
        return 1;
    return 0;
}


static float
randMinus1_1()
{
    float r = 2.0 * ((float)rand() / (float)RAND_MAX);
    return (r-1.0);
}

// ****************************************************************************
//  Method: avtPICSFilter::GetIntegralCurvesFromInitialSeeds
//
//  Purpose:
//      Get the seed points out of the attributes.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//    Dave Pugmire, Tue Aug 12 13:44:10 EDT 2008
//    Moved the box extents code from PreExecute to here.
//    Attempt to slightly adjust seed points not in the DS.
//
//    Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//    Add dataSpatialDimension.
//
//    Hank Childs, Tue Aug 19 14:41:44 PDT 2008
//    Make sure we initialize the bounds, especially if we are in 2D.
//
//    Dave Pugmire, Wed Aug 20 10:37:24 EST 2008
//    Bug fix. The loop index "i" was being changed when trying to "wiggle"
//    seed points into domains.
//
//   Dave Pugmire, Fri Aug 22 14:47:11 EST 2008
//   Add a seed point id attribute to each streamline.
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Add 3 point density vars.
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Dave Pugmire, Tue Mar 31 17:01:17 EDT 2009
//   Initialize time step in domain and start time of streamlines.
//
//   Hank Childs, Mon Apr  6 17:42:55 PDT 2009
//   Change seedTimeStep0 to seedTime0 (integers were mistakenly being
//   send in as doubles).
//
//   Hank Childs, Fri Apr 10 23:31:22 CDT 2009
//   Put if statements in front of debug's.  The generation of strings to
//   output to debug was doubling the total integration time.
//
//   Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//   Changed interface to DebugStream to obtain current debug level.
//
//   Hank Childs, Sun May  3 12:32:13 CDT 2009
//   Added support for point list sources.
//
//   Dave Pugmire, Mon Jun 8 2009, 11:44:01 EDT 2009
//   Set what scalars to compute on the avtStreamline object.
//
//   Dave Pugmire, Tue Aug 18 09:10:49 EDT 2009
//   Add ability to restart integration of streamlines.
//
//   Dave Pugmire, Tue Nov  3 09:15:41 EST 2009
//   Bug fix. Seed points with multiple domains need to be given a separate ID.
//
//   Dave Pugmire, Thu Dec  3 13:28:08 EST 2009
//   Renamed this method.
//
//   Dave Pugmire (for Christoph Garth), Wed Jan 20 09:28:59 EST 2010
//   Add circle source.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Sat Jun  5 16:30:00 PDT 2010
//   Have derived type set up point list.
//
// ****************************************************************************

void
avtPICSFilter::GetIntegralCurvesFromInitialSeeds(std::vector<avtIntegralCurve *> &curves)
{
    std::vector<avtVector> seedPts = GetInitialLocations();

    vector<vector<int> > ids;
    CreateIntegralCurvesFromSeeds(seedPts, curves, ids);
}

// ****************************************************************************
//  Method: avtPICSFilter::AddSeedpoints
//
//  Purpose:
//      Add additional seed points.
//
//  Programmer: Dave Pugmire
//  Creation:   December 3, 2009
//
// ****************************************************************************

void
avtPICSFilter::AddSeedpoints(std::vector<avtVector> &pts,
                             std::vector<std::vector<int> > &ids)
{
    if (icAlgo == NULL)
        EXCEPTION1(ImproperUseException, "Improper call of avtPICSFilter::AddSeedpoints");
    
    vector<avtIntegralCurve *> ics;
    CreateIntegralCurvesFromSeeds(pts, ics, ids);
    icAlgo->AddIntegralCurves(ics);
}

// ****************************************************************************
//  Method: avtPICSFilter::CreateIntegralCurvesFromSeeds
//
//  Purpose:
//      Create integral curves from seed points.
//
//  Programmer: Dave Pugmire
//  Creation:   December 3, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Feb 23 09:42:25 EST 2010
//   Removed avtStreamlineWrapper:Dir
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Sat Jun  5 16:26:12 CDT 2010
//   Use avtIntegralCurves.
//
// ****************************************************************************

void
avtPICSFilter::CreateIntegralCurvesFromSeeds(std::vector<avtVector> &pts,
                                                std::vector<avtIntegralCurve *> &curves,
                                                std::vector<std::vector<int> > &ids)
{
    for (int i = 0; i < pts.size(); i++)
    {
        double xyz[3] = {pts[i].x, pts[i].y, pts[i].z};
        vector<int> dl;
        
        intervalTree->GetElementsListFromRange(xyz, xyz, dl);
        
        vector<int> seedPtIds;
        
        for (int j = 0; j < dl.size(); j++)
        {
            DomainType dom(dl[j], seedTimeStep0);
            
            if (integrationDirection == VTK_INTEGRATE_FORWARD ||
                integrationDirection == VTK_INTEGRATE_BOTH_DIRECTIONS)
            {
                avtIntegralCurve *ic = 
                    CreateIntegralCurve(solver,
                                        avtIntegralCurve::DIRECTION_FORWARD,
                                        seedTime0, pts[i], 
                                        GetNextCurveID());

                ic->domain = dom;
                ic->termination     = termination;
                ic->terminationType = terminationType;
            
                curves.push_back(ic);
                seedPtIds.push_back(ic->id);
            }
            
            if (integrationDirection == VTK_INTEGRATE_BACKWARD ||
                integrationDirection == VTK_INTEGRATE_BOTH_DIRECTIONS)
            {
                avtIntegralCurve *ic = 
                    CreateIntegralCurve(solver,
                                        avtIntegralCurve::DIRECTION_BACKWARD,
                                        seedTime0, pts[i], 
                                        GetNextCurveID());
                ic->domain = dom;
                ic->termination     = -termination;
                ic->terminationType = terminationType;
            
                curves.push_back(ic);
                seedPtIds.push_back(ic->id);
            }
        }
        
        ids.push_back(seedPtIds);
    }
    
    //Sort them on domain.
    std::sort(curves.begin(), curves.end(), avtIntegralCurve::DomainCompare);

    for (int i = 0; i < curves.size(); i++)
    {
        avtIntegralCurve *ic = curves[i];
        if (DebugStream::Level5())
        {
            avtVector loc;
            ic->CurrentLocation(loc);
            debug5<<"Create seed: id= "<<ic->id<<", dom= "<<ic->domain
                  <<", loc= " << loc <<endl;
        }
    }
}

// ****************************************************************************
//  Method: avtPICSFilter::ModifyContract
//
//  Purpose:
//      Modifies the contract for the PICS filter.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2010
//
//  Modifications:
//
//   Dave Pugmire, Mon Jun 14 14:16:57 EDT 2010
//   Allow serial algorithm to be run in parallel on single domain datasets.
//
//   Hank Childs, Fri Sep  3 12:10:47 PDT 2010
//   Make sure that we tell upstream filters that we want continuous velocity
//   fields ... this is important for the Reflect operator.
//
//   Hank Childs, Tue Sep  7 23:29:40 PDT 2010
//   Fix problem with previous change regarding removing the color variable.
//
// ****************************************************************************

avtContract_p
avtPICSFilter::ModifyContract(avtContract_p in_contract)
{
    //See if we can set pathlines.
    if (doPathlines)
    {
        std::string db = GetInput()->GetInfo().GetAttributes().GetFullDBName();
        ref_ptr<avtDatabase> dbp = avtCallback::GetDatabase(db, 0, NULL);
        if (*dbp == NULL)
            EXCEPTION1(InvalidFilesException, db.c_str());
        avtDatabaseMetaData *md = dbp->GetMetaData(0);
        if (md->GetTimes().size() == 1)
            doPathlines = false;
    }

    lastContract = in_contract;

    avtDataRequest_p in_dr = in_contract->GetDataRequest();

    // If we are part of a plot, the avtPICSPlot requested "colorVar", so remove that from the
    // contract now.
    bool removeColorVar = false;
    if (integrationType == STREAMLINE_INTEGRATE_M3D_C1_INTEGRATOR ||
        doPathlines)
        removeColorVar = true;
    avtDataRequest_p out_dr = NULL;
    if (removeColorVar)
        out_dr = new avtDataRequest(in_dr,in_dr->GetOriginalVariable());
    else
        out_dr = new avtDataRequest(in_dr);
    out_dr->SetVelocityFieldMustBeContinuous(true);

    if ( integrationType == STREAMLINE_INTEGRATE_M3D_C1_INTEGRATOR )
    {
        // Add in the other fields that the M3D Interpolation needs
        // for doing their Newton's Metod.

        // Assume the user has selected B as the primary variable.
        // Which is ignored.

        // Single variables stored as attributes on the header
        out_dr->AddSecondaryVariable("hidden/header/linear");  // /linear
        out_dr->AddSecondaryVariable("hidden/header/ntor");    // /ntor
        
        out_dr->AddSecondaryVariable("hidden/header/bzero");    // /bzero
        out_dr->AddSecondaryVariable("hidden/header/rzero");    // /rzero

        // The mesh - N elements x 7
        out_dr->AddSecondaryVariable("hidden/elements"); // /time_000/mesh/elements

        // Variables on the mesh - N elements x 20
        out_dr->AddSecondaryVariable("hidden/equilibrium/f");  // /equilibrium/fields/f
        out_dr->AddSecondaryVariable("hidden/equilibrium/psi");// /equilibrium/fields/psi

        out_dr->AddSecondaryVariable("hidden/f");      // /time_XXX/fields/f
        out_dr->AddSecondaryVariable("hidden/f_i");    // /time_XXX/fields/f_i
        out_dr->AddSecondaryVariable("hidden/psi");    // /time_XXX/fields/psi
        out_dr->AddSecondaryVariable("hidden/psi_i");  // /time_XXX/fields/psi_i
    }

    if (doPathlines)
    {
        out_dr->AddSecondaryVariable(pathlineNextTimeVar.c_str());
        pathlineVar = in_dr->GetOriginalVariable();
    }
    
    avtContract_p out_contract;
    if ( *out_dr )
        out_contract = new avtContract(in_contract, out_dr);
    else
        out_contract = new avtContract(in_contract);

    //out_contract->GetDataRequest()->SetDesiredGhostDataType(NO_GHOST_DATA);
    out_contract->GetDataRequest()->SetDesiredGhostDataType(GHOST_ZONE_DATA);

#ifdef PARALLEL
    out_contract->SetReplicateSingleDomainOnAllProcessors(true);
#endif

    if (doPathlines)
    {
        bool needExpr = true;
        ExpressionList *elist = ParsingExprList::Instance()->GetList();

        for (int i = 0; i < elist->GetNumExpressions(); i++)
        {
            if (elist->GetExpressions(i).GetName() == pathlineNextTimeVar)
            {
                needExpr = false;
                break;
            }
        }
        if (needExpr)
        {
            pathlineVar = out_dr->GetVariable(); // HANK: ASSUMPTION
            std::string meshname = out_dr->GetVariable(); // Can reuse varname here.
            Expression *e = new Expression();
            e->SetName(pathlineNextTimeVar);
            char defn[1024];
            SNPRINTF(defn, 1024, "conn_cmfe(<[1]id:%s>, %s)", pathlineVar.c_str(), meshname.c_str());
            e->SetDefinition(defn);
            e->SetType(Expression::VectorMeshVar);
            elist->AddExpressions(*e);
            delete e;
        }
    }

    return avtDatasetOnDemandFilter::ModifyContract(out_contract);
}


// ****************************************************************************
//  Method: avtPICSFilter::ExamineContract
//
//  Purpose:
//      Retrieve active time step from current contract.
//
//  Programmer: Gunther H. Weber
//  Creation:   April 2, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtPICSFilter::ExamineContract(avtContract_p in_contract)
{
    avtDatasetOnDemandFilter::ExamineContract(in_contract);
    activeTimeStep = in_contract->GetDataRequest()->GetTimestep();
}


// ****************************************************************************
//  Method: avtPICSFilter::GetTerminatedIntegralCurves
//
//  Purpose:
//      Return list of terminated integral curves.
//
//  Programmer: Dave Pugmire
//  Creation:   Mon Aug 17 09:23:32 EDT 2009
//
//  Modifications:
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
// ****************************************************************************

void
avtPICSFilter::GetTerminatedIntegralCurves(vector<avtIntegralCurve *> &ics)
{
    ics.resize(0);
    if (icAlgo)
        icAlgo->GetTerminatedICs(ics);
}


// ****************************************************************************
//  Method: avtPICSFilter::DeleteIntegralCurves
//
//  Purpose:
//      Delete integral curves
//
//  Programmer: Dave Pugmire
//  Creation:   Tue May 25 10:15:35 EDT 2010
//
//  Modifications:
//
// ****************************************************************************

void
avtPICSFilter::DeleteIntegralCurves(vector<int> &icIDs)
{
    icAlgo->DeleteIntegralCurves(icIDs);
}
