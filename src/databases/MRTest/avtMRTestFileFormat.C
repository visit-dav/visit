/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

// *************************************************************************
//                           avtMRTestFileFormat.C
//
//  Purpose: Sample database that generates multi resolution data.
//
//  Notes:
//    The reader generates a 2D multi resolution mesh that has 64 x 64
//    ones at the coarsest level and has an infinite number of levels. The
//    reader supports multi resolution data selections. The grids refine
//    by a factor of 2 in each direction for each level. The grid returned
//    is aligned on tile boundaries. At level 0 there is a single tile, at
//    level 1, there are 2 x 2 tiles, at level 2 there are 4 x 4 tiles etc.
//
// *************************************************************************

#include <avtMRTestFileFormat.h>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>

#include <avtDatabaseMetaData.h>
#include <avtParallel.h>
#include <avtMultiresSelection.h>
#include <avtView2D.h>
#include <avtView3D.h>

#include <vector>

using     std::string;
using     std::vector;

// ****************************************************************************
//  Code to calculate the patch values.
// ****************************************************************************

class complex
{
  public:
    complex() : a(0.), b(0.) { }
    complex(float A, float B) : a(A), b(B) { }
    complex(const complex &obj) : a(obj.a), b(obj.b) { }
    complex operator = (const complex &obj) { a = obj.a; b = obj.b; return *this;}
    complex operator + (const complex &obj) const
    {
        return complex(a + obj.a, b + obj.b);
    }
    complex operator * (const complex &obj) const
    {
        return complex(a * obj.a - b * obj.b, a * obj.b + b * obj.a);
    }
    float mag2() const
    {
        return a*a + b*b;
    }
    float mag() const
    {
        return sqrt(a*a + b*b);
    }
  private:
    float a,b;
};

#define MAXIT 30

static float
mandelbrot(const complex &c)
{
    complex z;
    for (int zit = 0; zit < MAXIT; zit++)
    {
        z = (z * z) + c;
        if (z.mag2() > 4.)
            return float(zit+1);
    }
    return 0;
}


// ****************************************************************************
//  Method: avtMRTestFileFormat constructor
//
//  Programmer: Eric Brugger
//  Creation:   Fri Dec 20 12:20:07 PDT 2013
//
//  Modifications:
//    Eric Brugger, Wed Jan  8 17:09:17 PST 2014
//    I modified the reader to also create 3d meshes.
//
// ****************************************************************************

avtMRTestFileFormat::avtMRTestFileFormat(const char *fname)
    : avtSTMDFileFormat(&fname, 1)
{
    filename = fname;

    meshNx = 4096, meshNy = 4096; meshNz = 512;
    meshXmin = 0., meshXmax = 4096., meshYmin = 0., meshYmax = 4096.;
    meshZmin = 0., meshZmax = 512.;
    coarseNx = 64, coarseNy = 64; coarseNz = 8;
    maxLevel2d = 18; maxLevel3d = 6;
}


// ****************************************************************************
//  Method: avtMRTestFileFormat destructor
//
//  Programmer: Eric Brugger
//  Creation:   Fri Dec 20 12:20:07 PDT 2013
//
//  Modifications:
//
// ****************************************************************************

avtMRTestFileFormat::~avtMRTestFileFormat()
{
}


// ****************************************************************************
//  Method: avtMRFileFormat::CanCacheVariable
//
//  Purpose:
//    We can't have VisIt caching chunks of the mesh and variables above it.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Dec 20 12:20:07 PDT 2013
//
// ****************************************************************************

bool
avtMRTestFileFormat::CanCacheVariable(const char *var)
{
    return false;
}


// ****************************************************************************
//  Method: avtMRFileFormat::RegisterDataSelections
//
//  Purpose:
//    The MR format can handle multi resolution data selections. So, we
//    implement this method here to access the data selections.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Dec 20 12:20:07 PDT 2013
//
// ****************************************************************************

void
avtMRTestFileFormat::RegisterDataSelections(
    const vector<avtDataSelection_p> &sels,
    vector<bool> *selsApplied)
{
    selectionsList    = sels;
    selectionsApplied = selsApplied;
}


