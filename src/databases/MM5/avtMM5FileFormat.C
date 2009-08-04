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
//                            avtMM5FileFormat.C                             //
// ************************************************************************* //

#include <avtMM5FileFormat.h>

#include <string>

#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkVisItUtility.h>
#include <snprintf.h>

#include <avtDatabaseMetaData.h>

#include <InvalidVariableException.h>


// ****************************************************************************
//  Method: avtMM5 constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 11 16:39:09 PST 2006
//
// ****************************************************************************

avtMM5FileFormat::avtMM5FileFormat(const char *filename)
    : avtMTSDFileFormat(&filename, 1), meshNames()
{
    mm5file = 0;
    meshNamesCreated = 0;
}

// ****************************************************************************
//  Method: avtMM5 destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 11 16:39:09 PST 2006
//
// ****************************************************************************

avtMM5FileFormat::~avtMM5FileFormat()
{
    if(mm5file != 0)
        mm5_file_close(mm5file);
}

// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 11 16:39:09 PST 2006
//
// ****************************************************************************

int
avtMM5FileFormat::GetNTimesteps(void)
{
    Initialize();

    return mm5file->ntimes;
}

// ****************************************************************************
// Method: avtMM5FileFormat::GetTimes
//
// Purpose: 
//   Returns the times in the file.
//
// Arguments:
//   times : The vector of times.
//
// Note:       This returns xtime but it would be nice if we could someday
//             return some type of enumerated time values such as the
//             date strings from the file.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 12 10:55:36 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtMM5FileFormat::GetTimes(std::vector<double> &times)
{
    double dtime = 0.;

    Initialize();

    for(int i = 0; i < mm5file->ntimes; ++i)
    {
        if(mm5file->fields_over_time[i].nfields > 0)
        {
            dtime = mm5file->fields_over_time[i].fields[0].header.xtime;
            times.push_back(dtime);
        }
    }
}

// ****************************************************************************
// Method: avtMM5FileFormat::Initialize
//
// Purpose: 
//   Opens the MM5 file and constructs the mesh name map.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 12 10:56:54 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtMM5FileFormat::Initialize()
{
    if(mm5file == 0)
    {
        mm5file = mm5_file_open(filenames[0]);

#if 0
        // If the file was opened then write a log file containing the
        // contents of the file. Do this to debug4 someday.
        if(mm5file != 0)
        {
            FILE *log = fopen("/data_vobs/VisIt/bin/mm5.log", "wt");
            mm5_file_print_structure(mm5file, log);
            fclose(log);
        }
#endif
    }

    if(!meshNamesCreated && mm5file != 0 && mm5file->ntimes > 0)
    {
        if(mm5file->fields_over_time[0].nfields > 0)
        {
            for(int i = 0; i < mm5file->fields_over_time[0].nfields; ++i)
            {
                const int *dims = mm5file->fields_over_time[0].
                    fields[i].header.end_index;
                int ndims = mm5file->fields_over_time[0].
                    fields[i].header.ndim;

                if(ndims > 1)
                {
                    intVector meshDims;
                    std::string meshName;
                    char tmp[100];

                    for(int d = 0; d < ndims; ++d)
                    {
                        meshDims.push_back(dims[d]);

                        if(d != 0)
                            SNPRINTF(tmp, 100, "x%d", dims[d]);
                        else
                            SNPRINTF(tmp, 100, "%d", dims[d]);
                        meshName += tmp;
                    }

                    if(meshNames.find(meshName) == meshNames.end())
                        meshNames[meshName] = meshDims;
                }
            }

            meshNamesCreated = true;
        }
    }
}

// ****************************************************************************
//  Method: avtMM5FileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 11 16:39:09 PST 2006
//
//  Modifications:
//
// ****************************************************************************

void
avtMM5FileFormat::FreeUpResources(void)
{
    if(mm5file != 0)
    {
        mm5_file_close(mm5file);
        mm5file = 0;
    }

    meshNames.clear();
    meshNamesCreated = false;
}


// ****************************************************************************
//  Method: avtMM5FileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 11 16:39:09 PST 2006
//
//  Modifications:
//
// ****************************************************************************

