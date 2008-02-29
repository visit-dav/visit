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
//                             avtClipFilter.C                               //
// ************************************************************************* //

#include <avtClipFilter.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkIdList.h>
#include <vtkImplicitBoolean.h>
#include <vtkMergePoints.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkSphere.h>
#include <vtkStructuredPoints.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVisItClipper.h>
#include <vtkVisItUtility.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <BadVectorException.h>
#include <TimingsManager.h>
#include <maptypes.h>


// ****************************************************************************
//  Method: avtClipFilter constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 7, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Jun 20 13:35:27 PDT 2001
//    Removed vtkGeometryFilter.
//
//    Hank Childs, Sun Aug 18 09:45:58 PDT 2002
//    Initialized some new data members for tracking connectivity.
//
//    Jeremy Meredith, Fri Aug  8 09:18:40 PDT 2003
//    Removed subdivision and connectivity flags.  Added fastClipper.
//
//    Jeremy Meredith, Wed May  5 13:05:35 PDT 2004
//    Made my fast clipper support 2D, and removed the old generic
//    VTK data set clipper.
//
//    Hank Childs, Thu Mar 10 14:33:32 PST 2005
//    Remove filters.  They are now instantiated on the fly.
//
// ****************************************************************************

avtClipFilter::avtClipFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtClipFilter destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 7, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Jun 20 13:35:27 PDT 2001
//    Removed vtkGeometryFilter.
//
//    Jeremy Meredith, Mon Aug 11 17:04:29 PDT 2003
//    Added fastClipper.
//
//    Jeremy Meredith, Wed May  5 13:05:35 PDT 2004
//    Made my fast clipper support 2D, and removed the old generic
//    VTK data set clipper.
//
//    Hank Childs, Thu Mar 10 14:33:32 PST 2005
//    Remove filters.  They are now instantiated on the fly.
//
// ****************************************************************************

avtClipFilter::~avtClipFilter()
{
}

// ****************************************************************************
//  Method:  avtClipFilter::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August  8, 2003
//
// ****************************************************************************

avtFilter *
avtClipFilter::Create()
{
    return new avtClipFilter();
}


// ****************************************************************************
//  Method:      avtClipFilter::SetAtts
//
//  Purpose:
//      Sets the attributes for the Clip operator.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 25, 2001
//
//  Modifications:
//
// ****************************************************************************

void
avtClipFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const ClipAttributes*)a;
    double *d; 
    if (atts.GetPlane1Status())
    {
        d = atts.GetPlane1Normal();
        if (d[0] == 0. && d[1] == 0. && d[2] == 0.)
        {  
            EXCEPTION1(BadVectorException, "Normal");
            return;
        }  
    }
    if (atts.GetPlane2Status())
    {
        d = atts.GetPlane2Normal();
        if (d[0] == 0. && d[1] == 0. && d[2] == 0.)
        {  
            EXCEPTION1(BadVectorException, "Normal");
            return;
        }  
    }
    if (atts.GetPlane3Status())
    {
        d = atts.GetPlane3Normal();
        if (d[0] == 0. && d[1] == 0. && d[2] == 0.)
        {  
            EXCEPTION1(BadVectorException, "Normal");
            return;
        }  
    }
}


// ****************************************************************************
//  Method: avtClipFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtClipFilter with the given
//      parameters would result in an equivalent avtClipFilter.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 7, 2001 
//
// ****************************************************************************

bool
avtClipFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(ClipAttributes*)a);
}


// ****************************************************************************
//  Method: avtClipFilter::ClipAgainstPlanes
//
//  Purpose:
//      Clips a vtkDataSet against a list of planes and returns the resultant
//      vtkUnstructuredGrid.
//
//  Programmer: Sean Ahern
//  Creation:   February 19, 2008
//
// ****************************************************************************