// ****************************************************************************
//  Method: avtMRTestFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//    This database meta-data object is like a table of contents for the
//    file.  By populating it, you are telling the rest of VisIt what
//    information it can request from you.
//
//  Programmer: brugger -- generated by xml2avt
//  Creation:   Fri Dec 20 12:20:07 PDT 2013
//
//  Modifications:
//    Eric Brugger, Wed Jan  8 17:09:17 PST 2014
//    I modified the reader to also create 3d meshes.
//
// ****************************************************************************

void
avtMRTestFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    avtMeshMetaData *mesh2d = new avtMeshMetaData;
    mesh2d->name = "Mesh";
    mesh2d->meshType = AVT_RECTILINEAR_MESH;
    mesh2d->numBlocks = 1;
    mesh2d->blockOrigin = 0;
    mesh2d->spatialDimension = 2;
    mesh2d->topologicalDimension = 2;

    mesh2d->hasSpatialExtents = true;
    mesh2d->minSpatialExtents[0] = meshXmin;
    mesh2d->maxSpatialExtents[0] = meshXmax;
    mesh2d->minSpatialExtents[1] = meshYmin;
    mesh2d->maxSpatialExtents[1] = meshYmax;

    md->Add(mesh2d);

    avtMeshMetaData *mesh3d = new avtMeshMetaData;
    mesh3d->name = "Mesh3d";
    mesh3d->meshType = AVT_RECTILINEAR_MESH;
    mesh3d->numBlocks = 1;
    mesh3d->blockOrigin = 0;
    mesh3d->spatialDimension = 3;
    mesh3d->topologicalDimension = 3;

    mesh3d->hasSpatialExtents = true;
    mesh3d->minSpatialExtents[0] = meshXmin;
    mesh3d->maxSpatialExtents[0] = meshXmax;
    mesh3d->minSpatialExtents[1] = meshYmin;
    mesh3d->maxSpatialExtents[1] = meshYmax;
    mesh3d->minSpatialExtents[2] = meshZmin;
    mesh3d->maxSpatialExtents[2] = meshZmax;

    md->Add(mesh3d);

    md->Add(new avtScalarMetaData("Mandelbrot", "Mesh", AVT_ZONECENT));

    md->Add(new avtScalarMetaData("Mandelbrot3d", "Mesh3d", AVT_ZONECENT));

    md->SetFormatCanDoMultires(true);

    //
    // Indicate that we can decompose the data in parallel.
    //
    md->SetFormatCanDoDomainDecomposition(true);

    //
    // Providing cycles and times and saying they are accurate causes this
    // routine to only get called once. Ideally it would be nice to say that
    // we don't have times or cycles.
    //
    md->SetCycle(timestep, 0);
    md->SetCycleIsAccurate(true, timestep);

    md->SetTime(timestep, 0.);
    md->SetTimeIsAccurate(true, timestep);
}


// ****************************************************************************
//  Method: avtMRTestFileFormat::GetMesh
//
//  Purpose:
//    Gets the mesh associated with this file.  The mesh is returned as a
//    derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//    vtkUnstructuredGrid, etc).
//
//  Arguments:
//    domain      The index of the domain.  If there are NDomains, this
//                value is guaranteed to be between 0 and NDomains-1,
//                regardless of block origin.
//    meshname    The name of the mesh of interest.  This can be ignored if
//                there is only one mesh.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Dec 20 12:20:07 PDT 2013
//
//  Modifications:
//    Eric Brugger, Wed Jan  8 17:09:17 PST 2014
//    I modified the reader to also create 3d meshes.
//
// ****************************************************************************

vtkDataSet *
avtMRTestFileFormat::GetMesh(int domain, const char *meshname)
{
    GetSelection();

    if (PAR_Rank() > 0)
        return NULL;

    if (strcmp(meshname, "Mesh") == 0)
    {
        return GetMesh2d();
    }
    else
    {
        return GetMesh3d();
    }
}


// ****************************************************************************
//  Method: avtMRTestFileFormat::GetVar
//
//  Purpose:
//    Gets a scalar variable associated with this file.  Although VTK has
//    support for many different types, the best bet is vtkFloatArray, since
//    that is supported everywhere through VisIt.
//
//  Arguments:
//    domain     The index of the domain.  If there are NDomains, this
//               value is guaranteed to be between 0 and NDomains-1,
//               regardless of block origin.
//    varname    The name of the variable requested.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Dec 20 12:20:07 PDT 2013
//
//  Modifications:
//    Eric Brugger, Wed Jan  8 17:09:17 PST 2014
//    I modified the reader to also create 3d meshes.
//
// ****************************************************************************

