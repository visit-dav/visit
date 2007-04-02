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
//                          avtTetradFileFormat.C                            //
// ************************************************************************* //

#include <avtTetradFileFormat.h>

#include <stdlib.h>

#include <vtkCellType.h>
#include <vtkFloatArray.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>

#include <DebugStream.h>
#include <InvalidDBTypeException.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>
#include <InvalidTimeStepException.h>


typedef struct
{
    double dtime;
    int index;
} time_index_pair;


using std::string;
using std::vector;


static herr_t       CGetTimesteps(hid_t, const char *, void *);
static herr_t       CGetVars(hid_t, const char *, void *);
static int          TimeIndexPairSorter(const void *, const void *);


// ****************************************************************************
//  Method: avtTetradFileFormat constructor
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2002
//
//  Modifications:
//
//    Hank Childs, Wed Oct 16 10:18:38 PDT 200
//    Add a check to make sure that this is really the right kind of HDF5 file.
//
// ****************************************************************************

avtTetradFileFormat::avtTetradFileFormat(const char *fname)
    : avtMTSDFileFormat(&fname, 1)
{
    file_handle = H5Fopen(fname, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (file_handle < 0)
    {
        EXCEPTION1(InvalidDBTypeException, "Cannot be a Tetrad file, since "
                                           "it is not even an HDF5 file.");
    }
    int cell_array = H5Dopen(file_handle, "CellArray");
    if (cell_array < 0)
    {
        EXCEPTION1(InvalidDBTypeException, "Cannot be a Tetrad file, since "
                          "it is does not contain the dataset \"cell_array\"");
    }
    H5Dclose(cell_array);
    mesh_for_all_timesteps = NULL;
}


// ****************************************************************************
//  Method: avtTetradFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2002
//
// ****************************************************************************

avtTetradFileFormat::~avtTetradFileFormat()
{
    H5Fclose(file_handle);
    if (mesh_for_all_timesteps != NULL)
    {
        mesh_for_all_timesteps->Delete();
    }
}


// ****************************************************************************
//  Method: avtTetradFileFormat::GetCycles
//
//  Purpose:
//      Gets the cycle list in the file.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2002
//
// ****************************************************************************

void
avtTetradFileFormat::GetCycles(vector<int> &c)
{
    c.clear();
    for (int i = 0 ; i < timesteps.size() ; i++)
    {
        c.push_back(i+1);
    }
}


// ****************************************************************************
//  Method: avtTetradFileFormat::GetNTimesteps
//
//  Purpose:
//      Gets the number of timesteps in the file.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2002
//
// ****************************************************************************

int
avtTetradFileFormat::GetNTimesteps(void)
{
    return timesteps.size();
}


// ****************************************************************************
//  Method: avtTetradFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh for a particular timestep.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2002
//
// ****************************************************************************

vtkDataSet *
avtTetradFileFormat::GetMesh(int, const char *)
{
    if (mesh_for_all_timesteps == NULL)
    {
        mesh_for_all_timesteps = ConstructMesh();
    }

    //
    // The calling routine will assume that it owns this object, so save off
    // a reference to it.
    //
    mesh_for_all_timesteps->Register(NULL);

    return mesh_for_all_timesteps;
}


// ****************************************************************************
//  Method: avtTetradFileFormat::ConstructMesh
//
//  Purpose:
//      This does the HDF5 and VTK calls to construct the mesh.
//
//  Programmer: Hank Childs
//  Creation:   October 7, 2002
//
//  Modifications:
//    Brad Whitlock, Mon Feb 23 14:24:04 PST 2004
//    Made it build on Windows.
//
// ****************************************************************************

vtkDataSet *
avtTetradFileFormat::ConstructMesh(void)
{
    //
    // Start off by reading in the "CellArray".  This is the connectivity of 
    // the hexahedrons.
    //
    int cell_array = H5Dopen(file_handle, "CellArray");
    if (cell_array < 0)
    {
        EXCEPTION1(InvalidVariableException, "mesh");
    }
    int space_id = H5Dget_space(cell_array);
    hsize_t vdims[3];
    H5Sget_simple_extent_dims(space_id, vdims, NULL);
    debug5 << "The connectivity has dimensions = " << int(vdims[0]) << " x "
           << int(vdims[1]) << endl;
    if (vdims[1] != 8)
    {
        EXCEPTION1(InvalidVariableException, "mesh");
    }
    int ncells = vdims[0];
    int *connectivity = new int[8*vdims[0]];
    H5Dread(cell_array, H5T_NATIVE_INT, H5S_ALL, space_id, H5P_DEFAULT,
            connectivity);
    H5Dclose(cell_array);

    //
    // Now read in the "VertArray".  These are the points.
    //
    int vert_array = H5Dopen(file_handle, "VertArray");
    if (vert_array < 0)
    {
        EXCEPTION1(InvalidVariableException, "mesh");
    }
    space_id = H5Dget_space(vert_array);
    H5Sget_simple_extent_dims(space_id, vdims, NULL);
    debug5 << "The vertices have dimensions = " << int(vdims[0]) << " x "
           << int(vdims[1]) << endl;
    if (vdims[1] != 3)
    {
        EXCEPTION1(InvalidVariableException, "mesh");
    }

    //
    // We should have everything we need -- just create the VTK object.
    //
    int npts = vdims[0];
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(npts);
    float *pts_ptr = (float *) pts->GetVoidPointer(0);
    H5Dread(vert_array, H5T_NATIVE_FLOAT, H5S_ALL, space_id, H5P_DEFAULT,
            pts_ptr);
    H5Dclose(vert_array);

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(pts);
    pts->Delete();
    ugrid->Allocate(ncells);
    for (int i = 0 ; i < ncells ; i++)
    {
        vtkIdType ids[8];
        ids[0] = connectivity[8*i+4];
        ids[1] = connectivity[8*i+5];
        ids[2] = connectivity[8*i+6];
        ids[3] = connectivity[8*i+7];
        ids[4] = connectivity[8*i];
        ids[5] = connectivity[8*i+1];
        ids[6] = connectivity[8*i+2];
        ids[7] = connectivity[8*i+3];
        ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, ids);
    }

    return ugrid;
}


