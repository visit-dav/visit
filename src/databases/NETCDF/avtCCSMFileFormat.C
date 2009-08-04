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
//                            avtCCSMFileFormat.C                            //
// ************************************************************************* //

#include <avtCCSMFileFormat.h>

#include <string>

#include <vtkCellArray.h>
#include <vtkRectilinearGrid.h>
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
#include <avtMTSDFileFormatInterface.h>

#include <DebugStream.h>

#include <InvalidVariableException.h>
#include <InvalidDBTypeException.h>

#include <NETCDFFileObject.h>


using     std::string;

// ****************************************************************************
// Method: GetDimensionInfo
//
// Purpose: 
//   Gets the size of a NETCDF dimension.
//
// Arguments:
//   file : The NETCDF file object
//   dName : The name of the dimension to query.
//   size  : The size of the dimension.
//
// Returns:    True if the dimension was located, false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 5 12:03:15 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

static bool
GetDimensionInfo(NETCDFFileObject *file, const char *dName, size_t *size)
{
    int status, nDims, nVars, nGlobalAtts, unlimitedDimension;
    status = nc_inq(file->GetFileHandle(), &nDims, &nVars, &nGlobalAtts,
                    &unlimitedDimension);
    if(status != NC_NOERR)
    {
        file->HandleError(status);
        return false;
    }

    int i;
    for(i = 0; i < nDims; ++i)
    {
        char   dimName[NC_MAX_NAME+1];
        size_t dimSize;
        if((status = nc_inq_dim(file->GetFileHandle(), i, dimName,
           &dimSize)) == NC_NOERR)
        {
            debug5 << "Dimension " << i << ": " << dimName << ", " << dimSize << endl;
            if(strcmp(dimName, dName) == 0)
            {
                *size = dimSize;
                return true;
            }
        }
        else
        {
            file->HandleError(status);
        }
    }

    return false;
}

// ****************************************************************************
// Method: avtCCSMFileFormat::Identify
//
// Purpose: 
//   Identifies the file as CCSM.
//
// Arguments:
//   fileObject : The file object used to perform the query.
//
// Returns:    True if the file looks like CCSM, false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 5 12:12:45 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
avtCCSMFileFormat::Identify(NETCDFFileObject *fileObject)
{
    // Make sure it's the right type of file.
    bool isCCSM = false;
    size_t sz;
    int tmp = 0;
    if(GetDimensionInfo(fileObject, "time", &sz))
    {
        size_t latSize = 0, lonSize = 0;
        bool hasLongitude = GetDimensionInfo(fileObject, "lon", &lonSize) ||
                            GetDimensionInfo(fileObject, "longitude", &lonSize);
        bool hasLatitude = GetDimensionInfo(fileObject, "lat", &latSize) ||
                           GetDimensionInfo(fileObject, "latitude", &latSize);

        isCCSM = (sz > 1 && 
                  (hasLongitude && lonSize > 1) &&
                  (hasLatitude && latSize > 1));
    }
    return isCCSM;
}

// ****************************************************************************
//  Method: CCSMCommonPluginInfo::SetupCCSMDatabase
//
//  Purpose:
//      Sets up a CCSM database.
//
//  Arguments:
//      list    A list of file names.
//      nList   The number of timesteps in list.
//      nBlocks The number of blocks in the list.
//
//  Returns:    A CCSM database from list.
//
//  Programmer: whitlocb -- generated by xml2info
//  Creation:   Wed Jul 11 18:26:32 PST 2007
//
// ****************************************************************************
avtFileFormatInterface *
avtCCSMFileFormat::CreateInterface(NETCDFFileObject *f,
    const char *const *list, int nList, int nBlock)
{
    avtMTSDFileFormat **ffl = new avtMTSDFileFormat*[nList];
    for (int i = 0 ; i < nList ; i++)
    {
        if(f != 0)
        {
            ffl[i] = new avtCCSMFileFormat(list[i], f);
            f = 0;
        }
        else
            ffl[i] = new avtCCSMFileFormat(list[i]);
    }
    return new avtMTSDFileFormatInterface(ffl, nList);
}

// ****************************************************************************
//  Method: avtCCSM constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 11 11:28:20 PDT 2007
//
// ****************************************************************************