vtkDataArray *
avtMRTestFileFormat::GetVar(int domain, const char *varname)
{
    GetSelection();

    if (PAR_Rank() > 0)
        return NULL;

    if (strcmp(varname, "Mandelbrot") == 0)
    {
        return GetVar2d();
    }
    else
    {
        return GetVar3d();
    }
}
 

// ****************************************************************************
//  Method: avtMRTestFileFormat::GetSelection
//
//  Purpose:
//    Get the multi resolution data selection.
//
//  Programmer: Eric Brugger
//  Creation:   Thu Jan 30 09:45:05 PST 2014
//
// ****************************************************************************

void
avtMRTestFileFormat::GetSelection()
{
    //
    // Get the multi resolution data selection. Set default values for the
    // transform matrix and cell area in case there isn't a multi resolution
    // data selection.
    //
    for (int i = 0; i < 16; ++i)
        transformMatrix[i] = DBL_MAX;
    cellArea = .002;

    selection = NULL;
    for (size_t i = 0; i < selectionsList.size(); ++i)
    {
        if (string(selectionsList[i]->GetType()) == "Multi Resolution Data Selection")
        {
            selection = (avtMultiresSelection *) *(selectionsList[i]);
            selection->GetCompositeProjectionTransformMatrix(transformMatrix);
            cellArea = selection->GetDesiredCellArea();

            (*selectionsApplied)[i] = true;
        }
    }
}


// ****************************************************************************
//  Method: avtMRTestFileFormat::CalculateMesh2d
//
//  Purpose:
//    Calculates the parameters defining the mesh for the current multi
//    resolution data selection.
//
//  Arguments:
//    tileXmin  The tile aligned minimum X value of the mesh.
//    tileXmax  The tile aligned maximum X value of the mesh.
//    tileYmin  The tile aligned minimum Y value of the mesh.
//    tileYmax  The tile aligned maximum Y value of the mesh.
//    nx        The number of zones in the X direction.
//    ny        The number of zones in the Y direction.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Dec 20 12:20:07 PDT 2013
//
//  Modifications:
//    Eric Brugger, Wed Jan  8 17:09:17 PST 2014
//    I modified the reader to also create 3d meshes.
//
// ****************************************************************************

