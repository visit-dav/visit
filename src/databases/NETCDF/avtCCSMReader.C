/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                            avtCCSMReader.C                                //
// ************************************************************************* //

#include <avtCCSMReader.h>

#include <string>

#include <vtkCellArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>

#include <vtkUnsignedCharArray.h>
#include <vtkShortArray.h>
#include <vtkIntArray.h>
#include <vtkLongArray.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>

#include <netcdf.h>

#include <snprintf.h>

#include <avtDatabaseMetaData.h>

#include <DebugStream.h>

#include <InvalidVariableException.h>

#include <NETCDFFileObject.h>

// ****************************************************************************
//  Method: avtCCSM constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 11 11:28:20 PDT 2007
//
// ****************************************************************************

avtCCSMReader::avtCCSMReader(const char *filename, NETCDFFileObject *f) : 
    avtNETCDFReaderBase(filename, f)
{
    initialized = false;
    dimSizes = 0;
}

avtCCSMReader::avtCCSMReader(const char *filename) : 
    avtNETCDFReaderBase(filename)
{
    initialized = false;
    dimSizes = 0;
}

// ****************************************************************************
// Method: avtCCSMReader::~avtCCSMReader
//
// Purpose: 
//   Destructor for the avtCCSMReader class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 5 12:17:36 PDT 2007
//
// Modifications:
//   
//    Kathleen Bonnell, Tue Nov  3 11:58:06 PST 2009
//    Don't delete fileObject here, base class does it.
//
// ****************************************************************************

avtCCSMReader::~avtCCSMReader()
{
    FreeUpResources();

    delete [] dimSizes;
}

// ****************************************************************************
//  Method: avtCCSMReader::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 11 11:28:20 PDT 2007
//
//  Modifications:
//    Brad Whitlock, Thu Oct 29 11:04:18 PDT 2009
//    I made it support both time-varying and static variables.
//
// ****************************************************************************

