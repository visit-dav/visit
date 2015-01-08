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

// ***************************************************************************
//                            avtAMRTestFileFormat.C                          
//
//  Purpose: Sample database that generates AMR data.
//
//  Notes:
//    The reader generates a 2D 50 x 50 top level mesh that extends from
//    0 to 50. in both the x and y directions. The refinement ratios are
//    2 in both the x and y directions. The domains are numbered as follows:
//
//    Level 0:                       0
//
//    Level 1:                    3  4
//                                1  2
//
//    Level 2:             17 18 19 20
//                         13 14 15 16
//                          9 10 11 12
//                          5  6  7  8
//
//    Level 3: 77 78 79 80 81 82 83 84
//             69 70 71 72 73 74 75 76
//             61 62 63 64 65 66 67 68
//             53 54 55 56 57 58 59 60
//             45 46 47 48 49 50 51 52
//             37 38 39 40 41 42 43 44
//             29 30 31 32 33 34 35 36
//             21 22 23 24 25 26 27 28
//
//    This results in: domain 0 having children 1, 2, 3, 4
//                     domain 1 having children 5, 6, 9, 10
//                     domain 2 having children 7, 8, 11, 12
//                                  ...
//
//  Programmer:  Eric Brugger
//  Creation:    November 20, 2013
//
//  Modifications:
//
// ****************************************************************************


#include <avtAMRTestFileFormat.h>

#include <vtkRectilinearGrid.h>
#include <vtkFloatArray.h>

#include <avtDatabaseMetaData.h>
#include <avtStructuredDomainNesting.h>
#include <avtVariableCache.h>

#include <BadIndexException.h>
#include <DebugStream.h>

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
// Method: avtAMRTestFileFormat::GetLevelAndLocalPatch
//
// Purpose:
//   Get the level and local patch numbers given a domain number.
//
// Arguments:
//   domain     : The domain.
//   level      : The level associated with the domain.
//   localPatch : The local patch number associated with the domain.
//
// Programmer: Eric Brugger
// Creation:   November 20, 2013
//
// Modifications:
//
// ****************************************************************************

void
avtAMRTestFileFormat::GetLevelAndLocalPatch(int domain, int &level,
    int &localPatch)
{
    level = 0;
    while (domain > patchOffsetForLevel[level] - 1)
        level++;
    level --;
    localPatch = domain - patchOffsetForLevel[level];
}

// ****************************************************************************
// Method: avtAMRTestFileFormat::PopulateDomainNesting
//
// Purpose:
//   Populate the domain nesting structure and put it in the cache.
//
// Programmer: Eric Brugger
// Creation:   November 20, 2013
//
// Modifications:
//
// ****************************************************************************

void
avtAMRTestFileFormat::PopulateDomainNesting()
{
    avtStructuredDomainNesting *dn = new avtStructuredDomainNesting(
            totalPatches, numLevels);

    std::vector<int> refinementRatios(3, 1);
    std::vector<double> cs(3);
    for (int level = 0; level < numLevels; level++)
    {
        if (level > 0)
        {
            refinementRatios[0] = 2;
            refinementRatios[1] = 2;
        }
        dn->SetLevelRefinementRatios(level, refinementRatios);

        for (int i = 0; i < 3; i++)
            cs[i] = 1. / double(refinementForLevel[level]); 
        dn->SetLevelCellSizes(level, cs);
    }

    for (int patch = 0; patch < totalPatches; patch++)
    {
        //
        // Get the level and local patch number.
        //
        int level, localPatch;
        GetLevelAndLocalPatch(patch, level, localPatch);

        int xDist, yDist;
        int xStart, yStart;
        int xEnd, yEnd;

        xDist = 50;
        yDist = 50;
        xStart = (localPatch % refinementForLevel[level])     * xDist;
        xEnd   = (localPatch % refinementForLevel[level] + 1) * xDist - 1;
        yStart = (localPatch / refinementForLevel[level])     * yDist;
        yEnd   = (localPatch / refinementForLevel[level] + 1) * yDist - 1;

        std::vector<int> logExts(6);
        logExts[0] = xStart;
        logExts[3] = xEnd;
        logExts[1] = yStart;
        logExts[4] = yEnd;
        logExts[2] = 0;
        logExts[5] = 0;

        int nChildPatches;
        if (level < numLevels - 1)
            nChildPatches = 4;
        else
            nChildPatches = 0;

        std::vector<int> childPatches(nChildPatches);
        if (nChildPatches == 4)
        {
            int offset = patchOffsetForLevel[level+1];
            int nx = refinementForLevel[level];
            int i = (localPatch % nx) * 2;
            int j = (localPatch / nx) * 2;

            childPatches[0] = offset + j * nx * 2 + i;
            childPatches[1] = offset + j * nx * 2 + i + 1;
            childPatches[2] = offset + (j + 1) * nx * 2 + i;
            childPatches[3] = offset + (j + 1) * nx * 2 + i + 1;
        }

        dn->SetNestingForDomain(patch, level, childPatches, logExts);
    }

    dn->SetNumDimensions(2);
    void_ref_ptr vr = void_ref_ptr(dn, avtStructuredDomainNesting::Destruct);
    cache->CacheVoidRef("any_mesh", AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
                        0, -1, vr);
}