avtCCSMFileFormat::avtCCSMFileFormat(const char *filename, NETCDFFileObject *f)
    : avtMTSDFileFormat(&filename, 1)
{
    fileObject = f;
    initialized = false;
    dimSizes = 0;

    // Make sure it's the right type of file.
    size_t sz;
    if(!GetDimensionInfo(fileObject, "time", &sz))
    {
        EXCEPTION1(InvalidDBTypeException, filename);
    }
}

avtCCSMFileFormat::avtCCSMFileFormat(const char *filename)
    : avtMTSDFileFormat(&filename, 1)
{
    fileObject = new NETCDFFileObject(filename);
    initialized = false;
    dimSizes = 0;

    // Make sure it's the right type of file.
    size_t sz;
    if(!GetDimensionInfo(fileObject, "time", &sz))
    {
        EXCEPTION1(InvalidDBTypeException, filename);
    }
}

// ****************************************************************************
// Method: avtCCSMFileFormat::~avtCCSMFileFormat
//
// Purpose: 
//   Destructor for the avtCCSMFileFormat class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 5 12:17:36 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

avtCCSMFileFormat::~avtCCSMFileFormat()
{
    FreeUpResources();

    delete fileObject;
    delete dimSizes;
}

// ****************************************************************************
//  Method: avtCCSMFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 11 11:28:20 PDT 2007
//
// ****************************************************************************

void
avtCCSMFileFormat::FreeUpResources()
{
    debug4 << "avtCCSMFileFormat::FreeUpResources" << endl;
    fileObject->Close();
}

// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 11 11:28:20 PDT 2007
//
// ****************************************************************************

int
avtCCSMFileFormat::GetNTimesteps(void)
{
    size_t sz = 0;
    if(!GetDimensionInfo(fileObject, "time", &sz))
        sz = 1;

    return (int)sz;
}

void
avtCCSMFileFormat::GetCycles(std::vector<int> &cycles)
{
    int nts = GetNTimesteps();
    for(int i = 0; i < nts; ++i)
        cycles.push_back(i);
}

void
avtCCSMFileFormat::GetTimes(std::vector<double> &times)
{
    // Read the times from the DB...
    int nts = GetNTimesteps();
    for(int i = 0; i < nts; ++i)
        times.push_back((double)i);
}

// ****************************************************************************
//  Method: avtCCSMFileFormat::PopulateDatabaseMetaData
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
//
//    Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//    Changed interface to DebugStream to obtain current debug level.
// ****************************************************************************

void
avtCCSMFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    const char *mName = "avtCCSMFileFormat::PopulateDatabaseMetaData: ";
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

    if(md != 0)
         md->SetDatabaseComment("Read as CCSM data");

    // Get the size of all of the dimensions in the file.
    size_t *dimSizes = new size_t[nDims];
    int timedim = -1;
    int i;
    for(i = 0; i < nDims; ++i)
    {
        char   dimName[NC_MAX_NAME+1];
        size_t dimSize;
        if((status = nc_inq_dim(fileObject->GetFileHandle(), i, dimName,
           &dimSize)) == NC_NOERR)
        {
            if(strcmp(dimName, "time") == 0)
                timedim = i;

            dimSizes[i] = dimSize;
        }
        else
        {
            dimSizes[i] = 1;
            fileObject->HandleError(status);
        }
    }

    // Iterate over the variables and create a list of meshes names and add
    // the variable to the metadata.
    meshNames.clear();
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

            int nDims = 0;
            int maxDim = dimSizes[vardims[0]];
            int maxDimIndex = 0;
            bool hasTimeDimension = false;
            for(int dim = 0; dim < varndims; ++dim)
            {
                int d = dimSizes[vardims[dim]];
                if(d > 1)
                    ++nDims;
                if(d > maxDim)
                {
                    maxDim = d;
                    maxDimIndex = dim;
                }
                if(vardims[dim] == timedim)
                    hasTimeDimension = true;
            }

            // The variable did not have a time dimension so skip it.
            if(!hasTimeDimension)
            {
                debug4 << mName << "Rejecting " << varname
                       << " because it has no time dimension." << endl;
                continue;
            }

            if(nDims < 3)
            {
                debug4 << mName << "Rejecting " << varname
                       << " because it can't yield 2D data over time." << endl;
                continue;
            }