vtkUnstructuredGrid *
avtClipFilter::ClipAgainstPlanes(vtkDataSet *in, bool nodesCritical,
                                 vtkPlane *p1, vtkPlane *p2,
                                 vtkPlane *p3)
{
    // The three planes.
    vtkImplicitBoolean *funcs1 = NULL, *funcs2 = NULL, *funcs3 = NULL;
    funcs1 = vtkImplicitBoolean::New();
    funcs1->AddFunction(p1);
    if (p2 != NULL)
    {
        funcs2 = vtkImplicitBoolean::New();
        funcs2->AddFunction(p2);
    }
    if (p3 != NULL)
    {
        funcs3 = vtkImplicitBoolean::New();
        funcs3->AddFunction(p3);
    }

    // Set up and apply the clipping filters.
    vtkUnstructuredGrid *ug = vtkUnstructuredGrid::New();

    vtkVisItClipper *clipper1 = NULL, *clipper2 = NULL,
                    *clipper3 = NULL, *last = NULL;

    clipper1 = vtkVisItClipper::New();
    clipper1->SetInput(in);
    clipper1->SetClipFunction(funcs1);
    clipper1->SetInsideOut(true);
    clipper1->SetRemoveWholeCells(nodesCritical);
    last = clipper1;

    if (p2 != NULL)
    {
        clipper2 = vtkVisItClipper::New();
        clipper2->SetInput(clipper1->GetOutput());
        clipper2->SetClipFunction(funcs2);
        clipper2->SetInsideOut(true);
        clipper2->SetRemoveWholeCells(nodesCritical);
        last = clipper2;
    }
    if (p3 != NULL)
    {
        clipper3 = vtkVisItClipper::New();
        clipper3->SetInput(clipper2->GetOutput());
        clipper3->SetClipFunction(funcs3);
        clipper3->SetInsideOut(true);
        clipper3->SetRemoveWholeCells(nodesCritical);
        last = clipper3;
    }
    last->SetOutput(ug);
    last->Update();

    funcs1->Delete();
    clipper1->Delete();
    if (p2 != NULL)
    {
        funcs2->Delete();
        clipper2->Delete();
    }
    if (p3 != NULL)
    {
        funcs3->Delete();
        clipper3->Delete();
    }

    return ug;
}

// ****************************************************************************
//  Method: avtClipFilter::ExecuteDataTree
//
//  Purpose:
//      Sends the specified input and output through the Clip filter.
//
//  Programmer: Jeremy Meredith
//  Creation:   August  8, 2003
//
//  Modifications:
//    Jeremy Meredith, Wed May  5 13:05:35 PDT 2004
//    Made my fast clipper support 2D, and removed the old generic
//    VTK data set clipper.
//
//    Hank Childs, Thu Mar 10 14:33:32 PST 2005
//    Instantiate filters on the fly.
//
//    Hank Childs, Sun Mar 27 12:00:18 PST 2005
//    Renamed to ProcessOneChunk.  Changed memory management.  Fix small
//    memory leak in error condition.
//
//    Kathleen Bonnell, Mon Jul 31 11:32:48 PDT 2006 
//    Handle 1D RectilinearGrids. 
//
//    Kathleen Bonnell, Thu Aug 24 16:23:16 PDT 2006 
//    Fix determination of 1D rgrid. 
//
//    Jeremy Meredith, Tue Aug 29 13:36:22 EDT 2006
//    Removed vtkClipPolyData; our fast clipper now supports poly data.
//    Made use of "nodesAreCritical", which specifies that cells should
//    be used for connectivity only, not interpolation, which means that
//    clip should either keep the cell whole or remove it entirely.
//
//    Hank Childs, Thu Aug 31 11:08:53 PDT 2006
//    Fix up bad merge.
//
//    Hank Childs, Wed Sep  6 16:49:15 PDT 2006
//    Fix memory issue.
//
//    Sean Ahern, Thu Feb 14 16:51:22 EST 2008
//    Converted to ExecuteDataTree, since I moved avtClipFilter from an
//    avtStreamer to an avtSIMODataTreeIterator.  Added pipelined clips to get
//    accurate cell clips when multiple planes are used.
//
// ****************************************************************************