void
avtCCSMReader::PopulateDatabaseMetaData(int timeState, avtDatabaseMetaData *md)
{
    const char *mName = "avtCCSMReader::PopulateDatabaseMetaData: ";
    debug4 << mName << endl;
    if(DebugStream::Level4())
        fileObject->PrintFileContents(DebugStream::Stream4());

    int status, nDims, nVars, nGlobalAtts, unlimitedDimension;
    status = nc_inq(fileObject->GetFileHandle(), &nDims, &nVars, &nGlobalAtts,
                    &unlimitedDimension);
    if(status != NC_NOERR)
    {
        fileObject->HandleError(status);
        return;
    }

    // Get the size of all of the dimensions in the file.
    dimSizes = new size_t[nDims];
    int i;
    for(i = 0; i < nDims; ++i)
    {
        char   dimName[NC_MAX_NAME+1];
        size_t sz;
        if((status = nc_inq_dim(fileObject->GetFileHandle(), i, dimName,
           &sz)) == NC_NOERR)
        {
            dimSizes[i] = sz;
        }
        else
        {
            dimSizes[i] = 1;
            fileObject->HandleError(status);
        }
    }

    // Get the time dimension.
    int timedim = -1, time_nts = 0;
    std::string timedimname;
    GetTimeDimension(fileObject, timedim, time_nts, timedimname);

    // Iterate over the variables and create a list of meshes names and add
    // the variable to the metadata.
    meshNameToDimensions.clear();
    for(i = 0; i < nVars; ++i)
    {
        char varname[NC_MAX_NAME+1];
        nc_type vartype;
        int  varndims;
        int  vardims[NC_MAX_VAR_DIMS];
        int  varnatts;
        if((status = nc_inq_var(fileObject->GetFileHandle(), i, varname,
                                &vartype, &varndims, 
                                vardims, &varnatts)) == NC_NOERR)
        {
            // Count the number of variable dimensions that are greater
            // than 1.
            if (varndims == 0)
                continue;

            int nSpatialDimsGt1 = 0;
            int maxDim = dimSizes[vardims[0]];
            bool hasTimeDimension = false;
            for(int dim = 0; dim < varndims; ++dim)
            {
                int d = dimSizes[vardims[dim]];
                if(vardims[dim] != timedim)
                {
                    if(d > 1)
                        ++nSpatialDimsGt1;
                }

                if(d > maxDim)
                    maxDim = d;

                if(vardims[dim] == timedim)
                    hasTimeDimension = true;
            }

            int requiredDims = 2;
            if(hasTimeDimension)
                requiredDims = 3;

            if(nSpatialDimsGt1 < 2 || nSpatialDimsGt1 > 3)
            {
                debug4 << mName << "Rejecting " << varname
                       << " because it can't yield 2D or 3D data over time." << endl;
                continue;
            }

#define TIME_DIMENSION -1

            char tmp[100];
            std::string meshName("mesh");
            intVector meshDims, vDims;
            int elems = 1;
            debug4 << mName << "vardims=";
            for(int dim = varndims-1; dim >= 0; dim--)
            {
                debug4 << vardims[dim] << ", ";

                int d;
                if(vardims[dim] != timedim)
                {
                    vDims.push_back(vardims[dim]);
                    d = dimSizes[vardims[dim]];
                    elems *= d;
                }
                else
                    d = TIME_DIMENSION;

                meshDims.push_back(d);
            }
            debug4 << endl;

            // Remove all of the trailing 1's in the meshDims array.
            while(meshDims.size() > 0 &&
                  meshDims[meshDims.size()-1] == 1)
            {
                meshDims.pop_back();
                vDims.pop_back();
            }

            // Make sure it's not 1 or 0 in some of its dimensions
            if(meshDims.size() >= 2 && elems != maxDim && elems > 0)
            {
                // Come up with the mesh name that we'll use for this
                // variable. Note that we don't consider the time dimension
                // so time-varying and non-time-varying variables can share
                // the same dimension if possible.
                int nSpatialDims = 0;
                for(int j = 0; j < meshDims.size(); ++j)
                {
                    if(meshDims[j] != TIME_DIMENSION)
                    {
                        if(nSpatialDims != 0)
                            SNPRINTF(tmp, 100, "x%d", meshDims[j]);
                        else
                            SNPRINTF(tmp, 100, "%d", meshDims[j]);
                        ++nSpatialDims;

                        meshName += tmp;
                    }
                }
                // Print the meshDimensions.
                debug4 << mName << "meshName=" << meshName << ", dims[x,y[,z][,t]]=";
                for(int j = 0; j < meshDims.size(); ++j)
                    debug4 << meshDims[j] << ",";
                debug4 << endl;

                std::string globalMesh = std::string("global/") + meshName;
 
                // Add the name of the mesh to the list of meshes.
                if(meshNameToDimensions.find(meshName) == meshNameToDimensions.end())
                {
                    // Filter out time from the dimensions so time varying and static
                    // variables can share the same mesh.
                    intVector meshDimsWithoutTime;
                    for(int dim = 0; dim < meshDims.size(); ++dim)
                        if(meshDims[dim] != TIME_DIMENSION)
                            meshDimsWithoutTime.push_back(meshDims[dim]);

                    meshNameToDimensions[meshName] = meshDimsWithoutTime;

                    if(md != 0)
                    {
                        avtMeshMetaData *mmd = new avtMeshMetaData(meshName, 
                            1, 1, 1, 0, nSpatialDims, nSpatialDims,
                            AVT_RECTILINEAR_MESH);
                        mmd->validVariable = nSpatialDims <= 3;
                        // Try and read the mesh units
                        const char *latvars[] = {"lat", "latitude", "LATIXY", "latixy"};
                        const char *lonvars[] = {"lon", "longitude", "LONGXY", "longxy"};
                        for(int v = 0; v < 4; ++v)
                            if(fileObject->ReadStringAttribute(latvars[v], "units", mmd->yUnits))
                                break;
                        for(int v = 0; v < 4; ++v)
                            if(fileObject->ReadStringAttribute(lonvars[v], "units", mmd->xUnits))
                                break;
                        // Provide the dimension names as the axis labels.
                        char dimName[NC_MAX_NAME+1];
                        size_t sz;
                        if((status = nc_inq_dim(fileObject->GetFileHandle(), vDims[0], dimName,
                            &sz)) == NC_NOERR)
                        {
                            mmd->xLabel = dimName;
                        }
                        if((status = nc_inq_dim(fileObject->GetFileHandle(), vDims[1], dimName,
                            &sz)) == NC_NOERR)
                        {
                            mmd->yLabel = dimName;
                        }
                        if(nSpatialDims == 3)
                        {
                            if((status = nc_inq_dim(fileObject->GetFileHandle(), vDims[2], dimName,
                                &sz)) == NC_NOERR)
                            {
                                mmd->zLabel = dimName;
                            }
                        }
                        md->Add(mmd);
                    }

                    // Create a global mesh too.
                    meshNameToDimensions[globalMesh] = meshDimsWithoutTime;
                    if(md != 0)
                    {
                        int td = nSpatialDims;
                        int sd = (nSpatialDims < 3) ? (nSpatialDims+1) : 3;
                        avtMeshMetaData *mmd = new avtMeshMetaData(globalMesh, 
                            1, 1, 1, 0, sd, td,
                            AVT_UNSTRUCTURED_MESH);
                        mmd->validVariable = nSpatialDims <= 3;
                        md->Add(mmd);
                    }
                }

                // Try and get the variable units.
                if(md != 0)
                {
                    avtScalarMetaData *smd = new avtScalarMetaData(varname, meshName,
                        AVT_NODECENT);
                    smd->hasUnits = fileObject->ReadStringAttribute(
                        varname, "units", smd->units);
                    smd->validVariable = nSpatialDims <= 3;
                    md->Add(smd);

                    // So we can keep track of the var's size
                    varToDimensions[varname] = meshDims;

                    debug4 << "Variable " << varname << " on mesh " << meshName << " with size: {";
                    for(int j = 0; j < meshDims.size(); ++j)
                        debug4 << meshDims[j] << ", ";
                    debug4 << "}" << endl;

                    // Add a global variable too.
                    std::string globalVar = std::string("global/") + varname;
                    smd = new avtScalarMetaData(globalVar,
                        globalMesh, AVT_ZONECENT);
                    smd->hasUnits = fileObject->ReadStringAttribute(
                        varname, "units", smd->units);
                    smd->validVariable = nSpatialDims <= 3;
                    md->Add(smd);
                    varToDimensions[globalVar] = meshDims;
                } 
            }
        } // if nc_inc_var
    } // for nVars

    initialized = true;
}

