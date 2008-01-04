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
//                            avtVis5DFileFormat.C                           //
// ************************************************************************* //

#include <avtVis5DFileFormat.h>

#include <string>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>

#include <avtDatabaseMetaData.h>

#include <InvalidVariableException.h>
#include <DebugStream.h>

// Include projections file
#include <proj.C>


// ****************************************************************************
//  Method: avtVis5D constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jul 14 14:07:16 PST 2006
//
// Modifications:
//   
// ****************************************************************************

avtVis5DFileFormat::avtVis5DFileFormat(const char *filename)
    : avtMTSDFileFormat(&filename, 1)
{
    v5dfile = 0;
}

// ****************************************************************************
// Method: avtVis5DFileFormat::~avtVis5DFileFormat
//
// Purpose: 
//   Destructor for the avtVis5DFileFormat class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 14 16:50:40 PST 2006
//
// Modifications:
//   
// ****************************************************************************

avtVis5DFileFormat::~avtVis5DFileFormat()
{
    if(v5dfile != 0)
    {
        v5dCloseFile(v5dfile);
    }
}



// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jul 14 14:07:16 PST 2006
//
// Modifications:
//   
// ****************************************************************************

int
avtVis5DFileFormat::GetNTimesteps(void)
{
    Initialize();
    
    return (v5dfile != 0) ? v5dfile->NumTimes : 0;
}

// ****************************************************************************
// Method: avtVis5DFileFormat::GetCycles
//
// Purpose: 
//   Gets the cycles.
//
// Arguments:
//   cycles : The cycles vector. We use it to store the observation times.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 14 16:49:33 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtVis5DFileFormat::GetCycles(std::vector<int> &cycles)
{
    Initialize();

    if(v5dfile != 0)
    {
//
// TODO: Vis5D has dates and times that could be returned. However,
//       those times would fit better into a scheme where we could
//       return the values as date:time.
//
        debug4 << "avtVis5DFileFormat::GetCycles: {";
        for(int i = 0; i < v5dfile->NumTimes; ++i)
        {
            int yyddd  = v5dYYDDDtoDays(v5dfile->DateStamp[i]);
            int hhmmss = v5dHHMMSStoSeconds(v5dfile->TimeStamp[i]);
            debug4 << ", " << yyddd;
            cycles.push_back(yyddd);
        }
        debug4 << "}" << endl;
    }
}

// ****************************************************************************
//  Method: avtVis5DFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jul 14 14:07:16 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtVis5DFileFormat::FreeUpResources(void)
{
    if(v5dfile != 0)
    {
        v5dCloseFile(v5dfile);
        v5dfile = 0;
    }
}

// ****************************************************************************
// Method: avtVis5DFileFormat::Initialize
//
// Purpose: 
//   Opens the Vis5D file and reads its structure.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 14 16:49:08 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtVis5DFileFormat::Initialize()
{
    if(v5dfile == 0)
    {
        v5dfile = v5dOpenFile(filenames[0], NULL);
    }
}

// ****************************************************************************
//  Method: avtVis5DFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jul 14 14:07:16 PST 2006
//
//  Modifications:
//
// ****************************************************************************

void
avtVis5DFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    Initialize();

    // Don't alphabetize the variables in the client menus.
    md->SetMustAlphabetizeVariables(false);

    if(v5dfile != 0)
    {
#if 0
        // Print the structure of the file.
        v5dPrintStruct(v5dfile);
#endif

        //
        // Add the mesh
        //
        std::string meshName("grid");
        avtMeshType mt;
        if(v5dfile->Projection == 0 || v5dfile->Projection == 1)
            mt = AVT_RECTILINEAR_MESH;
        else
            mt = AVT_CURVILINEAR_MESH;
        avtMeshMetaData *mmd = new avtMeshMetaData(meshName,
            1, 1, 1, 0, 3, 3,
            mt);

        if(v5dfile->Projection == 1)
        {
            mmd->xUnits = "degrees";
            mmd->yUnits = "degrees";

            mmd->xLabel = "W Longitude";
            mmd->yLabel = "N Latitude";
        }
        else if(v5dfile->Projection != 0)
        {
            mmd->xUnits = "degrees";
            mmd->yUnits = "degrees";

            mmd->xLabel = "Longitude";
            mmd->yLabel = "Latitude";
        }

        mmd->zLabel = "Levels";
        if(v5dfile->VerticalSystem == 1 || v5dfile->VerticalSystem == 2)
            mmd->zUnits = "km";
        else if(v5dfile->VerticalSystem == 3)
            mmd->zUnits = "mb";
        md->Add(mmd);

        //
        // Add the scalars
        //
        for(int i = 0; i < v5dfile->NumVars; ++i)
        {
            char varName[10], units[20], *cptr = 0;
            strcpy(varName, v5dfile->VarName[i]);
            strcpy(units, v5dfile->Units[i]);
            for(cptr = varName + 8; cptr >= varName; --cptr)
            {
                if(*cptr == ' ')
                    *cptr = '\0';
                else
                    break;
            }
            for(cptr = units + 18; cptr >= varName; --cptr)
            {
                if(*cptr == ' ')
                    *cptr = '\0';
                else
                    break;
            }
            if(varName[0] != '\0')
            {
                avtScalarMetaData *smd = new avtScalarMetaData(
                    varName, meshName, AVT_NODECENT);
                smd->hasUnits = units[0] != '\0';
                if(smd->hasUnits)
                    smd->units = std::string(units);

                smd->hasDataExtents = true;
                smd->minDataExtents = v5dfile->MinVal[i];
                smd->maxDataExtents = v5dfile->MaxVal[i];

                md->Add(smd);
            }
        }
    }
}


