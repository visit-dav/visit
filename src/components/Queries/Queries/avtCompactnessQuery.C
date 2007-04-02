/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                           avtCompactnessQuery.C                           //
// ************************************************************************* //

#include <avtCompactnessQuery.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkDataSetRemoveGhostCells.h>
#include <vtkGeometryFilter.h>
#include <vtkPolyData.h>
#include <vtkPolyDataRelevantPointsFilter.h>
#include <vtkUnsignedCharArray.h>
#include <vtkVisItFeatureEdges.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <avtCallback.h>
#include <avtParallel.h>

#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <InvalidDimensionsException.h>
#include <NonQueryableInputException.h>

#include <snprintf.h>
#include <float.h>
#include <math.h>

using     std::string;
using     std::vector;

#if !defined(M_PI)
#define M_PI 3.14159265358979323846
#endif

// ****************************************************************************
//  Method: avtCompactnessQuery constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   April  9, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Apr 17 12:51:55 PDT 2003
//    Added more queries, and made it inherit from avtTwoPassDatasetQuery.
//
// ****************************************************************************

avtCompactnessQuery::avtCompactnessQuery() : avtTwoPassDatasetQuery()
{
    strcpy(descriptionBuffer, "Calculating compactness measures");

    totalRotVolume   = 0;
    totalXSectArea   = 0;
    distBound_da_xsa = 0;
    distBound_da_vol = 0;
    distBound_dv_xsa = 0;
    distBound_dv_vol = 0;
    distOrigin_da = 0;

    densityValid         = true;
    totalRotMass         = 0;
    centMassX            = 0;
    centMassY            = 0;
    distBound_dv_den_vol = 0;
    distCMass_dv_den_vol = 0;
}


// ****************************************************************************
//  Method: avtCompactnessQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtCompactnessQuery::~avtCompactnessQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtCompactnessQuery::VerifyInput
//
//  Purpose:
//    Rejects non-queryable input && input that has topological dimension == 0
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 3, 2004
//
//  Modifications:
//
// ****************************************************************************

void
avtCompactnessQuery::VerifyInput()
{
    avtDataObjectQuery::VerifyInput();
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 0)
    {
        EXCEPTION1(NonQueryableInputException,
            "Requires plot with topological dimension > 0.");
    }
}


// ****************************************************************************
//  Method: avtCompactnessQuery::PreExecute
//
//  Purpose:
//      This is called before all of the domains are executed.
//
//  Programmer: Jeremy Meredith
//  Creation:   April  9, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Apr 17 12:52:13 PDT 2003
//    Added more queries.
//
//    Jeremy Meredith, Wed Jul 23 13:28:57 PDT 2003
//    Turned xBound and yBound into class data members, and made them
//    STL vectors.  This way we can easily collect the boundary points
//    across all domains and processors.
//
// ****************************************************************************

void
avtCompactnessQuery::PreExecute(void)
{
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    if (atts.GetSpatialDimension() != 2)
    {
        EXCEPTION2(InvalidDimensionsException, "Compactness measures",
                                               "2-dimensional");
    }

    numDomains = 0;
    totalRotVolume   = 0;
    totalXSectArea   = 0;
    distBound_da_xsa = 0;
    distBound_da_vol = 0;
    distBound_dv_xsa = 0;
    distBound_dv_vol = 0;
    distOrigin_da = 0;

    densityValid         = true;
    totalRotMass         = 0;
    centMassX            = 0;
    centMassY            = 0;
    distBound_dv_den_vol = 0;
    distCMass_dv_den_vol = 0;

    xBound.clear();
    yBound.clear();
}