// ****************************************************************************
//  Method: avtTetradFileFormat::GetVar
//
//  Purpose:
//      Gets the variable for a particular timestep.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2002
//
// ****************************************************************************

vtkDataArray *
avtTetradFileFormat::GetVar(int ts, const char *var)
{
    if (ts < 0 || ts >= timesteps.size())
    {
        EXCEPTION2(InvalidTimeStepException, ts, timesteps.size());
    }

    //
    // Set up the variable name.
    //
    char buffer[1024];
    sprintf(buffer, "/root/%s/%s", var, timesteps[ts].c_str());
    debug5 << "Reading in var " << buffer << endl;

    //
    // Tell HDF5 that this is the dataset we are interested in.
    //
    int var_handle = H5Dopen(file_handle, buffer);
    if (var_handle < 0)
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    //
    // Determine how big the variable is.
    //
    int space_id = H5Dget_space(var_handle);
    hsize_t vdims[3];
    H5Sget_simple_extent_dims(space_id, vdims, NULL);
    debug5 << "The variable has dimensions " << int(vdims[0]) << endl;

    //
    // Set up a VTK object and allocate memory for it.  That way HDF5 can read
    // the variable straight into the memory we have set up.
    //
    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetNumberOfTuples(vdims[0]);
    float *ptr = arr->GetPointer(0);

    //
    // The actual HDF5 call to get the variable.
    //
    H5Dread(var_handle, H5T_NATIVE_FLOAT, H5S_ALL, space_id, H5P_DEFAULT, ptr);
    H5Dclose(var_handle);
    
    return arr;
}


// ****************************************************************************
//  Method: avtTetradFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Fills out what variables are in this file.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2002
//
//  Modifications:
//
//    Hank Childs, Wed Oct 16 10:18:38 PDT 2002
//    Account for the timesteps being put in lexicographical, instead of
//    numerical, order.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added timeState argument
// ****************************************************************************