void
avtMRTestFileFormat::CalculateMesh2d(double &tileXmin, double &tileXmax,
    double &tileYmin, double &tileYmax, int &nx, int &ny)
{
    //
    // Calculate the extents and view area.
    //
    double extents[6] = {meshXmin, meshXmax, meshYmin, meshYmax, 0., 0.};
    double viewArea = (meshXmax - meshXmin) * (meshYmax - meshYmin);
    if (transformMatrix[0] != DBL_MAX && transformMatrix[1] != DBL_MAX &&
        transformMatrix[2] != DBL_MAX && transformMatrix[3] != DBL_MAX)
    {
        avtView2D::CalculateExtentsAndArea(extents, viewArea, transformMatrix);
    }

    //
    // Calculate the extents of the mesh, with the extents aligning with
    // tile boundaries.
    //
    double viewSize = sqrt(viewArea);

    double meshXRange = meshXmax - meshXmin;
    double meshYRange = meshYmax - meshYmin;

    double meshVolume = meshXRange * meshYRange;
    double coarseVolume = meshVolume / (coarseNx * coarseNy);
    double coarseSize = sqrt(coarseVolume);
    double cellSize2 = viewSize * cellArea;
    int level = std::min(maxLevel2d,
        std::max(0, int(ceil(log(coarseSize / cellSize2) / log(2.)))));

    double tileXRange = meshXRange / pow(2., level); 
    double tileYRange = meshYRange / pow(2., level);

    int iTile = std::max(0., floor((extents[0] - meshXmin) / tileXRange));
    tileXmin = meshXmin + iTile * tileXRange;
    iTile = std::min(pow(2., level), ceil((extents[1] - meshXmin) / tileXRange));
    tileXmax = meshXmin + iTile * tileXRange;

    iTile = std::max(0., floor((extents[2] - meshYmin) / tileYRange));
    tileYmin = meshYmin + iTile * tileYRange;
    iTile = std::min(pow(2., level), ceil((extents[3] - meshYmin) / tileYRange));
    tileYmax = meshYmin + iTile * tileYRange;

    //
    // Handle the cases where we end up with a mesh with zero or negative
    // extents in one or both directions.
    //
    if (tileXmin <= meshXmin && tileXmax <= meshXmin)
    {
        //
        // We have gone off the right edge.
        //
        tileXmin = meshXmin;
        tileXmax = meshXmin + tileXRange;
    }
    else if (tileXmin >= meshXmax && tileXmax >= meshXmax)
    {
        //
        // We have gone off the left edge.
        //
        tileXmin = meshXmax - tileXRange;
        tileXmax = meshXmax;
    }
    if (tileYmin <= meshYmin && tileYmax <= meshYmin)
    {
        //
        // We have gone off the top edge.
        //
        tileYmin = meshYmin;
        tileYmax = meshYmin + tileYRange;
    }
    else if (tileYmin >= meshYmax && tileYmax >= meshYmax)
    {
        //
        // We have gone off the bottom edge.
        //
        tileYmin = meshYmax - tileYRange;
        tileYmax = meshYmax;
    }

    //
    // Determine the number of zones in each direction.
    //
    nx = int((tileXmax - tileXmin) / tileXRange) * coarseNx;
    ny = int((tileYmax - tileYmin) / tileYRange) * coarseNy;

    //
    // Set the actual multi resolution selection back into the selection.
    //
    if (selection != NULL)
    {
        extents[0] = tileXmin > meshXmin ? tileXmin : -DBL_MAX;
        extents[1] = tileXmax < meshXmax ? tileXmax : DBL_MAX;
        extents[2] = tileYmin > meshYmin ? tileYmin : -DBL_MAX;
        extents[3] = tileYmax < meshYmax ? tileYmax : DBL_MAX;
        extents[4] = 0.;
        extents[5] = 0.;

        double xDelta, yDelta;
        xDelta = (tileXmax - tileXmin) / nx;
        yDelta = (tileYmax - tileYmin) / ny;

        cellArea = sqrt(xDelta * xDelta + yDelta * yDelta);
        selection->SetActualExtents(extents);
        selection->SetActualCellArea(cellArea);
    }
}


// ****************************************************************************
//  Method: avtMRTestFileFormat::CalculateMesh3d
//
//  Purpose:
//    Calculates the parameters defining the mesh for the current multi
//    resolution data selection.
//
//  Arguments:
//    tileXmin  The tile aligned minimum X value of the mesh.
//    tileXmax  The tile aligned maximum X value of the mesh.
//    tileYmin  The tile aligned minimum Y value of the mesh.
//    tileYmax  The tile aligned maximum Y value of the mesh.
//    tileZmin  The tile aligned minimum Z value of the mesh.
//    tileZmax  The tile aligned maximum Z value of the mesh.
//    nx        The number of zones in the X direction.
//    ny        The number of zones in the Y direction.
//    nz        The number of zones in the Z direction.
//
//  Programmer: Eric Brugger
//  Creation:   Wed Jan  8 17:09:17 PST 2014
//
//  Modifications:
//
// ****************************************************************************