#define TIME_DIMENSION -1

            char tmp[100];
            std::string meshName("mesh");
            intVector meshDims;
            int j, elems = 1;
            for(j = 0; j < varndims; ++j)
            {
                int d;
                if(vardims[j] != timedim)
                {
                    d = dimSizes[vardims[j]];
                    elems *= d;
                }
                else
                    d = TIME_DIMENSION;
                
                meshDims.push_back(d);
            }

            // Remove all of the trailing 1's in the meshDims array.
            while(meshDims.size() > 0 &&
                  meshDims[meshDims.size()-1] == 1)
            {
                meshDims.pop_back();
            }

            // Make sure it's not 1 or 0 in some of its dimensions
            if(meshDims.size() >= 2 && elems != maxDim && elems > 0)
            {
                // Come up with the mesh name that we'll use for this
                // variable.
                int nValidDims = 0;
                for(j = meshDims.size()-1; j >= 0; --j)
                {
                    if(meshDims[j] != TIME_DIMENSION)
                    {
                        if(nValidDims != 0)
                            SNPRINTF(tmp, 100, "x%d", meshDims[j]);
                        else
                            SNPRINTF(tmp, 100, "%d", meshDims[j]);
                        ++nValidDims;

                        meshName += tmp;
                    }
                }
                string globalMesh = string("global/") + meshName;
 
                // Add the name of the mesh to the list of meshes.
                if(meshNames.find(meshName) == meshNames.end())
                {
                    meshNames[meshName] = meshDims;

                    if(md != 0)
                    {
                        avtMeshMetaData *mmd = new avtMeshMetaData(meshName, 
                            1, 1, 1, 0, nValidDims, nValidDims,
                            AVT_RECTILINEAR_MESH);
                        mmd->validVariable = nValidDims <= 3;
                        md->Add(mmd);
                    }

                    // Create a global mesh too.
                    meshNames[globalMesh] = meshDims;
                    if(md != 0)
                    {
                        int td = nValidDims;
                        int sd = (nValidDims < 3) ? (nValidDims+1) : 3;
                        avtMeshMetaData *mmd = new avtMeshMetaData(globalMesh, 
                            1, 1, 1, 0, sd, td,
                            AVT_UNSTRUCTURED_MESH);
                        mmd->validVariable = nValidDims <= 3;
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
                    smd->validVariable = nValidDims <= 3;
                    md->Add(smd);

                    // So we can keep track of the mesh to var mapping.
                    meshToVar[varname] = meshName;

                    debug4 << "Variable " << varname << " on mesh " << meshName << " with size: {";
                    for(j = 0; j < meshDims.size(); ++j)
                        debug4 << ", " << meshDims[j];
                    debug4 << "}" << endl;

                    // Add a global variable too.
                    string globalVar = string("global/") + varname;
                    smd = new avtScalarMetaData(globalVar,
                        globalMesh, AVT_ZONECENT);
                    smd->hasUnits = fileObject->ReadStringAttribute(
                        varname, "units", smd->units);
                    smd->validVariable = nValidDims <= 3;
                    md->Add(smd);
                    meshToVar[globalVar] = globalMesh;
                } 
            }
        } // if nc_inc_var
    } // for nVars

    initialized = true;
}

// ****************************************************************************
// Method: avtCCSMFileFormat::ReadArray
//
// Purpose: 
//   Reads a variable into a float array.
//
// Arguments:
//   varname : The name of the variable to read.
//
// Returns:    A float array or NULL.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 5 12:18:25 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

