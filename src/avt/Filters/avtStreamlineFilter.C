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

#include <avtDataset.h>
#include <avtDataTree.h>
#include <avtDatasetExaminer.h>
#include <avtExtents.h>
#include <avtIVPVTKField.h>
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
#include <Expression.h>
#include <ExpressionList.h>
#include <ParsingExprList.h>

#ifdef PARALLEL
#include <time.h> // needed for nanosleep
#include <mpi.h>
#endif

#define INIT_POINT(p, a, b, c) (p)[0] = a; (p)[1] = b; (p)[2] = c;

static const int STREAMLINE_TAG = 420000;
static const int STATUS_TAG  = 420001;
static const int DOMAIN_TAG = 420002;
static const int MAX_SLMSG_SZ = 10*1024*1024;

static const int MSG_STATUS = 420003;
static const int MSG_DONE = 420004;
static const int MSG_BALANCE = 420005;
static const int MSG_SEND_SL = 420005;
static const int MSG_LOAD_DOMAIN = 420006;


// ****************************************************************************
//  Method: avtStreamlineWrapper constructor
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//   Dave Pugmire, Wed Aug 20 10:37:24 EST 2008
//   Initialize some previously unitialized member data.
//
//   Dave Pugmire, Fri Aug 22 14:47:11 EST 2008
//   Add a seed point id attribute to each streamline.
//
// ****************************************************************************

avtStreamlineWrapper::avtStreamlineWrapper()
{
    status = UNSET;
    domain = -1;
    numTimesCommunicated = 0;
    sl = NULL;
    dir = FWD;
    maxCnt = sum= numDomainsVisited = 0;
    id = -1;
}


// ****************************************************************************
//  Method: avtStreamlineWrapper constructor
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//   Dave Pugmire, Wed Aug 20 10:37:24 EST 2008
//   Initialize some previously unitialized member data.
//
//   Dave Pugmire, Fri Aug 22 14:47:11 EST 2008
//   Add a seed point id attribute to each streamline.
//
// ****************************************************************************

avtStreamlineWrapper::avtStreamlineWrapper(avtStreamline *s, Dir slDir, int ID)
{
    sl = s;
    status = UNSET;
    domain = -1;
    dir = slDir;
    numTimesCommunicated = 0;
    maxCnt = sum= numDomainsVisited = 0;
    id = ID;
}


// ****************************************************************************
//  Method: avtStreamlineWrapper destructor
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

avtStreamlineWrapper::~avtStreamlineWrapper()
{
    if (sl)
        delete sl;
}


// ****************************************************************************
//  Method: avtStreamlineWrapper::Debug
//
//  Purpose:
//      Outputs debug information.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//   Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//   Print out how many steps have been taken.
//
// ****************************************************************************

void
avtStreamlineWrapper::Debug()
{
    debug1 << "avtStreamlineWrapper::Debug()\n";

    avtVec ends[2];
    sl->PtEnds(ends[0], ends[1]);
    if (dir == FWD)
        debug1<<"******seed: ["<<ends[1].values()[0]<<", "
              <<ends[1].values()[1]<<", "<<ends[1].values()[2]<<"]";
    else
        debug1<<"******seed: ["<<ends[0].values()[0]<<", "
              <<ends[0].values()[1]<<", "<<ends[0].values()[2]<<"]";

    debug1<<" Dom= [ "<<domain<<", ";
    for (int i = 0; i < seedPtDomainList.size(); i++)
        debug1<<seedPtDomainList[i]<<", ";
    debug1<<"] ";
    debug1<< " steps= "<<sl->size()<<endl;
}


// ****************************************************************************
//  Method: avtStreamlineWrapper::Serialize
//
//  Purpose:
//      Serializes into a byte stream.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//   Dave Pugmire, Fri Aug 22 14:47:11 EST 2008
//   Add a seed point id attribute to each streamline.
//
// ****************************************************************************

void
avtStreamlineWrapper::Serialize(MemStream::Mode mode, MemStream &buff, 
                                avtIVPSolver *solver)
{
    //debug1 << "avtStreamlineWrapper::Serialize. sz= "<<buff.buffLen()<< endl;

    buff.io(mode, dir);
    buff.io(mode, id);
    buff.io(mode, domain);
    buff.io(mode, status);
    buff.io(mode, numTimesCommunicated);
    buff.io(mode, domainVisitCnts);
    if (mode == MemStream::READ)
    {
        if (sl)
            delete sl;
        sl = new avtStreamline;
    }
    sl->Serialize(mode, buff, solver);
    //debug1 << "DONE: avtStreamlineWrapper::Serialize. sz= "<< buff.buffLen()
    //       << endl;
}


// ****************************************************************************
//  Method: avtStreamlineWrapper::GetVTKPolyData
//
//  Purpose:
//      Converts the data into a VTK poly data object.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//   Step derivative is not giving the right answer. So, use the velEnd vector
//   for coloring by speed.
//
//   Dave Pugmire, Fri Aug 22 14:47:11 EST 2008
//   Add new coloring methods, length, time and ID.
//
// ****************************************************************************

vtkPolyData *
avtStreamlineWrapper::GetVTKPolyData(int spatialDim, int coloringMethod, 
                                     int displayMethod, vector<float> &thetas)
{
    if (sl == NULL || sl->size() == 0)
        return NULL;

    vtkPoints *points = vtkPoints::New();
    vtkCellArray *cells = vtkCellArray::New();
    vtkFloatArray *scalars = vtkFloatArray::New();
    
    cells->InsertNextCell(sl->size());
    scalars->Allocate(sl->size());
    avtStreamline::iterator siter;
    
    unsigned int i = 0;
    float val = 0.0, theta = 0.0;
    debug1<<"Create vtkPolyData\n";
    for(siter = sl->begin(); siter != sl->end(); ++siter, i++)
    {
        debug1<<i<<": "<< (*siter)->front()<<endl;
        points->InsertPoint(i, (*siter)->front()[0], (*siter)->front()[1], 
                            (spatialDim > 2 ? (*siter)->front()[2] : 0.0));
        cells->InsertCellPoint(i);

        avtIVPStep *step = (*siter);

        // Set the speed/vorticity.
        if (coloringMethod == STREAMLINE_COLOR_SPEED)
        {
            avtVec deriv = step->velEnd;
            val = deriv.values()[0]*deriv.values()[0] 
                + deriv.values()[1]*deriv.values()[1];
            if (spatialDim == 3)
                val += deriv.values()[2]*deriv.values()[2];
            val = sqrt(val);
        }
        else if (coloringMethod ==  STREAMLINE_COLOR_ARCLENGTH)
        {
            val += step->length();
        }
        else if (coloringMethod ==  STREAMLINE_COLOR_TIME)
        {
            val = step->tEnd;
        }
        else if (coloringMethod ==  STREAMLINE_COLOR_ID)
        {
            val = (float)id;
        }
        
        if (coloringMethod == STREAMLINE_COLOR_VORTICITY || 
            displayMethod == STREAMLINE_DISPLAY_RIBBONS)
        {
            double dT = (step->tEnd - step->tStart);
            float scaledVort = step->vorticity * dT;
            theta += scaledVort;
            thetas.push_back(theta);
            if (coloringMethod == STREAMLINE_COLOR_VORTICITY)
                val = scaledVort;
        }
        
        scalars->InsertTuple1(i, val);
    }
    
    //Create the polydata.
    vtkPolyData *pd = vtkPolyData::New();
    pd->SetPoints(points);
    pd->SetLines(cells);
    scalars->SetName("colorVar");
    pd->GetPointData()->SetScalars(scalars);

    points->Delete();
    cells->Delete();
    scalars->Delete();

    return pd;
}


// ****************************************************************************
//  Method: avtStreamlineWrapper::GetStartPoint
//
//  Purpose:
//      Gets the starting point.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

void
avtStreamlineWrapper::GetStartPoint(pt3d &pt)
{
    avtVec p = sl->PtStart();
    
    pt.xyz[0] = p.values()[0];
    pt.xyz[1] = p.values()[1];
    pt.xyz[2] = p.values()[2];
}


// ****************************************************************************
//  Method: avtStreamlineWrapper::GetEndPoint
//
//  Purpose:
//      Gets the ending point.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

void
avtStreamlineWrapper::GetEndPoint(pt3d &pt)
{
    avtVec ptBwd(0,0,0), ptFwd(0,0,0);
    
    sl->PtEnds(ptBwd, ptFwd);
    if (dir == FWD)
    {
        pt.xyz[0] = ptFwd.values()[0];
        pt.xyz[1] = ptFwd.values()[1];
        pt.xyz[2] = ptFwd.values()[2];
    }
    else
    {
        pt.xyz[0] = ptBwd.values()[0];
        pt.xyz[1] = ptBwd.values()[1];
        pt.xyz[2] = ptBwd.values()[2];
    }
    
    //debug1<<"avtStreamlineWrapper::GetEndPoint() = ["<<pt.xyz[0]<<" "
    //      <<pt.xyz[1]<<" "<<pt.xyz[2]<<"]"<<endl;
}


// ****************************************************************************
//  Method: avtStreamlineWrapper::UpdateDomainCount
//
//  Purpose:
//      Updates the domain count.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

void
avtStreamlineWrapper::UpdateDomainCount(int dom)
{
    if (dom+1 > domainVisitCnts.size())
        domainVisitCnts.resize(dom+1, 0);

    domainVisitCnts[dom]++; 
}


// ****************************************************************************
//  Method: avtStreamlineWrapper::ComputeStatistics
//
//  Purpose:
//      Computes statistics on the number of domains visited.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

void
avtStreamlineWrapper::ComputeStatistics()
{
    maxCnt = 0;
    sum = 0;
    numDomainsVisited = 0;
    for (int i = 0; i < domainVisitCnts.size(); i++)
    {
        int cnt = domainVisitCnts[i];
        if (cnt > maxCnt)
            maxCnt = cnt;
        if (cnt > 0)
            numDomainsVisited++;
        sum += cnt;
    }
}


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
// ****************************************************************************