void
avtMRTestFileFormat::CalculateMesh3d(double &tileXmin, double &tileXmax,
    double &tileYmin, double &tileYmax, double &tileZmin, double &tileZmax,
    int &nx, int &ny, int &nz)
{
    //
    // Calculate the extents and view area.
    //
    double extents[6] = {meshXmin, meshXmax, meshYmin, meshYmax,
                         meshZmin, meshZmax};
    double viewArea = (meshXmax - meshXmin) * (meshYmax - meshYmin);
    if (transformMatrix[0] != DBL_MAX && transformMatrix[1] != DBL_MAX &&
        transformMatrix[2] != DBL_MAX && transformMatrix[3] != DBL_MAX)
    {
        avtView3D::CalculateExtentsAndArea(extents, viewArea, transformMatrix);
    }

    //
    // Calculate the extents of the mesh, with the extents aligning with
    // tile boundaries.
    //
    double viewSize = sqrt(viewArea);

    double meshXRange = meshXmax - meshXmin;
    double meshYRange = meshYmax - meshYmin;
    double meshZRange = meshZmax - meshZmin;

    double meshVolume = meshXRange * meshYRange * meshZRange;
    double coarseVolume = meshVolume / (coarseNx * coarseNy * coarseNz);
    double coarseSize = pow(coarseVolume, 1. / 3.);
    double cellSize2 = viewSize * cellArea;
    int level = std::min(maxLevel3d,
        std::max(0, int(ceil(log(coarseSize / cellSize2) / log(2.)))));

    double tileXRange = meshXRange / pow(2., level); 
    double tileYRange = meshYRange / pow(2., level);
    double tileZRange = meshZRange / pow(2., level);

    int iTile = std::max(0., floor((extents[0] - meshXmin) / tileXRange));
    tileXmin = meshXmin + iTile * tileXRange;
    iTile = std::min(pow(2., level), ceil((extents[1] - meshXmin) / tileXRange));
    tileXmax = meshXmin + iTile * tileXRange;

    iTile = std::max(0., floor((extents[2] - meshYmin) / tileYRange));
    tileYmin = meshYmin + iTile * tileYRange;
    iTile = std::min(pow(2., level), ceil((extents[3] - meshYmin) / tileYRange));
    tileYmax = meshYmin + iTile * tileYRange;

    iTile = std::max(0., floor((extents[4] - meshZmin) / tileZRange));
    tileZmin = meshZmin + iTile * tileZRange;
    iTile = std::min(pow(2., level), ceil((extents[5] - meshZmin) / tileZRange));
    tileZmax = meshZmin + iTile * tileZRange;

    //
    // Handle the cases where we end up with a mesh with zero or negative
    // extents in one or both directions.
    //
    if (tileXmin <= meshXmin && tileXmax <= meshXmin)
    {
        //
        // We have gone off the right edge.
        //
        tileXmin = meshXmin;
        tileXmax = meshXmin + tileXRange;
    }
    else if (tileXmin >= meshXmax && tileXmax >= meshXmax)
    {
        //
        // We have gone off the left edge.
        //
        tileXmin = meshXmax - tileXRange;
        tileXmax = meshXmax;
    }
    if (tileYmin <= meshYmin && tileYmax <= meshYmin)
    {
        //
        // We have gone off the top edge.
        //
        tileYmin = meshYmin;
        tileYmax = meshYmin + tileYRange;
    }
    else if (tileYmin >= meshYmax && tileYmax >= meshYmax)
    {
        //
        // We have gone off the bottom edge.
        //
        tileYmin = meshYmax - tileYRange;
        tileYmax = meshYmax;
    }
    if (tileZmin <= meshZmin && tileZmax <= meshZmin)
    {
        //
        // We have gone off the front edge.
        //
        tileZmin = meshZmin;
        tileZmax = meshZmin + tileZRange;
    }
    else if (tileZmin >= meshZmax && tileZmax >= meshZmax)
    {
        //
        // We have gone off the back edge.
        //
        tileZmin = meshZmax - tileZRange;
        tileZmax = meshZmax;
    }

    //
    // Determine the number of zones in each direction.
    //
    nx = int((tileXmax - tileXmin) / tileXRange) * coarseNx;
    ny = int((tileYmax - tileYmin) / tileYRange) * coarseNy;
    nz = int((tileZmax - tileZmin) / tileZRange) * coarseNz;

    //
    // Set the actual multi resolution selection back into the selection.
    //
    if (selection != NULL)
    {
        extents[0] = tileXmin > meshXmin ? tileXmin : -DBL_MAX;
        extents[1] = tileXmax < meshXmax ? tileXmax : DBL_MAX;
        extents[2] = tileYmin > meshYmin ? tileYmin : -DBL_MAX;
        extents[3] = tileYmax < meshYmax ? tileYmax : DBL_MAX;
        extents[4] = tileZmin > meshZmin ? tileZmin : -DBL_MAX;
        extents[5] = tileZmax < meshZmax ? tileZmax : DBL_MAX;

        double xDelta, yDelta, zDelta;
        xDelta = (tileXmax - tileXmin) / nx;
        yDelta = (tileYmax - tileYmin) / ny;
        zDelta = (tileZmax - tileZmin) / nz;

        cellArea = sqrt(xDelta * xDelta + yDelta * yDelta + zDelta * zDelta);
        selection->SetActualExtents(extents);
        selection->SetActualCellArea(cellArea);
    }
}


