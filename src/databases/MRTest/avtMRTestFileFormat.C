/*****************************************************************************
*
* Copyright (c) 2000 - 2013, Lawrence Livermore National Security, LLC
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
#include <avtMultiresSelection.h>

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
//
// ****************************************************************************

avtMRTestFileFormat::avtMRTestFileFormat(const char *fname)
    : avtSTMDFileFormat(&fname, 1)
{
    filename = fname;

    meshNx = 4096, meshNy = 4096;
    meshXmin = 0., meshXmax = 4096., meshYmin = 0., meshYmax = 4096.;
    coarseNx = 64, coarseNy = 64;
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
//
// ****************************************************************************

void
avtMRTestFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "Mesh";
    mesh->meshType = AVT_RECTILINEAR_MESH;
    mesh->numBlocks = 1;
    mesh->blockOrigin = 0;
    mesh->spatialDimension = 2;
    mesh->topologicalDimension = 2;
    md->Add(mesh);

    md->SetFormatCanDoMultires(true);

    md->Add(new avtScalarMetaData("Mandelbrot", "Mesh", AVT_ZONECENT));
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
//
// ****************************************************************************

vtkDataSet *
avtMRTestFileFormat::GetMesh(int domain, const char *meshname)
{
    //
    // Determine the mesh starting location and size of each cell.
    //
    double tileXmin, tileXmax, tileYmin, tileYmax;
    int nx, ny;

    CalculateMesh(tileXmin, tileXmax, tileYmin, tileYmax, nx, ny);

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
//
// ****************************************************************************

vtkDataArray *
avtMRTestFileFormat::GetVar(int domain, const char *varname)
{
    //
    // Determine the mesh starting location and size of each cell.
    //
    double tileXmin, tileXmax, tileYmin, tileYmax;
    int nx, ny;

    CalculateMesh(tileXmin, tileXmax, tileYmin, tileYmax, nx, ny);

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
    for (int i = 0; i < nx; i++)
    {
        for (int j = 0; j < ny; j++)
        {
            double x = (xStart + (double(i) + 0.5) * xDelta) - 2.;
            double y = (yStart + (double(j) + 0.5) * yDelta) - 2.;
            ptr[j*nx+i] = mandelbrot(complex(x, y));
        }
    }

    return scalars;
}


// ****************************************************************************
//  Method: avtMRTestFileFormat::CalculateMesh
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
//
// ****************************************************************************

void
avtMRTestFileFormat::CalculateMesh(double &tileXmin, double &tileXmax,
    double &tileYmin, double &tileYmax, int &nx, int &ny)
{
    //
    // Get the multi resolution data selection.
    //
    double frustum[6] = {meshXmin, meshXmax, meshYmin, meshYmax, 0., 0.};
    double cellSize = .002;

    avtMultiresSelection *selection = NULL;
    for (int i = 0; i < selectionsList.size(); i++)
    {
        if (string(selectionsList[i]->GetType()) == "Multi Resolution Data Selection")
        {
            selection = (avtMultiresSelection *) *(selectionsList[i]);
            selection->GetDesiredFrustum(frustum);
            cellSize = selection->GetDesiredCellSize();

            (*selectionsApplied)[i] = true;
        }
    }
    if (frustum[0] == DBL_MAX && frustum[1] == -DBL_MAX)
    {
        frustum[0] = meshXmin;
        frustum[1] = meshXmax;
        frustum[2] = meshYmin;
        frustum[3] = meshYmax;
    }

    //
    // Calculate the extents of the mesh, with the extents aligning with
    // tile boundaries.
    //
    double xRange = frustum[1] - frustum[0];
    double yRange = frustum[3] - frustum[2];
    double maxRange = std::max(xRange, yRange);
    double diag = sqrt(2.) * maxRange;

    double meshXRange = meshXmax - meshXmin;
    double meshYRange = meshYmax - meshYmin;

    double meshMaxRange = std::max(meshXRange, meshYRange);
    double meshDiag = sqrt(2.) * meshMaxRange;
    double coarseDiag = meshDiag / coarseNx;
    double cellDiag = diag * cellSize;
    int level = std::max(0, int(ceil(log(coarseDiag / cellDiag) / log(2.))));

    double tileXRange = meshXRange / pow(2., level); 
    double tileYRange = meshYRange / pow(2., level);

    int iTile = std::max(0., floor((frustum[0] - meshXmin) / tileXRange));
    tileXmin = meshXmin + iTile * tileXRange;
    iTile = std::min(pow(2., level), ceil((frustum[1] - meshXmin) / tileXRange));
    tileXmax = meshXmin + iTile * tileXRange;

    iTile = std::max(0., floor((frustum[2] - meshYmin) / tileYRange));
    tileYmin = meshYmin + iTile * tileYRange;
    iTile = std::min(pow(2., level), ceil((frustum[3] - meshYmin) / tileYRange));
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
        frustum[0] = tileXmin;
        frustum[1] = tileXmax;
        frustum[2] = tileYmin;
        frustum[3] = tileYmax;
        frustum[4] = 0.;
        frustum[5] = 0.;

        double xDelta, yDelta;
        xDelta = (tileXmax - tileXmin) / nx;
        yDelta = (tileYmax - tileYmin) / ny;

        cellSize = sqrt(xDelta * xDelta + yDelta * yDelta);
        selection->SetActualFrustum(frustum);
        selection->SetActualCellSize(cellSize);
    }
}