// ****************************************************************************
// Method: avtVis5DFileFormat::CreateCurvilinearMesh
//
// Purpose: 
//   Returns a curvilinear mesh using Vis5D's transforms to create the
//   mesh coordinates.
//
// Returns:    A new mesh or 0.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 18 15:46:55 PST 2006
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
avtVis5DFileFormat::CreateCurvilinearMesh()
{
    // Set up a projection context so we can use Vis5D's code to convert
    // from rcl grid coordinates to lat,lon,height coordinates that we
    // can feed to VisIt.
    ProjectionContext ctx;
    memset(&ctx, 0, sizeof(ProjectionContext));
    setup_ctx_projection(&ctx, v5dfile->Projection, v5dfile->ProjArgs,
        v5dfile->Nr, v5dfile->Nc);

    // Set up some coordinates in r,c,l form.
    int nPts = v5dfile->Nr * v5dfile->Nc;
    float *rc = new float[nPts];
    float *cc = new float[nPts];
    for(int j = 0; j < v5dfile->Nr; ++j)
    {
        int jNc = j * v5dfile->Nc;
        for(int i = 0; i < v5dfile->Nc; ++i)
        {
            int index = jNc + i;
            rc[index] = j;
            cc[index] = i;
        }
    }

    //
    // Now that we have arrays in r,c form, convert them to lat,lon values.
    //
    float *lat = new float[nPts];
    float *lon = new float[nPts];
    grid_to_geo(&ctx, nPts, rc, cc, lat, lon);
    delete [] rc;
    delete [] cc;

    //
    // Calculate the Z coordinates.
    //
    float *height = new float[v5dfile->Nl[0]];
    if(v5dfile->VerticalSystem == 2 ||
       v5dfile->VerticalSystem == 3)
    {
        for(int i = 0; i < v5dfile->Nl[0]; ++i)
            height[i] = v5dfile->VertArgs[i];
    }
    else
    {
        float z = v5dfile->VertArgs[0];
        for(int i = 0; i < v5dfile->Nl[0]; ++i)
        {
            height[i] = z;
            z += v5dfile->VertArgs[1];
        }
    }

    int dims[3];
    dims[0] = v5dfile->Nc;
    dims[1] = v5dfile->Nr;
    dims[2] = v5dfile->Nl[0];
    int nnodes = nPts * v5dfile->Nl[0];

    // Now create a VTK structured mesh.
    vtkStructuredGrid    *sgrid  = vtkStructuredGrid::New(); 
    vtkPoints            *points = vtkPoints::New();
    sgrid->SetPoints(points);
    points->Delete();
    sgrid->SetDimensions(dims);
    points->SetNumberOfPoints(nnodes);
    
    float *tmp = (float *)points->GetVoidPointer(0);
    for(int k = 0; k < dims[2]; ++k)
    {
        int index = 0;
        for(int j = 0; j < dims[1]; ++j)
        {
            for(int i = 0; i < dims[0]; ++i, ++index)
            {
                *tmp++ = lon[index];
                *tmp++ = lat[index];
                *tmp++ = height[k];
            }
        }
    }

    delete [] lat;
    delete [] lon;
    delete [] height;

    return sgrid;   
}