avtDataTree_p
avtClipFilter::ExecuteDataTree(vtkDataSet *inDS, int domain, std::string label)
{
    if (inDS == NULL || inDS->GetNumberOfPoints() == 0 || inDS->GetNumberOfCells() == 0)
        return NULL;

    vtkDataSet *outDS[3];
    int nDataSets = 0;

    if (atts.GetQuality() == ClipAttributes::Accurate)
    {
        nDataSets = ComputeAccurateClip(inDS, outDS, atts, domain, label);
    } else
    {
        nDataSets = ComputeFastClip(inDS, outDS, atts, domain, label);
    }

    // Count the cells for debugging.
    int cellCount = 0;
    for(int i=0;i<nDataSets;i++)
        cellCount += outDS[i]->GetNumberOfCells();
    debug4 << "After clipping, domain " << domain << " has " <<
        cellCount << " cells."  << endl;

    // Create a data tree from the grids.
    if (nDataSets == 0)
        return NULL;
    avtDataTree_p outDT = new avtDataTree(nDataSets, outDS, domain, label);
    for(int i=0;i<nDataSets;i++)
    {
        if(outDS[i] != NULL)
        {
            outDS[i]->Delete();
        }
    }

    return outDT;
}

int
avtClipFilter::ComputeAccurateClip(vtkDataSet *inDS, vtkDataSet **outDS,
                                   ClipAttributes &atts, int domain, std::string label)
{
    // Gather global plane clipping information.
    bool nodesAreCritical = GetInput()->GetInfo().GetAttributes().NodesAreCritical();
    int nDataSets = 0;

    if (atts.GetFuncType() == ClipAttributes::Plane)
    {
        // Set up the planes.
        vector<vtkPlane*> planes;
        vector<vtkPlane*> inversePlanes;
        if (atts.GetPlane1Status() == true)
        {
            vtkPlane *plane = vtkPlane::New();
            plane->SetOrigin(atts.GetPlane1Origin());
            plane->SetNormal(atts.GetPlane1Normal());
            planes.push_back(plane);

            vtkPlane *inversePlane = vtkPlane::New();
            inversePlane->SetOrigin(atts.GetPlane1Origin());
            double n[3];
            plane->GetNormal(n);
            n[0] = -n[0];
            n[1] = -n[1];
            n[2] = -n[2];
            inversePlane->SetNormal(n);
            inversePlanes.push_back(inversePlane);
        }
        if (atts.GetPlane2Status() == true)
        {
            vtkPlane *plane = vtkPlane::New();
            plane->SetNormal(atts.GetPlane2Normal());
            plane->SetOrigin(atts.GetPlane2Origin());
            planes.push_back(plane);

            vtkPlane *inversePlane = vtkPlane::New();
            inversePlane->SetOrigin(atts.GetPlane2Origin());
            double n[3];
            plane->GetNormal(n);
            n[0] = -n[0];
            n[1] = -n[1];
            n[2] = -n[2];
            inversePlane->SetNormal(n);
            inversePlanes.push_back(inversePlane);
        }
        if (atts.GetPlane3Status() == true)
        {
            vtkPlane *plane = vtkPlane::New();
            plane->SetNormal(atts.GetPlane3Normal());
            plane->SetOrigin(atts.GetPlane3Origin());
            planes.push_back(plane);

            vtkPlane *inversePlane = vtkPlane::New();
            inversePlane->SetOrigin(atts.GetPlane3Origin());
            double n[3];
            plane->GetNormal(n);
            n[0] = -n[0];
            n[1] = -n[1];
            n[2] = -n[2];
            inversePlane->SetNormal(n);
            inversePlanes.push_back(inversePlane);
        }
        int planeCount = planes.size();

        // Check if we have any work to do.
        if (planeCount == 0)
        {
            // Nothing to do!  Just return an avtDataTree of our input.
            outDS[nDataSets++] = inDS;
            return 1;
        }

        if (atts.GetPlaneInverse() == true)
        {
            // Only one clip needed here.
            switch(planeCount)
            {
            case 1:
                // The inverse of the first plane.
                outDS[nDataSets++] =
                    ClipAgainstPlanes(inDS, nodesAreCritical,
                                    inversePlanes[0]);
                break;
            case 2:
                // The first plane, and the inverse of the second plane.
                outDS[nDataSets++] =
                    ClipAgainstPlanes(inDS, nodesAreCritical,
                                    inversePlanes[0],
                                    inversePlanes[1]);
                break;
            case 3:
                // The first plane, the second plane, and the inverse of the second plane.
                outDS[nDataSets++] =
                    ClipAgainstPlanes(inDS, nodesAreCritical,
                                    inversePlanes[0],
                                    inversePlanes[1],
                                    inversePlanes[2]);
                break;
            default:
                break;
            }
        } else
        {
            // Up to three separate clips required.
            if (planeCount >= 1)
            {
                // The first plane.
                outDS[nDataSets++] = ClipAgainstPlanes(inDS, nodesAreCritical, planes[0]);
            }
            if (planeCount >= 2)
            {
                // The inverse of the first plane, and the second plane.
                outDS[nDataSets++] =
                    ClipAgainstPlanes(inDS, nodesAreCritical,
                                    inversePlanes[0], planes[1]);
            }
            if (planeCount >= 3)
            {
                // The inverse of the first plane, the inverse of the second
                // plane, and the third plane.
                outDS[nDataSets++] =
                    ClipAgainstPlanes(inDS, nodesAreCritical,
                                    inversePlanes[0],
                                    inversePlanes[1], planes[2]);
            }
        }

        // Delete the planes
        vector<vtkPlane*>::iterator it;
        for(it = planes.begin(); it != planes.end(); it++)
            (*it)->Delete();
        for(it = inversePlanes.begin(); it != inversePlanes.end(); it++)
            (*it)->Delete();
    }

    if (atts.GetFuncType() == ClipAttributes::Sphere)
    {
        vtkImplicitBoolean *funcs = vtkImplicitBoolean::New();
        double  rad = atts.GetRadius();
        double *cent;
        cent = atts.GetCenter();
        vtkSphere *sphere = vtkSphere::New();
        sphere->SetCenter(cent);
        sphere->SetRadius(rad);
        funcs->AddFunction(sphere);
        sphere->Delete();
        bool inverse = atts.GetSphereInverse();

        vtkVisItClipper *clipper = vtkVisItClipper::New();
        clipper->SetInput(inDS);
        clipper->SetClipFunction(funcs);
        clipper->SetInsideOut(inverse);
        clipper->SetRemoveWholeCells(nodesAreCritical);
        vtkUnstructuredGrid *ug = vtkUnstructuredGrid::New();
        clipper->SetOutput(ug);
        clipper->Update();

        outDS[nDataSets++] = ug;

        funcs->Delete();
        clipper->Delete();
    }

    return nDataSets;
}