avtStreamlineFilter::avtStreamlineFilter()
{
    normalizedVecExprName = "";
    maxStepLength = 0.;
    terminationType = STREAMLINE_TERMINATE_DISTANCE;
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
    haveGhostZones = false;
    intervalTree = NULL;
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
// ****************************************************************************

avtStreamlineFilter::~avtStreamlineFilter()
{
    std::map<int, vtkVisItCellLocator*>::const_iterator it;
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
        int proc = DomainToRank(dom);
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
// ****************************************************************************

void
avtStreamlineFilter::SetDomain(avtStreamlineWrapper *slSeg)
{
    pt3d endPt;
    slSeg->GetEndPoint(endPt);
    
    slSeg->seedPtDomainList.resize(0);
    intervalTree->GetElementsListFromRange(endPt.xyz, endPt.xyz, 
                                           slSeg->seedPtDomainList);

    slSeg->domain = -1;
    // 1 domain, easy.
    if (slSeg->seedPtDomainList.size() == 1)
        slSeg->domain = slSeg->seedPtDomainList[0];

    // Point in multiple domains. See if we can shorten the list by 
    //looking at "my" domains.
    else if (slSeg->seedPtDomainList.size() > 1)
    {
        // See if the point is contained in a domain owned by "me".
        vector<int> newDomList;
        bool foundOwner = false;
        for (int i = 0; i < slSeg->seedPtDomainList.size(); i++)
        {
            int dom = slSeg->seedPtDomainList[i];
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
// ****************************************************************************

vtkDataSet *
avtStreamlineFilter::GetDomain(int domain)
{
    int timerHandle = visitTimer->StartTimer();
    if (! DomainLoaded(domain))
        numDomainsLoaded++;

    vtkDataSet *ds = NULL;

    if (OperatingOnDemand())
        ds = avtDatasetOnDemandFilter::GetDomain(domain);
    else
        ds = dataSets[domain];
    
    debug1<<"GetDomain("<<domain<<") = "<<ds<<endl;
    
    IOTime += visitTimer->StopTimer(timerHandle, "GetDomain()");
    return ds;
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
// ****************************************************************************

bool
avtStreamlineFilter::DomainLoaded(int domain) const
{
    //debug1<< "avtStreamlineFilter::DomainLoaded("<<domain<<");\n";
#ifdef PARALLEL
    if (OperatingOnDemand())
        return avtDatasetOnDemandFilter::DomainLoaded(domain);

    return rank == domainToRank[domain];
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
// ****************************************************************************

void
avtStreamlineFilter::SetTermination(int type, double term)
{
    terminationType = type;
    termination = term;
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
// ****************************************************************************

void
avtStreamlineFilter::SetStreamlineAlgorithm(int algo, int maxCnt, int domCache)
{
    method = algo;
    maxCount = maxCnt;
    cacheQLen = domCache;
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
// ****************************************************************************

void
avtStreamlineFilter::Execute(void)
{
    debug1 << "********************avtStreamlineFilter::Execute(void)\n";
    int executeFunc = visitTimer->StartTimer();
    int wallTimer = visitTimer->StartTimer();

    Initialize();

    vector<avtStreamlineWrapper *> seedpoints;

    GetSeedPoints(seedpoints);
    numSeedPts = seedpoints.size();


#ifdef PARALLEL
    SetMaxQueueLength(cacheQLen);

    if (method == STREAMLINE_STAGED_LOAD_ONDEMAND)
        StagedLoadOnDemand(seedpoints);
    else if (method == STREAMLINE_PARALLEL_STATIC_DOMAINS)
        ParallelBalancedStaticDomains(seedpoints, true, MAX_CNT);   
    else if (method == STREAMLINE_MASTER_SLAVE)
        MasterSlave(seedpoints);

    //    else if (method == STATIC_DOMAINS)
    //        ParallelBalancedStaticDomains(seedpoints, false);
    //    else if (method == BALANCED_STATIC_DOMAINS)
    //        ParallelBalancedStaticDomains(seedpoints, false, MAX_CNT);
    //else if (method == ASYNC_BALANCED_STATIC_DOMAINS)
    //        ParallelBalancedStaticDomains(seedpoints, true, MAX_CNT);
    //    else if (method == BALANCED_LOAD_ONDEMAND)
    //        ParallelBalancedLoadOnDemand(seedpoints, loadFactor, MAX_CNT, 
    //                                     maxCount);
    //    else if (method == ASYNC_BALANCED_LOAD_ONDEMAND)
    //        ParallelBalancedStaticDomains(seedpoints, true, MAX_CNT, true);
#else
    SetMaxQueueLength(cacheQLen);
    StagedLoadOnDemand(seedpoints);
#endif
    
    totalTime = visitTimer->StopTimer(executeFunc, 
                                      "avtStreamlineFilter::Execute()");
    wallTime = visitTimer->StopTimer(wallTimer, "Execute");

    //FinalizeStatistics();
    //ReportTimings();

    delete intervalTree;
    intervalTree = NULL;
}

// ****************************************************************************
//  Method: avtStreamlineFilter::InitStatistics
//
//  Purpose:
//      Initialize code to collect statistics.
//
//  Programmer: Dave Pugmire
//  Creation:   June 23, 2008
//
// Modifications:
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Add busyloop and sleep statistics.
//
// ****************************************************************************

void
avtStreamlineFilter::InitStatistics()
{
    // counts.
    numDomainsLoaded = 0;
    numSLCommunicated = 0;
    numStatusCommunicated = 0;
    numIntegrationSteps = 0;
    numIterations = 0;
    numBusyLoopIterations = 0;
    numBytesSent = 0;
    
    //timers.
    integrationTime = 0.0;
    communicationTime = 0.0;
    IOTime = 0.0;
    gatherTime1 = 0.0;
    gatherTime2 = 0.0;
    asyncSLTime = 0.0;
    asyncTermTime = 0.0;
    asyncSendCleanupTime = 0.0;
    totalTime = 0.0;
    sortTime = 0.0;
    sleepTime = 0.0;
}

static void
computeMeanStdDev( int nProcs, int rank, float val, float &mean, float &stdDev )
{
    mean = 0.0;
    stdDev = 0.0;
    
#ifdef PARALLEL
    float *input = new float[nProcs], *output = new float[nProcs];

    for (int i = 0; i < nProcs; i++)
        input[i] = 0.0;
    input[rank] = val;
    
    SumFloatArrayAcrossAllProcessors(input, output, nProcs);
    mean = output[0];
    for (int i = 1; i < nProcs; i++)
        mean += output[i];
    mean /= (float)nProcs;

    float sum = 0.0;
    for (int i = 0; i < nProcs; i++)
    {
        float x = output[i] - mean;
        sum += x*x;
    }
    sum /= (float)nProcs;
    stdDev = sqrt(sum);

    delete [] input;
    delete [] output;
#endif
}

// ****************************************************************************
//  Method: avtStreamlineFilter::ComputeStatistics
//
//  Purpose:
//      Compute statistics over a value.
//
//  Programmer: Dave Pugmire
//  Creation:   Dec 12, 2008
//
// Modifications:
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Overhaul how statistics are computed. Add mean and std deviation.
//
// ****************************************************************************

void
avtStreamlineFilter::ComputeStatistics( float val,
                                        SLStatistics &stats,
                                        bool skipMaster)
{
    stats.min = val;
    stats.max = val;
    stats.mean = val;
    stats.sigma = 0.0;
    stats.total = val;
    stats.value = val;
#ifdef PARALLEL
    float *input = new float[nProcs], *output = new float[nProcs];

    for (int i = 0; i < nProcs; i++)
        input[i] = 0.0;
    input[rank] = val;
    
    SumFloatArrayAcrossAllProcessors(input, output, nProcs);
    
    stats.total = 0.0;
    for (int i = 0; i < nProcs; i++)
        stats.total += output[i];
    if (skipMaster)
    {
        stats.total -= output[0];
        stats.mean = stats.total / (float)(nProcs-1);
    }
    else
        stats.mean = stats.total / (float)nProcs;

    float sum = 0.0;
    for (int i = 0; i < nProcs; i++)
    {
        if (skipMaster && i == 0)
            continue;
        float x = output[i] - stats.mean;
        sum += (x*x);
    }
    if (skipMaster)
        sum /= (float)(nProcs-1);
    else
        sum /= (float)nProcs;
    stats.sigma = sqrt(sum);

    vector<float> arr(nProcs,0.0);
    for (int i = 0; i < nProcs; i++)
        arr[i] = output[i];
    
    sort(arr.begin(), arr.end());
    stats.min = arr[0];
    
    if (skipMaster)
        stats.min = arr[1];
    stats.max = arr[arr.size()-1];

    debug1<<"[";
    for(int i=0; i<nProcs; i++) debug1<<output[i]<<" ";
    debug1<<"]\n";
    debug1<<"Stats: "<<stats.min<<", "<<stats.max<<", "<<stats.mean<<", "<<setw(10)<<setprecision(6)<<stats.sigma<<" T: "<<stats.total<<endl;

    delete [] input;
    delete [] output;
#endif
}

// ****************************************************************************
//  Method: avtStreamlineFilter::FinalizeStatistics
//
//  Purpose:
//      Finalize calculation of statistics.
//
//  Programmer: Dave Pugmire
//  Creation:   June 23, 2008
//
// Modifications:
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Handle additional statistics.
//
// ****************************************************************************

void
avtStreamlineFilter::FinalizeStatistics()
{
    totalNumDomainsLoaded = numDomainsLoaded;
    totalNumSLCommunicated = numSLCommunicated;
    totalNumStatusCommunicated = numStatusCommunicated;
    totalNumIntegrationSteps = numIntegrationSteps;
    totalNumIterations = numIterations;
    totalNumBusyLoopIterations = numBusyLoopIterations;
    totalNumBytesSent = numBytesSent;

    //Timing statistics.
    ComputeStatistics(totalTime, totalTimeStats);
    debug1<<"INT TIME\n";
    ComputeStatistics(integrationTime, integrationTimeStats, (method==STREAMLINE_MASTER_SLAVE));
    debug1<<"IO TIME\n";
    ComputeStatistics(IOTime,IOTimeStats, (method==STREAMLINE_MASTER_SLAVE));
    ComputeStatistics(communicationTime, commTimeStats);
    ComputeStatistics(sleepTime, sleepTimeStats,(method==STREAMLINE_MASTER_SLAVE));
    ComputeStatistics(sortTime, sortTimeStats);

    //Operation statistics.
    debug1<<"LOAD DOM\n";
    ComputeStatistics((float)numDomainsLoaded, loadDomStats, (method==STREAMLINE_MASTER_SLAVE));
    ComputeStatistics((float)numSLCommunicated, slCommStats);
    ComputeStatistics((float)numStatusCommunicated, statusCommStats);

    ComputeStatistics((float)numIterations, iterationStats);
    debug1<<"INT STEPS\n";
    ComputeStatistics((float)numIntegrationSteps, intStepStats,  (method==STREAMLINE_MASTER_SLAVE));
    ComputeStatistics((float)numBusyLoopIterations, busyLoopStats);
    ComputeStatistics((float)numBytesSent, bytesSentStats);
}


// ****************************************************************************
//  Function: ToString
//
//  Purpose:
//      Convert arguments to a string.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// Modifications:
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Add masterslave algorithm.
//   
// ****************************************************************************

static string
ToString(int method, int maxCount, double loadFactor, double underWorkedFactor, int balanceNumToSend)
{
    string str;
    char buf[1024];
    if (method ==  STREAMLINE_STAGED_LOAD_ONDEMAND)
        sprintf(buf, "StagedLoadOnDemand");
    else if (method == STREAMLINE_PARALLEL_STATIC_DOMAINS)
        sprintf(buf, "ASYNC_BalancedStatic:\n\tmaxCnt = %d", maxCount);
    else if (method == STREAMLINE_MASTER_SLAVE)
        sprintf(buf, "Master_Slave:\n\tmaxCnt = %d", maxCount);
    
    str = buf;
    return str;
}


// ****************************************************************************
//  Method: avtStreamlineFilter::ReportTimings
//
//  Purpose:
//      Output timings to a stream.
//
//  Programmer: Dave Pugmire
//  Creation:   April 4, 2008
// Modifications:
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Overhaul how timings are reported.
//
// ****************************************************************************

void
avtStreamlineFilter::ReportTimings(ostream &os)
{
    os << endl << endl;
    os << "Method = " 
       << ToString(method, maxCount, loadFactor, 
                   underWorkedFactor, balanceNumToSend)<<endl;

    os << "NumPts = " << numSeedPts << " NCPUs = " << PAR_Size() 
       << " nDomains = " << numDomains << " domainQLen = " << cacheQLen << endl;

    os << "****** Total stats:" << endl;
    //Domains loaded.
    os <<"Dom Loaded   = " <<setw(6)<< (int)loadDomStats.total
       <<" mMA ["<<(int)loadDomStats.min<<", "
       <<(int)loadDomStats.max<<", "
       <<loadDomStats.mean<<"]"
       <<" s= "<<loadDomStats.sigma
       <<" : dom/SL = "<<(float)loadDomStats.total / (float)numSeedPts<<endl;
    
    //SL Comm
    os << "SL Comm      = " <<setw(6)<< (int)slCommStats.total
       << " mMA ["<<(int)slCommStats.min<<", "
       <<(int)slCommStats.max<<", "
       <<slCommStats.mean<<"]"
       <<" s= "<<slCommStats.sigma
       <<" : comm / SL = "<< slCommStats.total / (float)numSeedPts<<endl;

    //Status
    os << "Status Comm  = " <<setw(6)<< statusCommStats.total
       << " mMA ["<<statusCommStats.min<<", "
       <<statusCommStats.max<<", "
       <<statusCommStats.mean<<"]"
       <<" s= "<<statusCommStats.sigma
       <<" : comm / SL = " 
       << (float)statusCommStats.total / (float)numSeedPts <<endl;
  
    os <<"Integrate    = " <<setw(6)<< intStepStats.total
       <<" mMA ["<<(int)intStepStats.min<<", "
       <<(int)intStepStats.max<<", "
       <<intStepStats.mean<<"]"
       <<" s= "<<intStepStats.sigma
       << " mMA ["<<100.0*intStepStats.min/intStepStats.total
       << ", "<<100.0*intStepStats.max/intStepStats.total
       <<", "<<100.0*intStepStats.mean/intStepStats.total<<" %]"
       <<" s= "<<100.0*intStepStats.sigma/intStepStats.total<<" %"
       <<" intg / SL = " 
       <<(float)intStepStats.total / (float)numSeedPts <<endl;

    os <<"Iters        = " <<setw(6)<< iterationStats.total
       <<" mMA ["<<(int)iterationStats.min<<", "
       <<(int)iterationStats.max<<", "
       <<(float)iterationStats.mean<<"]"
       <<" s= "<<iterationStats.sigma
       <<" : it / SL = " << (float)totalNumIterations / (float)numSeedPts 
       << endl;
    os << "Busy Loop    = " <<setw(6)<< busyLoopStats.total
       << " mMA ["<<(int)busyLoopStats.min<<", "<<(int)busyLoopStats.max
       << ", "<<busyLoopStats.mean<<"]"
       << " s= "<<busyLoopStats.sigma
       <<" : it / SL = " << (float)busyLoopStats.total / (float)numSeedPts 
       << endl;
    float nKBSent = (float)bytesSentStats.total/(1024.0);
    os <<"N KBSent     = " <<setw(6)<<(int)nKBSent
       <<" mMA ["<<bytesSentStats.min/1024.0<<", "
       <<bytesSentStats.max/1024.0<<", "
       <<bytesSentStats.mean/1024.0<<"] "
       <<" s= "<<bytesSentStats.sigma/1024.0
       <<" kb / sl = "<<nKBSent / (float)numSeedPts <<endl;
    os << "SingleSLComm: Max = " <<totalMaxSLCommunications<<endl;
    
    os << "****** Total timings:" << endl;
    os << "WallTime : " << wallTime << endl;
    

    os <<"TotalTime: " << totalTimeStats.total
       <<" mMA ["<<totalTimeStats.min<<", "
       <<totalTimeStats.max<<", "
       <<totalTimeStats.mean<<"]"
       <<" s= "<<totalTimeStats.sigma
       <<" : T / SL = "
       << totalTimeStats.total / (float)numSeedPts << endl;
    os <<"TotalIntg: " << integrationTimeStats.total
       <<" = "<<100.0*(integrationTimeStats.total/totalTimeStats.total) << " %"
       <<" mMA ["<<integrationTimeStats.min<<", "
       <<integrationTimeStats.max<<", "
       <<integrationTimeStats.mean<<"]"
       <<" s= "<<integrationTimeStats.sigma
       << " : T / SL = " << integrationTimeStats.total / (float)numSeedPts << endl;

    os <<"TotalIO  : " << IOTimeStats.total
       <<" = "<<100.0*(IOTimeStats.total/totalTimeStats.total) << " %"
       <<" mMA ["<<IOTimeStats.min<<", "
       <<IOTimeStats.max<<", "
       <<IOTimeStats.mean<<"]"
       <<" s= "<<IOTimeStats.sigma
       <<" : T / SL = "<< IOTimeStats.total / (float)numSeedPts << endl;
    os << "TotalComm: " << commTimeStats.total
       << " = "<<100.0*(commTimeStats.total/totalTimeStats.total) << " %"
       <<" mMA ["<<commTimeStats.min<<", "
       <<commTimeStats.max<<", "
       <<commTimeStats.mean<<"]"
       <<" s= "<<commTimeStats.sigma
       <<" : T / SL = "<< commTimeStats.total / (float)numSeedPts << endl;

    os << "TotalSort: " << sortTimeStats.total
       <<" = "<<100.0*(sortTimeStats.total/totalTimeStats.total) << " %" 
       << " : T / SL = " << sortTimeStats.total / (float)numSeedPts << endl;
    
    os << "TotalSleep: " << sleepTimeStats.total << " = " 
       << 100.0*(sleepTimeStats.total/totalTimeStats.total) << " % "
       << "mMA ["<<sleepTimeStats.min<<", "
       <<sleepTimeStats.max<<", "
       <<sleepTimeStats.mean<<"]"
       <<" s= "<<sleepTimeStats.sigma
       << " : T / SL = " << sleepTimeStats.total / (float)numSeedPts << endl;

    float extraTime = totalTimeStats.total - integrationTimeStats.total - IOTimeStats.total
                    - commTimeStats.total - sortTimeStats.total - sleepTimeStats.total;

    os << "ExtraTime: " <<extraTime<<" : [ " 
       << 100.0*(extraTime/totalTimeStats.total)<<" %]"<<endl;
    os << endl;

    os << "Per process timings"<<endl;
    os << "NDomainsLoaded  = " << loadDomStats.value << " ";
    os << "NSLCommunications = " << slCommStats.value << " ";
    os << "NStatComms = " << statusCommStats.value << " ";
    os << "SingleSLCommunication: Max = "<< maxSLCommunications << endl;
    nKBSent = (float)bytesSentStats.value / (1024.0);
    os << "NKBytesSent = " << nKBSent << " ";
    os << "NIntegrations = " << intStepStats.value;
    os << " ["<<100.0*intStepStats.value/intStepStats.total<<"%]"<<endl;
    os << "NIterations = " << iterationStats.value << " ";
    os << "NBusyLoopIterations = " << busyLoopStats.value << endl;
    os << "TotalTime = " << totalTimeStats.value << ", ";
    os << "Comm: " << commTimeStats.value << " [" 
       << 100.0 * (commTimeStats.value/totalTimeStats.value)  << "]%, ";
    os << "IO: " << IOTime << " [" << 100.0 * (IOTimeStats.value/totalTimeStats.value)  << "]%, ";
    os << "****Int: " << integrationTimeStats.value << " [" 
       << 100.0 * (integrationTimeStats.value/totalTimeStats.value) << "]% ";
    os << "Sleep: " << sleepTimeStats.value << " [" 
       << 100.0 * (sleepTimeStats.value/totalTimeStats.value) << "]% ";
    os << endl;
    
    extraTime = totalTimeStats.value - commTimeStats.value - IOTimeStats.value 
              - integrationTimeStats.value - sortTimeStats.value - sleepTimeStats.value;
    os << "ExtraTime: " << extraTime << " [" << 100.0*extraTime/totalTimeStats.value 
       <<" %]" << endl;
    os << endl;
}


// ****************************************************************************
//  Method: avtStreamlineFilter::ReportTimings
//
//  Purpose:
//      Set up a file and output timings to it.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

void
avtStreamlineFilter::ReportTimings()
{
    char f[128];
    sprintf(f, "timings%03d.txt", PAR_Rank());
    ofstream os;
    os.open(f, ios::out);

    ReportTimings(os);
    os.close();
    if (PAR_Rank() == 0)
        ReportTimings(cout);
}
    

// ****************************************************************************
//  Method: avtStreamlineFilter::ReportStatistics
//
//  Purpose:
//      Report statistics about number of domains per processor.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

void
avtStreamlineFilter::ReportStatistics(
                                   vector<avtStreamlineWrapper *> &streamlines)
{
    debug1 << "Streamline statistics: "<<streamlines.size()<<endl;
    maxSLCommunications = 0;
    int domainVisitCnt = 0;
    
    for (int i = 0; i < streamlines.size(); i++)
    {
        avtStreamlineWrapper *slSeg = streamlines[i];
        if (slSeg->numTimesCommunicated > maxSLCommunications)
            maxSLCommunications = slSeg->numTimesCommunicated;
        
        slSeg->ComputeStatistics();
        domainVisitCnt += slSeg->numDomainsVisited;
    }
    
    float avgDomainsPerSL = (float)domainVisitCnt / (float)streamlines.size();
    debug1 << "average domains per SL: " << avgDomainsPerSL << endl;
}


// ****************************************************************************
//  Method: avtStreamlineFilter::Initialize
//
//  Modifications:
//
//    Hank Childs, Mon Jul 21 13:09:13 PDT 2008
//    Remove the "area code" from the initialization so it will compile on
//    my box.
//
//   Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//   Add dataSpatialDimension
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Add statusMsgSz.
//
//
// ****************************************************************************

void
avtStreamlineFilter::Initialize()
{
    InitStatistics();
    dataSpatialDimension = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();

    int timerHandle = visitTimer->StartTimer();

    // Get/Compute the interval tree.
    avtIntervalTree *it_tmp = GetMetaData()->GetSpatialExtents();
    if (it_tmp == NULL)
    {
        if (OperatingOnDemand())
        {
            // It should be there, or else we would have precluded 
            // OnDemand processing in the method CheckOnDemandViability.
            // Basically, this should never happen, so throw an exception.
            EXCEPTION0(ImproperUseException);
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
    rank = PAR_Rank();
    nProcs = PAR_Size();
    statusMsgSz = numDomains+2;
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
        numDomainsLoaded += ds_list.domains.size();

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

    IOTime += visitTimer->StopTimer(timerHandle, "GetDomain()");

    /*
    debug1<< "Domain/Data setup:\n";
    for (int i = 0; i < numDomains; i++)
        debug1<<i<<": rank= "<< domainToRank[i]<<" ds= "<<dataSets[i]<<endl;
    */

    // Some methods need random number generator.
    srand(2776724); //My childhood phone number...
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
// ****************************************************************************

bool
avtStreamlineFilter::PointInDomain(pt3d &pt, int domain)
{
    //debug1<< "avtStreamlineFilter::PointInDomain(["<<pt.xyz[0]<<" "
    //      <<pt.xyz[1]<<" "<<pt.xyz[2]<<"], dom= "<<domain<<");\n";
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
        intervalTree->GetElementExtents(domain, bbox);
        //debug1<<"[ "<<bbox[0]<<" "<<bbox[1]<<" ] [ "<<bbox[2]<<" "<<bbox[3]<<" ] [ "<<bbox[4]<<" "<<bbox[5]<<" ]"<<endl;
        if (pt.xyz[0] < bbox[0] || pt.xyz[0] > bbox[1] ||
            pt.xyz[1] < bbox[2] || pt.xyz[1] > bbox[3])
        {
            return false;
        }
        
        if(dataSpatialDimension == 3 &&
           (pt.xyz[2] < bbox[4] || pt.xyz[2] > bbox[5]))
        {
            return false;
        }

        //If we don't have ghost zones, then we can rest assured that the
        //point is in this domain. For ghost zones, we have to check cells.
        if (ds->GetCellData()->GetArray("avtGhostZones") == NULL)
            return true;
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
    double p[3] = {pt.xyz[0],pt.xyz[1],pt.xyz[2]}, resPt[3]={0.0,0.0,0.0};
    int foundCell = -1, subId = 0;
    int success = cellLocator->FindClosestPointWithinRadius(p, rad, resPt, 
                                                       foundCell, subId, dist);
    debug1<< "suc = "<<success<<" dist = "<<dist<<" resPt= ["<<resPt[0]
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
// ****************************************************************************

bool
avtStreamlineFilter::OwnDomain(int domain)
{
#ifdef PARALLEL
    if (OperatingOnDemand())
        return true;
    return rank == DomainToRank(domain);
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
avtStreamlineFilter::DomainToRank(int domain)
{
    // First time through, compute the mapping.
    if (domainToRank.size() == 0)
        ComputeDomainToRankMapping();

    if (domain < 0 || domain >= domainToRank.size())
        EXCEPTION1(ImproperUseException, "Domain out of range.");
    
    //debug1<<"avtStreamlineFilter::DomainToRank("<<domain<<") = "<<domainToRank[domain]<<endl;

    return domainToRank[domain];
}

#if 0

// ****************************************************************************
//  Method: avtStreamlineFilter::AsynchronousParallelStaticDomains
//
//  Purpose:
//      Calculates a streamline with ParallelStaticDomains algorithm.
//      Each processor owns a fixed set of domains. When a streamline exists a
//      domain owned by the current processor, it is communicated to the 
//      processor that owns the domain.
//
//  Programmer: Dave Pugmire
//  Creation:   March 4, 2008
//
// ****************************************************************************

void
avtStreamlineFilter::AsynchronousParallelStaticDomains(
                                              std::vector<pt3d> &allSeedpoints)
{
    debug1 << "avtStreamlineFilter::AsynchronousParallelStaticDomains()"<<endl;
#ifdef PARALLEL
    debug1 << "Seedpoint cnt = " << allSeedpoints.size() << endl;

    //Get "my" seed points.
    vector<avtStreamlineWrapper *> streamlines;
    for (int i = 0; i < allSeedpoints.size(); i++)
    {
        vector<int> dl;
        intervalTree->GetElementsListFromRange(allSeedpoints[i].values(), 
                                               allSeedpoints[i].values(), dl);
        if (dl.size() > 0 && OwnDomain(dl[0]))
        {
            avtStreamlineWrapper *slSeg = 
                                new avtStreamlineWrapper(allSeedpoints[i], i);
            streamlines.push_back(slSeg);
        }
    }
    debug1 << "My seedpoints: " << streamlines.size() << endl;

    // MPI communications
    const int StreamlineXferReqTag = 42000;
    const int StreamlinesTerminatedTag = 420001;
#ifdef LONG_PROTOCOL
    const int StreamlineTagStart = 42003;
#endif
    
    // Post non-blocking receives
    int terminateMsgBuffer[1]; // Number of streamlines that terminated
#ifdef LONG_PROTOCOL
    int streamlineXferReqMsgBuffer[2]; // Size of streamline in bytes 
                                       // and tag of actual streamline send
#else
    const int maxStreamlineMemSize = 10*1024*1024;
    unsigned char *streamlineXferBuffer = 
                                       new unsigned char[maxStreamlineMemSize];
#endif
    MPI_Request terminateRecvRequest;
    MPI_Irecv(static_cast<void*>(terminateMsgBuffer), 1, MPI_INT, 
              MPI_ANY_SOURCE, StreamlinesTerminatedTag, VISIT_MPI_COMM, 
              &terminateRecvRequest);
    MPI_Request xferReqRecvRequest;
#ifdef LONG_PROTOCOL
    MPI_Irecv(static_cast<void*>(streamlineXferReqMsgBuffer), 2, MPI_INT, 
              MPI_ANY_SOURCE, StreamlineXferReqTag, VISIT_MPI_COMM, 
              &xferReqRecvRequest);
#else
    MPI_Irecv(static_cast<void*>(streamlineXferBuffer), maxStreamlineMemSize,
              MPI_CHAR, MPI_ANY_SOURCE, StreamlineXferReqTag, VISIT_MPI_COMM, 
              &xferReqRecvRequest);
#endif

    std::map<MPI_Request, unsigned char*> requestToBufferMap;

    // Keep working while there is work to do.
    int numActiveStreamlines = allSeedpoints.size();
    int numTerminatedSinceLastBroadcast = 0;

    while (numActiveStreamlines)
    {
        debug1 << "There are " << numActiveStreamlines 
               << " streamlines active. I have " << streamlines.size()
               << " streamlines." << std::endl;
        for (int i = 0; i < streamlines.size(); i++)
        {
            avtStreamlineWrapper *slSeg = streamlines[i];
            //debug1 << i << ": Integrate seedPt " << slSeg->seedPt.xyz[0]<<" "
            //       << slSeg->seedPt.xyz[1] <<" "<<slSeg->seedPt.xyz[2]<<endl;
            IntegrateStreamline(slSeg, false);
            debug1 << "Back from IntegrateStreamline. Pts = : " 
                   << slSeg->slpts.size();

            // After integration, streamline is either terminated, 
            // or needs a new domain.
            if (slSeg->status == TERMINATE)
            {
                debug1 << " -> Terminated." << std::endl;
                terminatedStreamlines.push_back(slSeg);
                numTerminatedSinceLastBroadcast++;
            }
            else if (slSeg->status == OUTOFBOUND)
            {
                int communicationTimer = visitTimer->StartTimer();
                int domainOwner = DomainToRank(slSeg->domain);
                debug1 << "-> Exit -> Pass to " << slSeg->domain << " rank = " 
                       << domainOwner << endl;
                int slSegSize = slSeg->NumBytesToSerialize();
#ifdef LONG_PROTOCOL
                int xferMsg[2] = { StreamlineTagStart + slSeg->uniqueTag, 
                                   slSegSize }; 
                unsigned char *mpiMsg = new unsigned char[sizeof(xferMsg)];
                memcpy(mpiMsg, xferMsg, sizeof(xferMsg));
                // Send the xfer request
                MPI_Request  reqId;
                //debug1 << "Sending xfer request {" << xferMsg[0] << ", " 
                //       << xferMsg[1] << "}" << std::endl;
                MPI_Isend(static_cast<void*>(mpiMsg), 2, MPI_INT, domainOwner, 
                          StreamlineXferReqTag, VISIT_MPI_COMM, &reqId);
                requestToBufferMap[reqId] = mpiMsg;

                // Send the actual streamline
                mpiMsg = new unsigned char[slSegSize];
                slSeg->Serialize(mpiMsg);

                //debug1 << "Sending streamline with tag " 
                //       << StreamlineTagStart + slSeg->uniqueTag << std::endl;
                MPI_Isend(static_cast<void*>(mpiMsg), slSegSize, MPI_CHAR, 
                          domainOwner, StreamlineTagStart + slSeg->uniqueTag, 
                          VISIT_MPI_COMM, &reqId);
                requestToBufferMap[reqId] = mpiMsg;
#else
                if (slSegSize <= maxStreamlineMemSize)
                {
                    unsigned char *mpiMsg = new unsigned char[slSegSize];
                    slSeg->Serialize(mpiMsg);
                    MPI_Request  reqId;
                    MPI_Isend(static_cast<void*>(mpiMsg), slSegSize, MPI_CHAR, 
                              domainOwner, StreamlineXferReqTag, 
                              VISIT_MPI_COMM, &reqId);
                    requestToBufferMap[reqId] = mpiMsg;
                }
                else
                {
                    std::cerr << "********************* Streamline too large "
                      << " to transfer. Terminating prematurely." << std::endl;
                    debug1 << "********************* Streamline too large to "
                      << "transfer. Terminating prematurely." << std::endl;
                    terminatedStreamlines.push_back(slSeg);
                    numTerminatedSinceLastBroadcast++;
                }
#endif
                communicationTime += visitTimer->StopTimer(communicationTimer, 
                                                    "StreamlineCommunication");
            }
        }

        // All streamlines processed. Empty the list.
        streamlines.resize(0);

        // Broadcast number of terminated streamlines since last broadcast
        int communicationTimer = visitTimer->StartTimer();

        // There seems to be no-nonblocking broadcast in MPI-1
        if (numTerminatedSinceLastBroadcast)
        {
            debug1 << "Sending notification that " 
                   << numTerminatedSinceLastBroadcast 
                   << " streamlines terminated to processor ";
            for (int procNo=0; procNo < PAR_Size(); ++procNo)
            {
                if (procNo != PAR_Rank()) // Do not send to ourselves
                {
                    debug1 << procNo << " ";
                    unsigned char *mpiMsg = new unsigned char[sizeof(int)];
                    memcpy(mpiMsg, &numTerminatedSinceLastBroadcast, 
                           sizeof(int));
                    MPI_Request reqId;
                    MPI_Isend(static_cast<void*>(mpiMsg), 1, MPI_INT, procNo, 
                              StreamlinesTerminatedTag, VISIT_MPI_COMM, &reqId);
                    requestToBufferMap[reqId] = mpiMsg;
                }
            }
            numActiveStreamlines -= numTerminatedSinceLastBroadcast;
            debug1 << std::endl << "Updating own active streamline counter. "
                   << "There are " << numActiveStreamlines 
                   << " remaining streamlines." << std::endl;
            numTerminatedSinceLastBroadcast = 0;
        }

        // Wait for messages: new work & status updates
        // ... get all pending requests
        int numPending = requestToBufferMap.size() + 2; // all send requests +
                                                        // two receive requests
        MPI_Request *mpiRequests = new MPI_Request[numPending];
        MPI_Request *mpiRequestsSave = new MPI_Request[numPending];
        int currPending = 0;
        mpiRequests[currPending] = mpiRequestsSave[currPending] 
                     = terminateRecvRequest; ++currPending;
        mpiRequests[currPending] = mpiRequestsSave[currPending] 
                     = xferReqRecvRequest; ++currPending;
        for (std::map<MPI_Request, unsigned char*>::const_iterator it =
                  requestToBufferMap.begin()
             ; it != requestToBufferMap.end(); ++it)
        {
            mpiRequests[currPending] = mpiRequestsSave[currPending] 
                    = it->first; ++currPending;
        }

        // ... wait for some to finish
        int idx;
        MPI_Status status;
        int *indices = new int[numPending];
        MPI_Status *statuses = new MPI_Status[numPending];
        int numFinished;
        MPI_Waitsome(numPending, mpiRequests, &numFinished, indices, statuses);

        //debug1 << "There are " << numFinished << " finished MPI requests."
        //       << std::endl;

        for (int i = 0; i < numFinished; ++i)
        {
            int idx = indices[i];
            //debug1 << "Handling " << i << " (Source=" 
            //       << statuses[i].MPI_SOURCE << " Tag=" 
            //       << statuses[i].MPI_TAG << " Error= " 
            //       <<statuses[i].MPI_ERROR << ") ReqId = " 
            //       << mpiRequestsSave[idx] << std::endl;

            // ... handle the message
            if (idx == 0)
            {
                // Handle terminate message
                debug1 << i << ": Received notification that " 
                       << terminateMsgBuffer[0] 
                       << " streamlines terminated. (Source=" 
                       << statuses[i].MPI_SOURCE << " Tag=" 
                       << statuses[i].MPI_TAG << " Error= " 
                       << statuses[i].MPI_ERROR << ") ReqId="  
                       << mpiRequestsSave[idx] << std::endl;
                numActiveStreamlines -= terminateMsgBuffer[0];
                debug1 << "There are " << numActiveStreamlines 
                       << " remaining active streamlines." << std::endl;
                // Post new receive
                MPI_Irecv(static_cast<void*>(terminateMsgBuffer), 1, MPI_INT,
                          MPI_ANY_SOURCE, StreamlinesTerminatedTag, 
                          VISIT_MPI_COMM, &terminateRecvRequest);
            }
            else if(idx == 1)
            {
#ifdef LONG_PROTOCOL
                // Handle streamline transfer
                debug1 << i << ": Received request to transfer a streamline "
                       << " with tag " << streamlineXferReqMsgBuffer[0]  
                       << " ." << std::endl;
                unsigned char *buffer = 
                              new unsigned char[streamlineXferReqMsgBuffer[1]];

                // Receive streamline
                MPI_Recv(static_cast<void*>(buffer), 
                         streamlineXferReqMsgBuffer[1],MPI_CHAR,MPI_ANY_SOURCE,
                         streamlineXferReqMsgBuffer[0], VISIT_MPI_COMM, 
                         MPI_STATUS_IGNORE);
                // ... and add it to our work load
                avtStreamlineWrapper *slSeg = new avtStreamlineWrapper;
                streamlines.push_back(slSeg);
                slSeg->UnSerialize(buffer);

                // Post new receive for exchange
                MPI_Irecv(static_cast<void*>(streamlineXferReqMsgBuffer), 2, 
                          MPI_INT, MPI_ANY_SOURCE, StreamlineXferReqTag, 
                          VISIT_MPI_COMM, &xferReqRecvRequest);
#else
                debug1 << i << ": Received new streamline from processor " 
                       << statuses[i].MPI_SOURCE << std::endl;
                avtStreamlineWrapper *slSeg = new avtStreamlineWrapper;
                streamlines.push_back(slSeg);
                slSeg->UnSerialize(streamlineXferBuffer);

                // Post new receive for exchange
                MPI_Irecv(static_cast<void*>(streamlineXferBuffer), 
                          maxStreamlineMemSize, MPI_CHAR, MPI_ANY_SOURCE,
                          StreamlineXferReqTag, VISIT_MPI_COMM, 
                          &xferReqRecvRequest);
#endif
            }
            else
            {
                // Handle finished send -> free buffer
                //debug1 << i << ": Freeing buffer for finished send." 
                //       << std::endl;
                std::map<MPI_Request, unsigned char*>::iterator it =
                                 requestToBufferMap.find(mpiRequestsSave[idx]);
                if (it != requestToBufferMap.end())
                {
                    delete[] it->second;
                    requestToBufferMap.erase(it);
                    //debug1 << "Freed." << std::endl;
                }
                else
                    debug1 << "Unknown send operation finished!" << std::endl;
            }
        }

        delete[] indices;
        delete[] mpiRequests;
        delete[] mpiRequestsSave;
        
        communicationTime += visitTimer->StopTimer(communicationTimer, 
                                                    "StreamlineCommunication");
    }
        
    int communicationTimer = visitTimer->StartTimer();
    // Cancel pending receives
    MPI_Cancel(&terminateRecvRequest);
    MPI_Cancel(&xferReqRecvRequest);

    // Wait for pending sends and free buffers
    if (requestToBufferMap.size())
    {
        debug1 << "Waiting for pending sends." << std::endl;
        int numPending = requestToBufferMap.size();
        MPI_Request *mpiRequests = new MPI_Request[numPending];
        int currPending = 0;
        for (std::map<MPI_Request, unsigned char*>::const_iterator it = 
              requestToBufferMap.begin(); it != requestToBufferMap.end(); ++it)
            mpiRequests[currPending++] = it->first;
        MPI_Waitall(numPending, mpiRequests, MPI_STATUSES_IGNORE);
        debug1 << "Freeing send buffers." << std::endl;
        for (std::map<MPI_Request, unsigned char*>::const_iterator it = 
              requestToBufferMap.begin(); it != requestToBufferMap.end(); ++it)
            delete[] it->second;
    }
    communicationTime += visitTimer->StopTimer(communicationTimer, 
                                                    "StreamlineCommunication");

    // Create output
    debug1 << "Make output: " << terminatedStreamlines.size() << endl;
    CreateStreamlineOutput(terminatedStreamlines);
    ReportStatistics(terminatedStreamlines);

    for (int i = 0; i < terminatedStreamlines.size(); i++)
    {
        avtStreamlineWrapper *slSeg = terminatedStreamlines[i];
        delete slSeg;
    }
#endif
}
#endif


// ****************************************************************************
//  Method: avtStreamlineFilter::AsyncExchangeStreamlines
//
//  Purpose:
//      Exchange streamlines asynchronously.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
// Modifications:
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Add early terminations flag.
//
// ****************************************************************************

bool
avtStreamlineFilter::AsyncExchangeStreamlines(
                       std::vector<avtStreamlineWrapper *> &streamlines,
                       std::vector< std::vector< avtStreamlineWrapper *> > 
                                                         &distributeStreamlines,
                       int &earlyTerminations )
{
    bool newStreamlines = false;
    earlyTerminations = 0;
#ifdef PARALLEL
    int communicationTimer = visitTimer->StartTimer();    
    // Do the SL sends.
    for (int i = 0; i < nProcs; i++)
    { 
        vector<avtStreamlineWrapper *> &sl = distributeStreamlines[i];
        
        if (i != rank)
            AsyncSendSLs(i, sl);
        else // Pass them to myself....
        {
            for (int j = 0; j < sl.size(); j++)
                streamlines.push_back(sl[j]);
        }
        sl.resize(0);
    }

    // See if there are any recieves....
    int numNewSLs = AsyncRecvStreamlines(streamlines, earlyTerminations);
    newStreamlines = (numNewSLs > 0);

    //Cleanup after the sends.
    for (int p = 0; p < nProcs; p++)
    {
        if (p != rank)
        {
            for (int s = 0; s < distributeStreamlines[p].size(); s++)
                delete distributeStreamlines[p][s];
        }
        distributeStreamlines[p].resize(0);
    }

    communicationTime += visitTimer->StopTimer(communicationTimer, 
                                               "StreamlineCommunication");    
#endif
    return newStreamlines;
}


// ****************************************************************************
//  Method: avtStreamlineFilter::AsyncExchangeStatus
//
//  Purpose:
//      Notify other processors of status (for use when doing asynchronous
//      processing).
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

void
avtStreamlineFilter::AsyncExchangeStatus(int numTerminated, 
                                         int &otherTerminates,
                                         bool recvBalanceInfo, 
                                         bool sendBalanceInfo,
                                         int slCount)
{
#ifdef PARALLEL
    // Send the terminations.
    if (numTerminated > 0 || sendBalanceInfo)
        AsyncSendStatus(numTerminated, sendBalanceInfo, slCount);
    
    AsyncRecvStatus(otherTerminates, recvBalanceInfo);
#endif
}


// ****************************************************************************
//  Method: avtStreamlineFilter::StaticDomainExchangeStreamlines
//
//  Purpose:
//      Exchange streamlines when doing static processing.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

bool
avtStreamlineFilter::StaticDomainExchangeStreamlines(
                          std::vector<avtStreamlineWrapper *> &streamlines,
                          std::vector< std::vector< avtStreamlineWrapper *> > 
                                                        &distributeStreamlines)
{
    bool done = true;

#if 0
#ifdef PARALLEL
    int *cnts = new int[nProcs], *slCount = new int[nProcs*nProcs],
        *gatherSz = new int[nProcs*nProcs];

    //We have integrated everything on this domain. Compute exchange vector.
    //debug1 << "Exchange counts\n";
    for (int i = 0; i < nProcs; i++)
    {
        cnts[i] = distributeStreamlines[i].size();
        //debug1 << i << ": " << cnts[i] << endl;
        for (int j = 0; j < nProcs; j++)
            slCount[i*nProcs+j] = 0;
    }
    int gather = visitTimer->StartTimer();
    MPI_Allgather(cnts, nProcs, MPI_INT, slCount, nProcs, MPI_INT, 
                  VISIT_MPI_COMM);
    gatherTime1 += visitTimer->StopTimer(gather, "MPI_Allgather_1");

    
    /*
    debug1 << "slCount:\n";
    for (int i = 0; i < nProcs; i++)
    {
        for (int j = 0; j < nProcs; j++)
        debug1 << slCount[i*nProcs+j] << " ";
        debug1 << endl;
    }
    */
    
    //We are done if nobody has any streamlines to exchange.
    for (int i = 0; i < nProcs*nProcs; i++)
        if (slCount[i] > 0)
        {
            //debug1 << "Not done! "<< i << " cnt is " << slCount[i] << endl;
            done = false;
            break;
        }
    
    //debug1 << "Done = " << done << endl;
    if (! done)
    {
        // Not done, so compute the size of all our streamlines to pass around.
        for (int i = 0; i < nProcs; i++)
        {
            int sz = 0;
            for (int s = 0; s < distributeStreamlines[i].size(); s++)
                sz += distributeStreamlines[i][s]->NumBytesToSerialize();

            cnts[i] = sz;
            //debug1 << i << ": " << cnts[i] << endl;
            for (int j = 0; j < nProcs; j++)
                gatherSz[i*nProcs+j] = 0;
        }
        
        //debug1 << "Communicate sizes... \n";
        MPI_Allgather(cnts, nProcs, MPI_INT, gatherSz, nProcs, MPI_INT,
                      VISIT_MPI_COMM);

        //Do the sends.
        vector<avtStreamlineWrapper *> sentSL;
        vector< unsigned char *> sendBuffers;
        std::vector<MPI_Request> sendReq;
        int tag = 200;

        // Do the sends.
        for (int i = 0; i < nProcs; i++)
        {
            //debug1 << "I have " << distributeStreamlines[i].size() 
            //       << " for " << i << endl;
            if (distributeStreamlines[i].size() == 0)
                continue;

            if (i == rank) // Send it to "me"
            {
                //debug1 << "  Send " << distributeStreamlines[i].size() 
                //       << " to me.\n";
                for (int j = 0; j < distributeStreamlines[i].size(); j++)
                {
                    avtStreamlineWrapper *slSeg = distributeStreamlines[i][j];
                    streamlines.push_back(slSeg);
                }
            }
            else
            {
                // Send streamline batch to each processor.
                unsigned char *buff = new unsigned char[ cnts[i] ];
                int nBytes = 0;
                sendBuffers.push_back(buff);
                //debug1 << "buff = [" << (void *)buff << "]\n";
                
                for (int j = 0; j < distributeStreamlines[i].size(); j++)
                {
                    avtStreamlineWrapper *slSeg = distributeStreamlines[i][j];
                    slSeg->numTimesCommunicated++;
                    int len = slSeg->Serialize(&buff[nBytes]);
                    //debug1<< j<<": size = " << len << endl;
                    nBytes += len;
                    delete slSeg;
                    
                    numSLCommunicated++;
                }

                //Send this packet of streamlines off.
                MPI_Request req;
                //debug1 << "  Send " << rank << " ==["<< cnts[i] << "]==> " 
                //       << i << endl;
                MPI_Isend(buff, cnts[i], MPI_CHAR, i,tag,VISIT_MPI_COMM, &req);
                numBytesSent += cnts[i];
                sendReq.push_back(req);
            }
            distributeStreamlines[i].resize(0);
        }

        // Do the receives.
        for (int i = 0; i < nProcs; i++)
        {
            int numRecv = slCount[i*nProcs+rank];
            if (i == rank || numRecv == 0)
                continue;
            
            MPI_Status stat;
            int recvSz = gatherSz[i*nProcs+rank];
            
            //debug1 << "Recv " << i << " == " << numRecv << " ["<< recvSz 
            //       << "]==> " << rank << endl;
            unsigned char *buff = new unsigned char[ recvSz ];
            MPI_Recv(buff, recvSz, MPI_CHAR, i, tag, VISIT_MPI_COMM, &stat);
            
            int offset = 0;
            for (int j = 0; j < numRecv; j++)
            {
                avtStreamlineWrapper *slSeg = new avtStreamlineWrapper;
                streamlines.push_back(slSeg);
                int len = slSeg->UnSerialize(&buff[offset], solver);
                //debug1<< j<<": size = " << len << endl;
                offset += len;
                //slSeg->Debug();
            }
        }
        
        // Wait for all the sends to finish.
        if (sendReq.size() > 0)
        {
            vector<MPI_Status> stats(sendReq.size());
            MPI_Waitall(sendReq.size(), &sendReq[0], &stats[0]);
        }

        //All done, clean up.
        for (int i = 0; i < sendBuffers.size(); i++)
        {
            unsigned char *bf = sendBuffers[i];
            delete [] bf;
        }
    }


    delete [] cnts;
    delete [] slCount;
    delete [] gatherSz;
    //debug1 << "All DONE!\n";
#endif
#endif
    return done;
}

#if 0

// ****************************************************************************
//  Method: avtStreamlineFilter::OLD_ParallelBalancedStaticDomains
//
//  Purpose:
//      Calculates a streamline with ParallelStaticDomains algorithm.
//      Each processor owns a fixed set of domains. However, communication is 
//      done more frequently to try and avoid processor idle time. When a 
//      streamline exists on a domain owned by the current processor, it is 
//      communicated to the processor that owns the domain.
//
//  Programmer: Dave Pugmire
//  Creation:   March 17, 2008
//
// ****************************************************************************

void
avtStreamlineFilter::OLD_ParallelBalancedStaticDomains(std::vector<pt3d> &allSeedpoints,
                                                       BalanceType balance,
                                                       int maxPts,
                                                       int divFactor)
{
    debug1 << "avtStreamlineFilter::ParallelBalancedStaticDomains()\n";
#ifdef PARALLEL
    cout << "Seedpoint cnt = " << allSeedpoints.size() << endl;

    //Get "my" seed points.
    vector<avtStreamlineWrapper *> streamlines;
    for (int i = 0; i < allSeedpoints.size(); i++)
    {
        vector<int> dl;
        intervalTree->GetElementsListFromRange(allSeedpoints[i].values(), 
                                               allSeedpoints[i].values(), dl);
        //double x=allSeedpoints[i].xyz[0],y=allSeedpoints[i].xyz[1],
        //       z=allSeedpoints[i].xyz[2];
        //debug1 << "Seed pt " << i << ": [" << x << " " << y << " " << z 
        //       << "] in domain " << dl[0] << " owned by " 
        //       << DomainToRank(dl[0]) << ". Domain count = " << dl.size() 
        //       << endl;
        if (dl.size() > 0 && OwnDomain(dl[0]))
        {
            avtStreamlineWrapper *slSeg = 
                                    new avtStreamlineWrapper(allSeedpoints[i]);
            streamlines.push_back(slSeg);
            //debug1 << "Seed pt: " << slSeg->seedPt.xyz[0] << " " 
            //  << slSeg->seedPt.xyz[1] << " " << slSeg->seedPt.xyz[2] << endl;
        }
    }
    debug1 << "My seedpoints: " << streamlines.size() << endl;

    vector< vector< avtStreamlineWrapper *> > distributeStreamlines;
    distributeStreamlines.resize(nProcs);
    int *cnts = new int[nProcs], *gather = new int[nProcs*nProcs];
    
    // Keep working while there is work to do.
    int cnt = 0;
    while (true)
    {
        cnt++;
        debug1 << "Iteration = " << cnt << ". My SL count = "
               << streamlines.size() << endl;

        // Determine balancing, if any.
        int imax = streamlines.size();
        if (balance == MAX_CNT && (imax > maxPts && maxPts > 0))
            imax = maxPts;
        else if (balance == MAX_PCT && (imax > maxPts && divFactor > 1))
        {
            imax = imax / divFactor;
            if (imax == 0)
                imax = 1;
        }
        else if (balance == NO_BALANCE)
        {
            //Nothing.
        }

        // Start integrating streamlines!
        for (int i = 0; i < imax; i++)
        {
            avtStreamlineWrapper *slSeg = streamlines[i];
            debug1 << i << ": Integrate seedPt " << slSeg->seedPt.xyz[0]<< " "
                   <<slSeg->seedPt.xyz[1] << " " << slSeg->seedPt.xyz[2]<<endl;
            IntegrateStreamline(slSeg, false);
            debug1 << "Back from IntegrateStreamline. Pts = : " 
                   << slSeg->slpts.size() << endl;

            // After integration, streamline is either terminated, 
            // or needs a new domain.
            if (slSeg->status == TERMINATE)
            {
                debug1 << ".....Terminated.\n";
                terminatedStreamlines.push_back(slSeg);
            }
            else if (slSeg->status == OUTOFBOUND)
            {
                int domainOwner = DomainToRank(slSeg->domain);
                debug1 << ".....Exit Pass to " << slSeg->domain << " rank = " 
                       << domainOwner << endl;
                distributeStreamlines[domainOwner].push_back(slSeg);
            }
        }

        // Do the communication step.
        int communicationTimer = visitTimer->StartTimer();

        // For unprocessed streamlines, pass them to myself.
        if (streamlines.size()-imax > 0)
            debug1 << "   Defer count: " << streamlines.size()-imax << endl;
        for (int i = imax; i < streamlines.size(); i++)
        {
            avtStreamlineWrapper *slSeg = streamlines[i];
            distributeStreamlines[rank].push_back(slSeg);
        }
        
        // All streamlines processed. empty the list.
        streamlines.resize(0);

        bool done = StaticDomainExchangeStreamlines(streamlines, 
                                                    distributeStreamlines);
        communicationTime += visitTimer->StopTimer(communicationTimer, 
                                                    "StreamlineCommunication");
        
        if (done)
            break;
    }

    debug1 << "Make output: " << terminatedStreamlines.size() << endl;
    CreateStreamlineOutput(terminatedStreamlines);
    ReportStatistics(terminatedStreamlines);

    delete [] cnts;
    delete [] gather;
    for (int i = 0; i < terminatedStreamlines.size(); i++)
    {
        avtStreamlineWrapper *slSeg = 
                             (avtStreamlineWrapper *) terminatedStreamlines[i];
        delete slSeg;
    }
#endif
}
#endif


// ****************************************************************************
//  Method: avtStreamlineFilter::AsyncStaticDomains
//
//  Purpose:
//      Do asynchronous processing with a static domain decomposition.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Fri Aug 22 14:47:11 EST 2008
//   Memory leak fix.
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Add early terminations flag.
//
//   Dave Pugmire, Tue Dec 23 13:52:42 EST 2008
//   Moved ReportStatistics to this method.
//
// ****************************************************************************

void
avtStreamlineFilter::AsyncStaticDomains(
                            std::vector<avtStreamlineWrapper *> &allSeedpoints)
{
    debug1<<"avtStreamlineFilter::AsyncStaticDomains()\n";

#ifdef PARALLEL
    int totalNumActiveStreamlines = allSeedpoints.size();

    allSLCounts.resize(nProcs,0);
    //Get "my" seed points.
    vector<avtStreamlineWrapper *> streamlines, terminatedStreamlines;
    int numTerminated = 0;

    for (int i = 0; i < allSeedpoints.size(); i++)
    {
        avtStreamlineWrapper *slSeg = allSeedpoints[i];
        pt3d endPt;
        slSeg->GetEndPoint(endPt);
        if (OwnDomain(slSeg->domain))
        {
            if (PointInDomain(endPt, slSeg->domain))
                streamlines.push_back(slSeg);
            else
            {
                numTerminated++;
                delete slSeg;
            }
        }
        else
            delete slSeg;
    }
    
    debug1<< "I have "<<streamlines.size()<<" pts. Early termination= "
          <<numTerminated<<endl;
    
    // Pre-load all "my" domains.
    for (int i = 0; i < numDomains; i++)
        if (rank == DomainToRank(i))
            GetDomain(i);

    // Init asynchronous stuff.
    InitRequests();
    terminationSends.resize(nProcs,-1);
     
    // Keep working while there is work to do.
    bool newStreamlines = false;
    vector< vector< avtStreamlineWrapper *> > distributeStreamlines(nProcs);

    if (numTerminated > 0)
    {
        totalNumActiveStreamlines -= numTerminated;
        int otherTerminates = 0;
        AsyncExchangeStatus(numTerminated, otherTerminates, false, false,
                            streamlines.size());
        totalNumActiveStreamlines -= otherTerminates;
        numTerminated = 0;
    }

    int SLIntegrationCnt = 0;
    while (totalNumActiveStreamlines > 0)
    {
        debug1 << "Iteration = " << numIterations << ". My SL count = "
               << streamlines.size() << ". Total cnt = " 
               << totalNumActiveStreamlines << endl;

        bool domainSetChanged = false, streamlineSetChanged = false;
        int currSLCount = streamlines.size();

        // Determine balancing, if any.
        int imax = streamlines.size();
        if ((imax > maxCount && maxCount > 0))
            imax = maxCount;

        // Start integrating streamlines!
        debug1 << "for(i = 0, " << imax << ")\n";
        for (int i = 0; i < imax; i++)
        {
            avtStreamlineWrapper *slSeg = streamlines[i];
            debug1 << i <<": Integrate seedPt. Dom= " << slSeg->domain 
                   << " Status = "<<slSeg->status<<endl;
            IntegrateStreamline(slSeg);

            SLIntegrationCnt++;
            debug1<< "BACK FROM ISL. status = " << slSeg->status << endl;
            // After integration, streamline is either terminated, or needs 
            // a new domain.
            if (slSeg->status == avtStreamlineWrapper::TERMINATE)
            {
                debug1 << "TERMINATED.\n";
                terminatedStreamlines.push_back(slSeg);
                numTerminated++;
                totalNumActiveStreamlines--;
                streamlineSetChanged = true;
            }
            else if (slSeg->status == avtStreamlineWrapper::OUTOFBOUNDS)
            {
                debug1<<"OUT of bounds.... Figure out what to do.\n";
                //slSeg->Debug();
                
                MemStream buff;
                slSeg->Serialize(MemStream::WRITE, buff, solver);
                bool deleteSLSeg = true;
                
                for (int i = 0; i < slSeg->seedPtDomainList.size(); i++)
                {
                    int domRank = DomainToRank(slSeg->seedPtDomainList[i]);
                    if (domRank == rank)
                    {
                        distributeStreamlines[ rank ].push_back(slSeg);
                        deleteSLSeg = false;
                    }
                    else
                    {
                        streamlineSetChanged = true;
                        avtStreamlineWrapper *newSeg = new avtStreamlineWrapper;
                        buff.rewind();
                        newSeg->Serialize(MemStream::READ, buff, solver);
                        newSeg->domain = slSeg->seedPtDomainList[i];
                        distributeStreamlines[ domRank ].push_back(newSeg);
                    }
                }
                if ( deleteSLSeg )
                    delete slSeg;
            }
        }

        bool integratedStreamlines = false;
        if (imax > 0)
        {
            numIterations++;
            integratedStreamlines = true;
        }

        //At this point, everything between 0 and imax has been processed.
        // Pass everything else to myself.
        if (streamlines.size() > 0)
        {
            for (int i = imax; i < streamlines.size(); i++)
                distributeStreamlines[rank].push_back(streamlines[i]);
            streamlines.resize(0);
        }

        bool justFinished = (imax>0 && distributeStreamlines[rank].size()==0);
        //debug1 << "justFinished = " << justFinished << " imax = "<<imax
        //       << " SLSz = " << distributeStreamlines[rank].size()
        //       << " nTerm = " << numTerminated<<endl;
        
        // Communication....
        int communicationTimer = visitTimer->StartTimer();
        // Pass streamlines that have exited my domains.
        int earlyTerminates;
        bool newStreamlines = AsyncExchangeStreamlines(streamlines, 
                                                       distributeStreamlines,
                                                       earlyTerminates);
        if (newStreamlines)
            streamlineSetChanged = true;
            
        //Send/Recv status.
        int otherTerminates = 0;
        AsyncExchangeStatus(numTerminated, otherTerminates, false, false,
                            streamlines.size());
        totalNumActiveStreamlines -= otherTerminates;
        numTerminated = 0;

        communicationTime += visitTimer->StopTimer(communicationTimer, 
                                                   "Communication");

        int timer = visitTimer->StartTimer();
        // If we have new streamlines, sort them on the domain.
        if (streamlineSetChanged)
            SortStreamlines(streamlines);
        sortTime += visitTimer->StopTimer(timer, "Sorting");
    }

    CleanupAsynchronous();
    debug1 << "Make output: " << terminatedStreamlines.size() << endl;
    
    CreateStreamlineOutput(terminatedStreamlines);
    ReportStatistics(terminatedStreamlines);

    for (int i = 0; i < terminatedStreamlines.size(); i++)
        delete terminatedStreamlines[i];
#endif
}


// ****************************************************************************
//  Method: avtStreamlineFilter::ParallelBalancedStaticDomains
//
//  Purpose:
//      Calculates a streamline with ParallelStaticDomains algorithm.
//      Each processor owns a fixed set of domains. However, communication is 
//      done more frequently to try and avoid processor idle time. When a 
//      streamline exists a domain owned by the current processor, it is 
//      communicated to the processor that owns the domain.
//
//  Programmer: Dave Pugmire
//  Creation:   March 17, 2008
//
// ****************************************************************************

//DRP
void
avtStreamlineFilter::ParallelBalancedStaticDomains(
                           std::vector<avtStreamlineWrapper *> &allSeedpoints,
                           bool asynchronous, BalanceType balance,
                           bool loadOnDemand)
{
    return AsyncStaticDomains(allSeedpoints);

#if 0
    debug1 << "avtStreamlineFilter::ParallelBalancedStaticDomains()\n";
#ifdef PARALLEL
    int totalNumActiveStreamlines = allSeedpoints.size();

    allSLCounts.resize(nProcs,0);
    //Get "my" seed points.
    vector<avtStreamlineWrapper *> streamlines;
    for (int i = 0; i < allSeedpoints.size(); i++)
    {
        avtStreamlineWrapper *slSeg = allSeedpoints[i];
        SetDomain(slSeg);
        
        for (int j = 0; j < slSeg->seedPtDomainList.size(); j++)
        {
            int dom = slSeg->seedPtDomainList[j];
            if (OwnDomain(dom))
            {
                pt3d endPt;
                slSeg->GetEndPoint(endPt);
                
                if (PointInDomain(endPt, dom))
                    streamlines.push_back(slSeg);
            }
        }
    }

    // Pre-load all "my" domains.
    for (int i = 0; i < numDomains; i++)
        if (rank == DomainToRank(i))
            GetDomain(i);

    // Init asynchronous stuff.
    if (asynchronous)
    {
        InitRequests();
        terminationSends.resize(nProcs,-1);
        if (loadOnDemand)
        {
            // Initialize the domainsLoaded array to the static assignemnt.
            domainsLoaded.resize(nProcs);
            for (int i = 0; i < nProcs; i++)
            {
                domainsLoaded[i].resize(numDomains);
                for (int j = 0; j < numDomains; j++)
                    domainsLoaded[i][j] = (DomainToRank(j) == i);
            }
            for (int i = 0; i < nProcs; i++)
            {
                debug1 << i<<": [ ";
                for (int j = 0; j < numDomains; j++)
                {
                    debug1 << " " << domainsLoaded[i][j];
                }
                debug1 << " ]\n";
            }
        }
    }

     
    // Keep working while there is work to do.
    int numTerminated = 0, SLIntegrationCnt = 0;
    bool newStreamlines = false;
    vector< vector< avtStreamlineWrapper *> > distributeStreamlines(nProcs);

    numIterations = 0;
    while (totalNumActiveStreamlines > 0)
    {
        debug1 << "Iteration = " << numIterations << ". My SL count = " 
               << streamlines.size() << ". Total cnt = " 
               << totalNumActiveStreamlines << endl;

        bool domainSetChanged = false, streamlineSetChanged = false;
        int currSLCount = streamlines.size();

        // Determine balancing, if any.
        int imax = streamlines.size();
        if (balance == MAX_CNT && (imax > maxCount && maxCount > 0))
            imax = maxCount;
        /*
        else if (balance == MAX_PCT && (imax > maxPts && divFactor > 1))
        {
            imax = imax / divFactor;
            if (imax == 0)
                imax = 1;
        }
        */
        else if (balance == NO_BALANCE)
        {
            //Nothing.
        }


        // Start integrating streamlines!
        debug1 << "for(i = 0, " << imax << ")\n";
        for (int i = 0; i < imax; i++)
        {
            avtStreamlineWrapper *slSeg = streamlines[i];
            //debug1 << i << ": Integrate seedPt " << slSeg->seedPt.xyz[0]
            //       << " "<< slSeg->seedPt.xyz[1] << " " 
            //       << slSeg->seedPt.xyz[2]<<endl;
            if (loadOnDemand)
            {
                if (!DomainLoaded(slSeg->domain))
                    domainSetChanged = true;
                GetDomain(slSeg->domain);
            }
            
            debug1 << i <<": Integrate seedPt. Dom= " << slSeg->domain 
                   << " Status = "<<slSeg->status<<endl;

            IntegrateStreamline(slSeg);
            //FIX THIS
            EXCEPTION0(ImproperUseException);
            SLIntegrationCnt++;
            
            debug1<< "BACK FROM ISL. status = " << slSeg->status << endl;
            // After integration, streamline is either terminated, 
            // or needs a new domain.
            if (slSeg->status == avtStreamlineWrapper::TERMINATE)
            {
                debug1 << "TERMINATED.\n";
                terminatedStreamlines.push_back(slSeg);
                numTerminated++;
                totalNumActiveStreamlines--;
                streamlineSetChanged = true;
            }
            else if (slSeg->status == avtStreamlineWrapper::OUTOFBOUNDS)
            {
                // Handle this case.... (pass it to ourselves....)
                if (loadOnDemand)
                    EXCEPTION0(ImproperUseException);
                
                vector<int> ranks, doms;
                ComputeRankList(slSeg->seedPtDomainList, ranks, doms);
                for (int i = 0; i < ranks.size(); i++)
                {
                    slSeg->domain = doms[i];
                    distributeStreamlines[ ranks[i] ].push_back(slSeg);
                    if (ranks[i] != rank)
                        streamlineSetChanged = true;
                }
            }
        }

        bool integratedStreamlines = false;
        if (imax > 0)
        {
            numIterations++;
            integratedStreamlines = true;
        }

        //At this point, everything between 0 and imax has been processed.
        // Pass everything else to myself.
        if (streamlines.size() > 0)
        {
            for (int i = imax; i < streamlines.size(); i++)
                distributeStreamlines[rank].push_back(streamlines[i]);
            streamlines.resize(0);
        }
        bool justFinished = (imax>0 && distributeStreamlines[rank].size() == 0);
        debug1 << "justFinished = " << justFinished << " imax = "<<imax
               << " SLSz = " << distributeStreamlines[rank].size()
               << " nTerm = " << numTerminated<<endl;
        
        // Communication....
        int communicationTimer = visitTimer->StartTimer();
        if (asynchronous)
        {
            // Pass streamlines that have exited my domains.
            int earlyTerminates;
            bool newStreamlines = AsyncExchangeStreamlines(streamlines, 
                                                          distributeStreamlines,
                                                           earlyTerminates);
            if (newStreamlines)
                streamlineSetChanged = true;
            
            //Send/Recv status.
            int otherTerminates = 0;

            bool sendBalance = false;
            debug1 << "domainSetChanged = " << domainSetChanged 
                   << " SLSetChanged = " << streamlineSetChanged << endl;
            
            if (loadOnDemand && (domainSetChanged || streamlineSetChanged))
                sendBalance = true;
            
            int numTermToSend = 0;//numTerminated;
            sendBalance = false;
            if (justFinished)
            {
                numTermToSend = numTerminated;
                if (loadOnDemand)
                    sendBalance = true;
            }
            AsyncExchangeStatus(numTermToSend, otherTerminates, loadOnDemand, 
                                sendBalance, streamlines.size());
            if (justFinished)
                numTerminated = 0;
            totalNumActiveStreamlines -= otherTerminates;

            if (loadOnDemand && totalNumActiveStreamlines > 0)
            {
                allSLCounts[rank] = streamlines.size();
                AsyncFigureOutBalancing(streamlines);
            }

            CheckPendingSendRequests();
        }
        else
        {
            bool done = StaticDomainExchangeStreamlines(streamlines, 
                                                        distributeStreamlines);
            if (done)
                totalNumActiveStreamlines = 0;
        }
        communicationTime += visitTimer->StopTimer(communicationTimer, 
                                                   "Communication");

        //Cleanup any streamlines we have sent....
        for (int p = 0; p < nProcs; p++)
        {
            if (p != rank)
            {
                for (int s = 0; s < distributeStreamlines[p].size(); s++)
                    delete distributeStreamlines[p][s];
            }
            distributeStreamlines[p].resize(0);
        }

        int timer = visitTimer->StartTimer();
        // If we have new streamlines, sort them on the domain.
        if ((loadOnDemand && (streamlineSetChanged || domainSetChanged)) ||
             (!loadOnDemand && (numDomains/nProcs > 1)))
            SortStreamlines(streamlines);
        sortTime += visitTimer->StopTimer(timer, "Sorting");
    }

    debug1 << "All done!\n";
    //All done.
    if (asynchronous)
        CleanupAsynchronous();

    debug1 << "Make output: " << terminatedStreamlines.size() << endl;
    
    CreateStreamlineOutput(terminatedStreamlines);
    ReportStatistics(terminatedStreamlines);

    for (int i = 0; i < terminatedStreamlines.size(); i++)
        delete terminatedStreamlines[i];
#endif
#endif

}


// ****************************************************************************
//  Function: slDomainCompare
//
//  Purpose:
//      Contain the domains between to streamline wrappers.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

vector<int> *domainArrayPtr = NULL;

bool slDomainCompare(const avtStreamlineWrapper *slA, 
                     const avtStreamlineWrapper *slB)
{
    if (slA->domain == slB->domain)
        return false;
    
    // We want to sort such that the higher counts are before lower counts.
    int cntA = (*domainArrayPtr)[ slA->domain ];
    int cntB = (*domainArrayPtr)[ slB->domain ];

    if (cntA > cntB)
        return true;
    else if (cntA == cntB && (slA->domain < slB->domain))
        return true;
    
    return false;
}


// ****************************************************************************
//  Method: avtStreamlineFilter::SortStreamlines
//
//  Purpose:
//      Sort streamlines based on the domains they span.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Fri Aug 22 14:47:11 EST 2008
//   Fix memory leak. domainCnt not delete if allSameDomain is true.
//
// ****************************************************************************

void
avtStreamlineFilter::SortStreamlines(
                              std::vector<avtStreamlineWrapper *> &streamlines)
{
    debug1 << "SortStreamlines(" << streamlines.size() << ");\n";
    // No point in sorting two or fewer elements.
    if (streamlines.size() <= 2)
        return;
    
    bool sameDomain = true;
    
    vector<int> *domainCnt = new vector<int>(numDomains,0);
    int domain = -1;
    bool allSameDomain = true;
    for (int i = 0; i < streamlines.size(); i++)
    {
        avtStreamlineWrapper *slSeg = (avtStreamlineWrapper *)streamlines[i];
        if (i == 0)
            domain = slSeg->domain;
        if (domain != slSeg->domain)
            allSameDomain = false;
        
        (*domainCnt)[slSeg->domain]  = (*domainCnt)[slSeg->domain] + 1;
    }
    
    if (allSameDomain)
    {
        delete domainCnt;
        return;
    }
    
    domainArrayPtr = domainCnt;
    std::sort(streamlines.begin(), streamlines.end(), slDomainCompare);
    domainArrayPtr = NULL;
    delete domainCnt;
}

typedef struct
{
    int rank;
    int numStreamlines, extraCapacity;
    int *domains;
    float balance;
} busyInfo;

int busyInfoCompare(const void *a, const void *b)
{
    const busyInfo *aa = (const busyInfo *)a;
    const busyInfo *bb = (const busyInfo *)b;
    if (aa->numStreamlines > bb->numStreamlines)
        return -1;
    if (aa->numStreamlines < bb->numStreamlines)
        return 1;
    return 0;
}

bool intCompare(const int x, const int y)
{
    if (x > y)
        return true;
    return false;
}

// ****************************************************************************
//  Method: avtStreamlineFilter::AsyncFigureOutBalancing
//
//  Purpose:
//      Figure out how to do load balancing when we are doing asynchronous 
//      processing.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

void
avtStreamlineFilter::AsyncFigureOutBalancing(
                             std::vector<avtStreamlineWrapper *> &streamlines)
{
#ifdef PARALLEL
    debug1 << "AsyncFigureOutBalancing()\n";
    PrintLoadBalanceInfo();
    
    // If SL count not above threshold, don't send.
    int N = 2;
    if (streamlines.size() < N*maxCount)
        return;

    int totalNumSLs = 0;
    for (int i = 0; i < nProcs; i++)
        totalNumSLs += allSLCounts[i];

    // If SL count is zero, return.
    if (totalNumSLs == 0)
        return;

    // Compute load balancing information for all processors.
    vector<float> loadBalanceVec;
    vector<int> canAccept;
    canAccept.resize(nProcs);
    loadBalanceVec.resize(nProcs);
    
    float perfectBalance = (float)totalNumSLs / (float)nProcs;
    for (int i = 0; i < nProcs; i++)
    {
        loadBalanceVec[i] = (float)allSLCounts[i] / perfectBalance;
        if (allSLCounts[i] < (int)perfectBalance)
            canAccept[i] = (int)perfectBalance - allSLCounts[i];
        else
            canAccept[i] = 0;
    }

    // I'm not overworked, continue.
    if (loadBalanceVec[rank] < loadFactor)
        return;

    // I am overworked.
    debug1 << "I am overworked.\n";
    for (int i = 0; i < nProcs; i++)
        debug1<<"canAccept: " << i << " = " << canAccept[i] << endl;

    // Determine which domain to send out.
    // These are sorted, so just pick the last streamline.
    int domain = streamlines[ streamlines.size()-1]->domain;

    // Find candidates.
    vector<int> slackers, slackersWithDomain;
    vector<int> slackersWithDomainSLCount;
    bool SLsSent = false;
    for (int i = 0; i < nProcs; i++)
    {
        if (i != rank && loadBalanceVec[i] < underWorkedFactor)
        {
            slackers.push_back(i);
            if (domainsLoaded[i][domain])
            {
                slackersWithDomain.push_back(i);
                slackersWithDomainSLCount.push_back(allSLCounts[i]);
            }
        }
    }
    
    // If nobody is under-worked, quit.
    if (slackers.size() == 0)
        return;

    debug1 << "Try to send from domain= " << domain << " slackerSz = " 
           << slackers.size() << " slackersWDom = " 
           << slackersWithDomain.size() << endl;

    int target = -1;
    // Nobody has this domain, so randomly pick someone.
    if (slackersWithDomain.size() > 0)
    {
        float r = (float)rand() / (float)RAND_MAX;
        int idx = (int)((r * (slackersWithDomain.size()-1)) + 0.5);
        target = slackersWithDomain[idx];
        debug1 << "DOMAIN: Offload to " << target << endl;
    }
    else
    {
        // Nobody has the domain, randomly select someone...
        float r = (float)rand() / (float)RAND_MAX;
        int idx = (int)((r * (slackers.size()-1)) + 0.5);
        target = slackers[idx];
        debug1 << "RANDOM: Offload to " << target << endl;
    }


    vector<avtStreamlineWrapper *> send;
    // If -1, send everything to create balance.
    int numToSend;
    if (balanceNumToSend == -1)
    {
        int numToPerfectBalance = streamlines.size() - (int)perfectBalance;
        if (numToPerfectBalance > canAccept[target])
            numToSend = canAccept[target];
        else
            numToSend = numToPerfectBalance;
    }
    else
        numToSend = balanceNumToSend;
    
    // Find up to "numToSend" SLs from the same domain.
    for (int i = 0; i < numToSend; i++)
    {
        avtStreamlineWrapper *slSeg = streamlines[ streamlines.size()-1 - i];
        if (slSeg->domain != domain)
            break;
        send.push_back(slSeg);
    }
    numToSend = send.size();

    debug1 << "Offloading "<<numToSend<<" SLs (dom= "<<domain<<") to " 
           << target << endl;
    //Update the arrays with the stuff we just sent.
    domainsLoaded[target][domain] = 1;
    allSLCounts[target] += send.size();

    AsyncSendSLs(target, send);
    streamlines.resize(streamlines.size() - send.size());

#endif
}

// ****************************************************************************
//  Method: avtStreamlineFilter::FigureOutBalancing
//
//  Purpose:
//      Figure out how to do load balancing.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

void
avtStreamlineFilter::FigureOutBalancing(
                             std::vector<avtStreamlineWrapper *> &streamlines, 
                             float loadFactor)
{
#ifdef PARALLEL
    debug1 << "avtStreamlineFilter::FigureOutBalancing()\n";
    if (loadFactor < 1.0)
        loadFactor = 1.0;

    // Figure out how busy I am. Streamline count.
    int numStreamlines = streamlines.size();
    
    // Get my loaded domains.
    vector<int> myLoadedDomains, domainList(numDomains,0);
    GetLoadedDomains(myLoadedDomains);
    for (int i = 0; i < myLoadedDomains.size(); i++)
        domainList[ myLoadedDomains[i] ] = 1;

    // Everyone communicates their status to everyone else.
    int buf = numStreamlines;

    int *busyVec = new int[nProcs];
    int *allProcDomains = new int[nProcs*numDomains];
    int gather = visitTimer->StartTimer();
    MPI_Allgather(&numStreamlines, 1, MPI_INT, busyVec, 1, MPI_INT, 
                  VISIT_MPI_COMM);
    MPI_Allgather(&domainList[0], numDomains, MPI_INT, allProcDomains, 
                  numDomains, MPI_INT, VISIT_MPI_COMM);
    gatherTime2 += visitTimer->StopTimer(gather, "MPI_Allgather_2");

    int totalNumStreamlines = 0;
    for (int i = 0; i < nProcs; i++)
        totalNumStreamlines += busyVec[i];
        
    float perfectBalance = (float) totalNumStreamlines / (float)nProcs;
    float *balanceVec = new float[nProcs];
    for (int i = 0; i < nProcs; i++)
        balanceVec[i] = (float) busyVec[i] / perfectBalance;

    // Balance if someone is above some threshold.....
    bool needsBalancing = false;
    for (int i = 0; i < nProcs; i++)
        if (balanceVec[i] > loadFactor)
        {
            needsBalancing = true;
            break;
        }

    needsBalancing = true;
    if (needsBalancing)
    {
        busyInfo *info = new busyInfo[nProcs];
        
        // Build a table of information, then sort it.
        // Heavily loaded procs at the front of the list, 
        // lightly loaded at the back.
        for (int i = 0; i < nProcs; i++)
        {
            info[i].rank = i;
            info[i].numStreamlines = busyVec[i];
            //NOTE: negative extraCapacity means we are over the perfect 
            //      balance.
            info[i].extraCapacity = (int)perfectBalance 
                                     -  info[i].numStreamlines;
            info[i].balance = (float)busyVec[i] / perfectBalance;
            info[i].domains = new int[numDomains];
            for (int j = 0; j < numDomains; j++)
                info[i].domains[j] = allProcDomains[i*nProcs +j];
        }
        qsort(info, nProcs, sizeof(busyInfo), busyInfoCompare);
        
        for (int i = 0; i < nProcs; i++)
        {
            debug1 << "Rank = " << setw(2) << info[i].rank << " DOM = ";
            debug1 << "**[ ";
            for (int j = 0; j < numDomains; j++)
                debug1 << info[i].domains[j] << " ";
            debug1 << " ]** : nSL = " << setw(4) << info[i].numStreamlines 
                   << " cap = " << setw(4) << info[i].extraCapacity 
                   << " bal = " << info[i].balance << endl;
        }

        // Exchange streamlines: Busy give stuff to the lightly loaded.
        // Start indexing from both ends. Make sure we don't cross!
        for (int i = 0; i < nProcs; i++)
        {
            // Quit if we no more balancing needs to happen.
            if (info[i].balance < loadFactor)
                break;
            
            int getRidOf = -info[i].extraCapacity;
            
            for (int j=nProcs-1; j > i; j--)
            {
                if (info[j].extraCapacity > 0)
                {
                    int avail = info[j].extraCapacity;
                    int numToExchange = getRidOf;
                    if (numToExchange > avail)
                        numToExchange = avail;
                    info[j].extraCapacity -= numToExchange;

                    debug1 << info[i].rank << " == [" << numToExchange 
                           << "] ==> " << info[j].rank << endl;
                    if (numToExchange > 0)
                        SendStreamlinesTo(numToExchange, info[i].rank, 
                                          info[j].rank, streamlines);

                    getRidOf -= numToExchange;
                    if (getRidOf == 0)
                        break;
                }
            }
        }
        
        delete [] info;
    }

    delete [] balanceVec;
    delete [] busyVec;
    delete [] allProcDomains;
#endif

}

// ****************************************************************************
//  Method: avtStreamlineFilter::InitRequests
//
//  Purpose:
//      Initialize the request buffers.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

void
avtStreamlineFilter::InitRequests()
{
#ifdef PARALLEL
    
    statusRecvRequests.resize(nProcs, MPI_REQUEST_NULL);
    slRecvRequests.resize(nProcs, MPI_REQUEST_NULL);

    for (int i = 0; i < nProcs; i++)
    {
        if (i != rank)
        {
            PostRecvStatusReq(i);
            PostRecvSLReq(i);
        }
    }

#endif
}


// ****************************************************************************
//  Method: avtStreamlineFilter::CleanupAsynchronous
//
//  Purpose:
//      Claenup the buffers used when doing asynchronous processing.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

void
avtStreamlineFilter::CleanupAsynchronous()
{
#ifdef PARALLEL
    debug1 << "CleanupAsynchronous()\n";

    for (int i = 0; i < statusRecvRequests.size(); i++)
    {
        MPI_Request req = statusRecvRequests[i];
        if (req != MPI_REQUEST_NULL)
            MPI_Cancel(&req);
    } 

    for (int i = 0; i < slRecvRequests.size(); i++)
    {
        MPI_Request req = slRecvRequests[i];
        if (req != MPI_REQUEST_NULL)
            MPI_Cancel(&req);
    }

    // Cleanup recv buffers.
    std::map<MPI_Request, unsigned char*>::const_iterator it;
    for (it = recvSLBufferMap.begin(); it != recvSLBufferMap.end(); ++it)
        if (it->second)
            delete[] it->second;

    std::map<MPI_Request, int*>::const_iterator itt;
    for (itt = recvIntBufferMap.begin(); itt != recvIntBufferMap.end(); ++itt)
        if (itt->second)
            delete[] itt->second;
    
    debug1 << "DONE CleanupAsynchronous()\n";
#endif
}


// ****************************************************************************
//  Method: avtStreamlineFilter::CheckPendingSendRequests
//
//  Purpose:
//      Check to see if there are any pending send requests.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

void
avtStreamlineFilter::CheckPendingSendRequests()
{
#ifdef PARALLEL
    debug1 << "CheckPendingSendRequests();\n";
    //int timer = visitTimer->StartTimer();

    if (sendSLBufferMap.size() > 0)
    {
        vector<MPI_Request> req, copy;

        int notCompleted = 0;
        std::map<MPI_Request, unsigned char*>::const_iterator it;
        for (it = sendSLBufferMap.begin(); it != sendSLBufferMap.end(); ++it)
        {
            if (it->first != MPI_REQUEST_NULL && it->second != NULL)
            {
                req.push_back(it->first);
                copy.push_back(it->first);
            }
            else
                notCompleted++;
        }

        debug1 << "\tCheckPendingSendRequests() SL completed = "<<req.size()
               <<" not completed: "<<notCompleted<<endl;

        if (req.size() > 0)
        {
            // See if any sends have completed. Delete buffers if they have.
            int num = 0, *indices = new int[req.size()];
            MPI_Status *status = new MPI_Status[req.size()];
            MPI_Testsome(req.size(), &req[0], &num, indices, status);
            
            for (int i = 0; i < num; i++)
            {
                int idx = indices[i];
                MPI_Request r = copy[idx];
                unsigned char *buff = sendSLBufferMap[r];
                debug1 << "\tidx = " << idx << " r = " << r << " buff = " 
                       << (void *)buff << endl;
                if (buff)
                    delete [] buff;
                debug1 << "Delete done!\n";
                sendSLBufferMap[r] = NULL;
            }
            
            delete [] indices;
            delete [] status;
        }
    }

    if (sendIntBufferMap.size() > 0)
    {
        vector<MPI_Request> req, copy;
        std::map<MPI_Request, int*>::const_iterator it;
        int notCompleted = 0;

        for (it = sendIntBufferMap.begin(); it != sendIntBufferMap.end(); ++it)
        {
            if (it->first != MPI_REQUEST_NULL && it->second != NULL)
            {
                req.push_back(it->first);
                copy.push_back(it->first);
            }
            notCompleted++;
        }

        debug1 << "\tCheckPendingSendRequests() INT completed = "<<req.size()
               <<" not completed: "<<notCompleted<<endl;
        
        if (req.size() > 0)
        {
            // See if any sends have completed. Delete buffers if they have.
            int num = 0, *indices = new int[req.size()];
            MPI_Status *status = new MPI_Status[req.size()];
            MPI_Testsome(req.size(), &req[0], &num, indices, status);
            
            for (int i = 0; i < num; i++)
            {
                int idx = indices[i];
                MPI_Request r = copy[idx];
                int *buff = sendIntBufferMap[r];
                debug1 << "\tidx = " << idx << " r = " << r << " buff = " 
                       << (void *)buff << endl;
                if (buff)
                    delete [] buff;
                sendIntBufferMap[r] = NULL;
            }
            
            delete [] indices;
            delete [] status;
        }
    }

    //asyncSendCleanupTime += visitTimer->StopTimer(timer, 
    //                                              "CheckPendingSendRequests");
    debug1 << "DONE  CheckPendingSendRequests()\n";
#endif
}


// ****************************************************************************
//  Method: avtStreamlineFilter::PostRecvStatusReq
//
//  Purpose:
//      Receives status requests.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

void
avtStreamlineFilter::PostRecvStatusReq(int proc)
{
#ifdef PARALLEL
    MPI_Request req;
    int *buff = new int[statusMsgSz];
    MPI_Irecv(buff, statusMsgSz, MPI_INT, proc, STATUS_TAG, VISIT_MPI_COMM, 
              &req);
    //debug1 << "Post Statusrecv for req = " << req << " from " << proc<<endl;
    statusRecvRequests[proc] = req;
    recvIntBufferMap[req] = buff;
#endif
}

void
avtStreamlineFilter::PostRecvSLReq(int proc)
{
#ifdef PARALLEL
    MPI_Request req;
    unsigned char *buff = new unsigned char[ MAX_SLMSG_SZ ];
    int err = MPI_Irecv(buff, MAX_SLMSG_SZ, MPI_CHAR, proc, STREAMLINE_TAG, 
                        VISIT_MPI_COMM, &req);
    debug1<<err<<" = MPI_Irecv(buff, "<<MAX_SLMSG_SZ<<", MPI_CHAR, "<<proc
          <<", "<<STREAMLINE_TAG<<", "<<req<<");\n";
    slRecvRequests[proc] = req;
    recvSLBufferMap[req] = buff;
#endif
}

int
avtStreamlineFilter::AsyncRecvStreamlines(
                                          std::vector<avtStreamlineWrapper *> &streamlines,
                                          int &earlyTerminations )
{
    int slCount = 0;
    earlyTerminations = 0;

#ifdef PARALLEL
    //int timer = visitTimer->StartTimer();
    //debug1 << "AsyncRecvStreamlines(slCnt = " << streamlines.size() << ");\n";

    while (true)
    {
        int nReq = slRecvRequests.size();
        MPI_Status *status = new MPI_Status[nReq];
        int *indices = new int[nReq];
        int num = 0, err;

        std::vector<MPI_Request> copy;
        for (int i = 0; i < slRecvRequests.size(); i++)
            copy.push_back(slRecvRequests[i]);

        err = MPI_Testsome(nReq, &copy[0], &num, indices, status);
        //debug1<<"MPI_Testsome(nReq= "<<nReq<<"); err= "<<err<<endl;

        if (num > 0)
        {
            for (int i = 0; i < num; i++)
            {
                int idx = indices[i];
                
                MPI_Request req = slRecvRequests[idx];
                if (req == MPI_REQUEST_NULL)
                    continue;
                
                unsigned char *msg = recvSLBufferMap[req];
                if (msg == NULL)
                    continue;
        
                MemStream buff(MAX_SLMSG_SZ, msg);
                delete [] msg;

                size_t numSLs;
                buff.read(numSLs);
                //debug1 << "Recv " << numSLs << " from " << idx << endl;

                vector<avtStreamlineWrapper *> recvSLs;
                for (int j = 0; j < numSLs; j++)
                {
                    //debug1 << "Unserialize : " << j << endl;
                    avtStreamlineWrapper *slSeg = new avtStreamlineWrapper;
                    slSeg->Serialize(MemStream::READ, buff, solver);
                    recvSLs.push_back(slSeg);
                }

                // Make sure the streamline is one one of my domains.
                for (int j = 0; j < recvSLs.size(); j++)
                {
                    avtStreamlineWrapper *slSeg = recvSLs[j];

                    pt3d pt;
                    slSeg->GetEndPoint(pt);
                    //slSeg->Debug();

                    if (PointInDomain(pt, slSeg->domain))
                    {
                        //debug1<<"It's a keeper\n";
                        streamlines.push_back(slSeg);
                        slCount++;
                    }
                    else
                    {
                        // Point not in domain.
                        delete slSeg;
                        //debug1<<"Throw it back!\n";
                        earlyTerminations++;
                    }
                }
                
                recvSLBufferMap[req] = NULL;
            }

            for (int i = 0; i < num; i++)
                PostRecvSLReq(indices[i]);
        }

        delete [] status;
        delete [] indices;
        
        if (num == 0)
            break;
    }

    //debug1 << "DONE: AsyncRecvStreamlines(slCnt = " << streamlines.size() 
    //       << ");\n";
    //asyncSLTime += visitTimer->StopTimer(timer, "AsyncRecvStreamlines");
#endif
    
    return slCount;
}

// ****************************************************************************
//  Method: avtStreamlineFilter::AsyncRecvs
//
//  Purpose:
//      Receive streamlines in an asynchronous setting.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

void
avtStreamlineFilter::AsyncRecvs(
                    std::vector<avtStreamlineWrapper *> &streamlines, 
                    bool blockingWait, int *numSLs, int *numTerm)
{
#ifdef PARALLEL
    /*
    debug1 << "AsyncRecvs(" << blockingWait << ")\n";

    *numSLs = 0;
    *numTerm = 0;
    int nTReq = statusRecvRequests.size();
    int nSReq = slRecvRequests.size();
    int nReq = nTReq + nSReq;

    std::vector<MPI_Request> copy;
    for (int i = 0; i < statusRecvRequests.size(); i++)
        copy.push_back(statusRecvRequests[i]);
    
    for (int i = 0; i < slRecvRequests.size(); i++)
        copy.push_back(slRecvRequests[i]);

    MPI_Status *status = new MPI_Status[nReq];
    int *indices = new int[nReq];
    int num = 0, err;

    if (blockingWait)
        err = MPI_Waitsome(nReq, &copy[0], &num, indices, status);
    else
        err = MPI_Testsome(nReq, &copy[0], &num, indices, status);

    debug1 << "AsyncRecvs(): Done waiting " << num << endl;
    if (num == 0)
        return;

    for (int i = 0; i < num; i++)
    {
        int idx = indices[i];
        // It's a terminate request...
        if (idx < nTReq)
        {
            MPI_Request req = terminateRecvRequests[idx];
            if (req == MPI_REQUEST_NULL)
                continue;
            int *buff = recvIntBufferMap[req];
            if (buff == NULL)
                continue;
            
            *numTerm += buff[0];

            // Mark this guy as "not busy".
            terminationSends[idx] = buff[0];

            delete [] buff;
            recvIntBufferMap[req] = NULL;
            PostRecvStatusReq(idx);
        }
        // Its a streamline.
        else
        {
            idx -= nTReq;
            MPI_Request req = slRecvRequests[idx];
            if (req == MPI_REQUEST_NULL)
                continue;
            
            unsigned char *buff = recvSLBufferMap[req];
            if (buff == NULL)
                continue;

            int num = 0, offset = 0;
            memcpy(&num, buff, sizeof(int));
            //debug1 << "Recv " << numSLs << " from " << idx << endl;
            offset += sizeof(int);

            for (int j = 0; j < num; j++)
            {
                avtStreamlineWrapper *slSeg = new avtStreamlineWrapper;
                int len  = slSeg->UnSerialize(&buff[offset]);
                offset += len;
                streamlines.push_back(slSeg);
            }
            *numSLs += num;
            delete [] buff;
            recvSLBufferMap[req] = NULL;

            PostRecvSLReq(idx);
        }
    }

    delete [] status;
    delete [] indices;
    */
#endif
}


// ****************************************************************************
//  Method: avtStreamlineFilter::AsyncRecvStatus
//
//  Purpose:
//      Report status about asynchronous receives.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

void
avtStreamlineFilter::AsyncRecvStatus(int &numTerminated, bool balanceInfo)
{
    numTerminated = 0;
#ifdef PARALLEL
    //debug1 << "AsyncRecvStatus()\n";
    //int timer = visitTimer->StartTimer();

    // Keep processing until there are no more terminations to recieve.
    while (true)
    {
        int nReq = statusRecvRequests.size();
        MPI_Status *status = new MPI_Status[nReq];
        int *indices = new int[nReq];
        int num = 0, err;

        std::vector<MPI_Request> copy;
        for (int i = 0; i < statusRecvRequests.size(); i++)
            copy.push_back(statusRecvRequests[i]);

        //debug1<< "MPI_Testsome("<<status<<" "<<indices<<" .....\n";
        err = MPI_Testsome(nReq, &copy[0], &num, indices, status);
        //debug1<< "MPI_Testsome returned. err = "<<err<<" num= "<<num<<endl;
        if (num > 0)
        {
            //debug1<<"MPI_Testsome() num = "<<num<<endl;
            for (int i = 0; i < num; i++)
            {
                int idx = indices[i];
                //debug1 << "idx = " << idx << endl;
                
                MPI_Request req = statusRecvRequests[idx];
                if (req == MPI_REQUEST_NULL)
                    continue;
                
                int *buff = recvIntBufferMap[req];
                /*
                debug1 << "Process the terminate msg: req = " <<req 
                       << " buff= "<< buff[0] << endl;
                */
                if (buff == NULL)
                    continue;
                
                numTerminated += buff[0];
                
                if (balanceInfo)
                {
                    allSLCounts[idx] = buff[1];
                    for (int j = 0; j < numDomains; j++)
                        domainsLoaded[idx][j] = 0;
                    
                    for (int j = 0; j < numDomains; j++)
                    {
                        int dom = buff[2+j];
                        if (dom == -1)
                            break;
                        domainsLoaded[idx][dom] = 1;
                    }
                }
                
                delete [] buff;
                recvIntBufferMap[req] = NULL;
            }
        
            for (int i = 0; i < num; i++)
                PostRecvStatusReq(indices[i]);
        }
            
        delete [] status;
        delete [] indices;
        if (num == 0)
            break;
    }

    //debug1 << "DONE AsyncRecvStatus() Terminated: "<<numTerminated<<endl;
    //asyncTermTime += visitTimer->StopTimer(timer, "AsyncRecvTerminate");
#endif
}


// ****************************************************************************
//  Method: avtStreamlineFilter::PrintLoadBalanceInfo
//
//  Purpose:
//      Print information about the load balance.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

void
avtStreamlineFilter::PrintLoadBalanceInfo()
{
#ifdef PARALLEL
    int nSLs = 0;
    for (int i = 0; i < nProcs; i++)
        nSLs += allSLCounts[i];
    
    float perfectBalance = (float)nSLs / (float)nProcs;
    debug1 << "Perfect balance = " << perfectBalance << endl;
    for (int i = 0; i < nProcs; i++)
    {
        int domCnt = 0;
        for (int j = 0; j < numDomains; j++)
            domCnt += domainsLoaded[i][j];

        float lf = (float) allSLCounts[i] / perfectBalance;
        char str[64];
        sprintf(str, "%6.3f", lf);
        debug1 << setw(3)<<i<<": SL= "<<setw(4)<<allSLCounts[i]<<" LF= "
               <<str<< " ";
        debug1 << "[ ";
        for (int j = 0; j < numDomains; j++)
            debug1 << domainsLoaded[i][j] << " ";
        debug1 << "] CNT= " << domCnt << endl;
    }
#endif
}


// ****************************************************************************
//  Method: avtStreamlineFilter::AsyncSendStatus
//
//  Purpose:
//      Report status about asynchronous sends.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// Modifications:
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Use statusMsgSz member data.
//
// ****************************************************************************

void
avtStreamlineFilter::AsyncSendStatus(int numTerm, bool sendBalanceInfo, int slCount)
{
#ifdef PARALLEL
    debug1 << "AsyncSendStatus(" << numTerm << ", " <<sendBalanceInfo<<", "
           <<slCount<<");\n";
    //        if (numTerm == 0)
    //            return;

    //int timer = visitTimer->StartTimer();

    //Do an asynch broadcast to everyone.
    vector<int> domainList;
    if (sendBalanceInfo)
    {
        for (int i = 0; i < numDomains; i++)
            if (DomainLoaded(i))
                domainList.push_back(i);
        
        //Marker for end of domain list.
        domainList.push_back(-1);
    }
    
    /*
    debug1 << "AsyncSendStatus: [";
    debug1 << numTerm << ", ";
    if (sendBalanceInfo)
    {
        debug1 << slCount << ", ";
        for (int i = 0; i < numDomains; i++)
            debug1 << DomainLoaded(i) << ", ";
    }
    debug1 << "]\n";
    */

    for (int i = 0; i < nProcs; i++)
    {
        if (i != rank)
        {
            MPI_Request req;
            int *msg;
            if (sendBalanceInfo)
            {
                msg = new int[statusMsgSz];
                msg[0] = numTerm;
                msg[1] = slCount;
                for (int j = 0; j < domainList.size(); j++)
                    msg[2+j] = domainList[j];
                /*
                for (int j = 0; j < numDomains; j++)
                    msg[2 +j] = DomainLoaded(j);
                MPI_Isend(msg, (numDomains+2), MPI_INT, i, STATUS_TAG, 
                          VISIT_MPI_COMM, &req);
                */
                
                MPI_Isend(msg, (domainList.size()+2), MPI_INT, i, STATUS_TAG, 
                          VISIT_MPI_COMM, &req);
                numBytesSent += (sizeof(int) * (numDomains+2));
            }
            else
            {
                msg = new int;
                msg[0] = numTerm;
                MPI_Isend(msg, 1, MPI_INT, i, STATUS_TAG, VISIT_MPI_COMM,&req);
                numBytesSent += (sizeof(int));
            }

            //debug1 << "MPI_Isend("<<msg[0]<< ", 1, MPI_INT, "<<i
            //       <<", TERMINATE_TAG, VISIT_MPI_COMM, &req); REQ = " 
            //       << req << endl;
            sendIntBufferMap[req] = msg;
            numStatusCommunicated++;
        }
    }
    
    debug1 << "DONE: AsyncSendStatus(" << numTerm << ", " <<sendBalanceInfo
           <<", "<<slCount<<");\n";
    //asyncTermTime += visitTimer->StopTimer(timer, "AsyncSendTerminate");
#endif
}


// ****************************************************************************
//  Method: avtStreamlineFilter::AsyncSendSL
//
//  Purpose:
//      Send a streamline in an asynchronous setting.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

void
avtStreamlineFilter::AsyncSendSL(int receiver, avtStreamlineWrapper *slSeg)
{
#ifdef PARALLEL
    //int timer = visitTimer->StartTimer();

    debug1 << "AsyncSendSL(int receiver, avtStreamlineWrapper *slSeg)\n";
    MPI_Request req;

    MemStream buff;
    size_t one = 1;
    buff.write(one);
    slSeg->numTimesCommunicated++;
    slSeg->Serialize(MemStream::WRITE, buff, solver);
    numSLCommunicated++;

    size_t sz = buff.buffLen();
    unsigned char *msg = new unsigned char[sz];
    memcpy(msg, buff.buff(), sz);

    MPI_Isend(msg, sz, MPI_CHAR, receiver, STREAMLINE_TAG,VISIT_MPI_COMM,&req);
    numBytesSent += sz;
    sendSLBufferMap[req] = msg;

    debug1 << "DONE  AsyncSendSL(int receiver, avtStreamlineWrapper *slSeg) "
           << "sz = " << sz << endl << endl;

    //asyncSLTime += visitTimer->StopTimer(timer, "AsyncSendSL");
#endif
}

// ****************************************************************************
//  Method: avtStreamlineFilter::AsyncSendSLs
//
//  Purpose:
//      Send multiple streamlines in an asynchronous setting.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Fri Aug 22 14:47:11 EST 2008
//   Memory leak fix.
//
// ****************************************************************************

void
avtStreamlineFilter::AsyncSendSLs(int receiver, 
                              const std::vector<avtStreamlineWrapper*> &slSegs)
{
#ifdef PARALLEL
    int numSL = slSegs.size();
    if (numSL == 0)
        return;

    //int timer = visitTimer->StartTimer();
    //debug1 << "AsyncSendSLs(recv= " << receiver << ", numSLs= " 
    //       << numSL << ");\n";
    MemStream buff;

    size_t szz = slSegs.size();
    buff.write(&szz, 1);
    for (int i = 0; i < slSegs.size(); i++)
    {
        avtStreamlineWrapper *slSeg = slSegs[i];
        slSeg->numTimesCommunicated++;
        slSeg->Serialize(MemStream::WRITE, buff, solver);
        delete slSeg;
        
        numSLCommunicated++;
    }

    // Break it up into multiple messages if needed.
    if (buff.buffLen() > MAX_SLMSG_SZ)
    {
        cerr << "SL msg too big!\n";
        return;
    }

    size_t sz = buff.buffLen();
    unsigned char *msg = new unsigned char[sz];
    memcpy(msg, buff.buff(), sz);

    /*
    //See how it compresses.
    unsigned long len = sz, resLen = sz;
    unsigned char *dest = new unsigned char[sz];
    compress(dest, &resLen, msg, sz);
    double ratio = (double)len/(double)resLen;
    debug1<< "Compression: len= " << sz << " --> " << resLen << " = " 
          <<ratio << endl;
    */
    
    MPI_Request req;
    int err = MPI_Isend(msg, sz, MPI_CHAR, receiver, STREAMLINE_TAG, VISIT_MPI_COMM, &req);
    //debug1<<err<<" = MPI_Isend(msg, "<<sz<<", MPI_CHAR, "<<receiver<<", "
    //      <<STREAMLINE_TAG<<", req= "<<req<<");\n";
    numBytesSent += sz;
    sendSLBufferMap[req] = msg;

    //debug1 << "DONE AsyncSendSLs()"<< endl;
    //asyncSLTime += visitTimer->StopTimer(timer, "AsyncSendSL");
#endif
}

// ****************************************************************************
//  Method: avtStreamlineFilter::SendStreamlinesTo
//
//  Purpose:
//      A function that manages many streamline sends.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

void
avtStreamlineFilter::SendStreamlinesTo(int num,
                             int sender, int receiver,
                             std::vector<avtStreamlineWrapper *> &streamlines)
{
#ifdef PARALLEL
#if 0
    int szTag = 1, buffTag = 2;
    int slEndIdx = streamlines.size() - 1;

    if (rank == sender)
    {
        int sz = 0;
        vector<avtStreamlineWrapper *> sentSL;
        for (int k = 0; k < num; k++, slEndIdx--)
        {
            avtStreamlineWrapper *slSeg = streamlines[slEndIdx];
            sz += slSeg->NumBytesToSerialize();
            sentSL.push_back(slSeg);
        }
        
        std::vector<MPI_Request> sendReq;
        MPI_Request req;
        MPI_Isend(&sz, 1, MPI_INT, receiver, szTag, VISIT_MPI_COMM, &req);
        numBytesSent += sizeof(int);
        sendReq.push_back(req);
        
        unsigned char *buff = new unsigned char[sz];
        int nBytes = 0;
        for (int k = 0; k < sentSL.size(); k++)
        {
            avtStreamlineWrapper *slSeg = sentSL[k];
            slSeg->numTimesCommunicated++;
            int len = slSeg->Serialize(&buff[nBytes]);
            nBytes += len;
            delete slSeg;
        }
        
        //debug1 << "  Send " << rank << " ==["<< sz << "]==> " << i << endl;
        MPI_Isend(buff, sz, MPI_CHAR, receiver, buffTag, VISIT_MPI_COMM, &req);
        numBytesSent += sz;
        sendReq.push_back(req);
        
        vector<MPI_Status> stats(sendReq.size());
        MPI_Waitall(sendReq.size(), &sendReq[0], &stats[0]);
        streamlines.resize(slEndIdx+1);
        delete [] buff;

        numSLCommunicated += num;
    }

    else if (rank == receiver)
    {
        int sz = 0;
        MPI_Status stat;
        MPI_Recv(&sz, 1, MPI_INT, sender, szTag, VISIT_MPI_COMM, &stat);

        unsigned char *buff = new unsigned char[sz];
        MPI_Recv(buff, sz, MPI_CHAR, sender, buffTag, VISIT_MPI_COMM, &stat);
                
        int offset = 0;
        for (int k = 0; k < num; k++)
        {
            avtStreamlineWrapper *slSeg = new avtStreamlineWrapper;
            int len = slSeg->UnSerialize(&buff[offset], solver);
            offset += len;
            streamlines.push_back(slSeg);
        }
        delete [] buff;
    }
#endif
#endif
}


#if 0
// ****************************************************************************
//  Method: avtStreamlineFilter::ParallelBalancedLoadOnDemand
//
//  Purpose:
//      Calculates a streamline with ParallelStaticDomains algorithm.
//      Each processor owns a fixed set of domains. However, communication is
//      done more frequently to try and avoid processor idle time. When a 
//      streamline exists a domain owned by the current processor, it is 
//      communicated to the processor that owns the domain.
//
//  Programmer: Dave Pugmire
//  Creation:   March 17, 2008
//
// ****************************************************************************

void
avtStreamlineFilter::ParallelBalancedLoadOnDemand(
                                        std::vector<pt3d> &allSeedpoints,
                                        float loadFactor, BalanceType balance,
                                        int maxPts, int divFactor)
{
    debug1 << "avtStreamlineFilter::ParallelBalancedLoadOnDemand()\n";
#ifdef PARALLEL
    cout << "Seedpoint cnt = " << allSeedpoints.size() << endl;

    // Load "my" statically determined domains. (Relax this later).
    for (int i = 0; i < numDomains; i++)
        if (DomainToRank(i) == rank)
            GetDomain(i);

    //Get "my" seed points.
    vector<avtStreamlineWrapper *> streamlines;
    for (int i = 0; i < allSeedpoints.size(); i++)
    {
        vector<int> dl;
        intervalTree->GetElementsListFromRange(allSeedpoints[i].values(), 
                                               allSeedpoints[i].values(), dl);
        //double x=allSeedpoints[i].xyz[0],y=allSeedpoints[i].xyz[1],
        //       z=allSeedpoints[i].xyz[2];
        //debug1 << "Seed pt " << i << ": [" << x << " " << y << " " << z 
        //       << "] in domain " << dl[0] << " owned by " 
        //       << DomainToRank(dl[0]) << ". Domain count = " << dl.size() 
        //       << endl;
        if (dl.size() > 0 && OwnDomain(dl[0]))
        {
            avtStreamlineWrapper *slSeg = 
                                    new avtStreamlineWrapper(allSeedpoints[i]);
            streamlines.push_back(slSeg);
            //debug1 << "Seed pt: " << slSeg->seedPt.xyz[0] << " " 
            //       << slSeg->seedPt.xyz[1] << " " << slSeg->seedPt.xyz[2] 
            //       << endl;
        }
    }
    debug1 << "My seedpoints: " << streamlines.size() << endl;

    vector< vector< avtStreamlineWrapper *> > distributeStreamlines;
    distributeStreamlines.resize(nProcs);
    int *cnts = new int[nProcs], *gather = new int[nProcs*nProcs];
    
    // Keep working while there is work to do.
    numIterations = 0;
    while (true)
    {
        debug1 << "Iteration = " << numIterations << ". My SL count = " 
               << streamlines.size() << endl;
        
        FigureOutBalancing(streamlines, loadFactor);

        int imax = streamlines.size();
        if (balance == MAX_CNT && (imax > maxPts && maxPts > 0))
            imax = maxPts;
        else if (balance == MAX_PCT && (imax > maxPts && divFactor > 1))
        {
            imax = imax / divFactor;
            if (imax == 0)
                imax = 1;
        }
        else if (balance == NO_BALANCE)
        {
            //Nothing.
        }

        
        debug1 << " **** for (int i = 0; i < " << imax << "; i++)\n";
        for (int i = 0; i < imax; i++)
        {
            avtStreamlineWrapper *slSeg = streamlines[i];
            //debug1 << i << ": Integrate seedPt " << slSeg->seedPt.xyz[0]
            //       << " "<< slSeg->seedPt.xyz[1] << " " 
            //       << slSeg->seedPt.xyz[2]<<endl;

            //Load on demand: Make sure we have "this" domain.
            GetDomain(slSeg->domain);
            IntegrateStreamline(slSeg);
            //FIX THIS
            EXCEPTION0(ImproperUseException);

            // After integration, streamline is either terminated, 
            // or needs a new domain.
            if (slSeg->status == TERMINATE)
            {
                terminatedStreamlines.push_back(slSeg);
                //debug1 << ".....Terimanted.\n";
            }
            else if (slSeg->status == OUTOFBOUND)
            {
                int domainOwner = DomainToRank(slSeg->domain);
                distributeStreamlines[domainOwner].push_back(slSeg);
                //debug1 << ".....Exit Pass to " << slSeg->domain << " rank = "
                //       << DomainToRank(slSeg->domain) << endl;
            }
        }
        if (imax > 0)
            numIterations++;

        int communicationTimer = visitTimer->StartTimer();

        // For unprocessed streamlines, pass them to myself.
        if (streamlines.size()-imax > 0)
            debug1 << "   Defer count: " << streamlines.size()-imax << endl;
        for (int i = imax; i < streamlines.size(); i++)
        {
            avtStreamlineWrapper *slSeg = streamlines[i];
            distributeStreamlines[rank].push_back(slSeg);
        }
        
        // All streamlines processed. empty the list.
        streamlines.resize(0);

        bool done = StaticDomainExchangeStreamlines(streamlines, 
                                                    distributeStreamlines);
        communicationTime += visitTimer->StopTimer(communicationTimer, 
                                                   "StreamlineCommunication");
        if (done)
            break;
    }

    debug1 << "Make output: " << terminatedStreamlines.size() << endl;
    CreateStreamlineOutput(terminatedStreamlines);
    ReportStatistics(terminatedStreamlines);

    delete [] cnts;
    delete [] gather;

    for (int i = 0; i < terminatedStreamlines.size(); i++)
    {
        avtStreamlineWrapper *slSeg = 
                             (avtStreamlineWrapper *) terminatedStreamlines[i];
        delete slSeg;
    }
#endif
}
#endif

// ****************************************************************************
//  Function: FindNextDomain
//
//  Purpose:
//      Find the next domain for a streamline,
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

static int
FindNextDomain(const vector<avtStreamlineWrapper *> &streamlines, 
               int numDomains)
{
    vector<int> domainCnt(numDomains, 0);
    for (int i = 0; i < streamlines.size(); i++)
    {       
        avtStreamlineWrapper *slSeg = streamlines[i];
        if (slSeg)
        {
            int cnt = domainCnt[slSeg->domain];
            domainCnt[slSeg->domain] = cnt+1;
        }
    }

    //Find the domain w/ dominant count.
    int maxDom = 0, maxCnt = domainCnt[0];
    for (int i = 1; i < domainCnt.size(); i++)
        if (domainCnt[i] > maxCnt)
        {
            maxCnt = domainCnt[i];
            maxDom = i;
        }
    return maxDom;
}



// ****************************************************************************
//  Method: avtStreamlineFilter::StagedLoadOnDemand
//
//  Purpose:
//      Calculates a streamline with StagedLoadOnDemand algorithm.
//      This is similar to the above LoadOnDemand algorithm.
//      The difference, is that it integrates all the seed points in loaded
//      domains. Once it has done that, it determines what domain to load next,
//      then continues.
//
//      This is accomplished with three lists, terminated (complete 
//      streamlines), active (streamlines in a loaded domain), inactive 
//      (streamlines in an UNloaded domain).
//      streamlines are moved from list to list until all streamlines are in 
//      the terminated list.
//
//  Programmer: Dave Pugmire
//  Creation:   March 4, 2008
//
//  Modifications:
//
//   Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//   Bug fix. If the seed is not found in any domains, put it in the terminated
//   streamlines array.
//
// ****************************************************************************

void
avtStreamlineFilter::StagedLoadOnDemand(
                               std::vector<avtStreamlineWrapper *> &seedpoints)
{
    int numSeedPoints = seedpoints.size();
    int i0 = 0, i1 = numSeedPoints;
    
#ifdef PARALLEL
    int nPts = numSeedPoints/nProcs;
    i0 = rank * nPts;
    i1 = i0 + nPts;
    // Last processor will get the slack.
    if (rank == nProcs-1)
        i1 = numSeedPoints;
    debug1 << "I have seed points: " << i0 << " to " << i1 
           << " out of a total of " << numSeedPoints << endl;
    debug1 << "I am " << PAR_Rank() << ". Total NProcs = " << PAR_Size()<<endl;
    //cout << "I am " << PAR_Rank() << ". Total NProcs = " << PAR_Size() << endl;
#endif

    numSeedPoints = i1-i0;
    debug1<< "Now, we have " << numSeedPoints << " seed points and "
          << numDomains << " domains.\n";

    //Get all the streamlines.
    vector<avtStreamlineWrapper *> inactiveStreamlines, terminatedStreamlines;

    for (int i = i0; i < i1; i++)
    {
        avtStreamlineWrapper *slSeg = seedpoints[i];
        pt3d endPt;
        slSeg->GetEndPoint(endPt);
        debug1<<"Check pt: "<<slSeg->domain<<endl;
        if (PointInDomain(endPt, slSeg->domain))
        {
            inactiveStreamlines.push_back(slSeg);
            debug1<<"Keeper\n";
        }
        else
        {
            debug1<<"TOSS IT\n";
            delete slSeg;
        }
    }

    numSeedPoints = inactiveStreamlines.size();

    while (terminatedStreamlines.size() < numSeedPoints)
    {
        debug1<<"********************\nwhile ("<<terminatedStreamlines.size()
              <<" < "<<numSeedPoints<<") {...}\n";

        // Get a set of activeStreamlines.
        vector<avtStreamlineWrapper *> activeStreamlines, outOfBounds;
        for (int i = 0; i < inactiveStreamlines.size(); i++)
        {
            avtStreamlineWrapper *slSeg = inactiveStreamlines[i];
            debug1<<"StagedLOD:: slSeg->domain = "<<slSeg->domain<<" loaded= "
                  << DomainLoaded(slSeg->domain)<<endl;

            if (slSeg->domain == -1)
                terminatedStreamlines.push_back(slSeg);
            else if (DomainLoaded(slSeg->domain))
                activeStreamlines.push_back(slSeg);
            else
                outOfBounds.push_back(slSeg);
        }

        // Integrate all the active streamlines.
        debug1<<"Integrate active streamlines. sz= "<<activeStreamlines.size()<<". Inact "<<inactiveStreamlines.size()<<endl;
        if (activeStreamlines.size() > 0)
        {
            //Integrate the active streamlines.
            for (int i = 0; i < activeStreamlines.size(); i++)
            {
                avtStreamlineWrapper *slSeg = activeStreamlines[i];
                
                IntegrateStreamline(slSeg);

                if (slSeg->status == avtStreamlineWrapper::TERMINATE)
                    terminatedStreamlines.push_back(slSeg);
                else
                {
                    pt3d endPt;
                    slSeg->GetEndPoint(endPt);
                    bool InDomain = false;
                    for (int j = 0; j < slSeg->seedPtDomainList.size(); j++)
                    {
                        if (PointInDomain(endPt, slSeg->seedPtDomainList[j]))
                        {
                            slSeg->domain = slSeg->seedPtDomainList[j];
                            outOfBounds.push_back(slSeg);
                            InDomain = true;
                            break;
                        }
                    }
                    if ( !InDomain )
                        terminatedStreamlines.push_back(slSeg);
                }
            }
        }
        else
        {
            // Nothing left in this domain, find the next domain to load.
            int dom = FindNextDomain(outOfBounds, numDomains);
            debug1<< "Find the next domain: OOB.size() = "
                  <<outOfBounds.size()<<" pickDom= " << dom << endl;
            
            if (dom >= 0)
                GetDomain(dom);
        }

        //Copy the outOfBounds streamlines to the inactive array.
        inactiveStreamlines.resize(outOfBounds.size());
        for (int i = 0; i < outOfBounds.size(); i++)
            inactiveStreamlines[i] = outOfBounds[i];
    }

    // All done, make the output.
    CreateStreamlineOutput(terminatedStreamlines);
    ReportStatistics(terminatedStreamlines);
    for (int i = 0; i < terminatedStreamlines.size(); i++)
    {
        avtStreamlineWrapper *slSeg = 
                             (avtStreamlineWrapper *) terminatedStreamlines[i];
        delete slSeg;
    }
}

// ****************************************************************************
//  Method: avtStreamlineFilter::AsyncSendStatus2
//
//  Purpose:
//      Report status about asynchronous sends.
//
//  Programmer: Dave Pugmire
//  Creation:   Dec 18, 2008
//
// Modifications:
//
//
// ****************************************************************************

void
avtStreamlineFilter::AsyncSendStatus2(int master,
                                      int numTerminated,
                                      std::vector<int> &status)
{
#ifdef PARALLEL
    int communicationTimer = visitTimer->StartTimer();
    
    debug1<<"AsyncSendStatus2( "<<numTerminated<<endl;
    int *msg = new int[statusMsgSz];
    
    MPI_Request req;
    msg[0] = MSG_STATUS;
    msg[1] = numTerminated;
    for (int i = 0; i < status.size(); i++)
        msg[2+i] = status[i];
        
    MPI_Isend(msg, statusMsgSz, MPI_INT, master, STATUS_TAG, VISIT_MPI_COMM, &req);
    numBytesSent += (sizeof(int) *statusMsgSz);
    numStatusCommunicated++;
    sendIntBufferMap[req] = msg;
    
    communicationTime += visitTimer->StopTimer(communicationTimer, 
                                               "StreamlineCommunication");        
#endif
}

// ****************************************************************************
//  Method: avtStreamlineFilter::AsyncSendStatus2
//
//  Purpose:
//      Report status about asynchronous sends.
//
//  Programmer: Dave Pugmire
//  Creation:   Dec 18, 2008
//
// Modifications:
//
//
// ****************************************************************************

bool
avtStreamlineFilter::AsyncRecvStatus2(int &numTerminated,
                                      std::vector< std::vector<int> > &slaveStatus)
{
    //debug1<<"AsyncRecvStatus2()\n";

    bool newStatus = false;
    numTerminated = 0;
#ifdef PARALLEL

    int communicationTimer = visitTimer->StartTimer();
    
    while (true)
    {
        int nReq = statusRecvRequests.size();
        MPI_Status *status = new MPI_Status[nReq];
        int *indices = new int[nReq];
        int num = 0, err;

        std::vector<MPI_Request> copy;
        for (int i = 0; i < statusRecvRequests.size(); i++)
            copy.push_back(statusRecvRequests[i]);

        //debug1<< "MPI_Testsome("<<status<<" "<<indices<<" .....\n";
        err = MPI_Testsome(nReq, &copy[0], &num, indices, status);
        //debug1<< "MPI_Testsome returned. err = "<<err<<" num= "<<num<<endl;
        if (num > 0)
        {
            newStatus = true;
            //debug1<<"MPI_Testsome() num = "<<num<<endl;
            for (int i = 0; i < num; i++)
            {
                int idx = indices[i];
                //debug1 << "idx = " << idx << endl;
                
                MPI_Request req = statusRecvRequests[idx];
                if (req == MPI_REQUEST_NULL)
                    continue;
                
                int *buff = recvIntBufferMap[req];
                /*
                debug1 << "Process the terminate msg: req = " <<req 
                       << " buff= "<< buff[0] << endl;
                */
                if (buff == NULL)
                    continue;
                
                int msgType = buff[0];
                numTerminated += buff[1];
                slaveStatus[idx][0] = 1; //Mark it as new.

                //Set status
                //debug1<<"Status from "<<idx<<" [";
                for (int j = 0; j < numDomains; j++)
                {
                    slaveStatus[idx][j+1] = buff[2+j];
                    //debug1<<slaveStatus[idx][j+1]<<" ";
                }
                //debug1<<"]\n";
                
                delete [] buff;
                recvIntBufferMap[req] = NULL;
            }
        
            for (int i = 0; i < num; i++)
                PostRecvStatusReq(indices[i]);
        }
            
        delete [] status;
        delete [] indices;
        if (num == 0)
            break;
    }
    
    communicationTime += visitTimer->StopTimer(communicationTimer, 
                                               "StreamlineCommunication");        
#endif
    return newStatus;
}


// ****************************************************************************
//  Method: avtStreamlineFilter::AsyncRecvMasterMsg
//
//  Purpose:
//      Receive messages from the master.
//
//  
//
//  Programmer: Dave Pugmire
//  Creation:   Dec 18, 2008
//
//  Modifications:
//
//
// ****************************************************************************

bool
avtStreamlineFilter::AsyncRecvMasterMsg(vector< vector<int> > &msgs)
{
    debug1<<"AsyncRecvMasterMsg()\n";
    bool done = false;

#ifdef PARALLEL
    int communicationTimer = visitTimer->StartTimer();
    
    while (true)
    {
        int nReq = statusRecvRequests.size();
        MPI_Status *status = new MPI_Status[nReq];
        int *indices = new int[nReq];
        int num = 0, err;

        std::vector<MPI_Request> copy;
        for (int i = 0; i < statusRecvRequests.size(); i++)
            copy.push_back(statusRecvRequests[i]);

        //debug1<< "MPI_Testsome("<<status<<" "<<indices<<" .....\n";
        err = MPI_Testsome(nReq, &copy[0], &num, indices, status);
        //debug1<< "MPI_Testsome returned. err = "<<err<<" num= "<<num<<endl;
        if (num > 0)
        {
            //debug1<<"MPI_Testsome() num = "<<num<<endl;
            for (int i = 0; i < num; i++)
            {
                int idx = indices[i];
                //debug1 << "idx = " << idx << endl;
                
                MPI_Request req = statusRecvRequests[idx];
                if (req == MPI_REQUEST_NULL)
                    continue;
                
                int *buff = recvIntBufferMap[req];
                /*
                debug1 << "Process the terminate msg: req = " <<req 
                       << " buff= "<< buff[0] << endl;
                */
                if (buff == NULL)
                    continue;
                
                int msgType = buff[0];
                if (msgType == MSG_DONE)
                    done = true;
                else if (msgType == MSG_LOAD_DOMAIN)
                {
                    vector<int> msg;
                    msg.push_back(msgType);
                    msg.push_back(buff[1]);
                    msgs.push_back(msg);
                }
                else if (msgType == MSG_SEND_SL)
                {
                    vector<int> msg;
                    msg.push_back(msgType);
                    msg.push_back(buff[1]); //dst
                    msg.push_back(buff[2]); //dom
                    msg.push_back(buff[3]); //num
                    msgs.push_back(msg);
                }
                /*
                else if (msgType == MSG_BALANCE)
                {
                    //Send num SLs from domain dom to process dst.
                    int dst = buff[1];
                    int dom = buff[2];
                    int num = buff[3];
                    bool addIt = true;
                    for (int j = 0; j < balanceInfo[dom].size(); j++)
                        if (balanceInfo[dom][j] == dst)
                        {
                            addIt = false;
                            break;
                        }
                    
                    if (addIt)
                    {
                        balanceInfo[dom].push_back(dst);
                        balanceInfo[dom].push_back(num);
                    }
                }
                */
                
                delete [] buff;
                recvIntBufferMap[req] = NULL;
            }
        
            for (int i = 0; i < num; i++)
                PostRecvStatusReq(indices[i]);
        }
            
        delete [] status;
        delete [] indices;
        if (num == 0)
            break;
    }
    communicationTime += visitTimer->StopTimer(communicationTimer,
                                               "StreamlineCommunication");
#endif
    return done;
}

// ****************************************************************************
//  Method: avtStreamlineFilter::AsyncSendSlaveMsgs
//
//  Purpose:
//      Send messages to slave processes.
//
//  
//
//  Programmer: Dave Pugmire
//  Creation:   Dec 18, 2008
//
//  Modifications:
//
//
// ****************************************************************************

void
avtStreamlineFilter::AsyncSendSlaveMsgs(int dst, int msg, std::vector<int> &info)
{
#ifdef PARALLEL
    int communicationTimer = visitTimer->StartTimer();
    
    int *buff = new int[statusMsgSz];
    
    MPI_Request req;
    buff[0] = msg;
    for (int j = 0; j < info.size(); j++)
        buff[1+j] = info[j];
        
    MPI_Isend(buff, statusMsgSz, MPI_INT, dst, STATUS_TAG, VISIT_MPI_COMM, &req);
    numBytesSent += (sizeof(int) *statusMsgSz);
    numStatusCommunicated++;
    sendIntBufferMap[req] = buff;
    communicationTime += visitTimer->StopTimer(communicationTimer, 
                                               "StreamlineCommunication");
#endif
}

// ****************************************************************************
//  Method: avtStreamlineFilter::AsyncSendSlaveMsgs
//
//  Purpose:
//      Send messages to slave processes.
//
//  
//
//  Programmer: Dave Pugmire
//  Creation:   Dec 18, 2008
//
//  Modifications:
//
//
// ****************************************************************************

void
avtStreamlineFilter::AsyncSendSlaveMsgs(int msg, std::vector<int> &info)
{
#ifdef PARALLEL
    debug1<<"AsyncSendSlaveMsgs()\n";

    for (int i = 1; i < nProcs; i++)
        AsyncSendSlaveMsgs(i, msg, info);

#endif
}

class SlaveInfo
{
  public:
    SlaveInfo( int r, int nDomains )
    {
        justUpdated = false;
        balance = 0.0;
        canGive = canAccept = slCount = slLoadedCount = slOOBCount = 0;
        domLoadedCount = 0;
        domainCnt.resize(nDomains, 0);
        domainLoaded.resize(nDomains, false);
        rank = r;
    }
    ~SlaveInfo() {}

    void AddSL( int slDomain)
    {
        //We assume that it will get loaded..
        domainLoaded[slDomain] = true;
        domainCnt[slDomain]++;
        slCount++;
        slLoadedCount++;
        justUpdated = false;
    }
    
    void RemoveSL( int dom )
    {
        domainCnt[dom]--;
        //We assume that it will get loaded..
        slCount--;
        if (domainLoaded[dom])
            slLoadedCount--;
        else
            slOOBCount--;
        justUpdated = false;
    }

    void Update( vector<int> &status, bool debug=false )
    {
        justUpdated = (status[0] != 0);
        if (status[0] == 0)
        {
            if (debug)
                Debug();
            return;
        }
        status[0] = 0;
        
        slCount = 0;
        slLoadedCount = 0;
        slOOBCount = 0;
        domLoadedCount = 0;
        for (int i = 0; i < domainLoaded.size(); i++)
        {
            int cnt = status[i+1];
            if (cnt == 0)
            {
                domainCnt[i] = 0;
                domainLoaded[i] = false;
            }
            else if (cnt > 0)
            {
                cnt -= 1;  //1 means domain is loaded, slCnt = 1+
                domainCnt[i] = cnt;
                domainLoaded[i] = true;
                slCount += cnt;
                slLoadedCount += cnt;
            }
            else
            {
                domainCnt[i] = -cnt;
                domainLoaded[i] = false;
                slCount += (-cnt);
                slOOBCount += (-cnt);
            }
        }
        for (int i = 0; i < domainLoaded.size(); i++)
            domLoadedCount += (domainLoaded[i] ? 1 : 0);
        
        if (debug)
            Debug();
    }

    void UpdateBalance(int perfectBalance)
    {
        if (!justUpdated)
            return;
        
        balance = (double)slCount / (double)(perfectBalance);
        canGive = 0;
        canAccept = 0;
        if (balance > 1.20)
            canGive = slCount - perfectBalance;
        else if (balance < 0.80)
            canAccept = perfectBalance - slCount;
    }
    
    void Reset()
    {
        justUpdated = false;
    }

    void Debug()
    {
        debug1<<setw(2)<<rank<<": "<<setw(3)<<slCount<<" ("<<setw(3)<<slLoadedCount<<", "<<setw(3)<<slOOBCount<<") [";
        for (int i = 0; i < domainCnt.size(); i++)
        {
            int N = 0;
            int cnt = domainCnt[i];
            if (domainLoaded[i])
                N = (cnt > 0 ? cnt+1 : 1);
            else
                N = -cnt;
            
            debug1<<setw(4)<<N<<" ";
        }
        debug1<<"] ("<<domLoadedCount<<")";
        if (justUpdated)
        {
            debug1<<" ***";
            if (slLoadedCount == 0)
                debug1<<" SLACKER: "<<rank;
            else
                debug1<<" UPDATE: "<<rank;              
        }
        debug1<<endl;
    }

    bool justUpdated;
    double balance;
    int canGive, canAccept, slCount, slLoadedCount, slOOBCount, rank;
    int domLoadedCount;
    vector<int> domainCnt;
    vector<bool> domainLoaded;
};

static bool domCntCompare( const int *a, const int *b) { return a[1] > b[1]; }

static const int NO_SORT = 0;
static const int RANDOM_SORT = 1;
static const int LOAD_SORT = 2;

// ****************************************************************************
//  Method: avtStreamlineFilter::FindSlackers
//
//  Purpose:
//      Find slaves who have no work to do.
//
//  
//
//  Programmer: Dave Pugmire
//  Creation:   Dec 18, 2008
//
//  Modifications:
//
//
// ****************************************************************************

void
avtStreamlineFilter::FindSlackers(std::vector<int> &slackers,
                                  int oobFactor,
                                  int SortMethod,
                                  bool checkJustUpdated)
{
#ifdef PARALLEL
    
    slackers.resize(0);
    for (int i = 1; i < slaveInfo.size(); i++)
        if (slaveInfo[i].slLoadedCount == 0 ||
            slaveInfo[i].justUpdated && checkJustUpdated)
        {
            if ( oobFactor != -1 &&
                 slaveInfo[i].slOOBCount > 0 &&
                 slaveInfo[i].slOOBCount < oobFactor)
                slackers.push_back(i);
            else
                slackers.push_back(i);          
        }

    if (SortMethod == RANDOM_SORT)
        random_shuffle(slackers.begin(), slackers.end());
    else if (SortMethod == LOAD_SORT)
    {
    }
    
#endif
}

// ****************************************************************************
//  Method: avtStreamlineFilter::UpdateStatus
//
//  Purpose:
//      Update status given new slave info.
//
//  
//
//  Programmer: Dave Pugmire
//  Creation:   Dec 18, 2008
//
//  Modifications:
//
//
// ****************************************************************************

void
avtStreamlineFilter::UpdateStatus( vector<avtStreamlineWrapper *> &streamlines,
                                   vector<vector<int> > &status)
{
#ifdef PARALLEL
    for (int i = 0; i < numDomains; i++)
    {
        slDomCnts[i] = 0;
        domLoaded[i] = 0;
    }
    for ( int i = 0; i < streamlines.size(); i++)
        slDomCnts[streamlines[i]->domain] ++;

    for (int i = 1; i < nProcs; i++)
        slaveInfo[i].Update(status[i]);


    for (int i = 1; i < slaveInfo.size(); i++)
        for ( int d = 0; d < numDomains; d++)
            if (slaveInfo[i].domainLoaded[d])
                domLoaded[d]++;

#endif
}

// ****************************************************************************
//  Method: avtStreamlineFilter::PrintStatus
//
//  Purpose:
//      Print status of slave information.
//
//  
//
//  Programmer: Dave Pugmire
//  Creation:   Dec 18, 2008
//
//  Modifications:
//
//
// ****************************************************************************

void avtStreamlineFilter::PrintStatus()
{
#ifdef PARALLEL
    debug1<<"DOM:               [";
    for ( int i = 0; i < numDomains; i++)
        debug1<<setw(4)<<i<<" ";
    debug1<<"]\n";
    debug1<<"Master:            [";
    for ( int i = 0; i < numDomains; i++)
        debug1<<setw(4)<<slDomCnts[i]<<" ";
    debug1<<"]\n";
    
    for (int i = 1; i < nProcs; i++)
        slaveInfo[i].Debug();
    debug1<<"DCounts:           [";
    for ( int i = 0; i < numDomains; i++)
        debug1<<setw(4)<<domLoaded[i]<<" ";
    debug1<<"]\n";

    vector<int> slaveSLs(numDomains,0);
    for (int i = 1; i < nProcs; i++)
        for (int j = 0; j < numDomains; j++)
            slaveSLs[j] += slaveInfo[i].domainCnt[j];
    debug1<<"SCounts:           [";
    int cnt = 0;
    for ( int i = 0; i < numDomains; i++)
    {
        debug1<<setw(4)<<slaveSLs[i]<<" ";
        cnt += slaveSLs[i];
    }
    debug1<<"] ("<<cnt<<")"<<endl;
    debug1<<endl;
#endif
}

// ****************************************************************************
//  Method: avtStreamlineFilter::Case1
//
//  Purpose:
//      Case1 of masterslave algorithm. Give SLs to slaves who have domain
//      loaded.
//
//  
//
//  Programmer: Dave Pugmire
//  Creation:   Dec 18, 2008
//
//  Modifications:
//
//
// ****************************************************************************

void
avtStreamlineFilter::Case1( vector<avtStreamlineWrapper *> &streamlines )
{
#ifdef PARALLEL
    if (streamlines.size() == 0)
        return;
    
    vector<int> slackers;
    FindSlackers(slackers, -1, RANDOM_SORT);
    
    vector< vector< avtStreamlineWrapper *> > distributeStreamlines(nProcs);
    bool streamlinesToSend = false;
    
    for (int i = 0; i < slackers.size(); i++)
    {
        if (streamlines.size() == 0)
            break;
        
        int cnt = 0;
        vector<avtStreamlineWrapper *> tmp;

        int slackerRank = slaveInfo[slackers[i]].rank;
        for (int j = 0; j < streamlines.size(); j++)
        {
            avtStreamlineWrapper *slSeg = streamlines[j];
            if (slaveInfo[slackerRank].domainLoaded[slSeg->domain] && cnt < maxCount)
            {
                distributeStreamlines[slackerRank].push_back(slSeg);
                slaveInfo[slackerRank].AddSL(slSeg->domain);
                cnt++;
            }
            else
                tmp.push_back(slSeg);
        }

        if (cnt > 0)
        {
            streamlinesToSend = true;
            streamlines.resize(0);
            for (int j = 0; j < tmp.size(); j++)
                streamlines.push_back(tmp[j]);

            if (distributeStreamlines[slackerRank].size() > 0)
            {
                debug1<<"Case 1: "<<slackerRank<<" Send "<<cnt<<" SLs [";
                for (int j = 0; j < distributeStreamlines[slackerRank].size(); j++)
                    debug1<<distributeStreamlines[slackerRank][j]->domain<<" ";
                debug1<<"]\n";
            }
        }
    }

    if (streamlinesToSend)
    {
        int earlyTerminations;
        AsyncExchangeStreamlines(streamlines, 
                                 distributeStreamlines,
                                 earlyTerminations);
        if (earlyTerminations != 0)
            EXCEPTION0(ImproperUseException);
    }
#endif
}

// ****************************************************************************
//  Method: avtStreamlineFilter::Case2
//
//  Purpose:
//      Case2 of masterslave algorithm. Give SLs to slaves and force domain
//      loads.
//
//  
//
//  Programmer: Dave Pugmire
//  Creation:   Dec 18, 2008
//
//  Modifications:
//
//
// ****************************************************************************

void
avtStreamlineFilter::Case2( vector<avtStreamlineWrapper *> &streamlines )
{
#ifdef PARALLEL
    if (streamlines.size() == 0)
        return;
    
    vector<int> slackers;
    FindSlackers(slackers, -1, RANDOM_SORT);
    
    vector< vector< avtStreamlineWrapper *> > distributeStreamlines(nProcs);
    bool streamlinesToSend = false;

    for (int s = 0; s < slackers.size(); s++)
    {
        if (streamlines.size() == 0)
            break;
        
        int slackerRank = slaveInfo[slackers[s]].rank;
    
        vector<int*> domCnts;
        for (int i = 0; i < slDomCnts.size(); i++)
            if (slDomCnts[i] > 0)
            {
                int *entry = new int[2];
                entry[0] = i;
                entry[1] = slDomCnts[i];
                domCnts.push_back(entry);
            }

        if (slDomCnts.size() == 0)
            continue;

        //Sort on SL count per domain.
        sort(domCnts.begin(),domCnts.end(), domCntCompare);
        if (false)
        {
            debug1<<"SL sort: [";
            for (int i = 0; i < domCnts.size(); i++)
                debug1<<domCnts[i][0]<<" "<<domCnts[i][1]<<", ";
            debug1<<"]\n";
        }

        int domToLoad = -1;
        for (int i = 0; i < domCnts.size(); i++)
        {
            bool anyLoaded = false;
            for (int j = 1; j < slaveInfo.size(); j++)
                if (slaveInfo[j].domainLoaded[domCnts[i][0]])
                    anyLoaded = true;
            if (!anyLoaded)
            {
                domToLoad = domCnts[i][0];
                break;
            }
        }

        if (domToLoad == -1)
        {
            random_shuffle(domCnts.begin(), domCnts.end());
            domToLoad = domCnts[0][0];
            //debug1<<"All doms loaded. Random pick: "<<domToLoad<<endl;
        }

        int cnt = 0;
        vector<avtStreamlineWrapper *> tmp;
        for (int i = 0; i < streamlines.size(); i++)
        {
            avtStreamlineWrapper *slSeg = streamlines[i];
            if (slSeg->domain == domToLoad && cnt < maxCount)
            {
                distributeStreamlines[slackerRank].push_back(slSeg);
                slaveInfo[slackerRank].AddSL(slSeg->domain);
                cnt++;
            }
            else
                tmp.push_back(slSeg);
        }
        
        for (int i = 0; i < domCnts.size(); i++)
            delete domCnts[i];

        if (cnt > 0)
        {
            streamlinesToSend = true;
            streamlines.resize(0);
            for (int j = 0; j < tmp.size(); j++)
                streamlines.push_back(tmp[j]);
            
            if (distributeStreamlines[slackerRank].size() > 0)
            {
                debug1<<"Case 2: "<<slackerRank<<" Send "<<cnt<<" SLs [";
                for (int j = 0; j < distributeStreamlines[slackerRank].size(); j++)
                    debug1<<distributeStreamlines[slackerRank][j]->domain<<" ";
                debug1<<"]\n";
            }
        }
    }

    if (streamlinesToSend)
    {
        int earlyTerminations;
        AsyncExchangeStreamlines(streamlines, 
                                 distributeStreamlines,
                                 earlyTerminations);
        if (earlyTerminations != 0)
            EXCEPTION0(ImproperUseException);
    }    
    
#endif
}

// ****************************************************************************
//  Method: avtStreamlineFilter::Case3
//
//  Purpose:
//      Case3 of masterslave algorithm. Send SLs to slaves with domain loaded.
//  
//
//  Programmer: Dave Pugmire
//  Creation:   Dec 18, 2008
//
//  Modifications:
//
//
// ****************************************************************************

void
avtStreamlineFilter::Case3( vector<avtStreamlineWrapper *> &streamlines,
                            int overloadFactor,
                            int NDomainFactor)
{
#ifdef PARALLEL
    vector<int> slackers;
    FindSlackers(slackers, NDomainFactor, RANDOM_SORT);

    vector<int> sender, recv, dom;
    for (int i = 0; i < slackers.size(); i++)
    {
        int slackerRank = slackers[i];
        //debug1<<"Case 3: slackerRank="<<slackerRank<<endl;
        
        for (int d = 0; d < numDomains; d++)
        {
            vector<int> domPartner;
            if ( !slaveInfo[slackerRank].domainLoaded[d] &&
                 slaveInfo[slackerRank].domainCnt[d] > 0)
            {
                //debug1<<"   dom= "<<d<<endl;
                // Find a partner who has the domain and has fewer than overloadFactor SLs.
                for (int j = 1; j < nProcs; j++)
                {
                    if (j != slackerRank && slaveInfo[j].domainLoaded[d] &&
                        slaveInfo[j].slCount < overloadFactor)
                    {
                        //debug1<<"      partner= "<<j<<endl;
                        domPartner.push_back(j);
                    }
                }
            }
            
            if (domPartner.size() > 0)
            {
                //debug1<<"domPartner: [";
                //for(int k=0; k<domPartner.size(); k++) debug1<<domPartner[k]<<" ";
                //debug1<<"]\n";
                
                random_shuffle(domPartner.begin(), domPartner.end());
                
                //debug1<<"random sort: [";
                //for(int k=0; k<domPartner.size(); k++) debug1<<domPartner[k]<<" ";
                //debug1<<"]\n";                
                for (int j = 0; j < domPartner.size(); j++)
                {
                    sender.push_back(slackerRank);
                    recv.push_back(domPartner[j]);
                    dom.push_back(d);
                    
                }
            }
        }
    }

    int maxSLsToSend = 5*maxCount;
    int maxDestSLs = overloadFactor;

    // Send messages out.
    for (int i = 0; i < sender.size(); i++)
    {
        SlaveInfo &recvSlave = slaveInfo[recv[i]];
        SlaveInfo &sendSlave = slaveInfo[sender[i]];
        int d = dom[i];

        int n = sendSlave.domainCnt[d];
        if (n > maxSLsToSend)
            n = maxSLsToSend;

        //Dest already has enough work.
        if (recvSlave.slCount > maxDestSLs)
            continue;
    
        // Cap it.
        if (recvSlave.slCount + n > maxDestSLs)
            n = maxDestSLs - recvSlave.slCount;

        vector<int> msg;
        msg.push_back(recvSlave.rank);
        msg.push_back(d);
        msg.push_back(n);
        
        for (int i = 0; i < n; i++)
        {
            recvSlave.AddSL(d);
            sendSlave.RemoveSL(d);
        }

        if (n > 0)
        {
            debug1<<"Case 3: "<<sendSlave.rank<<" ==["<<n<<"]==> "<<recvSlave.rank<<"  d= "<<d;
            debug1<<" ***   "<<recvSlave.rank<<" now has "<<recvSlave.slCount<<" cap= "<<maxDestSLs<<endl;
            AsyncSendSlaveMsgs(sendSlave.rank, MSG_SEND_SL, msg);
        }
    }
    
#endif
}

// ****************************************************************************
//  Method: avtStreamlineFilter::Case4
//
//  Purpose:
//      Case4 of masterslave algorithm. Tell slave to load domain.
//  
//
//  Programmer: Dave Pugmire
//  Creation:   Dec 18, 2008
//
//  Modifications:
//
//
// ****************************************************************************

void
avtStreamlineFilter::Case4( vector<avtStreamlineWrapper *> &streamlines,
                            int oobThreshold)
{
#ifdef PARALLEL
    vector<int> slackers;
    for (int i = 1; i < nProcs; i++)
        if (slaveInfo[i].slLoadedCount == 0 && slaveInfo[i].slOOBCount >= oobThreshold)
            slackers.push_back(i);
    random_shuffle(slackers.begin(), slackers.end());

    //debug1<<"Case 4- nSlackers= "<<slackers.size()<<endl;
    for (int i = 0; i < slackers.size(); i++)
    {
        int idx = slackers[i];
        vector<int> info(1);
        int domToLoad = -1, maxCnt=-1;
        for (int j = 0; j < slaveInfo[idx].domainCnt.size(); j++)
        {
            if (slaveInfo[idx].domainCnt[j] > 0 && slaveInfo[idx].domainCnt[j] > maxCnt)
            {
                domToLoad = j;
                maxCnt = slaveInfo[idx].domainCnt[j];
            }
        }
        
        if (domToLoad != -1)
        {
            info[0] = domToLoad;
            AsyncSendSlaveMsgs(idx, MSG_LOAD_DOMAIN, info);
            debug1<<"Case 4: "<<idx<<" load dom= "<<domToLoad<<" oobThreshold: "<<oobThreshold<<endl;
            slaveInfo[idx].domainLoaded[domToLoad] = true;
            slaveInfo[idx].slLoadedCount += slaveInfo[idx].domainCnt[domToLoad];
            slaveInfo[idx].slOOBCount -= slaveInfo[idx].domainCnt[domToLoad];
        }
    }
    
#endif
}

// ****************************************************************************
//  Method: avtStreamlineFilter::MasterSlave
//
//  Purpose:
//      Master slave algorithm
//
//  Programmer: Dave Pugmire
//  Creation:   November 17, 2008
//
//  Modifications:
//
//    Mark C. Miller, Sat Dec 20 01:09:55 PST 2008
//    Replaced calls to usleep() with nanosleep().
//
// ****************************************************************************

void
avtStreamlineFilter::MasterSlave(std::vector<avtStreamlineWrapper *> &seedpoints)
{
#ifdef PARALLEL
    int numSeedPoints = seedpoints.size();
    int totalNumActiveStreamlines = numSeedPoints;
    bool doSleeping = true;
    int MSleep = 500, SSleep = 1000;
    
    debug1<<"MasterSlave() slCount= "<<numSeedPoints<<endl;

    vector< vector< avtStreamlineWrapper *> > distributeStreamlines(nProcs);
    vector<avtStreamlineWrapper *> streamlines, terminatedStreamlines;

    int nSlaves = nProcs-1;
    statusMsgSz = numDomains+1+1;
    SortStreamlines(seedpoints);
    
    //Initialize async stuff.
    InitRequests();
    
    //Master.
    if ( PAR_Rank() == 0 )
    {
        //Master owns all the SLs.
        for (int i = 0; i < seedpoints.size(); i++)
            streamlines.push_back(seedpoints[i]);
        
        vector< vector<int> > status(nProcs);
        for ( int i = 0; i < nProcs; i++)
        {
            status[i].resize(numDomains+1,0);
            slaveInfo.push_back(SlaveInfo(i, numDomains));
        }
        slDomCnts.resize(numDomains,0);
        domLoaded.resize(numDomains,0);
        
        Barrier();
        
        while (totalNumActiveStreamlines > 0)
        {
            numIterations++;
            
            int terminates = 0;
            bool newStatus = AsyncRecvStatus2(terminates, status);
            totalNumActiveStreamlines -= terminates;
            
            if (newStatus)
            {
                debug1<<"Master: New status\n";
                UpdateStatus(streamlines, status);
                PrintStatus();

                Case4(streamlines, 3*maxCount);
                Case1(streamlines);
                Case4(streamlines, 1*maxCount);
                
                Case2(streamlines);
                int case3OverloadFactor = 10*maxCount, case3NDomainFactor = 3*maxCount;         
                Case3(streamlines,case3OverloadFactor, case3NDomainFactor);
                Case4(streamlines, 0);

                UpdateStatus(streamlines, status);
                debug1<<endl<<"Post-Mortem"<<endl;
                PrintStatus();
            }
            else
            {
                debug1<<"Nothing to do: "<<totalNumActiveStreamlines<<endl;
                int sleepTimer = visitTimer->StartTimer();              
                if (doSleeping)
                {
                    struct timespec ts = {0, MSleep*1000};
                    nanosleep(&ts, 0);
                }

                sleepTime += visitTimer->StopTimer(sleepTimer, "SleepTimer");           
                
                numBusyLoopIterations++;
                bool workWhileWait = false;
                if (workWhileWait && streamlines.size() > 0)
                {
                    //Integrate one SL a max number os steps.
                    avtStreamlineWrapper *sl = NULL;

                    // See if we have an already loaded domain...
                    for ( int i = 0; i < streamlines.size(); i++)
                    {
                        avtStreamlineWrapper *slSeg = streamlines[i];
                        if (DomainLoaded(slSeg->domain))
                        {
                            sl = slSeg;
                            break;
                        }
                    }

                    // if not, then load one.
                    if (sl == NULL)
                        sl = streamlines[0];

                    int maxSteps = 1;
                    IntegrateStreamline(sl, maxSteps);
                    debug1<<"busy wait: status= "<<sl->status<<endl;
                    
                    if (sl->status == avtStreamlineWrapper::TERMINATE)
                    {
                        terminatedStreamlines.push_back(sl);
                        totalNumActiveStreamlines--;
                        
                        // Remove from streamlines....
                        vector<avtStreamlineWrapper *>tmp;
                        for ( int i = 0; i < streamlines.size(); i++)
                            if (streamlines[i] != sl)
                                tmp.push_back(streamlines[i]);
                        streamlines.resize(0);
                        for (int i = 0; i < tmp.size(); i++)
                            streamlines.push_back(tmp[i]);
                    }
                }
            }
        }
        vector<int> info;
        AsyncSendSlaveMsgs(MSG_DONE, info);
    }
    
    // Slave
    else
    {
        //Ideas. Don't update status unless I integrate something. Master knows what it gave me.
        
        int myMaster = 0;
        vector<int> status(numDomains,0), prevStatus(numDomains,0);
        for (int i = 0; i < numDomains; i++)
            status[i] = prevStatus[i] = DomainLoaded(i);
        
        // Send initial status.
        debug1<<"Slave: Send initial status.\n";
        AsyncSendStatus2(myMaster, 0, status);
        Barrier();
        
        while (true)
        {
            int terminates = 0;
            bool didSomething = false;
            while (true)
            {
                numIterations++;
                // Keep integrating until I have no streamlines left.
                vector<avtStreamlineWrapper *> activeSLs;

                debug1<<"S: sls= "<<streamlines.size()<<endl;
                if (streamlines.size() > 0)
                {
                    SortStreamlines(streamlines);
                    for (int i = 0; i < streamlines.size(); i++)
                    {
                        didSomething = true;
                        if (i==0)
                            debug1<<"S: Integrate "<<streamlines.size()<<endl;

                        avtStreamlineWrapper *slSeg = streamlines[i];
                        if (!DomainLoaded(slSeg->domain))
                            activeSLs.push_back(slSeg);
                        else
                        {
                            IntegrateStreamline(slSeg);
                            if (slSeg->status == avtStreamlineWrapper::TERMINATE)
                            {
                                terminatedStreamlines.push_back(slSeg);
                                terminates++;
                            }
                            else
                                activeSLs.push_back(slSeg);
                        }
                    }
                
                    streamlines.resize(0);
                    for ( int i = 0; i < activeSLs.size(); i++)
                        streamlines.push_back(activeSLs[i]);
                }

                // Get any new streamlines.
                int earlyTerminations;
                bool newStreamlines = AsyncExchangeStreamlines(streamlines, 
                                                               distributeStreamlines,
                                                               earlyTerminations);
                terminates += earlyTerminations;

                // See if I get a pink slip....
                bool workToDo = false;
                for (int i = 0; !workToDo && i < streamlines.size(); i++)
                    if (DomainLoaded(streamlines[i]->domain))
                        workToDo = true;
                
                if (!workToDo)
                {
                    int sleepTimer = visitTimer->StartTimer();          
                    if (doSleeping)
                    {
                        struct timespec ts = {0, SSleep * 1000};
                        nanosleep(&ts, 0);
                    }
                    sleepTime += visitTimer->StopTimer(sleepTimer, "SleepTimer");                                   
                    break;
                }
            }

            if (!didSomething)
            {
                numBusyLoopIterations++;
                debug1<<"S: Did Nothing!\n";
            }

            //Check for any messages.
            vector< vector<int> > msgs;
            bool done = AsyncRecvMasterMsg(msgs);
            CheckPendingSendRequests();

            for (int i = 0; i < msgs.size(); i++)
            {
                if (msgs[i][0] == MSG_LOAD_DOMAIN)
                {
                    debug1<<"S: Loading domain= "<<msgs[i][1]<<endl;
                    for (int j = 0; j < streamlines.size(); j++)
                        if (streamlines[j]->domain == msgs[i][1])
                        {
                            debug1<<"S: We have SLs from this domain.\n";
                            break;
                        }
                    
                    GetDomain(msgs[i][1]);
                }
                
                else if (msgs[i][0] == MSG_SEND_SL)
                {
                    int dst = msgs[i][1];
                    int dom = msgs[i][2];
                    int num = msgs[i][3];
                    vector<avtStreamlineWrapper *> activeSLs;
                    for (int i = 0; i < streamlines.size(); i++)
                    {
                        avtStreamlineWrapper *slSeg = streamlines[i];
                        if (slSeg->domain == dom && distributeStreamlines[dst].size() < num)
                            distributeStreamlines[dst].push_back(slSeg);
                        else
                            activeSLs.push_back(slSeg);
                    }

                    streamlines.resize(0);
                    for ( int i = 0; i < activeSLs.size(); i++)
                        streamlines.push_back(activeSLs[i]);
                }
            }

            int earlyTerminations;
            bool newStreamlines = AsyncExchangeStreamlines(streamlines, 
                                                           distributeStreamlines,
                                                           earlyTerminations);
            if (done)
                break;

            
            //Update status to master.
            for (int i = 0; i < numDomains; i++)
                status[i] = (DomainLoaded(i) ? 1 : 0);
            
            for (int i = 0; i < streamlines.size(); i++)
            {
                avtStreamlineWrapper *slSeg = streamlines[i];
                if (! DomainLoaded(slSeg->domain))
                    status[slSeg->domain] --;
                else
                    status[slSeg->domain] ++;
            }
            debug1<<"Out of work: status= [";
            for (int i = 0; i < status.size(); i++)
                debug1<<status[i]<<" ";
            debug1<<"]\n";

             bool statusChanged = false;
            for(int i = 0; !statusChanged && i < status.size(); i++)
                statusChanged = (prevStatus[i] != status[i]);
            
            for(int i = 0; i < status.size(); i++)
                prevStatus[i] = status[i];

            //statusChanged = true;
            if (statusChanged || terminates > 0)
            {
                debug1<<"Slave: sc= "<<statusChanged<<" term= "<<terminates<<endl;
                AsyncSendStatus2(myMaster, terminates, status);
            }
        }
    }
    debug1<<rank<<": I am done: "<<terminatedStreamlines.size()<<endl;
    // All done, make the output.
    CreateStreamlineOutput(terminatedStreamlines);
    ReportStatistics(terminatedStreamlines);

    for (int i = 0; i < terminatedStreamlines.size(); i++)
    {
        avtStreamlineWrapper *slSeg = 
                             (avtStreamlineWrapper *) terminatedStreamlines[i];
        delete slSeg;
    }
    CleanupAsynchronous();
#endif
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
    debug1 << "\navtStreamlineFilter::IntegrateStreamline(dom= "
           << slSeg->domain<<")\n";

    slSeg->status = avtStreamlineWrapper::UNSET;
    //Get the required domain.
    vtkDataSet *ds = GetDomain(slSeg->domain);
    if (ds == NULL)
    {
        slSeg->status = avtStreamlineWrapper::TERMINATE;
    }
    else
    {
        // Integrate over this domain.
        slSeg->UpdateDomainCount(slSeg->domain);

        int integrationTimer = visitTimer->StartTimer();
        double extents[6] = { 0.,0., 0.,0., 0.,0. };
        intervalTree->GetElementExtents(slSeg->domain, extents);
        avtIVPSolver::Result result = IntegrateDomain(slSeg, ds, extents, maxSteps);
        integrationTime += visitTimer->StopTimer(integrationTimer, 
                                                 "StreamlineIntegration");
        numIntegrationSteps++;

        debug1<<"Back from SLINT\n";
        //SL exited this domain.
        if (slSeg->status == avtStreamlineWrapper::OUTOFBOUNDS)
        {
            SetDomain(slSeg);
        }
        //SL terminates.
        else
        {
            slSeg->status = avtStreamlineWrapper::TERMINATE;
            slSeg->domain = -1;
        }
    }
    
    debug1 << "   IntegrateStreamline DONE: status = " << (slSeg->status==avtStreamlineWrapper::TERMINATE ? "TERMINATE" : "OOB")
           << " domCnt= "<<slSeg->seedPtDomainList.size()<<endl;
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
// ****************************************************************************

avtIVPSolver::Result
avtStreamlineFilter::IntegrateDomain(avtStreamlineWrapper *slSeg, 
                                     vtkDataSet *ds,
                                     double *extents,
                                     int maxSteps )
{
    avtDataAttributes &a = GetInput()->GetInfo().GetAttributes();
    haveGhostZones = (a.GetContainsGhostZones()==AVT_NO_GHOSTS ? false : true);

    debug1<< "avtStreamlineFilter::IntegrateDomain(dom= "
          <<slSeg->domain<<") HGZ = "<<haveGhostZones <<endl;

    // prepare streamline integration ingredients
    vtkInterpolatedVelocityField* velocity=vtkInterpolatedVelocityField::New();
    
    // See if we have cell cenetered data...
    vtkCellDataToPointData *cellToPt = NULL;
    if (ds->GetPointData()->GetVectors() == NULL)
    {
        cellToPt = vtkCellDataToPointData::New();
        
        cellToPt->SetInput(ds);
        cellToPt->Update();
        velocity->AddDataSet(cellToPt->GetOutput());
    }
    else
        velocity->AddDataSet(ds);
    
    velocity->CachingOn();
    avtIVPVTKField field(velocity);
    bool timeMode = (terminationType==STREAMLINE_TERMINATE_TIME);
    double end = termination;
    if (slSeg->dir == avtStreamlineWrapper::BWD)
        end = - end;
    
    //slSeg->Debug();
    bool doVorticity = ((coloringMethod == STREAMLINE_COLOR_VORTICITY)
                        || (displayMethod == STREAMLINE_DISPLAY_RIBBONS));
    avtIVPSolver::Result result = slSeg->sl->Advance(&field,
                                                     timeMode,
                                                     end,
                                                     maxSteps,
                                                     doVorticity,
                                                     haveGhostZones,
                                                     extents);
    //slSeg->Debug();

    if (result == avtIVPSolver::OUTSIDE_DOMAIN)
    {
        slSeg->status = avtStreamlineWrapper::OUTOFBOUNDS;
        int oldDomain = slSeg->domain;

        //Set the new domain.
        SetDomain(slSeg);

        // See if we are really done.
        if (slSeg->seedPtDomainList.size() == 0 ||
             (slSeg->seedPtDomainList.size() == 1 && 
             (slSeg->domain == oldDomain || slSeg->domain == -1)))
        {
            debug1<<"TERMINATE: sz= "<<slSeg->seedPtDomainList.size()<<" dom= "
                  <<slSeg->domain<<" oldDom= "<<oldDomain<<endl;
            //slSeg->Debug();
            slSeg->status = avtStreamlineWrapper::TERMINATE;
        }
    }
    else
        slSeg->status = avtStreamlineWrapper::TERMINATE;
    
    velocity->Delete();
    if (cellToPt)
        cellToPt->Delete();

    return result;
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
    pt3d pt;
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
// ****************************************************************************

void
avtStreamlineFilter::GetSeedPoints(std::vector<avtStreamlineWrapper *> &pts)
{
    std::vector<pt3d> candidatePts;

    // Add seed points based on the source.
    if(sourceType == STREAMLINE_SOURCE_POINT)
    {
        double z0 = (dataSpatialDimension > 2) ? pointSource[2] : 0.0;
        pt3d pt(pointSource[0], pointSource[1], z0);
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
            pt3d p(pt[0], pt[1], pt[2]);
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
            pt3d p(pt[0], pt[1], pt[2]);
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
            pt3d p(pt[0], pt[1], pt[2]);
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
                    pt3d p(X,Y,Z);
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
        intervalTree->GetElementsListFromRange(candidatePts[i].xyz, 
                                               candidatePts[i].xyz, dl);

        //cout<<i<<": "<<candidatePts[i].xyz[0]<<" "<<candidatePts[i].xyz[1]<<" "<<candidatePts[i].xyz[2]<<" dl= "<<dl.size()<<endl;
        // seed in no domains, try to wiggle it into a DS.
        if (dl.size() == 0)
        {
            //Try to wiggle it by 0.5% of the dataset size.
            double offset[3], wiggle[3] = {dX*0.005, dY*0.005, dZ*0.005};
            bool foundGoodPt = false;
            for ( int w = 0; w < 100; w++ )
            {
                pt3d wigglePt(candidatePts[i].xyz[0]+wiggle[0]*randMinus1_1(),
                              candidatePts[i].xyz[1]+wiggle[1]*randMinus1_1(),
                              candidatePts[i].xyz[2]+wiggle[2]*randMinus1_1());
                
                vector<int> dl2;
                intervalTree->GetElementsListFromRange(wigglePt.xyz, wigglePt.xyz, dl2);
                //cout<<"Wiggle it: "<<i<<": "<<wigglePt.values()[0]<<" "<<wigglePt.values()[1]<<" "<<wigglePt.values()[2];
                //cout<<" domain cnt: "<<dl2.size()<<endl;
                if ( dl2.size() > 0 )
                {
                    candidatePts[i].xyz[0] = wigglePt.xyz[0];
                    candidatePts[i].xyz[1] = wigglePt.xyz[1];
                    candidatePts[i].xyz[2] = wigglePt.xyz[2];
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

        debug1<<"Candidate pt: "<<i<<" ["<<candidatePts[i].xyz[0]<<", "
              <<candidatePts[i].xyz[1]<<", "<<candidatePts[i].xyz[2];
        debug1<<" dom =[";
        for (int j = 0; j < dl.size();j++)
            debug1<<dl[j]<<", ";
        debug1<<"]\n";
        
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
        avtVec pt(ptDom[i].pt.xyz[0], ptDom[i].pt.xyz[1], ptDom[i].pt.xyz[2]);
        
        if (streamlineDirection == VTK_INTEGRATE_FORWARD ||
             streamlineDirection == VTK_INTEGRATE_BOTH_DIRECTIONS)
        {
            avtStreamline *sl = new avtStreamline(solver, 0.0, pt);
            avtStreamlineWrapper *slSeg;
            slSeg = new avtStreamlineWrapper(sl,
                                             avtStreamlineWrapper::FWD,
                                             ptDom[i].id);
            slSeg->domain = ptDom[i].domain;
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
// ****************************************************************************

avtContract_p
avtStreamlineFilter::ModifyContract(avtContract_p in_contract)
{
    avtDataRequest_p in_dr = in_contract->GetDataRequest();
    avtDataRequest_p out_dr = NULL;

    if (strcmp(in_dr->GetVariable(), "colorVar") == 0)
    {
        // The avtStreamlinePlot requested "colorVar", so remove that from the
        // contract now.
        out_dr = new avtDataRequest(in_dr,in_dr->GetOriginalVariable());
    }

    avtContract_p out_contract;
    if ( *out_dr )
        out_contract = new avtContract(in_contract, out_dr);
    else
        out_contract = new avtContract(in_contract);

    out_contract->GetDataRequest()->SetDesiredGhostDataType(GHOST_ZONE_DATA);

    return avtDatasetOnDemandFilter::ModifyContract(out_contract);
}