// ****************************************************************************
//  Method: avtVis5DFileFormat::GetMesh
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
//  Creation:   Fri Jul 14 14:07:16 PST 2006
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtVis5DFileFormat::GetMesh(int timestate, const char *meshname)
{
    // Make sure the file is open.
    Initialize();
    if(v5dfile == 0)
    {
        EXCEPTION1(InvalidVariableException, meshname);
    }

    float mins[2] = {0,0}, maxs[2] = {0,0};
    if(v5dfile->Projection == 0)
    {
        // Rectilinear grid, generic units

        // X extents
        mins[0] = v5dfile->ProjArgs[1];
        maxs[0] = v5dfile->ProjArgs[1] + v5dfile->ProjArgs[3]*v5dfile->Nc;

        // Y extents
        mins[1] = v5dfile->ProjArgs[0];
        maxs[1] = v5dfile->ProjArgs[0] + v5dfile->ProjArgs[2]*v5dfile->Nr;
    }
    else if(v5dfile->Projection == 1)
    {
        // Cylindrical Equidistant projection

        // X extents
        mins[0] = v5dfile->ProjArgs[1] - v5dfile->ProjArgs[3]*v5dfile->Nc;
        maxs[0] = v5dfile->ProjArgs[1];

        // Note that the globe coordinates go from large#->small#. We have
        // them reversed so the mesh will go the right way in XYZ space.

        // Y extents
        mins[1] = v5dfile->ProjArgs[0] - v5dfile->ProjArgs[2]*v5dfile->Nr;
        maxs[1] = v5dfile->ProjArgs[0];
    }
    else if((v5dfile->Projection >= 2 &&
             v5dfile->Projection <= 5) ||
             v5dfile->Projection == -1)
    {
        // curvilinear formats (they may have complex globe projections).
        return CreateCurvilinearMesh();
    }
    else
    {
        // Bad projection
        EXCEPTION1(InvalidVariableException, meshname);
    }

    //
    // Populate the X, Y coordinates.
    //
    int i, j, dims[3];
    dims[0] = v5dfile->Nc;
    dims[1] = v5dfile->Nr;
    dims[2] = v5dfile->Nl[0];
    vtkFloatArray *coords[3] = {0,0,0};
    for (i = 0 ; i < 2 ; i++)
    {
        // Default number of components for an array is 1.
        coords[i] = vtkFloatArray::New();
        coords[i]->SetNumberOfTuples(dims[i]);
        for (j = 0 ; j < dims[i] ; j++)
        {
            float t = float(j) / float(dims[i]-1);
            float value = (1.f-t)*mins[i] + t*maxs[i];
            coords[i]->SetComponent(j, 0, value);
        }
    }

    //
    // Populate the Z coordinates.
    //
    coords[2] = vtkFloatArray::New();
    coords[2]->SetNumberOfTuples(v5dfile->Nl[0]);
    if(v5dfile->VerticalSystem == 2 ||
       v5dfile->VerticalSystem == 3)
    {
        for(i = 0; i < v5dfile->Nl[0]; ++i)
            coords[2]->SetComponent(i, 0, v5dfile->VertArgs[i]);
    }
    else
    {
        float z = v5dfile->VertArgs[0];
        for(i = 0; i < v5dfile->Nl[0]; ++i)
        {
            coords[2]->SetComponent(i, 0, z);
            z += v5dfile->VertArgs[1];
        }
    }

    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New(); 
    rgrid->SetDimensions(dims);
    rgrid->SetXCoordinates(coords[0]);
    coords[0]->Delete();
    rgrid->SetYCoordinates(coords[1]);
    coords[1]->Delete();
    rgrid->SetZCoordinates(coords[2]);
    coords[2]->Delete();

    return rgrid;
}


// ****************************************************************************
//  Method: avtVis5DFileFormat::GetVar
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
//  Creation:   Fri Jul 14 14:07:16 PST 2006
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtVis5DFileFormat::GetVar(int timestate, const char *varname)
{
    const char *mName = "avtVis5DFileFormat::GetVar: ";
    // Make sure the file is open.
    Initialize();
    if(v5dfile == 0)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    // Figure out the index of the variable.
    int index = -1;
    for(int i = 0; i < v5dfile->NumVars; ++i)
    {
        char tmp[10];
        strcpy(tmp, v5dfile->VarName[i]);
        for(char *cptr = tmp + 8; cptr >= tmp; --cptr)
        {
            if(*cptr == ' ')
                *cptr = '\0';
            else
                break;
        }
        if(strcmp(tmp, varname) == 0)
        {
            index = i;
            break;
        }
    }

    if(index == -1)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    // Allocate a data array.
    vtkFloatArray *rv = vtkFloatArray::New();
    int nValues = v5dSizeofGrid(v5dfile, timestate, index);
    rv->SetNumberOfTuples(nValues);
    debug4 << mName << "Size of variable: " << nValues;

    // Read data into memory.
    float *fdata = new float[nValues];
    if(v5dReadGrid(v5dfile, timestate, index, fdata) == 0)
    {
        rv->Delete();
        delete [] fdata;
        EXCEPTION1(InvalidVariableException, varname);
    }

    // Reorder the data into x,y,z order.
    float *dest = (float*)rv->GetVoidPointer(0);
    int NZ = v5dfile->Nl[index];
    int NY = v5dfile->Nr;
    int NX = v5dfile->Nc;
    for(int k = 0; k < NZ; ++k)
    {
        for(int j = 0; j < NY; ++j)
        {
            for(int i = 0; i < NX; ++i)
            {
                *dest++ = fdata[k*NX*NY + i*NY + (NY-1-j)];
            }
        }
    }
    delete [] fdata;

    return rv;
}


// ****************************************************************************
//  Method: avtVis5DFileFormat::GetVectorVar
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
//  Creation:   Fri Jul 14 14:07:16 PST 2006
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtVis5DFileFormat::GetVectorVar(int timestate, const char *varname)
{
    return 0;
}