int
avtClipFilter::ComputeFastClip(vtkDataSet *inDS, vtkDataSet **outDS,
                               ClipAttributes &atts, int domain, std::string label)
{
    vtkVisItClipper *fastClipper = vtkVisItClipper::New();
    vtkImplicitBoolean *ifuncs = vtkImplicitBoolean::New();
    vtkDataSet *output = NULL;
 
    int nDataSets = 0;
    bool inverse = false; 
    bool funcSet = SetUpClipFunctions(ifuncs, inverse);
    if (!funcSet)
    {
        // we have no functions to work with.  Just return the input dataset.
        fastClipper->Delete();
        ifuncs->Delete();

        outDS[0] = inDS;
        return 1;
    }

    bool nodesAreCritical =
        GetInput()->GetInfo().GetAttributes().NodesAreCritical();

    //
    // Set up and apply the clipping filters
    // 
    bool doFast = true;
    if  (inDS->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        int dims[3];       
        ((vtkRectilinearGrid*)inDS)->GetDimensions(dims);
        if (dims[1] <= 1 && dims[2] <= 1)
        {
            doFast = false;
            output = Clip1DRGrid(ifuncs, inverse, (vtkRectilinearGrid*)inDS);
        }
    }
    if (doFast)
    {
        vtkUnstructuredGrid *ug = vtkUnstructuredGrid::New();
        fastClipper->SetInput(inDS);
        fastClipper->SetOutput(ug);
        fastClipper->SetClipFunction(ifuncs);
        fastClipper->SetInsideOut(inverse);
        fastClipper->SetRemoveWholeCells(nodesAreCritical);
        fastClipper->Update();

        output = ug;
    }

    ifuncs->Delete();

    if (output != NULL)
    {
        if (output->GetNumberOfCells() == 0)
        {
            output->Delete();
            return 0;
        }
    }

    fastClipper->Delete();

    outDS[0] = output;
    return 1;
}


