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
//                            avtXYZFileFormat.C                           //
// ************************************************************************* //

#include <avtXYZFileFormat.h>

#include <string>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>

#include <avtDatabaseMetaData.h>

#include <Expression.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <AtomicProperties.h>

using     std::string;


// ****************************************************************************
//  Method: avtXYZ constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   June 14, 2007
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 26 14:33:16 EDT 2007
//    Changed to read only the requested time steps on demand instead
//    of reading the whole file at once.
//
// ****************************************************************************

avtXYZFileFormat::avtXYZFileFormat(const char *fn)
    : avtMTSDFileFormat(&fn, 1)
{
    metaDataRead = false;
    filename = fn;
}


// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 14, 2007
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 26 14:33:16 EDT 2007
//    Changed to read only the requested time steps on demand instead
//    of reading the whole file at once.
//
// ****************************************************************************

int
avtXYZFileFormat::GetNTimesteps(void)
{
    ReadAllMetaData();
    return nTimeSteps;
}


// ****************************************************************************
//  Method: avtXYZFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 14, 2007
//
// ****************************************************************************

void
avtXYZFileFormat::FreeUpResources(void)
{
    for (int t=0; t<nTimeSteps; t++)
    {
        e[t].clear();
        x[t].clear();
        y[t].clear();
        z[t].clear();
        for (int i=0; i<MAX_XYZ_VARS; i++)
            v[i][t].clear();
    }
    e.clear();
    x.clear();
    y.clear();
    z.clear();
    for (int i=0; i<MAX_XYZ_VARS; i++)
        v[i].clear();
}


// ****************************************************************************
//  Method:  avtXYZFileFormat::OpenFileAtBeginning
//
//  Purpose:
//    Opens the file, or else seeks to the beginning.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 26, 2007
//
// ****************************************************************************
void
avtXYZFileFormat::OpenFileAtBeginning()
{
    if (!in.is_open())
    {
        in.open(filename.c_str());
        if (!in)
        {
            EXCEPTION1(InvalidFilesException, filename.c_str());
        }
    }
    else
    {
        in.clear();
        in.seekg(0, ios::beg);
    }
}


// ****************************************************************************
//  Method: avtXYZFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 14, 2007
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 26 14:33:16 EDT 2007
//    Changed to read only the requested time steps on demand instead
//    of reading the whole file at once.
//
// ****************************************************************************

void
avtXYZFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timestep)
{
    ReadAllMetaData();

    avtMeshMetaData *mmd = new avtMeshMetaData("mesh", 1, 0,0,0,
                                               3, 1,
                                               AVT_POINT_MESH);
    mmd->nodesAreCritical = true;
    md->Add(mmd);

    AddScalarVarToMetaData(md, "element", "mesh", AVT_NODECENT);
    for (int i=0; i<nVars; i++)
    {
        char name[20];
        sprintf(name, "var%d", i);
        AddScalarVarToMetaData(md, name, "mesh", AVT_NODECENT);
    }
}


// ****************************************************************************
//  Method: avtXYZFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestep    The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 14, 2007
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 26 14:33:16 EDT 2007
//    Changed to read only the requested time steps on demand instead
//    of reading the whole file at once.
//
// ****************************************************************************

vtkDataSet *
avtXYZFileFormat::GetMesh(int timestep, const char *meshname)
{
    ReadTimeStep(timestep);

    vtkPolyData *pd  = vtkPolyData::New();
    vtkPoints   *pts = vtkPoints::New();

    pts->SetNumberOfPoints(nAtoms);
    pd->SetPoints(pts);
    pts->Delete();
    for (int j = 0 ; j < nAtoms ; j++)
    {
        pts->SetPoint(j,
                      x[timestep][j],
                      y[timestep][j],
                      z[timestep][j]);
    }
 
    vtkCellArray *verts = vtkCellArray::New();
    pd->SetVerts(verts);
    verts->Delete();
    for (int k = 0 ; k < nAtoms ; k++)
    {
        verts->InsertNextCell(1);
        verts->InsertCellPoint(k);
    }


    return pd;
}


// ****************************************************************************
//  Method: avtXYZFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestep   The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 14, 2007
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 26 14:33:16 EDT 2007
//    Changed to read only the requested time steps on demand instead
//    of reading the whole file at once.
//
// ****************************************************************************

vtkDataArray *
avtXYZFileFormat::GetVar(int timestep, const char *varname)
{
    ReadTimeStep(timestep);

    // element is a built-in variable
    if (string(varname) == "element")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(nAtoms);
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<nAtoms; i++)
        {
            ptr[i] = e[timestep][i];
        }
        return scalars;
    }

    // if it's not element, then it's one of the vars;
    // figure out which one
    if (strlen(varname) != 4)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }
    int varindex = varname[3] - '0';
    if (varindex < 0 || varindex > nVars)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    // and now create the data array for it
    vtkFloatArray *scalars = vtkFloatArray::New();
    scalars->SetNumberOfTuples(nAtoms);
    float *ptr = (float *) scalars->GetVoidPointer(0);
    for (int i=0; i<nAtoms; i++)
    {
        ptr[i] = v[varindex][timestep][i];
    }
    return scalars;
}