// ****************************************************************************
// Method: CreateUCDSphere
//
// Purpose: 
//   Creates a UCD sphere with the given x,y,z sizes.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 5 12:19:33 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

static vtkUnstructuredGrid *
CreateUCDSphere(int X_RES, int Y_RES, int Z_RES)
{
const float earth_radius = 6378.f; // ~ Earth
const float atmosphere_thickness = 1000.f;
const float pole_factor = 0.996;

    if(Z_RES == 0)
        Z_RES = 1;

    int NSHELLS = Z_RES + 1;
    int npts = (X_RES * (Y_RES-1) + 2) * NSHELLS;
    int ncells = X_RES * Y_RES * Z_RES;
    vtkPoints *points = vtkPoints::New();
    points->SetNumberOfPoints(npts);
    float *fptr = (float *)points->GetVoidPointer(0);

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(points);
    ugrid->Allocate(ncells);

debug4 << "X_RES=" << X_RES << endl;
debug4 << "Y_RES=" << Y_RES << endl;
debug4 << "Z_RES=" << Z_RES << endl;
debug4 << "npts=" << npts << endl;
debug4 << "ncells=" << ncells << endl;

    const float rmin = earth_radius;
    const float rmax = rmin + atmosphere_thickness;
    int nnodes_per_shell = 0;
    for(int shell = 0; shell < NSHELLS; ++shell)
    {
        float ts = float(shell) / float(NSHELLS-1);
        float radius = (1.-ts)*rmin + ts*rmax;

        for(int ip = 0; ip < Y_RES+1; ++ip)
        {
            if(ip == 0)
            {
                *fptr++ = 0.;
                *fptr++ = -radius * pole_factor;
                *fptr++ = 0.;
                if(shell==0)
                    ++nnodes_per_shell;
            }
            else if(ip == Y_RES)
            {
                *fptr++ = 0.;
                *fptr++ = radius * pole_factor;
                *fptr++ = 0.;
                if(shell==0)
                    ++nnodes_per_shell;
            }
            else
            {
                float poleAngle = float(ip) / float(Y_RES) * M_PI;
                float sign = (poleAngle > M_PI_2) ? -1. : 1;
                float y = radius * cos(poleAngle) * -1;
                float yrad = radius * sin(poleAngle);

                for(int ri = 0; ri < X_RES; ++ri)
                {
                    float angle = float(ri) / float(X_RES) * -2. * M_PI;
                    float x = yrad * cos(angle);
                    float z = yrad * sin(angle);

                    *fptr++ = x;
                    *fptr++ = y * pole_factor;
                    *fptr++ = z;
                    if(shell==0)
                        ++nnodes_per_shell;
                }
            }
        }
    }
    vtkIdType verts[8];

    int i;
    for(int shell = 0; shell < Z_RES; ++shell)
    {
        int shell_offset = nnodes_per_shell * shell;
        int shell_offset2 = nnodes_per_shell * (shell+1);
        for(i = 0; i < X_RES; ++i)
        {
            if(i < X_RES-1)
            {
                verts[0] = 0 + shell_offset;
                verts[1] = i+2 + shell_offset;
                verts[2] = i+1 + shell_offset;

                verts[3] = 0   + shell_offset2;
                verts[4] = i+2 + shell_offset2;
                verts[5] = i+1 + shell_offset2;
            }
            else
            {
                verts[0] = 0+ shell_offset;
                verts[1] = 1+ shell_offset;
                verts[2] = i+1+ shell_offset;

                verts[3] = 0 + shell_offset2;
                verts[4] = 1 + shell_offset2;
                verts[5] = i+1 + shell_offset2;
            }

            if(Z_RES > 1)
                ugrid->InsertNextCell(VTK_WEDGE, 6, verts);
            else
                ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
        }
    }

    int row, nextrow;
    for(int shell = 0; shell < Z_RES; ++shell)
    {
        row = 1;
        nextrow = X_RES + 1;

        int shell_offset = nnodes_per_shell * shell;
        int shell_offset2 = nnodes_per_shell * (shell+1);
        for(int j = 0; j < Y_RES - 2; ++j)
        {
            for(i = 0; i < X_RES; ++i)
            {
                if(i < X_RES-1)
                {
                    verts[0] = row + i + shell_offset;
                    verts[1] = row + i + 1 + shell_offset;
                    verts[2] = nextrow + i + 1 + shell_offset;
                    verts[3] = nextrow + i + shell_offset;
    
                    verts[4] = row + i + shell_offset2;
                    verts[5] = row + i + 1 + shell_offset2;
                    verts[6] = nextrow + i + 1 + shell_offset2;
                    verts[7] = nextrow + i + shell_offset2;
                }
                else
                {
                    verts[0] = row + i + shell_offset;
                    verts[1] = row + shell_offset;
                    verts[2] = nextrow + shell_offset;
                    verts[3] = nextrow + i + shell_offset;

                    verts[4] = row + i + shell_offset2;
                    verts[5] = row + shell_offset2;
                    verts[6] = nextrow + shell_offset2;
                    verts[7] = nextrow + i + shell_offset2;
                }
                if(Z_RES > 1)
                    ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);
                else
                    ugrid->InsertNextCell(VTK_QUAD, 4, verts);
            }
    
            row += X_RES;
            nextrow += X_RES;
        }
    }

    int last = X_RES * (Y_RES-1) + 1;
    for(int shell = 0; shell < Z_RES; ++shell)
    {
        int shell_offset = nnodes_per_shell * shell;
        int shell_offset2 = nnodes_per_shell * (shell+1);
        for(i = 0; i < X_RES; ++i)
        {
            if(i < X_RES-1)
            {
                verts[0] = row + i + shell_offset;
                verts[1] = row + i + 1 + shell_offset;
                verts[2] = last + shell_offset;

                verts[3] = row + i + shell_offset2;
                verts[4] = row + i + 1 + shell_offset2;
                verts[5] = last + shell_offset2;
            }
            else
            {
                verts[0] = row + i + shell_offset;
                verts[1] = row + shell_offset;
                verts[2] = last + shell_offset;

                verts[3] = row + i + shell_offset2;
                verts[4] = row + shell_offset2;
                verts[5] = last + shell_offset2;
            }
            if(Z_RES > 1)
                ugrid->InsertNextCell(VTK_WEDGE, 6, verts);
            else
                ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
        }
    }

    return ugrid;
}