// ****************************************************************************
//  Method: avtAMRTestFileFormat constructor
//
//  Programmer: Eric Brugger
//  Creation:   November 20, 2013
//
//  Modifications:
//    Eric Brugger, Tue Nov 26 19:20:47 PST 2013
//    I modified the routine to output a mandelbrot set instead of a
//    distance field.
//
// ****************************************************************************

avtAMRTestFileFormat::avtAMRTestFileFormat(const char *fname)
    : avtSTMDFileFormat(&fname, 1)
{
    filename = fname;

    maxLevels = 10;

    nPatchesForLevel[0]    = 1;
    patchOffsetForLevel[0] = 0;
    refinementForLevel[0]  = 1;
    for (int level = 1; level < maxLevels; level++)
    {
        nPatchesForLevel[level]    = nPatchesForLevel[level-1] * 4;
        patchOffsetForLevel[level] = patchOffsetForLevel[level-1] +
                                     nPatchesForLevel[level-1];
        refinementForLevel[level]  = refinementForLevel[level-1] * 2;
    }

    numLevels = 6;
    totalPatches = patchOffsetForLevel[numLevels-1] +
                   nPatchesForLevel[numLevels-1];

    totalPatches = 0;
    for (int i = 0; i < numLevels; i++)
        totalPatches += int(pow(2., 2.*double(i)));
}

// ****************************************************************************
//  Method: avtAMRTestFileFormat destructor
//
//  Programmer: Eric Brugger
//  Creation:   November 20, 2013
//
//  Modifications:
//
// ****************************************************************************

avtAMRTestFileFormat::~avtAMRTestFileFormat()
{
}