// ****************************************************************************
//  Method:  avtCompactnessQuery::MidExecute
//
//  Purpose:
//    This is called after all of the domains are executed the first time.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 17, 2003
//
//  Modifications:
//    Jeremy Meredith, Wed Jul 23 13:34:18 PDT 2003
//    Turned xBound and yBound into class data members, and made them
//    STL vectors.  Added code to collect the boundary points
//    across all processors.
//
//    Jeremy Meredith, Mon Apr  5 14:17:25 PDT 2004
//    Added a check to make sure we got at least *some* boundary data.
//    We will wind up with invalid results if there are no boundaries.
//
// ****************************************************************************

void
avtCompactnessQuery::MidExecute(void)
{
    SumDoubleAcrossAllProcessors(totalXSectArea);
    SumDoubleAcrossAllProcessors(totalRotVolume);

    SumDoubleAcrossAllProcessors(totalRotMass);
    SumDoubleAcrossAllProcessors(centMassX);
    SumDoubleAcrossAllProcessors(centMassY);
    if (totalRotMass > 0)
    {
        centMassX /= totalRotMass;
        centMassY /= totalRotMass;
    }

#ifdef PARALLEL
    // Calculate up the boundary points array across all processors.
    int rank = PAR_Rank();
    int nprocs = PAR_Size();

    int mySize = xBound.size();
    int totalSize;
    MPI_Allreduce(&mySize, &totalSize, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    xBound.resize(totalSize);
    yBound.resize(totalSize);
    int position = mySize;
    for (int proc = 0; proc < nprocs; proc++)
    {
        if (proc == rank)
        {
            // Sending
            int len = mySize;
            MPI_Bcast(&len, 1, MPI_INT, proc, MPI_COMM_WORLD);
            MPI_Bcast(&xBound[0], len, MPI_FLOAT, proc, MPI_COMM_WORLD);
            MPI_Bcast(&yBound[0], len, MPI_FLOAT, proc, MPI_COMM_WORLD);
        }
        else
        {
            // Receiving
            int len;
            MPI_Bcast(&len, 1, MPI_INT, proc, MPI_COMM_WORLD);
            MPI_Bcast(&xBound[position], len, MPI_FLOAT, proc, MPI_COMM_WORLD);
            MPI_Bcast(&yBound[position], len, MPI_FLOAT, proc, MPI_COMM_WORLD);
            position += len;
        }
    }
#endif

    if (xBound.size() == 0)
        EXCEPTION1(VisItException, "There were no boundaries, but these "
                   "are needed to compute some of the compactness queries.  "
                   "You may be using this query in an unexpected way; please "
                   "contact a VisIt developer.");
}

// ****************************************************************************
//  Method: avtCompactnessQuery::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed.
//
//  Programmer: Jeremy Meredith
//  Creation:   April  9, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Apr 17 12:52:47 PDT 2003
//    Added new queries.  Moved some logic to MidExecute since this is 
//    now a two-pass query.
//
//    Kathleen Bonnell, Fri Jul 11 16:29:41 PDT 2003 
//    Renamed 'SetMessage' to 'SetResultMessage'. 
//
//    Jeremy Meredith, Wed Jul 23 13:34:18 PDT 2003
//    Turned xBound and yBound into class data members, and free them here.
//
// ****************************************************************************

void
avtCompactnessQuery::PostExecute(void)
{
    SumIntAcrossAllProcessors(numDomains);
    SumDoubleAcrossAllProcessors(distBound_da_xsa);
    SumDoubleAcrossAllProcessors(distBound_da_vol);
    SumDoubleAcrossAllProcessors(distBound_dv_xsa);
    SumDoubleAcrossAllProcessors(distBound_dv_vol);
    SumDoubleAcrossAllProcessors(distOrigin_da);

    if (totalXSectArea > 0)
    {
        distBound_da_xsa /= totalXSectArea;
        distBound_dv_xsa /= totalXSectArea;
    }
    if (totalRotVolume > 0)
    {
        distBound_da_vol /= totalRotVolume;
        distBound_dv_vol /= totalRotVolume;
    }
    if (totalRotMass > 0)
    {
        distBound_dv_den_vol /= totalRotMass;
        distCMass_dv_den_vol /= totalRotMass;
    }

    // We're done with the xBound and yBound arrays now; free up
    // their memory.
    xBound.clear();
    yBound.clear();

    //
    //  Parent class uses this message to set the Results message
    //  in the Query Attributes that is sent back to the viewer.
    //  That is all that is required of this query.
    //
    char msg[4096];
    SNPRINTF(msg, 4096,
             "%s"
             "Total cross sectional area (XSA): %f\n"
             "Total rotated (x=0) volume (VOL): %f\n"
             "\n"
             "Distance to boundary WRT dA / XSA = %f\n"
             "Distance to boundary WRT dA / VOL = %f\n"
             "Distance to boundary WRT dV / XSA = %f\n"
             "Distance to boundary WRT dV / VOL = %f\n"
             "Distance to origin   WRT dA       = %f\n",
             (numDomains > 1) ?
                   "Warning: multiple domains -- accuracy will suffer.\n" : "",
             totalXSectArea,
             totalRotVolume,
             distBound_da_xsa,
             distBound_da_vol,
             distBound_dv_xsa,
             distBound_dv_vol,
             distOrigin_da);


    // If we have a density variable available
    if (!densityValid)
    {
        char msg2[8192];
        SNPRINTF(msg2, 8192,
                 "%s\n"
                 "NOTE -- additional calculations are available if\n"
                 "        a density plot is selected for this query.\n",
                 msg);
        SetResultMessage(msg2);
    }
    else
    {
        char msg2[8192];
        SNPRINTF(msg2, 8192,
                 "%s\n\n"
                 "Density-based queries:\n"
                 "NOTE -- these assume the current variable is 'density'!\n"
                 "\n"
                 "Total rotated (x=0) mass (MASS): %f\n"
                 "Center of mass = (%f , %f)\n"
                 "\n"
                 "Density-weighted distance to boundary  WRT dV / MASS = %f\n"
                 "Density-weighted distance to cent mass WRT dV / MASS = %f\n",

                 msg,
                 totalRotMass,
                 centMassX, centMassY,
                 distBound_dv_den_vol,
                 distCMass_dv_den_vol);
        SetResultMessage(msg2);
    }
}

// ****************************************************************************
//  Method: avtCompactnessQuery::Execute
//
//  Purpose:
//      Processes a single domain.
//
//  Programmer: Jeremy Meredith
//  Creation:   April  9, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Apr 17 12:53:15 PDT 2003
//    Split part of the former Execute method into this method.
//    Added new queries.
//
//    Jeremy Meredith, Wed Jul 23 13:34:18 PDT 2003
//    Turned xBound and yBound into class data members, and made them
//    STL vectors.  Made it collect the boundary points across all domains.
//
//    Jeremy Meredith, Fri Apr  2 17:14:11 PST 2004
//    Added a relevant points filter.
//
//    Hank Childs, Mon Aug 30 17:15:30 PDT 2004
//    Remove call to SetGhostLevel.
//
//    Kathleen Bonnell, Wed May 17 15:22:06 PDT 2006 
//    Remove call to SetSoure(NULL) as it now removes information necessary
//    for the dataset.
//
// ****************************************************************************

void
avtCompactnessQuery::Execute1(vtkDataSet *ds, const int dom)
{
    int i;
    numDomains++;

    //
    // Remove ghost zones from the polygon dataset
    //
    vtkDataSetRemoveGhostCells *gzFilter2 = vtkDataSetRemoveGhostCells::New();

    gzFilter2->SetInput(ds);
    vtkDataSet *ds_2d_nogz = gzFilter2->GetOutput();

    ds_2d_nogz->Update();

    //
    // Create the area, volume and centroid arrays
    //
    int ncells2d = ds_2d_nogz->GetNumberOfCells();
    float *cellArea = new float[ncells2d];
    float *cellVol  = new float[ncells2d];
    float *cellCentX = new float[ncells2d];
    float *cellCentY = new float[ncells2d];
    for (i = 0 ; i < ncells2d ; i++)
    {
        vtkCell *cell = ds_2d_nogz->GetCell(i);
        Get2DCellCentroid(cell, cellCentX[i], cellCentY[i]);
        cellArea[i] = Get2DCellArea(cell);
        cellVol[i] = (2 * M_PI * cellCentY[i]) * cellArea[i];
    }

    //
    // Integrate the values
    //
    for (i = 0 ; i < ncells2d ; i++)
    {
        totalRotVolume += cellVol[i];
        totalXSectArea += cellArea[i];
    }

    //
    // Extract the default variable array, and assume it's density
    //
    vtkDataArray *den = ds_2d_nogz->GetCellData()->GetScalars();

    //
    // If we've got a valid density array, do some more queries
    //
    if (densityValid && den && den->GetNumberOfTuples() == ncells2d)
    {
        // calculate the whole mass
        for (i = 0 ; i < ncells2d ; i++)
        {
            float d = den->GetTuple1(i);
            totalRotMass += d * cellVol[i];
            centMassX += d * cellCentX[i] * cellVol[i];
            centMassY += d * cellCentY[i] * cellVol[i];
        }
    }
    else
    {
        densityValid = false;
    }

    //
    // Create the boundary edges and remove ghost zone edges
    //
    vtkGeometryFilter *geomFilter = vtkGeometryFilter::New();

    vtkVisItFeatureEdges *boundaryFilter = vtkVisItFeatureEdges::New();
    boundaryFilter->BoundaryEdgesOn();
    boundaryFilter->FeatureEdgesOff();
    boundaryFilter->NonManifoldEdgesOff();
    boundaryFilter->ManifoldEdgesOff();
    boundaryFilter->ColoringOff();

    vtkDataSetRemoveGhostCells *gzFilter1 = vtkDataSetRemoveGhostCells::New();

    geomFilter->SetInput(ds);
    boundaryFilter->SetInput(geomFilter->GetOutput());
    boundaryFilter->GetOutput()->SetUpdateGhostLevel(2);
    boundaryFilter->GetOutput()->Update();

    vtkPolyData *allLines = boundaryFilter->GetOutput();
    //allLines->SetSource(NULL);

    gzFilter1->SetInput(allLines);
    vtkDataSet *ds_1d_nogz = gzFilter1->GetOutput();
    ds_1d_nogz->Update();

    // We need polydata, and should have it by now.
    if (ds_1d_nogz->GetDataObjectType() != VTK_POLY_DATA)
    {
        debug1 << "Did not get poly data from ghost zone filter output\n";
        return;
    }
    vtkPolyData *pd_1d_nogz_allpts = (vtkPolyData*)ds_1d_nogz;

    //
    // If we had stripped some lines by using a ghost zone filter, then
    // we had better make sure the points attached to them go away.
    //
    vtkPolyDataRelevantPointsFilter *relPts;
    relPts = vtkPolyDataRelevantPointsFilter::New();
    relPts->SetInput(pd_1d_nogz_allpts);
    vtkPolyData *pd_1d_nogz = relPts->GetOutput();
    relPts->Update();

    //
    // Extract the boundary edges to a convenient format
    //
    vtkPoints *pts1d = pd_1d_nogz->GetPoints();
    int npts1d = pts1d->GetNumberOfPoints();
    int oldBoundSize = xBound.size();
    xBound.resize(oldBoundSize + npts1d);
    yBound.resize(oldBoundSize + npts1d);
    for (i=0; i<npts1d; i++)
    {
        double pt[3];
        pts1d->GetPoint(i, pt);
        xBound[oldBoundSize+i] = pt[0];
        yBound[oldBoundSize+i] = pt[1];
    }

    //
    // Clean up
    //
    delete[] cellArea;
    delete[] cellVol;
    delete[] cellCentX;
    delete[] cellCentY;
    gzFilter1->Delete();
    gzFilter2->Delete();
    geomFilter->Delete();
    boundaryFilter->Delete();
    relPts->Delete();
}

// ****************************************************************************
//  Method: avtCompactnessQuery::Execute2
//
//  Purpose:
//      Processes a single domain -- second pass.
//
//  Programmer: Jeremy Meredith
//  Creation:   April  9, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Apr 17 12:53:15 PDT 2003
//    Split part of the former Execute method into this method.
//    Added new queries.
//

//    Jeremy Meredith, Wed Jul 23 13:31:16 PDT 2003
//    Two things: 
//    1.  Moved the boundary point calculation out of here so we are now
//        collecting the points across all domains and processors.
//    2.  Put in a hack to disallow points along y=0 as being part of
//        the "Boundary", since we are assuming a rotation around y=0
//        already.
//
//    Hank Childs, Mon Aug 30 17:15:30 PDT 2004
//    Remove call to SetGhostLevel.
//
// ****************************************************************************

void
avtCompactnessQuery::Execute2(vtkDataSet *ds, const int dom)
{
    int i,j;

    //
    // Remove ghost zones from the polygon dataset
    //
    vtkDataSetRemoveGhostCells *gzFilter2 = vtkDataSetRemoveGhostCells::New();

    gzFilter2->SetInput(ds);
    vtkDataSet *ds_2d_nogz = gzFilter2->GetOutput();

    ds_2d_nogz->Update();

    //
    // Create the area, volume and centroid arrays
    //
    int ncells2d = ds_2d_nogz->GetNumberOfCells();
    float *cellArea = new float[ncells2d];
    float *cellVol  = new float[ncells2d];
    float *cellCentX = new float[ncells2d];
    float *cellCentY = new float[ncells2d];
    for (i = 0 ; i < ncells2d ; i++)
    {
        vtkCell *cell = ds_2d_nogz->GetCell(i);
        Get2DCellCentroid(cell, cellCentX[i], cellCentY[i]);
        cellArea[i] = Get2DCellArea(cell);
        cellVol[i] = (2 * M_PI * cellCentY[i]) * cellArea[i];
    }

    //
    // Calculate the distance to the boundary and origin
    //
    float *distOrigin   = new float[ncells2d];
    float *distBoundary = new float[ncells2d];
    int npts1d = xBound.size();
    for (i = 0 ; i < ncells2d ; i++)
    {
        // distance to origin
        distOrigin[i] = sqrt(cellCentX[i]*cellCentX[i] +
                             cellCentY[i]*cellCentY[i]);

        distBoundary[i] = FLT_MAX;
        for (j = 0 ; j < npts1d ; j++)
        {
            // semi-hack: ignore points along the y=0 line,
            // and assume they are not part of a boundary.
            // This is safe because we are already assuming
            // a rotation around y=0, and thus in 3D the y=0 
            // points have no surface area.
            if (yBound[j] < 0.00001)
                continue;

            float dx = cellCentX[i] - xBound[j];
            float dy = cellCentY[i] - yBound[j];
            float dist2 = dx*dx + dy*dy;
            if (dist2 < distBoundary[i])
                distBoundary[i] = dist2;
        }
        distBoundary[i] = sqrt(distBoundary[i]);
    }

    //
    // Integrate the values
    //
    for (i = 0 ; i < ncells2d ; i++)
    {
        distBound_da_xsa += cellArea[i] * distBoundary[i];
        distBound_da_vol += cellArea[i] * distBoundary[i];
        distBound_dv_xsa += cellVol[i]  * distBoundary[i];
        distBound_dv_vol += cellVol[i]  * distBoundary[i];
        distOrigin_da    += cellArea[i] * distOrigin[i];
    }

    //
    // Extract the default variable array, and assume it's density
    //
    vtkDataArray *den = ds_2d_nogz->GetCellData()->GetScalars();

    //
    // If we've got a valid density array, do some more queries
    //
    if (densityValid && den && den->GetNumberOfTuples() == ncells2d)
    {
        // calculate the two density-based queries
        for (i = 0 ; i < ncells2d ; i++)
        {
            float d = den->GetTuple1(i);
            
            float dx = cellCentX[i] - centMassX;
            float dy = cellCentY[i] - centMassY;
            float distCentMass = sqrt(dx*dx + dy*dy);

            distBound_dv_den_vol += cellVol[i] * distBoundary[i] * d;
            distCMass_dv_den_vol += cellVol[i] * distCentMass    * d;
        }
    }
    else
    {
        densityValid = false;
    }

    //
    // Clean up
    //
    delete[] cellArea;
    delete[] cellVol;
    delete[] cellCentX;
    delete[] cellCentY;
    delete[] distOrigin;
    delete[] distBoundary;
    gzFilter2->Delete();
}



// ****************************************************************************
//  Method:  avtCompactnessQuery::Get2DTriangleArea
//
//  Purpose:
//    Gets the area of a triangle.
//
//  Arguments:
//    p0,p1,p2   three two-float arrays
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 12, 2003
//
// ****************************************************************************
float
avtCompactnessQuery::Get2DTriangleArea(double *p0, double *p1, double *p2)
{
    double vx1 = p1[0]-p0[0];
    double vy1 = p1[1]-p0[1];
    double vx2 = p2[0]-p0[0];
    double vy2 = p2[1]-p0[1];

    return fabs((vx1*vy2)-(vy1*vx2))/2.;
}

// ****************************************************************************
//  Method:  avtCompactnessQuery::Get2DCellArea
//
//  Purpose:
//    Gets the area of a vtkCell, assuming it is 2D.
//
//  Arguments:
//    cell       the vtkCell to get the area of
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 12, 2003
//
// ****************************************************************************
float
avtCompactnessQuery::Get2DCellArea(vtkCell *cell)
{
    float area = 0;

    vtkPoints *pts = cell->GetPoints();
    int npts = cell->GetNumberOfPoints();
    int cellType = cell->GetCellType();
    int i;
    double p0[3],p1[3],p2[3],p3[3];
    switch (cellType)
    {
      case VTK_POLYGON:
      case VTK_QUAD:
      case VTK_TRIANGLE:
        pts->GetPoint(0, p0);
        for (i=0; i<npts - 2; i++)
        {
            pts->GetPoint(i+1, p1);
            pts->GetPoint(i+2, p2);
            area += Get2DTriangleArea(p0, p1, p2);
        }
        break;

      case VTK_PIXEL:
        pts->GetPoint(0, p0);
        pts->GetPoint(1, p1);
        pts->GetPoint(2, p2);
        pts->GetPoint(3, p3);

        area = Get2DTriangleArea(p0,p1,p2) + Get2DTriangleArea(p3,p2,p1);
        break;

      default:
        break;
    }

    return area;
}

// ****************************************************************************
//  Method:  avtCompactnessQuery::Get2DCellCentroid
//
//  Purpose:
//    Gets the x,y centroid of a vtkCell.
//
//  Arguments:
//    cell          (i)    the vtkCell to get the centroid of
//    xCent,yCent   (o)    the x,y location of the centroid
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 12, 2003
//
// ****************************************************************************
void
avtCompactnessQuery::Get2DCellCentroid(vtkCell *cell,
                                       float &xCent, float &yCent)
{
    xCent=0.;
    yCent=0.;

    vtkPoints *pts = cell->GetPoints();
    int npts = cell->GetNumberOfPoints();

    double pt[3];
    for (int i=0; i<npts; i++)
    {
        pts->GetPoint(i, pt);
        xCent += pt[0];
        yCent += pt[1];
    }

    xCent /= float(npts);
    yCent /= float(npts);
}