// ****************************************************************************
//  Method: avtCCSMReader::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 11 11:28:20 PDT 2007
//
//  Modifications:
//    Brad Whitlock, Thu Oct 29 11:04:53 PDT 2009
//    I added support for curvilinear grids.
//
// ****************************************************************************


vtkDataSet *
avtCCSMReader::GetMesh(int timestate, const char *var)
{
    const char *mName = "avtCCSMReader::GetMesh: ";
    debug4 << mName << "var=" << var << endl;
    vtkDataSet *retval = 0;
 
    // Populate the mesh names if we've not done so yet.
    if(!initialized)
        PopulateDatabaseMetaData(-1, 0);

    StringIntVectorMap::const_iterator mesh = meshNameToDimensions.find(var);
    if(mesh != meshNameToDimensions.end())
    {
        // Return the dimensions that we should care about.
        int dimCounts[3] = {1,1,1};
        int nValidDims = 0;
        int nnodes = 1;
        debug4 << mName << "meshDims={";
        for(int i = 0; i < mesh->second.size(); ++i)
        {
            debug4 << mesh->second[i] << ", ";
            if(mesh->second[i] != TIME_DIMENSION)
            {
                dimCounts[nValidDims++] = mesh->second[i];
                nnodes *= mesh->second[i];
            }
        }
        debug4 << endl;
        debug4 << mName << "nValidDims=" << nValidDims << ", dims={"
               << dimCounts[0] << ", " << dimCounts[1] << ", " << dimCounts[2]
               << "}" << endl;

        if(strncmp(var, "global/", 7) == 0)
        {
            debug4 << mName << "Creating global mesh" << endl;
            retval = CreateUCDSphere(dimCounts[0], dimCounts[1], dimCounts[2]);
        }
        else
        {
            // Let's try and read longxy latixy arrays and create a curvilinear mesh.
            float *xarray = 0, *yarray = 0;
            xarray = ReadArray("LONGXY");
            if(xarray == 0)
                xarray = ReadArray("longxy");
            yarray = ReadArray("LATIXY");
            if(yarray == 0)
                yarray = ReadArray("latixy");
            if(xarray != 0 && yarray != 0)
            {
                int dims[3] = {1,1,1};
                int ndims = nValidDims;
                dims[0] = dimCounts[0];
                dims[1] = dimCounts[1];
                dims[2] = dimCounts[2];
                debug4 << mName << "Creating sgrid " << endl;

                //
                // Create the vtkStructuredGrid and vtkPoints objects.
                //
                vtkStructuredGrid *sgrid  = vtkStructuredGrid::New(); 
                vtkPoints         *points = vtkPoints::New();
                sgrid->SetPoints(points);
                sgrid->SetDimensions(dims);
                points->Delete();
                points->SetNumberOfPoints(nnodes);

                //
                // Copy the coordinate values into the vtkPoints object.
                //
                float *pts = (float *) points->GetVoidPointer(0);
                float *xc = xarray;
                float *yc = yarray;
                if(ndims == 3)
                {
                    for(int k = 0; k < dims[2]; ++k)
                    {
                        xc = xarray;
                        yc = yarray;
                        for(int j = 0; j < dims[1]; ++j)
                        {
                            for(int i = 0; i < dims[0]; ++i)
                            {
                                *pts++ = *xc++;
                                *pts++ = *yc++;
                                *pts++ = k;
                            }
                        }
                    }
                }
                else if(ndims == 2)
                {
                    for(int j = 0; j < dims[1]; ++j)
                    {
                        for(int i = 0; i < dims[0]; ++i)
                        {
                            *pts++ = *xc++;
                            *pts++ = *yc++;
                            *pts++ = 0.;
                        }
                    }
                }

                retval = sgrid;
            }
            else
            {
                // We didn't get those arrays so create a rectilinear grid.
                delete [] xarray;
                delete [] yarray;
                debug4 << mName << "Creating rgrid " << endl;

                int   i, j;
                vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New(); 

                // Try and read the "lon" and "lat" arrays.
                xarray = ReadArray("longitude");
                if(xarray == 0)
                    xarray = ReadArray("lon");
                yarray = ReadArray("latitude");
                if(yarray == 0)
                    yarray = ReadArray("lat");
                if(xarray == 0 || yarray == 0)
                {
                     EXCEPTION1(InvalidVariableException, var);
                }

                float *zarray = 0;
                if(nValidDims == 3)
                {
                    zarray = new float[dimCounts[2]];
                    for(int i = 0; i < dimCounts[2]; ++i)
                        zarray[i] = i;
                }

                //
                // Populate the coordinates.  Put in 3D points with z=0 if
                // the mesh is 2D.
                //
                int            dims[3];
                vtkFloatArray *coords[3];
                float *coord_vals[3] = {xarray, yarray, zarray};
                for (i = 0 ; i < 3 ; i++)
                {
                    // Default number of components for an array is 1.
                    coords[i] = vtkFloatArray::New();

                    if (i < nValidDims)
                    {
                        dims[i] = dimCounts[i];
                        coords[i]->SetNumberOfTuples(dims[i]);
                        if(coord_vals[i] != 0)
                        {
                            for (j = 0 ; j < dims[i] ; j++)
                            {
                                double d = coord_vals[i][j];
                                coords[i]->SetComponent(j, 0, d);
                            }
                        }
                        else
                        {
                            for (j = 0 ; j < dims[i] ; j++)
                                coords[i]->SetComponent(j, 0, j);
                        }
                    }
                    else
                    {
                        dims[i] = 1;
                        coords[i]->SetNumberOfTuples(1);
                        coords[i]->SetComponent(0, 0, 0.);
                    }
                }
                rgrid->SetDimensions(dims);
                rgrid->SetXCoordinates(coords[0]);
                coords[0]->Delete();
                rgrid->SetYCoordinates(coords[1]);
                coords[1]->Delete();
                rgrid->SetZCoordinates(coords[2]);
                coords[2]->Delete();

                retval = rgrid;
            }

            delete [] xarray;
            delete [] yarray;
        }
    }
    else
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    return retval;
}