// ****************************************************************************
//  Method: avtClipFilter::SetUpClipFunctions
//
//  Purpose:
//      Reads in atts-information needed in setting up clip functions. 
//
//  Arguments:
//
//  Returns:   True if set-up of clip functions succesful, false otherwise.
//       
//  Programmer: Kathleen Bonnell 
//  Creation:   May 7, 2001 
//
//  Modifications:
//    Brad Whitlock, Thu Oct 31 11:55:34 PDT 2002
//    Updated it so it works with the new state object.
//
// ****************************************************************************

bool
avtClipFilter::SetUpClipFunctions(vtkImplicitBoolean *funcs, bool &inv)
{
    bool success = false;
    if (atts.GetFuncType() == ClipAttributes::Plane)
    {
        double *orig, *norm;
        if (atts.GetPlane1Status())
        {
            orig = atts.GetPlane1Origin();
            norm = atts.GetPlane1Normal();
            vtkPlane *plane = vtkPlane::New();
            plane->SetNormal(norm);
            plane->SetOrigin(orig);
            funcs->AddFunction(plane);
            plane->Delete();
            success = true;
        }
        if (atts.GetPlane2Status())
        {
            orig = atts.GetPlane2Origin();
            norm = atts.GetPlane2Normal();
            vtkPlane *plane = vtkPlane::New();
            plane->SetNormal(norm);
            plane->SetOrigin(orig);
            funcs->AddFunction(plane);
            plane->Delete();
            success = true;
        }
        if (atts.GetPlane3Status())
        {
            orig = atts.GetPlane3Origin();
            norm = atts.GetPlane3Normal();
            vtkPlane *plane = vtkPlane::New();
            plane->SetNormal(norm);
            plane->SetOrigin(orig);
            funcs->AddFunction(plane);
            plane->Delete();
            success = true;
        }
        //
        //  Okay, this may seem weird, but for 'erasing' with planes,
        //  'inside-out' is our 'normal' case, and 'inside-right'
        //  is our 'inverse'.  So use the opposite of the user setting 
        //  so that the correct 'side' will get set in the 
        //  clipping filter.
        //
        inv = !atts.GetPlaneInverse();
    }
    else if (atts.GetFuncType() == ClipAttributes::Sphere)
    {
        double  rad = atts.GetRadius();
        double *cent;
        cent = atts.GetCenter();
        vtkSphere *sphere = vtkSphere::New();
        sphere->SetCenter(cent);
        sphere->SetRadius(rad);
        funcs->AddFunction(sphere);
        sphere->Delete();
        success = true;
        inv = atts.GetSphereInverse();
    }

    return success;
}


// ****************************************************************************
//  Method: avtClipFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Indicate that this invalidates the zone numberings.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 7, 2001 
//
//  Modifications:
//    Hank Childs, Wed Jun  6 13:32:28 PDT 2001
//    Renamed from CopyDatasetMetaData.
//
//    Kathleen Bonnell, Thu Mar  2 14:26:06 PST 2006 
//    Set ZonesSplit.
//
//    Kathleen Bonnell, Fri Apr 28 10:57:21 PDT 2006 
//    Set OrigElementsRequiredForPick.
//
// ****************************************************************************

void
avtClipFilter::UpdateDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    GetOutput()->GetInfo().GetValidity().ZonesSplit();
    GetOutput()->GetInfo().GetAttributes().SetOrigElementsRequiredForPick(true);
}


