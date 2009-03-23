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
//                              avtStreamlineFilter.C                        //
// ************************************************************************* //

/**
TODO:
Sort: Bias sorting to "my domain". SLs on this domain will be coming to you.

Consider the leaveDomains SLs and the balancing at the same time.

 **/

#include <avtStreamlineFilter.h>
#include "avtSerialSLAlgorithm.h"
#include "avtParDomSLAlgorithm.h"
#include "avtMasterSlaveSLAlgorithm.h"
#include <math.h>
#include <visitstream.h>

#include <vtkCellDataToPointData.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkInterpolatedVelocityField.h>
#include <vtkLineSource.h>
#include <vtkPlaneSource.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkPointSource.h>
#include <vtkVisItStreamLine.h>
#include <vtkGlyph3D.h>

#include <vtkVisItCellLocator.h>

#include <avtCallback.h>
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
#include <avtIntervalTree.h>
#include <avtMetaData.h>
#include <avtParallel.h>
#include <avtStreamline.h>
#include <avtVector.h>

#include <DebugStream.h>
#include <MemStream.h>
#include <TimingsManager.h>
#include <InvalidFilesException.h>
#include <Expression.h>
#include <ExpressionList.h>
#include <ParsingExprList.h>

#include <snprintf.h>

#ifdef PARALLEL
#include <time.h> // needed for nanosleep
#include <mpi.h>
#endif

#define INIT_POINT(p, a, b, c) (p)[0] = a; (p)[1] = b; (p)[2] = c;

// ****************************************************************************
//  Method: avtStreamlineFilter constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//    Brad Whitlock, Wed Dec 22 12:42:30 PDT 2004
//    I added coloringMethod and support for ribbons.
//
//    Hank Childs, Sat Mar  3 09:52:01 PST 2007
//    Initialized useWholeBox.
//
//    Dave Pugmire, Thu Nov 15 12:09:08 EST 2007
//    Initialize streamline direction option.
//
//    Christoph Garth, Mon Feb 25 17:12:49 PST 2008
//    Port to new streamline infrastructure
//
//   Dave Pugmire, Wed Aug 6 15:16:23 EST 2008
//   Add accurate distance calculate option.
//
//   Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//   Add dataSpatialDimension
//
//   Dave Pugmire, Tue Aug 19 17:13:04EST 2008
//   Remove accurate distance calculate option.
//
//   Kathleen Bonnell, Wed Aug 27 15:13:07 PDT 2008
//   Initialize solver.
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Add 3 point density vars.
//
//   Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//   Added termination by number of steps.
//
//   Dave Pugmire, Mon Feb 23 13:38:49 EST 2009
//   Initialize the initial domain load count and timer.
//
//   Dave Pugmire (on behalf of Hank Childs), Tue Feb 24 09:39:17 EST 2009
//   Initial implemenation of pathlines.
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Hank Childs, Sun Mar 22 11:30:40 CDT 2009
//   Initialize specifyPoint.
//
// ****************************************************************************

avtStreamlineFilter::avtStreamlineFilter()
{
    doPathlines = false;
    pathlineNextTimeVar = "__pathlineNextTimeVar__";
    pathlineVar = "";

    maxStepLength = 0.;
    terminationType = avtIVPSolver::TIME;
    termination = 100.;
    showStart = true;
    radius = 0.125;
    pointDensity1 = 1;
    pointDensity2 = 1;
    pointDensity3 = 1;
    coloringMethod = STREAMLINE_COLOR_SPEED;
    displayMethod = STREAMLINE_DISPLAY_LINES;
    streamlineDirection = VTK_INTEGRATE_FORWARD;
    integrationType = STREAMLINE_INTEGRATE_DORLAND_PRINCE;
    relTol = 1e-7;
    absTol = 0;
    intervalTree = NULL;
    specifyPoint = false;
    solver = NULL;
    dataSpatialDimension = 3;

    //
    // Initialize source values.
    //
    sourceType = STREAMLINE_SOURCE_POINT;
    INIT_POINT(pointSource, 0., 0., 0.);
    INIT_POINT(lineStart, 0., 0., 0.);
    INIT_POINT(lineEnd, 1., 0., 0.);
    INIT_POINT(planeOrigin, 0., 0., 0.);
    INIT_POINT(planeNormal, 0., 0., 1.);
    INIT_POINT(planeUpAxis, 0., 1., 0.);
    planeRadius = 1.4142136;
    INIT_POINT(sphereOrigin, 0., 0., 0.);
    sphereRadius = 1.;
    INIT_POINT(boxExtents, 0., 1., 0.);
    INIT_POINT(boxExtents+3, 1., 0., 1.);
    useWholeBox = false;
    InitialIOTime = 0.0;
    InitialDomLoads = 0;
}


// ****************************************************************************
//  Method: avtStreamlineFilter destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//    Brad Whitlock, Wed Dec 22 14:18:03 PST 2004
//    Added ribbons.
//
//    Christoph Garth, Mon Feb 25 17:12:49 PST 2008
//    Port to new streamline infrastructure
//
//    Hank Childs, Fri Aug 22 09:41:02 PDT 2008
//    Move deletion of solver to PostExecute.
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
// ****************************************************************************

avtStreamlineFilter::~avtStreamlineFilter()
{
    std::map<DomainType, vtkVisItCellLocator*>::const_iterator it;
    for ( it = domainToCellLocatorMap.begin(); it != domainToCellLocatorMap.end(); it++ )
        it->second->Delete();
}