void
avtMM5FileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    Initialize();

    for(MeshNameMap::const_iterator pos = meshNames.begin();
        pos != meshNames.end(); ++pos)
    {
        int sdims, tdims; 
        sdims = tdims = pos->second.size();
        avtMeshMetaData *mmd = new avtMeshMetaData(pos->first,
            1, 1, 1, 0, sdims, tdims,
            AVT_RECTILINEAR_MESH);
        md->Add(mmd);
    }

    // Now iterate over the variables and add them.
    if(mm5file != 0 && mm5file->ntimes > 0)
    {
        if(mm5file->fields_over_time[0].nfields > 0)
        {
            for(int i = 0; i < mm5file->fields_over_time[0].nfields; ++i)
            {
                const int *dims = mm5file->fields_over_time[0].
                    fields[i].header.end_index;
                int ndims = mm5file->fields_over_time[0].
                    fields[i].header.ndim;
                std::string varname(mm5file->fields_over_time[0].
                    fields[i].header.name);

                if(ndims == 1)
                {
                    // Add the curve metadata.
                    avtCurveMetaData *cmd = new avtCurveMetaData;
                    cmd->name = varname;
                    cmd->yLabel = varname;
                    cmd->yUnits = std::string(mm5file->fields_over_time[0].
                        fields[i].header.unit);
                }
                else
                {
                    // Assemble the name of the mesh that the variable is on.
                    std::string meshName;
                    char tmp[100];
                    for(int d = 0; d < ndims; ++d)
                    {
                        if(d != 0)
                            SNPRINTF(tmp, 100, "x%d", dims[d]);
                        else
                            SNPRINTF(tmp, 100, "%d", dims[d]);
                        meshName += tmp;
                    }

                    // Add the scalar metadata
                    avtScalarMetaData *smd = new avtScalarMetaData(
                        varname, meshName, AVT_ZONECENT);
                    smd->hasUnits = true;
                    smd->units = std::string(mm5file->fields_over_time[0].
                        fields[i].header.unit);
                    md->Add(smd);
                }
            }

            meshNamesCreated = true;
        }
    }
}


// ****************************************************************************
//  Method: avtMM5FileFormat::GetMesh
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
//  Creation:   Tue Jul 11 16:39:09 PST 2006
//
//  Modifications:
//    Kathleen Bonnell, Mon Jul 14 13:57:25 PDT 2008
//    Specify curves as 1D rectilinear grids with yvalues stored in point data.
//
// ****************************************************************************

vtkDataSet *
avtMM5FileFormat::GetMesh(int timestate, const char *meshname)
{
    vtkDataSet *retval = 0;

    Initialize();

    std::string mName(meshname);
    MeshNameMap::const_iterator pos = meshNames.find(mName);
    if(pos != meshNames.end())
    {
        int   i, j;
        vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New(); 

        //
        // Populate the coordinates.  Put in 3D points with z=0 if
        // the mesh is 2D.
        //
        int dims[3];
        vtkFloatArray *coords[3];
        for (i = 0 ; i < 3 ; i++)
        {
            // Default number of components for an array is 1.
            coords[i] = vtkFloatArray::New();

            if (i < pos->second.size())
            {
                dims[i] = pos->second[i] + 1;
                coords[i]->SetNumberOfTuples(dims[i]);
                for (j = 0 ; j < dims[i] ; j++)
                {
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
    else
    {
        mm5_fieldinfo_t *field = mm5_file_find_field(mm5file, meshname, timestate);
        if(field == 0)
        {
            EXCEPTION1(InvalidVariableException, meshname);
        }

        // We found a curve.
        int nPts = field->header.end_index[0] *
            field->header.end_index[1] *
            field->header.end_index[2] *
            field->header.end_index[3];
        float *yValues = new float[nPts];
        if(mm5_file_read_field(mm5file, meshname, timestate, yValues) == 0)
        {
            delete [] yValues;
            EXCEPTION1(InvalidVariableException, meshname);
        }

        // Make a new curve.
        vtkRectilinearGrid *rg =vtkVisItUtility::Create1DRGrid(nPts, VTK_FLOAT);
        vtkFloatArray *xc = vtkFloatArray::SafeDownCast(rg->GetXCoordinates());
        vtkFloatArray *yv = vtkFloatArray::New();
        yv->SetNumberOfComponents(1);
        yv->SetNumberOfTuples(nPts);
        yv->SetName(meshname);

        for (int j = 0 ; j < nPts ; j++)
        {
            xc->SetValue(j, float(j));
            yv->SetValue(j, yValues[j]);
        }
 
        rg->GetPointData()->SetScalars(yv);
        yv->Delete();
        retval = rg;

        delete [] yValues;
    }

    return retval;
}


// ****************************************************************************
//  Method: avtMM5FileFormat::GetVar
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
//  Creation:   Tue Jul 11 16:39:09 PST 2006
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtMM5FileFormat::GetVar(int timestate, const char *varname)
{
    mm5_fieldinfo_t *field = 0;

    Initialize();
   
    if(mm5file == 0)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    field = mm5_file_find_field(mm5file, varname, timestate);

    if(field == 0)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    // Set up the return array.
    vtkFloatArray *rv = vtkFloatArray::New();
    int n_elements = field->header.end_index[0] *
            field->header.end_index[1] *
            field->header.end_index[2] *
            field->header.end_index[3];
    rv->SetNumberOfTuples(n_elements);
    float *fptr = (float *)rv->GetVoidPointer(0);

    // Read the data directly into the array.
    if(mm5_file_read_field(mm5file, varname, timestate, fptr) == 0)
    {
        rv->Delete();
        EXCEPTION1(InvalidVariableException, varname);
    }

    return rv;
}


// ****************************************************************************
//  Method: avtMM5FileFormat::GetVectorVar
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
//  Creation:   Tue Jul 11 16:39:09 PST 2006
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtMM5FileFormat::GetVectorVar(int timestate, const char *varname)
{
    EXCEPTION1(InvalidVariableException, varname);
}
