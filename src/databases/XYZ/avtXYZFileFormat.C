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
// ****************************************************************************

avtXYZFileFormat::avtXYZFileFormat(const char *fn)
    : avtMTSDFileFormat(&fn, 1)
{
    fileRead = false;
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
// ****************************************************************************

int
avtXYZFileFormat::GetNTimesteps(void)
{
    ReadFile();
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

    fileRead = false;
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
// ****************************************************************************

void
avtXYZFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    ReadFile();
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
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 14, 2007
//
// ****************************************************************************

vtkDataSet *
avtXYZFileFormat::GetMesh(int timestate, const char *meshname)
{
    ReadFile();

    vtkPolyData *pd  = vtkPolyData::New();
    vtkPoints   *pts = vtkPoints::New();

    pts->SetNumberOfPoints(nAtoms);
    pd->SetPoints(pts);
    pts->Delete();
    for (int j = 0 ; j < nAtoms ; j++)
    {
        pts->SetPoint(j,
                      x[timestate][j],
                      y[timestate][j],
                      z[timestate][j]);
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
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 14, 2007
//
// ****************************************************************************

vtkDataArray *
avtXYZFileFormat::GetVar(int timestate, const char *varname)
{
    ReadFile();

    // element is a built-in variable
    if (string(varname) == "element")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(nAtoms);
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<nAtoms; i++)
        {
            ptr[i] = e[timestate][i];
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
        ptr[i] = v[varindex][timestate][i];
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
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 14, 2007
//
// ****************************************************************************

vtkDataArray *
avtXYZFileFormat::GetVectorVar(int timestate, const char *varname)
{
    // No vector variables
    return NULL;
}

// ****************************************************************************
//  Method:  avtXYZFileFormat::ReadFile
//
//  Purpose:
//    Read the file.  We're going ahead and reading the entire thing
//    right now because it's hard to even count the number of timesteps
//    in this file without reading it all.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 14, 2007
//
// ****************************************************************************
void
avtXYZFileFormat::ReadFile()
{
    if (fileRead)
        return;

    ifstream in(filename.c_str());
    if (!in)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    char buff[1000];

    nTimeSteps = 0;
    nVars = 0;
    while (in)
    {
        in >> nAtoms;
        // this is the first read to return EOF after the last timestep
        if (!in)
            break;

        // the first line had better be a number
        if (nAtoms == 0)
        {
            EXCEPTION1(InvalidFilesException, filename.c_str());
        }

        // finish the first line
        in.getline(buff,1000);

        // read the molecule name
        char comment[1000];
        in.getline(comment,1000);

        // we got a new timestep; add to our arrays
        e.push_back(vector<int>(nAtoms));
        x.push_back(vector<float>(nAtoms));
        y.push_back(vector<float>(nAtoms));
        z.push_back(vector<float>(nAtoms));
        for (int i=0; i<MAX_XYZ_VARS; i++)
        {
            v[i].push_back(vector<float>(nAtoms));
        }

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
                           &x[nTimeSteps][a],
                           &y[nTimeSteps][a],
                           &z[nTimeSteps][a],
                           &v[0][nTimeSteps][a],
                           &v[1][nTimeSteps][a],
                           &v[2][nTimeSteps][a],
                           &v[3][nTimeSteps][a],
                           &v[4][nTimeSteps][a],
                           &v[5][nTimeSteps][a]);
            e[nTimeSteps][a] = ElementNameToAtomicNumber(element);
            nVars = n - 4;
        }

        // success; next time step
        nTimeSteps++;
    }

    // don't read it more than once
    fileRead = true;
}
