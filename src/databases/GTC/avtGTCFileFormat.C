/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                            avtGTCFileFormat.C                             //
// ************************************************************************* //

#include <avtGTCFileFormat.h>

#include <string>
#include <snprintf.h>

#include <vtkCellType.h>
#include <vtkFloatArray.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>

#include <InvalidDBTypeException.h>
#include <InvalidVariableException.h>

#include <visit-hdf5.h>
#include <hdf5.h>

#include <DebugStream.h>

// ****************************************************************************
//  Method: avtGTC constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Apr 27 13:18:22 PST 2006
//
// ****************************************************************************

avtGTCFileFormat::avtGTCFileFormat(const char *filename)
    : avtSTSDFileFormat(filename)
{
    fileId = -1;
    nVars = 0;
    initialized = false;

    // Make sure that the file is in fact GTC.
    Initialize(true, true);
}

// ****************************************************************************
// Method: avtGTCFileFormat::~avtGTCFileFormat
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 27 14:21:08 PST 2006
//
// Modifications:
//   
// ****************************************************************************

avtGTCFileFormat::~avtGTCFileFormat()
{
    if(fileId >= 0)
        H5Fclose(fileId);
}

// ****************************************************************************
//  Method: avtGTCFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Apr 27 13:18:22 PST 2006
//
// ****************************************************************************

void
avtGTCFileFormat::FreeUpResources(void)
{
    if(fileId >= 0)
    {
        H5Fclose(fileId);
        fileId = -1;
    }

    initialized = false;
}

// ****************************************************************************
// Method: avtGTCFileFormat::Initialize
//
// Purpose: 
//   Opens the file and makes some other checks.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 27 13:48:11 PST 2006
//
// Modifications:
//   
// ****************************************************************************

bool
avtGTCFileFormat::Initialize(bool throwE, bool extraChecks)
{
    const char *mName = "avtGTCFileFormat::Initialize: ";

    if(initialized)
        return true;

    // Turn off error message printing.
    H5Eset_auto(0,0);
    debug4 << mName << "Opening " << filename << endl;
    bool err = false;
    fileId = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (fileId < 0)
    {
        debug4 << mName << "Could not open " << filename << endl;
        if(throwE)
        {
            EXCEPTION1(InvalidDBTypeException, "Cannot be a GTC file "
                "since it is not even an HDF5 file.");
        }
        else
            err = true;
    }

    if(!err && extraChecks)
    {
        int val, ndims;
        hid_t sid;
        hsize_t dims[3];

        debug4 << mName << "Opening particle_data" << endl;
        hid_t handle = H5Dopen(fileId, "particle_data");
        if (handle < 0)
        {
            debug4 << mName << "Could not open particle_data" << endl;
            H5Fclose(fileId);
            if(throwE)
            {
                EXCEPTION1(InvalidDBTypeException, "Cannot be a GTC file, "
                    "since it is does not contain the dataset \"particle_data\"");
            }
            else
            {
                err = true;
                goto bail;
            }
        }

        // Get the variable's size.
        debug4 << mName << "Determining number of dimensions" << endl;
        sid = H5Dget_space(handle);
        ndims = H5Sget_simple_extent_ndims(sid);
        if(ndims < 0 || ndims > 2)
        {
            debug4 << mName << "Could not determine number of dimensions" << endl;
            H5Sclose(sid);
            H5Dclose(handle);
            H5Fclose(fileId);
            if(throwE)
            {
                EXCEPTION1(InvalidDBTypeException, "The GTC file has an "
                    "invalid number of dimensions");
            }
            else
            {
                err = true;
                goto bail;
            }
        }

        debug4 << mName << "Determining variable size" << endl;
        val = H5Sget_simple_extent_dims(sid, dims, NULL);
        nVars = dims[1];
        if(val < 0 || nVars < 3)
        {
            debug4 << mName << "Could not determine variable size" << endl;
            H5Sclose(sid);
            H5Dclose(handle);
            H5Fclose(fileId);
            if(throwE)
            {
                EXCEPTION1(InvalidDBTypeException, "The GTC file has an "
                    "insufficient number of coordinates");
            }
            else
            {
                err = true;
                goto bail;
            }
        }
        else
        {
            debug4 << mName << "variable size (" << dims[0] << ", "
                   << dims[1] << ")" << endl;
        }

bail:   ; // Skip the other stuff by jumping here
    }

    initialized = !err;

    return initialized;
}

// ****************************************************************************
//  Method: avtGTCFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Apr 27 13:18:22 PST 2006
//
// ****************************************************************************

void
avtGTCFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    // Add a point mesh
    std::string meshname = "particles";
    avtMeshMetaData *mmd = new avtMeshMetaData;
    mmd->name = meshname;
    mmd->spatialDimension = 3;
    mmd->topologicalDimension = 0;
    mmd->meshType = AVT_POINT_MESH;
    md->Add(mmd);

    // Add scalar variables.
    char varname[100];
    for(int i = 0; i < nVars; ++i)
    {
        SNPRINTF(varname, 100, "var%d", i);
        AddScalarVarToMetaData(md, varname, meshname, AVT_NODECENT);
    }
}