// ****************************************************************************
//  Method: avtMRTestFileFormat::GetMesh2d
//
//  Purpose:
//    Gets the 2d mesh associated with this file.
//
//  Programmer: Eric Brugger
//  Creation:   Wed Jan  8 17:09:17 PST 2014
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtMRTestFileFormat::GetMesh2d()
{
    //
    // Determine the mesh starting location and size of each cell.
    //
    double tileXmin, tileXmax, tileYmin, tileYmax;
    int nx, ny;

    CalculateMesh2d(tileXmin, tileXmax, tileYmin, tileYmax, nx, ny);

    double xStart, yStart;
    double xDelta, yDelta;

    xStart = tileXmin;
    yStart = tileYmin;
    xDelta = (tileXmax - tileXmin) / nx;
    yDelta = (tileYmax - tileYmin) / ny;

    //
    // Create the grid.
    //
    int dims[3];
    dims[0] = nx + 1;
    dims[1] = ny + 1;
    dims[2] = 1;

    vtkRectilinearGrid *rg = vtkRectilinearGrid::New();
    rg->SetDimensions(dims);

    vtkFloatArray  *xcoord = vtkFloatArray::New();
    vtkFloatArray  *ycoord = vtkFloatArray::New();
    vtkFloatArray  *zcoord = vtkFloatArray::New();

    xcoord->SetNumberOfTuples(dims[0]);
    ycoord->SetNumberOfTuples(dims[1]);
    zcoord->SetNumberOfTuples(dims[2]);

    float *ptr = xcoord->GetPointer(0);
    for (int i = 0; i < nx + 1; i++)
        ptr[i] = xStart + double(i) * xDelta;
    ptr = ycoord->GetPointer(0);
    for (int i = 0; i < ny + 1; i++)
        ptr[i] = yStart + double(i) * yDelta;
    ptr = zcoord->GetPointer(0);
    ptr[0] = 0.;

    rg->SetXCoordinates(xcoord);
    rg->SetYCoordinates(ycoord);
    rg->SetZCoordinates(zcoord);

    xcoord->Delete();
    ycoord->Delete();
    zcoord->Delete();

    return rg;
}


// ****************************************************************************
//  Method: avtMRTestFileFormat::GetMesh3d
//
//  Purpose:
//    Gets the 3d mesh associated with this file.
//
//  Programmer: Eric Brugger
//  Creation:   Wed Jan  8 17:09:17 PST 2014
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtMRTestFileFormat::GetMesh3d()
{
    //
    // Determine the mesh starting location and size of each cell.
    //
    double tileXmin, tileXmax, tileYmin, tileYmax, tileZmin, tileZmax;
    int nx, ny, nz;

    CalculateMesh3d(tileXmin, tileXmax, tileYmin, tileYmax, tileZmin, tileZmax,
                    nx, ny, nz);

    double xStart, yStart, zStart;
    double xDelta, yDelta, zDelta;

    xStart = tileXmin;
    yStart = tileYmin;
    zStart = tileZmin;
    xDelta = (tileXmax - tileXmin) / nx;
    yDelta = (tileYmax - tileYmin) / ny;
    zDelta = (tileZmax - tileZmin) / nz;

    //
    // Create the grid.
    //
    int dims[3];
    dims[0] = nx + 1;
    dims[1] = ny + 1;
    dims[2] = nz + 1;

    vtkRectilinearGrid *rg = vtkRectilinearGrid::New();
    rg->SetDimensions(dims);

    vtkFloatArray  *xcoord = vtkFloatArray::New();
    vtkFloatArray  *ycoord = vtkFloatArray::New();
    vtkFloatArray  *zcoord = vtkFloatArray::New();

    xcoord->SetNumberOfTuples(dims[0]);
    ycoord->SetNumberOfTuples(dims[1]);
    zcoord->SetNumberOfTuples(dims[2]);

    float *ptr = xcoord->GetPointer(0);
    for (int i = 0; i < nx + 1; i++)
        ptr[i] = xStart + double(i) * xDelta;
    ptr = ycoord->GetPointer(0);
    for (int i = 0; i < ny + 1; i++)
        ptr[i] = yStart + double(i) * yDelta;
    ptr = zcoord->GetPointer(0);
    for (int i = 0; i < nz + 1; i++)
        ptr[i] = zStart + double(i) * zDelta;

    rg->SetXCoordinates(xcoord);
    rg->SetYCoordinates(ycoord);
    rg->SetZCoordinates(zcoord);

    xcoord->Delete();
    ycoord->Delete();
    zcoord->Delete();

    return rg;
}