// ****************************************************************************
//  Method: avtXYZFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestep   The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 14, 2007
//
// ****************************************************************************

vtkDataArray *
avtXYZFileFormat::GetVectorVar(int timestep, const char *varname)
{
    // No vector variables
    return NULL;
}

// ****************************************************************************
//  Method:  avtXYZFileFormat::ReadTimeStep
//
//  Purpose:
//    Read only the atoms for the given time step.
//
//  Arguments:
//    timestep   the time state for which to read the atoms
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 26, 2007
//
// ****************************************************************************
void
avtXYZFileFormat::ReadTimeStep(int timestep)
{
    ReadAllMetaData();

    // don't read this time step if it's already in memory
    if (x[timestep].size() > 0)
        return;

    OpenFileAtBeginning();
    in.seekg(file_positions[timestep]);

    e[timestep].resize(nAtoms);
    x[timestep].resize(nAtoms);
    y[timestep].resize(nAtoms);
    z[timestep].resize(nAtoms);
    for (int i=0; i<MAX_XYZ_VARS; i++)
    {
        v[i][timestep].resize(nAtoms);
    }

    char buff[1000];

    // skip the number of atoms
    in.getline(buff,1000);
    // skip the molecule name
    in.getline(buff,1000);

    // read all the atoms
    for (int a=0; a<nAtoms; a++)
    {
        in.getline(buff,1000);
        char element[100];
        if (MAX_XYZ_VARS != 6)
        {
            EXCEPTION1(VisItException,
                       "Internal error: avtXYZFileFormat has "
                       "MAX_XYZ_VARS mismatch.");
        }
        int n = sscanf(buff,"%s %f %f %f %f %f %f %f %f %f",
                       element,
                       &x[timestep][a],
                       &y[timestep][a],
                       &z[timestep][a],
                       &v[0][timestep][a],
                       &v[1][timestep][a],
                       &v[2][timestep][a],
                       &v[3][timestep][a],
                       &v[4][timestep][a],
                       &v[5][timestep][a]);
        e[timestep][a] = ElementNameToAtomicNumber(element);
    }
}


// ****************************************************************************
//  Method:  avtXYZFileFormat::ReadMetaData
//
//  Purpose:
//    The metadata we need to read here is (a) count the number of
//    time steps, and (b) count how many entries are in the atoms
//    so we know how many variables to report.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 26, 2007
//
// ****************************************************************************
void
avtXYZFileFormat::ReadAllMetaData()
{
    if (metaDataRead)
        return;

    OpenFileAtBeginning();

    char buff[1000];

    nTimeSteps = 0;
    nVars = 0;
    while (in)
    {
        // get the current file position
        istream::pos_type current_pos = in.tellg();

        in >> nAtoms;
        // this is the first read to return EOF after the last timestep
        if (!in)
            break;

        // record the position as the start of this timestep
        file_positions.push_back(current_pos);

        // the first line had better be a number
        if (nAtoms == 0)
        {
            EXCEPTION1(InvalidFilesException, filename.c_str());
        }

        // finish the first line
        in.getline(buff,1000);

        // skip molecule name
        in.getline(buff,1000);

        // in the first time step, get the number of vars.
        if (nTimeSteps == 0)
        {
            in.getline(buff,1000);
            char element[100];
            float x,y,z,v0,v1,v2,v3,v4,v5;
            if (MAX_XYZ_VARS != 6)
            {
                EXCEPTION1(VisItException,
                           "Internal error: avtXYZFileFormat has "
                           "MAX_XYZ_VARS mismatch.");
            }
            int n = sscanf(buff,"%s %f %f %f %f %f %f %f %f %f",
                           element,&x,&y,&z,&v0,&v1,&v2,&v3,&v4,&v5);
            nVars = n - 4;

            // skip the remainin atoms
            for (int a=1; a<nAtoms; a++)
            {
                in.getline(buff,1000);
            }
        }
        else
        {
            // skip the remainin atoms
            for (int a=0; a<nAtoms; a++)
            {
                in.getline(buff,1000);
            }
        }

        // success; next time step
        nTimeSteps++;
    }

    // don't read the meta data more than once
    metaDataRead = true;

    // resize our atom arrays
    e.resize(nTimeSteps);
    x.resize(nTimeSteps);
    y.resize(nTimeSteps);
    z.resize(nTimeSteps);
    for (int i=0; i<MAX_XYZ_VARS; i++)
    {
        v[i].resize(nTimeSteps);
    }
    
}