// ****************************************************************************
//  Method: avtClipFilter::PeformRestriction
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   Apr 28, 2006 
//
//  Modifications:
//
// ****************************************************************************

avtContract_p
avtClipFilter::ModifyContract(avtContract_p spec)
{
 
    if (spec->GetDataRequest()->MayRequireZones() ||
        spec->GetDataRequest()->MayRequireNodes())
    {
        avtContract_p ns = new avtContract(spec);
        // Turn on both Nodes and Zones, to prevent another re-execution if 
        // user switches between zone and node pick.
        ns->GetDataRequest()->TurnZoneNumbersOn();
        ns->GetDataRequest()->TurnNodeNumbersOn();
        return ns;
    }
    return spec;
}


// ****************************************************************************
//  Method: avtClipFilter::Clip1DRGrid
//
//  Purpose:
//    Clips a 1D RectlinearGrid, and returns same.
//
//  Arguments:
//    ifuncs    The function to use in clipping.
//    inv       Whether or not the clip is an Inverse clip.
//    inGrid    The input grid.
//
//  Returns:
//    The clipped 1D RectlinearGrid.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 31, 2006 
//
//  Modifications:
//      Hank Childs, Fri Feb 15 14:45:45 PST 2008
//      Initialize some variables to make Klockwork happy.
//
// ****************************************************************************

vtkRectilinearGrid *
avtClipFilter::Clip1DRGrid(vtkImplicitBoolean *ifuncs, bool inv,
     vtkRectilinearGrid *inGrid)
{
    vtkDataArray *inXC = inGrid->GetXCoordinates();
    vtkDataArray *inVal = inGrid->GetPointData()->GetScalars();

    vtkRectilinearGrid *outGrid = 
        vtkVisItUtility::Create1DRGrid(0, inXC->GetDataType());
    vtkDataArray *outXC = outGrid->GetXCoordinates();
    vtkDataArray *outVal = inVal->NewInstance();

    int nx = inXC->GetNumberOfTuples();

    double lastX = inXC->GetTuple1(0);
    double lastVal = inVal->GetTuple1(0);
    double lastDist = ifuncs->EvaluateFunction(lastX, 0., 0.);
    if (lastDist > 0 && !inv)
    {
        outXC->InsertNextTuple1(lastX);
        outVal->InsertNextTuple1(lastVal);
    }

    int i, nPts = inXC->GetNumberOfTuples();

    for (int i = 1; i < nPts; i++)
    {
        int whichCase = 0;
        double x = inXC->GetTuple1(i);
        double val = inVal->GetTuple1(i);
        double dist = ifuncs->EvaluateFunction(x, 0., 0.);

        if ((dist > 0 && !inv) || (lastDist <= 0 && inv))
            whichCase += 1;
        if ((dist <= 0 && inv) || (lastDist > 0 && !inv))
            whichCase += 2;

        double x1, x2, d1 = 0., d2 = 1., v1, v2, newX = 0., newVal = 0.;
        switch(whichCase)
        {
            case 1 : 
                x1 = x; d1 = dist; v1 = val;
                x2 = lastX; d2 = lastDist; v2 = lastVal;
                break;
            case 2 : 
                x2 = x; d2 = dist; v2 = val;
                x1 = lastX; d1 = lastDist, v1 = lastVal;
                break;
            case 3 : 
                newX = x;
                newVal = val;
                break;
            case 0 :
            default : 
                break;
        }
        if (whichCase) 
        {
            if (whichCase != 3)
            {
                double percent = 1. - ((0. - d1)/(d2-d1));
                double bp = 1. -percent;
                newX =  x1*percent + x2*bp;     
                newVal = v1*percent + v2*bp;
            }
            outXC->InsertNextTuple1(newX);
            outVal->InsertNextTuple1(newVal);
        }
        lastX = x;
        lastVal = val;
        lastDist = dist;
    }
 
    outGrid->GetPointData()->SetScalars(outVal);
    outGrid->SetDimensions(outXC->GetNumberOfTuples(), 1, 1);
    outVal->Delete();
    return outGrid;
}