// ****************************************************************************
//  Method: avtMRTestFileFormat::GetVar2d
//
//  Purpose:
//    Gets the 2d variable associated with this file.
//
//  Programmer: Eric Brugger
//  Creation:   Tue Jan 28 09:30:10 PST 2014
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtMRTestFileFormat::GetVar2d()
{
    //
    // Determine the mesh starting location and size of each cell.
    //
    double tileXmin, tileXmax, tileYmin, tileYmax;
    int nx, ny;

    CalculateMesh2d(tileXmin, tileXmax, tileYmin, tileYmax, nx, ny);

    double xStart, yStart;
    double xDelta, yDelta;

    xStart = 4. * (tileXmin - meshXmin) / (meshXmax - meshXmin);
    yStart = 4. * (tileYmin - meshYmin) / (meshYmax - meshYmin);

    xDelta = (tileXmax - tileXmin) / nx;
    yDelta = (tileYmax - tileYmin) / ny;
    xDelta = xDelta / ((meshXmax - meshXmin) / 4.);
    yDelta = yDelta / ((meshYmax - meshYmin) / 4.);

    //
    // Create the variable.
    //
    vtkFloatArray *scalars = vtkFloatArray::New();
    scalars->SetNumberOfTuples(nx*ny);
    float *ptr = (float*)scalars->GetVoidPointer(0);
    for (int j = 0; j < ny; j++)
    {
        for (int i = 0; i < nx; i++)
        {
            double x = (xStart + (double(i) + 0.5) * xDelta) - 2.;
            double y = (yStart + (double(j) + 0.5) * yDelta) - 2.;
            ptr[j*nx+i] = mandelbrot(complex(x, y));
        }
    }

    return scalars;
}


// ****************************************************************************
//  Method: avtMRTestFileFormat::GetVar3d
//
//  Purpose:
//    Gets the 3d variable associated with this file.
//
//  Programmer: Eric Brugger
//  Creation:   Tue Jan 28 09:30:10 PST 2014
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtMRTestFileFormat::GetVar3d()
{
    //
    // Determine the mesh starting location and size of each cell.
    //
    double tileXmin, tileXmax, tileYmin, tileYmax, tileZmin, tileZmax;
    int nx, ny, nz;

    CalculateMesh3d(tileXmin, tileXmax, tileYmin, tileYmax, tileZmin, tileZmax,
                    nx, ny, nz);

    double xStart, yStart;
    double xDelta, yDelta, zDelta;

    xStart = 4. * (tileXmin - meshXmin) / (meshXmax - meshXmin);
    yStart = 4. * (tileYmin - meshYmin) / (meshYmax - meshYmin);

    xDelta = (tileXmax - tileXmin) / nx;
    yDelta = (tileYmax - tileYmin) / ny;
    zDelta = (tileZmax - tileZmin) / nz;
    xDelta = xDelta / ((meshXmax - meshXmin) / 4.);
    yDelta = yDelta / ((meshYmax - meshYmin) / 4.);
    zDelta = zDelta / ((meshZmax - meshZmin) / 4.);

    //
    // Create the variable.
    //
    vtkFloatArray *scalars = vtkFloatArray::New();
    scalars->SetNumberOfTuples(nx*ny*nz);
    float *ptr = (float*)scalars->GetVoidPointer(0);
    int nxy = nx * ny;
    for (int k = 0; k < nz; k++)
    {
        for (int j = 0; j < ny; j++)
        {
            for (int i = 0; i < nx; i++)
            {
                double x = (xStart + (double(i) + 0.5) * xDelta) - 2.;
                double y = (yStart + (double(j) + 0.5) * yDelta) - 2.;
                ptr[k*nxy+j*nx+i] = mandelbrot(complex(x, y));
            }
        }
    }

    return scalars;
}