void
avtTetradFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
    int timeState)
{
    int   i;

    //
    // Start off by walking through the HDF5 file and determining the
    // variables and times in this file.
    //
    H5Giterate(file_handle, "root", NULL, CGetVars, this);
    if (variables.size() > 0)
    {
        char buffer[1024];
        sprintf(buffer, "/root/%s", variables[0].c_str());
        H5Giterate(file_handle, buffer, NULL, CGetTimesteps, this);

        //
        // The timesteps were added in lexicographical order -- we need to
        // re-order them in numerical order.
        //
        int ntimes = timesteps.size();
        time_index_pair *pairs = new time_index_pair[ntimes];
        for (i = 0 ; i < ntimes ; i++)
        {
            pairs[i].dtime  = atof(timesteps[i].c_str());
            pairs[i].index = i;
        }
        qsort(pairs, ntimes, sizeof(time_index_pair), TimeIndexPairSorter);
 
        vector<string> tmpTimesteps;
        for (i = 0 ; i < ntimes ; i++)
        {
            tmpTimesteps.push_back(timesteps[pairs[i].index]);
        }
        timesteps = tmpTimesteps;
    }

    //
    // Set the information about the timesteps with the meta-data info.
    //
    md->SetNumStates(timesteps.size());
    vector<double> dtimesteps;
    for (i = 0 ; i < timesteps.size() ; i++)
    {
        dtimesteps.push_back(atof(timesteps[i].c_str()));
    }
    md->SetTimes(dtimesteps);
    md->SetTimesAreAccurate(true);

    //
    // Now construct the objects for the mesh and variables.
    //
    AddMeshToMetaData(md, "mesh", AVT_UNSTRUCTURED_MESH, NULL, 1, 0, 3, 3);
    for (i = 0 ; i < variables.size() ; i++)
    {
        AddScalarVarToMetaData(md, variables[i], "mesh", AVT_ZONECENT);
    }
}


// ****************************************************************************
//  Method: avtTetradFileFormat::GetVars
//
//  Purpose:
//      Called whenever a variable is found underneath 'root'.
//
//  Programmer: Hank Childs
//  Creation:   October 7, 2002
//
// ****************************************************************************

void
avtTetradFileFormat::GetVars(const char *vname)
{
    variables.push_back(vname);
}


// ****************************************************************************
//  Method: avtTetradFileFormat::GetTimesteps
//
//  Purpose:
//      Called whenever a timesteps is found underneath a variable.
//
//  Programmer: Hank Childs
//  Creation:   October 7, 2002
//
// ****************************************************************************

void
avtTetradFileFormat::GetTimesteps(const char *time_str)
{
    timesteps.push_back(time_str);
}


// ****************************************************************************
//  Function: CGetVars
//
//  Purpose:
//      Gets the variables in a directory.  This is a function that calls the
//      associated method with avtTetradFileFormat.
//
//  Programmer: Hank Childs
//  Creation:   October 7, 2002
//
// ****************************************************************************

herr_t
CGetVars(hid_t loc_id, const char *name, void *tetrad)
{
    avtTetradFileFormat *ff = (avtTetradFileFormat *) tetrad;
    ff->GetVars(name);

    return 0;
}


// ****************************************************************************
//  Function: CGetTimesteps
//
//  Purpose:
//      Gets the timesteps in a directory.  This is a function that calls the
//      associated method with avtTetradFileFormat.
//
//  Programmer: Hank Childs
//  Creation:   October 7, 2002
//
// ****************************************************************************

herr_t
CGetTimesteps(hid_t loc_id, const char *name, void *tetrad)
{
    avtTetradFileFormat *ff = (avtTetradFileFormat *) tetrad;
    ff->GetTimesteps(name);

    return 0;
}


// ****************************************************************************
//  Method: TimeIndexPairSorter
//
//  Purpose:
//      Sorts time-index pairs by the time field.
//
//  Arguments:
//      arg1    The first record.
//      arg2    The second record.
//
//  Returns:    <0 if arg1<arg2, 0 if arg1==arg2, >0 if arg2>arg1.
//
//  Programmer: Hank Childs
//  Creation:   October 16, 2002
//
// ****************************************************************************

int
TimeIndexPairSorter(const void *arg1, const void *arg2)
{
    time_index_pair *tp1 = (time_index_pair *) arg1;
    time_index_pair *tp2 = (time_index_pair *) arg2;

    if (tp1->dtime > tp2->dtime)
    {
        return 1;
    }
    else if (tp1->dtime < tp2->dtime)
    {
        return -1;
    }

    return 0;
}