// ****************************************************************************
//  Method: avtStreamlineFilter::ComputeRankList
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
avtStreamlineFilter::ComputeRankList(const vector<int> &domList, 
                                     vector<int> &ranks, vector<int> &doms)
{
    ranks.resize(0);
    
    vector<int> r;
    for (int i = 0; i < domList.size(); i++)
    {
        int dom = domList[i];
        DomainType d(dom, 0);
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
//  Method: avtStreamlineFilter::SetDomain
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
// ****************************************************************************

void
avtStreamlineFilter::SetDomain(avtStreamlineWrapper *slSeg)
{
    avtVector endPt;
    double t;
    slSeg->GetEndPoint(endPt, t);
    double xyz[3] = {endPt.x, endPt.y, endPt.z};

    t = t+1e-7; //DRP FIX THIS! The avtStreamline::Advance() needs to set this!
    int timeStep = GetTimeStep(t);

    debug5<<"SetDomain(): pt= "<<endPt<<" T= "<<t<<" step= "<<timeStep<<endl;
    
    slSeg->seedPtDomainList.resize(0);
    vector<int> doms;
    intervalTree->GetElementsListFromRange(xyz, xyz, doms);
    for (int i = 0; i < doms.size(); i++)
        slSeg->seedPtDomainList.push_back(DomainType(doms[i], timeStep));

    slSeg->domain = DomainType(-1,0);
    // 1 domain, easy.
    if (slSeg->seedPtDomainList.size() == 1)
        slSeg->domain = slSeg->seedPtDomainList[0];

    // Point in multiple domains. See if we can shorten the list by 
    //looking at "my" domains.
    else if (slSeg->seedPtDomainList.size() > 1)
    {
        // See if the point is contained in a domain owned by "me".
        vector<DomainType> newDomList;
        bool foundOwner = false;
        for (int i = 0; i < slSeg->seedPtDomainList.size(); i++)
        {
            DomainType dom = slSeg->seedPtDomainList[i];
            if (OwnDomain(dom))
            {
                // If point is inside domain, we are done.
                if (PointInDomain(endPt, dom))
                {
                    slSeg->seedPtDomainList.resize(0);
                    slSeg->seedPtDomainList.push_back(dom);
                    foundOwner = true;
                    break;
                }
            }
            else
                newDomList.push_back(dom);
        }

        // Update the list in slSeg.
        if (!foundOwner)
        {
            slSeg->seedPtDomainList.resize(0);
            for (int i = 0; i < newDomList.size(); i++)
                slSeg->seedPtDomainList.push_back(newDomList[i]);
        }
    }
    
    if (slSeg->seedPtDomainList.size() == 1)
        slSeg->domain = slSeg->seedPtDomainList[0];
    debug5<<"SetDomain: "<<slSeg->domain<<endl;
    /*
    debug1<<"::SetDomain() pt=["<<endPt.xyz[0]<<" "<<endPt.xyz[1]
          <<" "<<endPt.xyz[2]<<"] in domains: ";
    for (int i = 0; i < slSeg->seedPtDomainList.size(); i++)
        debug1<<slSeg->seedPtDomainList[i]<<", ";
    debug1<<endl;
    */
}


// ****************************************************************************
//  Method: avtStreamlineFilter::GetDomain
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
// ****************************************************************************

vtkDataSet *
avtStreamlineFilter::GetDomain(const DomainType &domain, double X, double Y,
                               double Z)
{
    //debug5<<"GetDomain("<<domain<<");\n";
    vtkDataSet *ds = NULL;

    debug5<<"OperatingOnDemand() = "<<OperatingOnDemand()<<endl;
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
    
    debug5<<"GetDomain("<<domain<<") = "<<ds<<endl;
    return ds;
}

// ****************************************************************************
//  Method: avtStreamlineFilter::GetTimeStep
//
//  Purpose:
//      Determine the time step from a t value.
//
//  Programmer: Dave Pugmire
//  Creation:   March 4, 2009
//
// ****************************************************************************

int
avtStreamlineFilter::GetTimeStep(double &t) const
{
    if (doPathlines)
    {
        for (int i = 0; i < domainTimeIntervals.size(); i++)
        {
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

    return 0;
}


// ****************************************************************************
//  Method: avtStreamlineFilter::DomainLoaded
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
avtStreamlineFilter::DomainLoaded(DomainType &domain) const
{
    //debug1<< "avtStreamlineFilter::DomainLoaded("<<domain<<");\n";
#ifdef PARALLEL
    if (OperatingOnDemand())
        return avtDatasetOnDemandFilter::DomainLoaded(domain.domain, domain.timeStep);

    return PAR_Rank() == domainToRank[domain.domain];
#endif
    
    return true;
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetMaxStepLength
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
avtStreamlineFilter::SetMaxStepLength(double len)
{
    maxStepLength = len;
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetIntegrationType
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
avtStreamlineFilter::SetIntegrationType(int type)
{
    integrationType = type;
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetTermination
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
avtStreamlineFilter::SetTermination(int type, double term)
{
    terminationType = avtIVPSolver::TIME;
    
    if (type == STREAMLINE_TERMINATE_DISTANCE)
        terminationType = avtIVPSolver::DISTANCE;
    else if (type == STREAMLINE_TERMINATE_TIME)
        terminationType = avtIVPSolver::TIME;
    else if (type == STREAMLINE_TERMINATE_STEP)
        terminationType = avtIVPSolver::STEP;

    termination = term;
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetPathlines
//
// Purpose: 
//   Turns pathlines on and off.
//
// Arguments:
//   algo : Type of algorithm
//   maxCnt : maximum number of streamlines to process before distributing.
//
// Programmer: Dave Pugmire
// Creation:   Thu Mar  5 09:51:00 EST 2009
//
//
// ****************************************************************************

void
avtStreamlineFilter::SetPathlines(bool pathlines)
{
    doPathlines = pathlines;
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetStreamlineAlgorithm
//
// Purpose: 
//   Sets the streamline algorithm and parameters.
//
// Arguments:
//   algo : Type of algorithm
//   maxCnt : maximum number of streamlines to process before distributing.
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
avtStreamlineFilter::SetStreamlineAlgorithm(int algo,
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
// Method: avtStreamlineFilter::SetTolerances
//
// Purpose: 
//   Sets the filter's integration tolerances
//
// Arguments:
//   reltol : The new relative tolerance.
//   abstol : The new absolute tolerance.
//
// Programmer: Christoph Garth
// Creation:   Mon Feb 25 16:14:44 PST 2008
//
// Modifications:
//
// ****************************************************************************

void
avtStreamlineFilter::SetTolerances(double reltol, double abstol)
{
    relTol = reltol;
    absTol = abstol;
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetColoringMethod
//
// Purpose: 
//   Set the coloring method to use, which determines which auxiliary arrays
//   (if any) are also generated.
//
// Arguments:
//   m : The coloring method.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 22 12:41:08 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
avtStreamlineFilter::SetColoringMethod(int m)
{
    coloringMethod = m;
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetDisplayMethod
//
// Purpose: 
//   Sets the streamline display method.
//
// Arguments:
//   d : The display method.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 22 14:18:47 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
avtStreamlineFilter::SetDisplayMethod(int d)
{
    displayMethod = d;
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetSourceType
//
// Purpose: 
//   Sets the type of source to be used in the streamline process.
//
// Arguments:
//   t : The new streamline source type.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 12:58:04 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
avtStreamlineFilter::SetSourceType(int t)
{
    sourceType = t;
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetPointSource
//
// Purpose: 
//   Sets the streamline point source.
//
// Arguments:
//   pt : The location of the point.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 12:58:36 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
avtStreamlineFilter::SetPointSource(double pt[3])
{
    pointSource[0] = pt[0];
    pointSource[1] = pt[1];
    pointSource[2] = pt[2];
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetLineSource
//
// Purpose: 
//   Sets the source line endpoints.
//
// Arguments:
//   pt1 : The first line endpoint.
//   pt2 : The second line endpoint.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 12:58:59 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
avtStreamlineFilter::SetLineSource(double pt[3], double pt2[3])
{
    for(int i = 0; i < 3; ++i)
    {
        lineStart[i] = pt[i];
        lineEnd[i] = pt2[i];
    }
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetPlaneSource
//
// Purpose: 
//   Sets the plane source information.
//
// Arguments:
//   O : The plane origin.
//   N : The plane normal.
//   U : The plane up axis.
//   R : The plane radius.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 12:59:47 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
avtStreamlineFilter::SetPlaneSource(double O[3], double N[3], double U[3], 
                                    double R)
{
    for(int i = 0; i < 3; ++i)
    {
        planeOrigin[i] = O[i];
        planeNormal[i] = N[i];
        planeUpAxis[i] = U[i];
    }
    planeRadius = R;
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetSphereSource
//
// Purpose: 
//   Sets the sphere source information.
//
// Arguments:
//   O : The sphere origin.
//   R : The sphere radius.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 13:00:34 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
avtStreamlineFilter::SetSphereSource(double O[3], double R)
{
    sphereOrigin[0] = O[0];
    sphereOrigin[1] = O[1];
    sphereOrigin[2] = O[2];
    sphereRadius = R;
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetBoxSource
//
// Purpose: 
//   Sets the box source information.
//
// Arguments:
//   E : The box extents.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 13:01:11 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
avtStreamlineFilter::SetBoxSource(double E[6])
{
    for(int i = 0; i < 6; ++i)
        boxExtents[i] = E[i];
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetShowStart
//
// Purpose: 
//   Indicates whether or not to show the stream starting points.
//
// Arguments:
//   val : Indicates whether or not to show the stream starting points.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 13:02:04 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
avtStreamlineFilter::SetShowStart(bool val)
{
    showStart = val;
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetRadius
//
// Purpose: 
//   Sets the radius used for tubes and ribbons.
//
// Arguments:
//   rad : The tube radius.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 13:02:45 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Dec 22 14:28:35 PST 2004
//   Renamed the method and made it set both the tube radius and the ribbon
//   width.
//
//   Brad Whitlock, Tue Jan 4 09:11:38 PDT 2005
//   Removed the code to set the tube and ribbon width since it now happens
//   later in the ExecuteData method.
//
// ****************************************************************************

void
avtStreamlineFilter::SetRadius(double rad)
{
    radius = rad;
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetPointDensity
//
// Purpose: 
//   Sets the point density used for streamlines. The meaning depends on the
//   source type.
//
// Arguments:
//   den : The new point density.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 13:03:12 PST 2002
//
// Modifications:
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Add 3 point density vars.
//   
// ****************************************************************************

void
avtStreamlineFilter::SetPointDensity(int den)
{
    pointDensity1 = (den > 0) ? den : 1;
    pointDensity2 = pointDensity1;
    pointDensity3 = pointDensity1;
}


// ****************************************************************************
// Method: avtStreamlineFilter::SetStreamlineDirection
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
// ****************************************************************************

void
avtStreamlineFilter::SetStreamlineDirection(int dir)
{
    streamlineDirection = dir;
}


// ****************************************************************************
//  Method: avtStreamlineFilter::CheckOnDemandViability
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
// ****************************************************************************

bool
avtStreamlineFilter::CheckOnDemandViability(void)
{
    // If we don't want on demand, don't provide it.
    if (method == STREAMLINE_PARALLEL_STATIC_DOMAINS)
    {
        debug1 << "avtStreamlineFilter::CheckOnDemandViability(): = " << 0 <<endl;
        return false;
    }
    
    avtIntervalTree *it = GetMetaData()->GetSpatialExtents();
    bool val = (it == NULL ? false : true);
    debug1 << "avtStreamlineFilter::CheckOnDemandViability(): = " << val <<endl;
    return val;
}


// ****************************************************************************
//  Method: avtStreamlineFilter::Execute
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
// ****************************************************************************

void
avtStreamlineFilter::Execute(void)
{
    Initialize();
    vector<avtStreamlineWrapper *> seedpoints;
    GetSeedPoints(seedpoints);
    numSeedPts = seedpoints.size();

    avtSLAlgorithm *slAlgo = NULL;
    SetMaxQueueLength(cacheQLen);

#ifdef PARALLEL
    if (method == STREAMLINE_STAGED_LOAD_ONDEMAND)
        slAlgo = new avtSerialSLAlgorithm(this);
    else if (method == STREAMLINE_PARALLEL_STATIC_DOMAINS)
        slAlgo = new avtParDomSLAlgorithm(this, maxCount);
    else if (method == STREAMLINE_MASTER_SLAVE)
    {
        slAlgo = avtMasterSlaveSLAlgorithm::Create(this,
                                                   maxCount,
                                                   PAR_Rank(),
                                                   PAR_Size(),
                                                   workGroupSz);
    }
#else
    slAlgo = new avtSerialSLAlgorithm(this);
#endif
    InitialIOTime = visitTimer->LookupTimer("Reading dataset");
    slAlgo->Initialize(seedpoints);
    slAlgo->Execute();
    slAlgo->PostExecute();

    delete slAlgo;
    
    delete intervalTree;
    intervalTree = NULL;
}

// ****************************************************************************
//  Method: avtStreamlineFilter::Initialize
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
// ****************************************************************************

void
avtStreamlineFilter::Initialize()
{
    //MOVE TO ALGO. InitStatistics();
    dataSpatialDimension = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();

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
            intervalTree = GetTypedInput()->CalculateSpatialIntervalTree();
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
    //MOVE TO ALGO statusMsgSz = numDomains+2;
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
        vector<int> myDoms;
        myDoms.resize(numDomains, 0);
        for (int i = 0; i < ds_list.domains.size(); i++)
            myDoms[ ds_list.domains[i] ] = rank;
        SumIntArrayAcrossAllProcessors(&myDoms[0],&domainToRank[0],numDomains);
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
    if ( ! OperatingOnDemand() )
        method = STREAMLINE_PARALLEL_STATIC_DOMAINS;
#else
    // for serial, it's all load on demand.
    method = STREAMLINE_STAGED_LOAD_ONDEMAND;
    // allow all domains to be in memory.
    cacheQLen = numDomains;
#endif

    debug5<< "Domain/Data setup:\n";
    for (int i = 0; i < numDomains; i++)
        debug5<<i<<": rank= "<< domainToRank[i]<<" ds= "<<dataSets[i]<<endl;

    // Some methods need random number generator.
    srand(2776724);

    numTimeSteps = 1;
    if (doPathlines)
    {
        std::string db = GetInput()->GetInfo().GetAttributes().GetFullDBName();
        ref_ptr<avtDatabase> dbp = avtCallback::GetDatabase(db, 0, NULL);
        if (*dbp == NULL)
            EXCEPTION1(InvalidFilesException, db.c_str());
        avtDatabaseMetaData *md = dbp->GetMetaData(0);
        debug5<<"Times: [";
        for (int i = 0; i < md->GetTimes().size()-1; i++)
        {
            vector<double> intv(2);
            intv[0] = md->GetTimes()[i];
            intv[1] = md->GetTimes()[i+1];
            if (intv[0] == intv[1])
            {
                intv[0] = (double)i;
                intv[1] = (double)i+1;
            }
            domainTimeIntervals.push_back(intv);
            debug5<<" ("<<intv[0]<<", "<<intv[1]<<")";
        }
        debug5<<"]"<<endl;
        
        numTimeSteps = domainTimeIntervals.size();
        if (numTimeSteps == 1)
            doPathlines = false;
    }
}


// ****************************************************************************
//  Method: avtStreamlineFilter::PointInDomain
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
// ****************************************************************************

bool
avtStreamlineFilter::PointInDomain(avtVector &pt, DomainType &domain)
{
    debug5<< "avtStreamlineFilter::PointInDomain("<<pt<<", dom= "<<domain<<");\n";
    // DAVE: HERE'S A SPOT
    vtkDataSet *ds = GetDomain(domain);

    if (ds == NULL)
    {
        EXCEPTION0(ImproperUseException);
        return false;
    }

    // If it's rectilinear, we can do bbox test...
    if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        double bbox[6];
        intervalTree->GetElementExtents(domain.domain, bbox);
        debug5<<"[ "<<bbox[0]<<" "<<bbox[1]<<" ] [ "<<bbox[2]<<" "<<bbox[3]<<" ] [ "<<bbox[4]<<" "<<bbox[5]<<" ]"<<endl;
        if (pt.x < bbox[0] || pt.x > bbox[1] ||
            pt.y < bbox[2] || pt.y > bbox[3])
        {
            return false;
        }
        
        if(dataSpatialDimension == 3 &&
           (pt.z < bbox[4] || pt.z > bbox[5]))
        {
            return false;
        }

        //If we don't have ghost zones, then we can rest assured that the
        //point is in this domain. For ghost zones, we have to check cells.
        if (ds->GetCellData()->GetArray("avtGhostZones") == NULL)
        {
            return true;
        }
    }

    vtkVisItCellLocator *cellLocator = domainToCellLocatorMap[domain];
    if ( cellLocator == NULL )
    {
        cellLocator = vtkVisItCellLocator::New();
        cellLocator->SetDataSet(ds);
        cellLocator->IgnoreGhostsOn();
        cellLocator->BuildLocator();
        domainToCellLocatorMap[domain] = cellLocator;
    }

    double rad = 1e-6, dist=0.0;
    double p[3] = {pt.x, pt.y, pt.z}, resPt[3]={0.0,0.0,0.0};
    int foundCell = -1, subId = 0;
    int success = cellLocator->FindClosestPointWithinRadius(p, rad, resPt, 
                                                       foundCell, subId, dist);
    debug5<< "suc = "<<success<<" dist = "<<dist<<" resPt= ["<<resPt[0]
          <<" "<<resPt[1]<<" "<<resPt[2]<<"]\n\n";

    return (success == 1 ? true : false);
}


// ****************************************************************************
//  Method: avtStreamlineFilter::OwnDomain
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
avtStreamlineFilter::OwnDomain(DomainType &domain)
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
//  Method: avtStreamlineFilter::ComputeDomainToRankMapping
//
//  Purpose:
//      Compute a mapping of which domains live on which processors.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

void
avtStreamlineFilter::ComputeDomainToRankMapping()
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
        debug1<<"Sum across all procs\n";
        SumIntArrayAcrossAllProcessors(&myDoms[0], &domainToRank[0], numDomains);
#endif

        for (int i = 0; i < numDomains; i++)
        {
            debug1<<"dom: "<<i<<": rank= "<<domainToRank[i]<<" ds= "<<dataSets[i] << endl;
        }
    }

    for (int i = 0; i < numDomains; i++)
        debug1<<i<<": rank= "<< domainToRank[i]<<endl;

#endif
}

int
avtStreamlineFilter::DomainToRank(DomainType &domain)
{
    // First time through, compute the mapping.
    if (domainToRank.size() == 0)
        ComputeDomainToRankMapping();

    if (domain.domain < 0 || domain.domain >= domainToRank.size())
        EXCEPTION1(ImproperUseException, "Domain out of range.");
    
    //debug1<<"avtStreamlineFilter::DomainToRank("<<domain<<") = "<<domainToRank[domain]<<endl;

    if (domain.timeStep != 0)
        EXCEPTION1(ImproperUseException, "Fix DomainToRank for time slices.");

    return domainToRank[domain.domain];
}

// ****************************************************************************
//  Method: avtStreamlineFilter::IntegrateDomain
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
// ****************************************************************************

avtIVPSolver::Result
avtStreamlineFilter::IntegrateDomain(avtStreamlineWrapper *slSeg, 
                                        vtkDataSet *ds,
                                        double *extents,
                                        int maxSteps )
{
    avtDataAttributes &a = GetInput()->GetInfo().GetAttributes();
    bool haveGhostZones = false; //(a.GetContainsGhostZones()==AVT_NO_GHOSTS ? false : true);

    debug5<< "avtStreamlineFilter::IntegrateDomain(dom= "
          <<slSeg->domain<<") HGZ = "<<haveGhostZones <<endl;

    // prepare streamline integration ingredients
    vtkInterpolatedVelocityField* velocity1=vtkInterpolatedVelocityField::New();
    if (doPathlines)
    {
        // Our expression will be the active variable, so reset it.
        if (ds->GetPointData()->GetArray(pathlineVar.c_str()) != NULL)
            ds->GetPointData()->SetActiveVectors(pathlineVar.c_str());
        if (ds->GetCellData()->GetArray(pathlineVar.c_str()) != NULL)
            ds->GetCellData()->SetActiveVectors(pathlineVar.c_str());
    }
    
    // See if we have cell cenetered data...
    vtkCellDataToPointData *cellToPt1 = NULL;
    if (ds->GetPointData()->GetVectors() == NULL)
    {
        cellToPt1 = vtkCellDataToPointData::New();
        
        cellToPt1->SetInput(ds);
        cellToPt1->Update();
        velocity1->AddDataSet(cellToPt1->GetOutput());
    }
    else
        velocity1->AddDataSet(ds);
    
    velocity1->CachingOn();

    vtkInterpolatedVelocityField* velocity2=NULL;
    vtkDataSet *ds2 = NULL;
    vtkCellDataToPointData *cellToPt2 = NULL;
    double t1, t2;
    if (doPathlines)
    {
        velocity2 = vtkInterpolatedVelocityField::New();
        ds2 = (vtkDataSet *) ds->NewInstance();
        ds2->ShallowCopy(ds);

        if (ds2->GetPointData()->GetVectors() != NULL)
            ds2->GetPointData()->SetActiveVectors(pathlineNextTimeVar.c_str());
        else
            ds2->GetCellData()->SetActiveVectors(pathlineNextTimeVar.c_str());
        
        if (ds->GetPointData()->GetVectors() != NULL)
            ds->GetPointData()->SetActiveVectors(pathlineVar.c_str());
        else
            ds->GetCellData()->SetActiveVectors(pathlineVar.c_str());
        
        
        // See if we have cell cenetered data...
        if (ds2->GetPointData()->GetVectors() == NULL)
        {
            cellToPt2 = vtkCellDataToPointData::New();
            
            cellToPt2->SetInput(ds2);
            cellToPt2->Update();
            velocity2->AddDataSet(cellToPt2->GetOutput());
        }
        else
            velocity2->AddDataSet(ds2);
        
        velocity2->CachingOn();

        std::string db = GetInput()->GetInfo().GetAttributes().GetFullDBName();
        ref_ptr<avtDatabase> dbp = avtCallback::GetDatabase(db, 0, NULL);
        if (*dbp == NULL)
            EXCEPTION1(InvalidFilesException, db.c_str());
        
        avtDatabaseMetaData *md = dbp->GetMetaData(slSeg->domain.timeStep,
                                                   false,false, false);
        t1 = md->GetTimes()[slSeg->domain.timeStep];
        t2 = md->GetTimes()[slSeg->domain.timeStep+1];
        if (t1 == t2)
        {
            t1 = (double)slSeg->domain.timeStep;
            t2 = (double)(slSeg->domain.timeStep+1);
        }
    }

    avtIVPField *field = NULL;
    if (doPathlines)
        field = new avtIVPVTKTimeVaryingField(velocity1, velocity2, t1, t2);
    else
        field = new avtIVPVTKField(velocity1);

    double end = termination;
    if (slSeg->dir == avtStreamlineWrapper::BWD)
        end = - end;

    //slSeg->Debug();
    bool doVorticity = ((coloringMethod == STREAMLINE_COLOR_VORTICITY)
                        || (displayMethod == STREAMLINE_DISPLAY_RIBBONS));

    avtIVPSolver::Result result = slSeg->sl->Advance(field,
                                                     terminationType,
                                                     end,
                                                     doVorticity,
                                                     haveGhostZones,
                                                     extents);
    //slSeg->Debug();
    debug5<<"Back from advance: result= "<<result<<endl;
    if (result == avtIVPSolver::OUTSIDE_DOMAIN)
    {
        slSeg->status = avtStreamlineWrapper::OUTOFBOUNDS;
        DomainType oldDomain = slSeg->domain;

        //Set the new domain.
        SetDomain(slSeg);
        
        // Not in any domains.
        if (slSeg->seedPtDomainList.size() == 0)
            slSeg->status = avtStreamlineWrapper::TERMINATE;

        // We are in the same domain.
        else if (slSeg->seedPtDomainList.size() == 1)
        {
            // pathline terminates if timestep is out of bounds.
            if (doPathlines && slSeg->domain.timeStep == -1)
                slSeg->status = avtStreamlineWrapper::TERMINATE;

            if (slSeg->domain == oldDomain)
                 slSeg->status = avtStreamlineWrapper::TERMINATE;
            else
                slSeg->status = avtStreamlineWrapper::OUTOFBOUNDS;
        }
        else
            slSeg->status = avtStreamlineWrapper::TERMINATE;
    }
    else
        slSeg->status = avtStreamlineWrapper::TERMINATE;
    
    velocity1->Delete();
    if (velocity2)
        velocity2->Delete();
    if (cellToPt1)
        cellToPt1->Delete();
    if (cellToPt2)
        cellToPt2->Delete();
    
    debug5<<"::IntegrateDomain() result= "<<result<<endl;
    return result;
}


// ****************************************************************************
//  Method: avtStreamlineFilter::IntegrateStreamline
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
// ****************************************************************************

void
avtStreamlineFilter::IntegrateStreamline(avtStreamlineWrapper *slSeg, int maxSteps)
{
    debug5 << "\navtStreamlineFilter::IntegrateStreamline(dom= "
           << slSeg->domain<<")\n";

    slSeg->status = avtStreamlineWrapper::UNSET;
    //Get the required domain.
    // DAVE: HERE'S A SPOT
    vtkDataSet *ds = GetDomain(slSeg->domain);
    if (ds == NULL)
    {
        slSeg->status = avtStreamlineWrapper::TERMINATE;
    }
    else
    {
        // Integrate over this domain.
        slSeg->UpdateDomainCount(slSeg->domain);

        double extents[6] = { 0.,0., 0.,0., 0.,0. };
        intervalTree->GetElementExtents(slSeg->domain.domain, extents);
        avtIVPSolver::Result result = IntegrateDomain(slSeg, ds, extents, maxSteps);
        debug5<<"ISL: result= "<<result<<endl;

        //SL exited this domain.
        if (slSeg->status == avtStreamlineWrapper::OUTOFBOUNDS)
        {
            debug5<<"OOB: call set domain\n";
            SetDomain(slSeg);
        }
        //SL terminates.
        else
        {
            debug5<<"Terminate!\n";
            debug5<<avtIVPSolver::OK<<endl;
            debug5<<avtIVPSolver::TERMINATE<<endl;
            debug5<<avtIVPSolver::OUTSIDE_DOMAIN<<endl;
            slSeg->status = avtStreamlineWrapper::TERMINATE;
            slSeg->domain.domain = -1;
            slSeg->domain.timeStep = -1;
        }
    }
    
    debug5 << "   IntegrateStreamline DONE: status = " << (slSeg->status==avtStreamlineWrapper::TERMINATE ? "TERMINATE" : "OOB")
           << " domCnt= "<<slSeg->seedPtDomainList.size()<<endl;
}

// ****************************************************************************
// Method: avtStreamlineFilter::SetZToZero
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
avtStreamlineFilter::SetZToZero(vtkPolyData *pd) const
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
//  Method: avtStreamlineFilter::PreExecute
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
// ****************************************************************************

void
avtStreamlineFilter::PreExecute(void)
{
    avtDatasetOnDemandFilter::PreExecute();

    // Create the solver. --Get from user prefs.
    if (integrationType == STREAMLINE_INTEGRATE_DORLAND_PRINCE)
    {
        solver = new avtIVPDopri5;
        solver->SetMaximumStepSize(maxStepLength);
        solver->SetTolerances(relTol, absTol);
    }
    else if (integrationType == STREAMLINE_INTEGRATE_ADAMS_BASHFORTH)
    {
        solver = new avtIVPAdamsBashforth;
        solver->SetMaximumStepSize(maxStepLength);
        solver->SetTolerances(relTol, absTol);
    }
}


// ****************************************************************************
//  Method: avtStreamlineFilter::PostExecute
//
//  Purpose:
//      Gets the variable extents and sets them.
//
//  Programmer: Hank Childs
//  Creation:   March 3, 2007
//
//  Modifications:
//
//    Hank Childs, Fri Aug 22 09:40:21 PDT 2008
//    Move the deletion of the solver here.
//
// ****************************************************************************

void
avtStreamlineFilter::PostExecute(void)
{
    avtDatasetOnDemandFilter::PostExecute();

    if (coloringMethod == STREAMLINE_COLOR_VORTICITY ||
        coloringMethod == STREAMLINE_COLOR_SPEED)
    {
        double range[2];
        avtDataset_p ds = GetTypedOutput();
        avtDatasetExaminer::GetDataExtents(ds, range, "colorVar");

        avtExtents *e;
        e = GetOutput()->GetInfo().GetAttributes()
                                            .GetCumulativeTrueDataExtents();
        e->Merge(range);
        e = GetOutput()->GetInfo().GetAttributes()
                                           .GetCumulativeCurrentDataExtents();
        e->Merge(range);
    }

    if (solver)
        delete solver;
    solver = NULL;
}


// ****************************************************************************
//  Method: avtStreamlineFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Allows the filter to change its output's data object information, which
//      is a description of the data object.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//
//    Brad Whitlock, Mon Jan 3 13:31:11 PST 2005
//    Set the flag that prevents normals from being generated if we're
//    displaying the streamlines as lines.
//
//    Hank Childs, Sat Mar  3 11:02:33 PST 2007
//    Make sure we have a valid active variable before setting its dimension.
//
//    Hank Childs, Sun Mar  9 07:47:05 PST 2008
//    Call the base class' method as well.
//
// ****************************************************************************

void
avtStreamlineFilter::UpdateDataObjectInfo(void)
{
    avtDatasetOnDemandFilter::UpdateDataObjectInfo();

    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    if(displayMethod == STREAMLINE_DISPLAY_LINES)
        GetOutput()->GetInfo().GetValidity().SetNormalsAreInappropriate(true);
    avtDataAttributes &atts = GetOutput()->GetInfo().GetAttributes();
    atts.SetTopologicalDimension(1);
    if (! atts.ValidVariable("colorVar"))
    {
        atts.AddVariable("colorVar");
        atts.SetActiveVariable("colorVar");
        atts.SetVariableDimension(1);
        atts.SetCentering(AVT_NODECENT);
    }
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
//  Method: avtStreamlineFilter::GetSeedPoints
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
// ****************************************************************************

void
avtStreamlineFilter::GetSeedPoints(std::vector<avtStreamlineWrapper *> &pts)
{
    std::vector<avtVector> candidatePts;

    // Add seed points based on the source.
    if(sourceType == STREAMLINE_SOURCE_POINT)
    {
        double z0 = (dataSpatialDimension > 2) ? pointSource[2] : 0.0;
        avtVector pt(pointSource[0], pointSource[1], z0);
        candidatePts.push_back(pt);
    }

    else if(sourceType == STREAMLINE_SOURCE_LINE)
    {
        vtkLineSource* line = vtkLineSource::New();
        double z0 = (dataSpatialDimension > 2) ? lineStart[2] : 0.;
        double z1 = (dataSpatialDimension > 2) ? lineEnd[2] : 0.;
        line->SetPoint1(lineStart[0], lineStart[1], z0);
        line->SetPoint2(lineEnd[0], lineEnd[1], z1);
        line->SetResolution(pointDensity1);
        line->Update();

        for (int i = 0; i< line->GetOutput()->GetNumberOfPoints(); i++)
        {
            double *pt = line->GetOutput()->GetPoint(i);
            avtVector p(pt[0], pt[1], pt[2]);
            candidatePts.push_back(p);
        }
        line->Delete();
    }
    else if(sourceType == STREAMLINE_SOURCE_PLANE)
    {
        vtkPlaneSource* plane = vtkPlaneSource::New();
        plane->SetXResolution(pointDensity1);
        plane->SetYResolution(pointDensity2);
        avtVector O(planeOrigin);
        avtVector U(planeUpAxis);
        avtVector N(planeNormal);
        U.normalize();
        N.normalize();
        if(dataSpatialDimension <= 2)
           N = avtVector(0.,0.,1.);
        // Determine the right vector.
        avtVector R(U % N);
        R.normalize();
        plane->SetOrigin(O.x, O.y, O.z);
        avtVector P1(U * (2./1.414214) * planeRadius + O);
        avtVector P2(R * (2./1.414214) * planeRadius + O);
        plane->SetPoint2(P1.x, P1.y, P1.z);
        plane->SetPoint1(P2.x, P2.y, P2.z);
        plane->SetNormal(N.x, N.y, N.z);
        plane->SetCenter(O.x, O.y, O.z);
        plane->SetResolution(pointDensity1,pointDensity2);
        plane->Update();

        for (int i = 0; i< plane->GetOutput()->GetNumberOfPoints(); i++)
        {
            double *pt = plane->GetOutput()->GetPoint(i);
            avtVector p(pt[0], pt[1], pt[2]);
            candidatePts.push_back(p);
        }
        plane->Delete();
    }

    else if(sourceType == STREAMLINE_SOURCE_SPHERE)
    {
        vtkSphereSource* sphere = vtkSphereSource::New();
        sphere->SetCenter(sphereOrigin[0], sphereOrigin[1], sphereOrigin[2]);
        sphere->SetRadius(sphereRadius);
        sphere->SetLatLongTessellation(1);
        double t = double(30 - pointDensity1) / 29.;
        double angle = t * 3. + (1. - t) * 30.;
        sphere->SetPhiResolution(int(angle));
        sphere->SetThetaResolution(int(angle));

        sphere->Update();
        for (int i = 0; i < sphere->GetOutput()->GetNumberOfPoints(); i++)
        {
            double *pt = sphere->GetOutput()->GetPoint(i);
            avtVector p(pt[0], pt[1], pt[2]);
            candidatePts.push_back(p);
        }
        sphere->Delete();
    }
    else if(sourceType == STREAMLINE_SOURCE_BOX)
    {
        int npts = (pointDensity1+1)*(pointDensity2+1);

        int nZvals = 1;
        if(dataSpatialDimension > 2)
        {
            npts *= (pointDensity3+1);
            nZvals = (pointDensity3+1);
        }

        //Whole domain, ask intervalTree.
        if (useWholeBox)
            intervalTree->GetExtents( boxExtents );
        
        float dX = boxExtents[1] - boxExtents[0];
        float dY = boxExtents[3] - boxExtents[2];
        float dZ = boxExtents[5] - boxExtents[4];

        // If using whole box, shrink the extents inward by 0.5%
        const float shrink = 0.005;
        if (useWholeBox)
        {
            if (dX > 0.0)
            {
                boxExtents[0] += (shrink*dX);
                boxExtents[1] -= (shrink*dX);
                dX = boxExtents[1] - boxExtents[0];
            }
            if ( dY > 0.0 )
            {
                boxExtents[2] += (shrink*dY);
                boxExtents[3] -= (shrink*dY);
                dY = boxExtents[3] - boxExtents[2];
            }
            if ( dZ > 0.0 )
            {
                boxExtents[4] += (shrink*dZ);
                boxExtents[5] -= (shrink*dZ);
                dZ = boxExtents[5] - boxExtents[4];
            }
        }

        int index = 0;
        for(int k = 0; k < nZvals; ++k)
        {
            float Z = 0.;
            if(dataSpatialDimension > 2)
                Z = (float(k) / float(pointDensity3)) * dZ + boxExtents[4];
            for(int j = 0; j < pointDensity2+1; ++j)
            {
                float Y = (float(j) / float(pointDensity2)) * dY +boxExtents[2];
                for(int i = 0; i < pointDensity1+1; ++i)
                {
                    float X = (float(i) / float(pointDensity1)) * dX 
                            + boxExtents[0];
                    avtVector p(X,Y,Z);
                    candidatePts.push_back(p);
                }
            }
        }
    }

    //Make sure we don't have any points on boundaries. These cause major 
    //heartburn.
    //Also, filter out any points that aren't inside the DS.
    vector<seedPtDomain> ptDom;
    double dataRange[6] = { 0.,0., 0.,0., 0.,0. };
    
    intervalTree->GetExtents(dataRange);
    double dX = dataRange[1]-dataRange[0];
    double dY = dataRange[3]-dataRange[2];
    double dZ = dataRange[5]-dataRange[4];
    double minRange = std::min(dX, std::min(dY,dZ));

    for (int i = 0; i < candidatePts.size(); i++)
    {
        vector<int> dl;
        seedPtDomain pd;
        double xyz[3] = {candidatePts[i].x,candidatePts[i].y,candidatePts[i].z};
        intervalTree->GetElementsListFromRange(xyz,xyz, dl);

        //cout<<i<<": "<<candidatePts[i].xyz[0]<<" "<<candidatePts[i].xyz[1]<<" "<<candidatePts[i].xyz[2]<<" dl= "<<dl.size()<<endl;
        // seed in no domains, try to wiggle it into a DS.
        if (dl.size() == 0)
        {
            //Try to wiggle it by 0.5% of the dataset size.
            double offset[3], wiggle[3] = {dX*0.005, dY*0.005, dZ*0.005};
            bool foundGoodPt = false;
            for ( int w = 0; w < 100; w++ )
            {
                double wigglePt[3] = {(candidatePts[i].x+wiggle[0]*randMinus1_1(),
                                       candidatePts[i].y+wiggle[1]*randMinus1_1(),
                                       candidatePts[i].z+wiggle[2]*randMinus1_1())};
                
                vector<int> dl2;
                intervalTree->GetElementsListFromRange(wigglePt, wigglePt, dl2);
                //cout<<"Wiggle it: "<<i<<": "<<wigglePt.values()[0]<<" "<<wigglePt.values()[1]<<" "<<wigglePt.values()[2];
                //cout<<" domain cnt: "<<dl2.size()<<endl;
                if ( dl2.size() > 0 )
                {
                    candidatePts[i] = wigglePt;
                    dl.resize(0);
                    for ( int j = 0; j < dl2.size(); j++ )
                        dl.push_back(dl2[j]);
                    foundGoodPt = true;
                    //cout<<"Wiggle it: "<<w<<": "<<wigglePt.values()[0]<<" "<<wigglePt.values()[1]<<" "<<wigglePt.values()[2]<<" dl= "<<dl.size()<<endl;
                    break;
                }
                
            }
            
            // Can't find a point inside, we will skip it.
            if (!foundGoodPt)
                continue;
        }

        debug5<<"Candidate pt: "<<i<<" "<<candidatePts[i];
        debug5<<" id= "<<i<<" dom =[";
        for (int j = 0; j < dl.size();j++)debug5<<dl[j]<<", ";
        debug5<<"]\n";
        
        // Add seed for each domain/pt. At this point, we don't know where 
        // the pt belongs....
        for (int j = 0; j < dl.size(); j++)
        {
            pd.pt = candidatePts[i];
            pd.domain = dl[j];
            pd.id = i;
            ptDom.push_back(pd);
        }
    }
    
    // Now, sort the ptDom.
    qsort(&ptDom[0], ptDom.size(), sizeof(seedPtDomain), comparePtDom);

    for (int i = 0; i < ptDom.size(); i++)
    {
        avtVec pt(ptDom[i].pt.x, ptDom[i].pt.y, ptDom[i].pt.z);
        
        if (streamlineDirection == VTK_INTEGRATE_FORWARD ||
             streamlineDirection == VTK_INTEGRATE_BOTH_DIRECTIONS)
        {
            avtStreamline *sl = new avtStreamline(solver, 0.0, pt);
            avtStreamlineWrapper *slSeg;
            slSeg = new avtStreamlineWrapper(sl,
                                             avtStreamlineWrapper::FWD,
                                             ptDom[i].id);
            slSeg->domain.domain = ptDom[i].domain;
            slSeg->domain.timeStep = 0; //DRP FIX THIS!
            pts.push_back(slSeg);
        }
        
        if (streamlineDirection == VTK_INTEGRATE_BACKWARD ||
             streamlineDirection == VTK_INTEGRATE_BOTH_DIRECTIONS)
        {
            avtStreamline *sl = new avtStreamline(solver, 0.0, pt);
            avtStreamlineWrapper *slSeg;
            slSeg = new avtStreamlineWrapper(sl, 
                                             avtStreamlineWrapper::BWD,
                                             ptDom[i].id);
            slSeg->domain = ptDom[i].domain;
            pts.push_back(slSeg);
        }
    }
}


// ****************************************************************************
//  Method: avtStreamlineFilter::StartSphere
//
//  Purpose:
//      Make the geometry for the sphere that the streamline originates from.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//   Add dataSpatialDimension.
//
// ****************************************************************************

vtkPolyData *
avtStreamlineFilter::StartSphere(float val, double pt[3])
{
    // Create the sphere polydata.
    vtkSphereSource *sphere = vtkSphereSource::New();
    sphere->SetCenter(pt[0], pt[1], pt[2]);
    sphere->SetRadius(radius * 2.);
    sphere->SetLatLongTessellation(1);
    sphere->SetPhiResolution(8);
    sphere->SetThetaResolution(8);
    vtkPolyData *sphereData = sphere->GetOutput();
    sphereData->Update();

    // Set the sphere's scalar to val.
    vtkFloatArray *arr = vtkFloatArray::New();
    int npts = sphereData->GetNumberOfPoints();
    arr->SetNumberOfTuples(npts);
    for (int i = 0; i < npts; ++i)
        arr->SetTuple1(i, val);

    // If we're not 3D, make the sphere be 2D.
    if(dataSpatialDimension <= 2)
        SetZToZero(sphereData);

    sphereData->GetPointData()->SetScalars(arr);
    arr->Delete();
    sphereData->Register(NULL);
    sphere->Delete();

    return sphereData;
}


// ****************************************************************************
//  Method: avtStreamlineFilter::ModifyContract
//
//  Purpose:
//      Tell the contract we need ghost data.
//
//  Programmer: Hank Childs
//  Creation:   June 12, 2008
//
//  Modifications:
//
//    Hank Childs, Mon Jul 21 14:09:03 PDT 2008
//    Remove "colorVar" and replace it with the gradient variable.  This is 
//    a trick because the streamline plot requested "colorVar", which is the
//    variable it wants to color by.
//
//   Dave Pugmire, Wed Aug 6 15:16:23 EST 2008
//   Add accurate distance calculate option.
//
//   Dave Pugmire, Tue Aug 19 17:13:04EST 2008
//   Remove accurate distance calculate option.
//
//   Dave Pugmire (on behalf of Hank Childs), Tue Feb 24 09:39:17 EST 2009
//   Initial implemenation of pathlines.  
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
// ****************************************************************************

avtContract_p
avtStreamlineFilter::ModifyContract(avtContract_p in_contract)
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
    avtDataRequest_p out_dr = NULL;

    if (strcmp(in_dr->GetVariable(), "colorVar") == 0 || doPathlines)
    {
        // The avtStreamlinePlot requested "colorVar", so remove that from the
        // contract now.
        out_dr = new avtDataRequest(in_dr,in_dr->GetOriginalVariable());
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