// ****************************************************************************
//  Method: avtCCSMReader::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 11 11:28:20 PDT 2007
//
//  Modifications:
//    Brad Whitlock, Thu Oct 29 11:00:44 PDT 2009
//    I made it use varToDimensions to get the variable size.
//
// ****************************************************************************

#define READVAR(VTKTYPE) \
        {\
            VTKTYPE *arr = VTKTYPE::New();\
            arr->SetNumberOfComponents(1);\
            arr->SetNumberOfTuples(nElems);\
            debug4 << "Allocated a " << \
                    #VTKTYPE \
                   << " of " << nElems << " elements" << endl; \
            if(fileObject->ReadVariableInto(realvar, t, dimStarts, dimCounts, \
               arr->GetVoidPointer(0)))\
            {\
                debug4 << mName << "Read " << nElems << " values" << endl; \
                retval = arr;\
            } \
            else\
            {\
                debug4 << mName << "Read failed!" << endl; \
                arr->Delete();\
            }\
        }

vtkDataArray *
avtCCSMReader::GetVar(int timestate, const char *var)
{
    const char *mName = "avtCCSMReader::GetVar: ";
    debug4 << mName << "var=" << var << endl;

    vtkDataArray *retval = 0;
    TypeEnum t = NO_TYPE;
    int ndims = 0;
    int *dims = 0;

    // Populate the mesh names if we've not done so yet.
    if(!initialized)
        PopulateDatabaseMetaData(-1, 0);

    // Skip past "global/" if it is prepended.
    const char *realvar = strstr(var, "/");
    if(realvar == 0)
        realvar = var;
    else
        realvar++;
    debug4 << mName << "realvar = " << realvar << endl;

    StringIntVectorMap::const_iterator minfo = varToDimensions.find(var);
    if(minfo == varToDimensions.end())
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    debug4 << mName << "Found variable dimensions." << endl;

    // Inq the variable to get its type.
    if(!fileObject->InqVariable(realvar, &t, &ndims, &dims))
        return 0;

    // Now, we have the mesh info, which describes the shape of the mesh
    // and from which we can create the size of the slice.
    int dimStarts[4] = {0,0,0,0};
    int dimCounts[4] = {1,1,1,1};
    int nElems = 1;

    debug4 << mName << "meshDims={";
    for(int i = 0; i < minfo->second.size(); ++i)
         debug4 << minfo->second[i] << ", ";
    debug4 << "}" << endl;

    int index = minfo->second.size()-1;
    for(int i = 0; i < minfo->second.size(); ++i, --index)
    {
        if(minfo->second[i] == TIME_DIMENSION)
        {
            dimStarts[index] = timestate;
            dimCounts[index] = 1;
        }
        else
        {
            dimStarts[index] = 0;
            dimCounts[index] = minfo->second[i];
            nElems *= dimCounts[index];
        }
    }

    debug4 << mName << "dimStarts = {"
           << dimStarts[0] << ", "
           << dimStarts[1] << ", "
           << dimStarts[2] << ", "
           << dimStarts[3] << "}\n";
    debug4 << mName << "dimCounts = {"
           << dimCounts[0] << ", "
           << dimCounts[1] << ", "
           << dimCounts[2] << ", "
           << dimCounts[3] << "}\n";

    if(t == CHARARRAY_TYPE || t == UCHARARRAY_TYPE)
        READVAR(vtkUnsignedCharArray)
    else if(t == SHORTARRAY_TYPE)
        READVAR(vtkShortArray)
    else if(t == INTEGERARRAY_TYPE)
        READVAR(vtkIntArray)
    else if(t == LONGARRAY_TYPE)
        READVAR(vtkLongArray)
    else if(t == FLOATARRAY_TYPE)
        READVAR(vtkFloatArray)
    else if(t == DOUBLEARRAY_TYPE)
        READVAR(vtkDoubleArray)
    else
    {
        debug4 << mName << "Unsupported type!" << endl;
    }

    return retval;
}