// ****************************************************************************
//  Method: avtGTCFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Notes:      The mesh data is weird. It seems that we can read all the
//              data and then read it like: x,y,z,v0,v1,v3,v3 but when trying
//              to read just the x,y,z's as a hyperslab, it gave a really
//              weird looking mesh. I believe the results from this approach.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Apr 27 13:18:22 PST 2006
//
// ****************************************************************************

vtkDataSet *
avtGTCFileFormat::GetMesh(const char *meshname)
{
    const char *mName = "avtGTCFileFormat::GetMesh: ";
    Initialize(false, false);

    debug4 << mName << "Opening particle_data" << endl;
    hid_t handle = H5Dopen(fileId, "particle_data");
    if (handle < 0)
    {
        EXCEPTION1(InvalidVariableException, meshname);
    }

    // Get the data space and the dimensions.
    hid_t dataspace = H5Dget_space(handle);
    hsize_t dims[2];
    H5Sget_simple_extent_dims(dataspace, dims, NULL);
    debug4 << mName << "particle_data dims(" << dims[0] << ", "
           << dims[1] << ")" << endl;

    // Read all the data
    int i, nPoints = int(dims[0]);
    float *xyz = new float[dims[0] * dims[1]];
    H5Sselect_all(dataspace);
    H5Dread(handle, H5T_NATIVE_FLOAT, H5S_ALL, dataspace, H5P_DEFAULT, xyz);
    H5Sclose(dataspace);
    H5Dclose(handle);

    debug4 << "Creating VTK points with " << nPoints << " points" << endl;
    vtkPoints *points  = vtkPoints::New();
    points->SetNumberOfPoints(nPoints);
    float *pts = (float *) points->GetVoidPointer(0);
    float *pdata = xyz;
    for(i = 0; i < nPoints; ++i)
    {
        *pts++ = pdata[0];
        *pts++ = pdata[1];
        *pts++ = pdata[2];
        pdata += dims[1];
    }
    delete [] xyz;

    // Create the point mesh
    debug4 << "Creating point mesh..." << endl;
    vtkUnstructuredGrid  *ugrid = vtkUnstructuredGrid::New(); 
    ugrid->SetPoints(points);
    ugrid->Allocate(nPoints);
    vtkIdType onevertex;
    for(i = 0; i < nPoints; ++i)
    {
        onevertex = i;
        ugrid->InsertNextCell(VTK_VERTEX, 1, &onevertex);
    }

    points->Delete();

    return ugrid;
}


// ****************************************************************************
//  Method: avtGTCFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Apr 27 13:18:22 PST 2006
//
// ****************************************************************************

vtkDataArray *
avtGTCFileFormat::GetVar(const char *varname)
{
    const char *mName = "avtGTCFileFormat::GetVar: ";
    Initialize(false, false);

    // Determine the variable index from the varname
    int var;
    if(sscanf(varname, "var%d", &var) != 1)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }
    debug4 << mName << "We want var " << var << " for "
           << varname << endl;

    debug4 << mName << "Opening particle_data" << endl;
    hid_t handle = H5Dopen(fileId, "particle_data");
    if (handle < 0)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    // Get the data space and the dimensions.
    hid_t dataspace = H5Dget_space(handle);
#if HDF5_VERSION_GE(1,6,4)
    hsize_t start[3];
#else
    hssize_t start[3];
#endif
    hsize_t dims[3], count[3];
    H5Sget_simple_extent_dims(dataspace, dims, NULL);
    debug4 << mName << "particle_data dims(" << dims[0] << ", "
           << dims[1] << ")" << endl;

    // Select the var'th row of data.
    start[0] = 0; start[1] = var;
    count[0] = dims[0]; count[1] = 1;
    H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, start, NULL, count, NULL);

    // Create a data space for the block of memory that we'll read the
    // data into.
    hsize_t dimsm[1] = {dims[0]};
    hid_t memspace = H5Screate_simple(1, dimsm, NULL);
    H5Sselect_all(memspace);

    // Read the data directly into the VTK data array.
    int nValues = int(dims[0]);
    debug4 << "Creating VTK float array with " << nValues << " values" << endl;
    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfTuples(nValues);
    float *f = (float *) rv->GetVoidPointer(0);
    debug4 << "Reading slab into data array" << endl;

    H5Dread(handle, H5T_NATIVE_FLOAT, memspace, dataspace, H5P_DEFAULT, f);
    H5Sclose(memspace);
    H5Sclose(dataspace);
    H5Dclose(handle);

    return rv;
}


// ****************************************************************************
//  Method: avtGTCFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Apr 27 13:18:22 PST 2006
//
// ****************************************************************************

vtkDataArray *
avtGTCFileFormat::GetVectorVar(const char *varname)
{
    EXCEPTION1(InvalidVariableException, varname);
}