float *
avtCCSMFileFormat::ReadArray(const char *varname)
{
    const char *mName = "avtCCSMFileFormat::ReadArray: ";
    TypeEnum t;
    int ndims, *dims = 0;
    float *arr = 0;

    if(fileObject->InqVariable(varname, &t, &ndims, &dims))
    {
        debug4 << mName << "Got information for " << varname << endl;

        // Determine the size of the variable.
        int nelems = 1;
        for(int i = 0; i < ndims; ++i)
            nelems *= dims[i];
        arr = new float[nelems];
        delete [] dims;

        // Read the variable.
        if(!fileObject->ReadVariableIntoAsFloat(varname, arr))
        {
            debug4 << mName << "Could not read variable as float." << endl;
            delete [] arr;
            arr = 0;
        }
        else
            debug4 << mName << "Variable " << varname << " was read." << endl;
    }

    return arr;
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
//  Method: avtCCSMFileFormat::GetMesh
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
// ****************************************************************************


vtkDataSet *
avtCCSMFileFormat::GetMesh(int timestate, const char *var)
{
    debug4 << "avtCCSMFileFormat::GetMesh: var=" << var << endl;
    vtkDataSet *retval = 0;
 
    // Populate the mesh names if we've not done so yet.
    if(!initialized)
        PopulateDatabaseMetaData(0, -1);

    MeshNameMap::const_iterator mesh = meshNames.find(var);
    if(mesh != meshNames.end())
    {
        // Return the dimensions that we should care about.
        int dimCounts[3] = {1,1,1};
        int nValidDims = 0;
        int nnodes = 1;
        for(int i = mesh->second.size()-1; i >= 0; --i)
        {
            if(mesh->second[i] != TIME_DIMENSION)
            {
                dimCounts[nValidDims++] = mesh->second[i];
                nnodes *= mesh->second[i];
            }
        }

        if(strncmp(var, "global/", 7) == 0)
        {
            retval = CreateUCDSphere(dimCounts[0], dimCounts[1], dimCounts[2]);
        }
        else
        {
            int   i, j;
            vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New(); 

            // Try and read the "lon" and "lat" arrays.
            float *xc = ReadArray("lon");
            if(xc == 0)
                xc = ReadArray("longitude");
            float *yc = ReadArray("lat");
            if(yc == 0)
                yc = ReadArray("latitude");
            if(xc == 0 || yc == 0)
            {
                 EXCEPTION1(InvalidVariableException, var);
            }

            //
            // Populate the coordinates.  Put in 3D points with z=0 if
            // the mesh is 2D.
            //
            int            dims[3];
            vtkFloatArray *coords[3];
            float *coord_vals[3] = {xc, yc, 0};

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
    }
    else
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    return retval;
}

// ****************************************************************************
//  Method: avtCCSMFileFormat::GetVar
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
avtCCSMFileFormat::GetVar(int timestate, const char *var)
{
    const char *mName = "avtCCSMFileFormat::GetVar: ";
    debug4 << mName << "var=" << var << endl;

    vtkDataArray *retval = 0;
    TypeEnum t = NO_TYPE;
    int ndims = 0;
    int *dims = 0;

    // Populate the mesh names if we've not done so yet.
    if(!initialized)
        PopulateDatabaseMetaData(0, -1);

    // Skip past "global/" if it is prepended.
    const char *realvar = strstr(var, "/");
    if(realvar == 0)
        realvar = var;
    else
        realvar++;
debug4 << "realvar = " << realvar << endl;

    MeshToVarMap::const_iterator pos = meshToVar.find(var);
    if(pos == meshToVar.end())
    {
        EXCEPTION1(InvalidVariableException, var);
    }
    debug4 << mName << "Variable " << var << " is on mesh "
           << pos->second << endl;

    MeshNameMap::const_iterator minfo = meshNames.find(pos->second);
    if(minfo == meshNames.end())
    {
        EXCEPTION1(InvalidVariableException, var);
    }
    debug4 << mName << "Found mesh info." << endl;

    // Inq the variable to get its type.
    if(!fileObject->InqVariable(realvar, &t, &ndims, &dims))
        return 0;

    // Now, we have the mesh info, which describes the shape of the mesh
    // and from which we can create the size of the slice.
    int dimStarts[4] = {0,0,0,0};
    int dimCounts[4] = {1,1,1,1};
    int nElems = 1;
    for(int i = 0; i < minfo->second.size(); ++i)
    {
        if(minfo->second[i] == TIME_DIMENSION)
        {
            dimStarts[i] = timestate;
            dimCounts[i] = 1;
        }
        else
        {
            dimStarts[i] = 0;
            dimCounts[i] = minfo->second[i];
            nElems *= dimCounts[i];
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


// ****************************************************************************
//  Method: avtCCSMFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
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
// ****************************************************************************

vtkDataArray *
avtCCSMFileFormat::GetVectorVar(int timestate, const char *varname)
{
    EXCEPTION1(InvalidVariableException, varname);
}