// ****************************************************************************
//  Method: avtAMRTestFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file. The mesh is returned as a
//      vtkRectilinearGrid.
//
//  Arguments:
//      domain    : The domain.
//      name      : The name of the mesh of interest. This is ignored.
//
//  Programmer: Eric Brugger
//  Creation:   November 20, 2013
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtAMRTestFileFormat::GetMesh(int domain, const char *name)
{
    if (domain < 0 || domain >= totalPatches)
    {
        EXCEPTION2(BadIndexException, domain, 1);
    }

    int nx, ny;
    nx = 51;
    ny = 51;

    double xDist, yDist;
    xDist = 50.;
    yDist = 50.;

    int dims[3];
    dims[0] = nx;
    dims[1] = ny;
    dims[2] = 1;

    vtkRectilinearGrid *rg = vtkRectilinearGrid::New();
    rg->SetDimensions(dims);

    vtkFloatArray  *xcoord = vtkFloatArray::New();
    vtkFloatArray  *ycoord = vtkFloatArray::New();
    vtkFloatArray  *zcoord = vtkFloatArray::New();

    xcoord->SetNumberOfTuples(dims[0]);
    ycoord->SetNumberOfTuples(dims[1]);
    zcoord->SetNumberOfTuples(dims[2]);

    //
    // Get the level and local patch number.
    //
    int level, localPatch;
    GetLevelAndLocalPatch(domain, level, localPatch);

    double xStart, yStart;
    double delta;

    xStart = (localPatch % refinementForLevel[level]) *
             (xDist / double(refinementForLevel[level]));
    yStart = (localPatch / refinementForLevel[level]) *
             (yDist / double(refinementForLevel[level]));
    delta = 1. / double(refinementForLevel[level]); 

    float *ptr = xcoord->GetPointer(0);
    for (int i = 0; i < nx; i++)
        ptr[i] = xStart + double(i) * delta;
    ptr = ycoord->GetPointer(0);
    for (int i = 0; i < ny; i++)
        ptr[i] = yStart + double(i) * delta;
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
//  Method: avtAMRTestFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.
//
//  Arguments:
//      domain   : The index of the domain.
//      name     : The name of the variable requested. This is ignored.
//
//  Programmer: Eric Brugger
//  Creation:   November 20, 2013
//
//  Modifications:
//    Eric Brugger, Tue Nov 26 19:20:47 PST 2013
//    I modified the routine to output a mandelbrot set instead of a
//    distance field.
//
// ****************************************************************************

vtkDataArray *
avtAMRTestFileFormat::GetVar(int domain, const char *name)
{
    int nx, ny;

    nx = 50;
    ny = 50;

    double xDist, yDist;
    xDist = 4.;
    yDist = 4.;

    //
    // Get the level and local patch number.
    //
    int level, localPatch;
    GetLevelAndLocalPatch(domain, level, localPatch);

    double xStart, yStart;
    double delta;

    xStart = (localPatch % refinementForLevel[level]) *
             (xDist / double(refinementForLevel[level]));
    yStart = (localPatch / refinementForLevel[level]) *
             (yDist / double(refinementForLevel[level]));
    delta = (xDist / 50.) / double(refinementForLevel[level]); 

    vtkFloatArray *scalars = vtkFloatArray::New();
    scalars->SetNumberOfTuples(nx*ny);
    float *ptr = (float*)scalars->GetVoidPointer(0);
    for (int i = 0; i < 50; i++)
    {
        for (int j = 0; j < 50; j++)
        {
            double x = (xStart + (double(i) + 0.5) * delta) - 2.;
            double y = (yStart + (double(j) + 0.5) * delta) - 2.;
            ptr[j*nx+i] = mandelbrot(complex(x, y));
        }
    }

    return scalars;
}

// ****************************************************************************
//  Method: avtAMRTestFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Return the database meta-data object. It is the table of contents
//      for the file. The "file" contains a single mesh and a single variable.
//
//  Programmer: Eric Brugger
//  Creation:   November 20, 2013
//
//  Modifications:
//    Eric Brugger, Tue Nov 26 19:20:47 PST 2013
//    I modified the routine to output a mandelbrot set instead of a
//    distance field.
//
// ****************************************************************************

void
avtAMRTestFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "Mesh";
    mesh->meshType = AVT_AMR_MESH;
    mesh->numBlocks = totalPatches;
    mesh->blockOrigin = 0;
    mesh->spatialDimension = 2;
    mesh->topologicalDimension = 2;

    mesh->hasSpatialExtents = true;
    mesh->minSpatialExtents[0] = 0;
    mesh->maxSpatialExtents[0] = 50;
    mesh->minSpatialExtents[1] = 0;
    mesh->maxSpatialExtents[1] = 50;

    mesh->blockTitle = "patches";
    mesh->blockPieceName = "patch";
    mesh->numGroups = numLevels;
    mesh->groupTitle = "levels";
    mesh->groupPieceName = "level";

    mesh->containsExteriorBoundaryGhosts = false;

    std::vector<int> groupIds(totalPatches);
    std::vector<std::string> blockPieceNames(totalPatches);
    int levelsOfDetail = 0;
    for (int i = 0; i < totalPatches; i++)
    {
        //
        // Get the level and local patch number.
        //
        int level, localPatch;
        GetLevelAndLocalPatch(i, level, localPatch);

        groupIds[i] = level;
        char tmpName[128];
        sprintf(tmpName, "level%d,patch%d", level, localPatch);
        blockPieceNames[i] = tmpName;
        levelsOfDetail = std::max(levelsOfDetail, level);
    }
    mesh->blockNames = blockPieceNames;
    mesh->LODs = levelsOfDetail;
    md->Add(mesh);
    md->AddGroupInformation(numLevels,totalPatches,groupIds);

    md->Add(new avtScalarMetaData("Mandelbrot", "Mesh", AVT_ZONECENT));

    PopulateDomainNesting();
}
