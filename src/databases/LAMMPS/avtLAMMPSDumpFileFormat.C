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
//                            avtLAMMPSDumpFileFormat.C                           //
// ************************************************************************* //

#include <avtLAMMPSDumpFileFormat.h>

#include <string>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <avtMTSDFileFormatInterface.h>

#include <avtDatabaseMetaData.h>

#include <Expression.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <AtomicProperties.h>

using     std::string;


// ****************************************************************************
//  Method: avtLAMMPS constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   February  9, 2009
//
//  Modifications:
//
// ****************************************************************************

avtLAMMPSDumpFileFormat::avtLAMMPSDumpFileFormat(const char *fn)
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
//  Creation:   February  9, 2009
//
//  Modifications:
//
// ****************************************************************************

int
avtLAMMPSDumpFileFormat::GetNTimesteps(void)
{
    ReadAllMetaData();
    return nTimeSteps;
}


// ****************************************************************************
//  Method: avtLAMMPSDumpFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  9, 2009
//
// ****************************************************************************

void
avtLAMMPSDumpFileFormat::FreeUpResources(void)
{
    for (int t=0; t<nTimeSteps; t++)
    {
        s[t].clear();
        x[t].clear();
        y[t].clear();
        z[t].clear();
        for (int i=0; i<MAX_LAMMPS_DUMP_VARS; i++)
            v[i][t].clear();
    }
    s.clear();
    x.clear();
    y.clear();
    z.clear();
    for (int i=0; i<MAX_LAMMPS_DUMP_VARS; i++)
        v[i].clear();
}


// ****************************************************************************
//  Method:  avtLAMMPSDumpFileFormat::OpenFileAtBeginning
//
//  Purpose:
//    Opens the file, or else seeks to the beginning.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  9, 2009
//
// ****************************************************************************
void
avtLAMMPSDumpFileFormat::OpenFileAtBeginning()
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
//  Method: avtLAMMPSDumpFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  9, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtLAMMPSDumpFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timestep)
{
    ReadAllMetaData();

    avtMeshMetaData *mmd = new avtMeshMetaData("mesh", 1, 0,0,0,
                                               3, 1,
                                               AVT_POINT_MESH);
    mmd->nodesAreCritical = true;
    md->Add(mmd);

    AddScalarVarToMetaData(md, "species", "mesh", AVT_NODECENT);
    for (int i=0; i<nVars; i++)
    {
        char name[20];
        sprintf(name, "var%d", i);
        AddScalarVarToMetaData(md, name, "mesh", AVT_NODECENT);
    }
}


// ****************************************************************************
//  Method: avtLAMMPSDumpFileFormat::GetMesh
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
//  Creation:   February  9, 2009
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtLAMMPSDumpFileFormat::GetMesh(int timestep, const char *meshname)
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
//  Method: avtLAMMPSDumpFileFormat::GetVar
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
//  Creation:   February  9, 2009
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtLAMMPSDumpFileFormat::GetVar(int timestep, const char *varname)
{
    ReadTimeStep(timestep);

    // element is a built-in variable
    if (string(varname) == "species")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(nAtoms);
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<nAtoms; i++)
        {
            ptr[i] = s[timestep][i];
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
//  Method: avtLAMMPSDumpFileFormat::GetVectorVar
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
//  Creation:   February  9, 2009
//
// ****************************************************************************

vtkDataArray *
avtLAMMPSDumpFileFormat::GetVectorVar(int timestep, const char *varname)
{
    // No vector variables
    return NULL;
}

// ****************************************************************************
//  Method:  avtLAMMPSDumpFileFormat::ReadTimeStep
//
//  Purpose:
//    Read only the atoms for the given time step.
//
//  Arguments:
//    timestep   the time state for which to read the atoms
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  9, 2009
//
// ****************************************************************************
void
avtLAMMPSDumpFileFormat::ReadTimeStep(int timestep)
{
    ReadAllMetaData();

    // don't read this time step if it's already in memory
    if (x[timestep].size() > 0)
        return;

    OpenFileAtBeginning();
    in.seekg(file_positions[timestep]);

    s[timestep].resize(nAtoms);
    x[timestep].resize(nAtoms);
    y[timestep].resize(nAtoms);
    z[timestep].resize(nAtoms);
    for (int i=0; i<MAX_LAMMPS_DUMP_VARS; i++)
    {
        v[i][timestep].resize(nAtoms);
    }

    char buff[1000];
    // read all the atoms
    for (int a=0; a<nAtoms; a++)
    {
        in.getline(buff,1000);
        if (MAX_LAMMPS_DUMP_VARS != 6)
        {
            EXCEPTION1(VisItException,
                       "Internal error: avtLAMMPSDumpFileFormat has "
                       "MAX_LAMMPS_DUMP_VARS mismatch.");
        }

        float tx, ty, tz, tv0, tv1, tv2, tv3, tv4, tv5;
        int index, ts;
        sscanf(buff,"%d %d %f %f %f %f %f %f %f %f %f",
               &index, &ts, &tx, &ty, &tz,
               &tv0, &tv1, &tv2, &tv3, &tv4, &tv5);
        s[timestep][index-1] = ts-1;
        x[timestep][index-1] = tx;
        y[timestep][index-1] = ty;
        z[timestep][index-1] = tz;
        v[0][timestep][index-1] = tv0;
        v[1][timestep][index-1] = tv1;
        v[2][timestep][index-1] = tv2;
        v[3][timestep][index-1] = tv3;
        v[4][timestep][index-1] = tv4;
        v[5][timestep][index-1] = tv5;
    }
}


// ****************************************************************************
//  Method:  avtLAMMPSDumpFileFormat::ReadMetaData
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
//  Creation:    February  9, 2009
//
// ****************************************************************************
void
avtLAMMPSDumpFileFormat::ReadAllMetaData()
{
    if (metaDataRead)
        return;

    OpenFileAtBeginning();

    char buff[1000];

    nAtoms = 0;
    nTimeSteps = 0;
    nVars = -1;

    while (in)
    {
        in.getline(buff,1000);
        if (strncmp(buff, "ITEM:", 5) != 0)
            continue;

        string item(&buff[6]);
        if (item == "TIMESTEP")
        {
            nTimeSteps++;
            in.getline(buff,1000);
            cycles.push_back(strtol(buff, NULL, 10));
        }
        else if (item == "NUMBER OF ATOMS")
        {
            in.getline(buff,1000);
            int n = strtol(buff, NULL, 10);
            if (nAtoms == 0)
                nAtoms = n;
            else
            {
                if (n != nAtoms)
                    EXCEPTION1(InvalidFilesException, filename.c_str());
            }
        }
        else if (item == "ATOMS")
        {
            istream::pos_type current_pos = in.tellg();
            file_positions.push_back(current_pos);
            if (nVars == -1)
            {
                in.getline(buff,1000);
                int index, ts;
                float tx, ty, tz, tv0, tv1, tv2, tv3, tv4, tv5;
                if (MAX_LAMMPS_DUMP_VARS != 6)
                {
                    EXCEPTION1(VisItException,
                               "Internal error: avtLAMMPSDumpFileFormat has "
                               "MAX_LAMMPS_DUMP_VARS mismatch.");
                }
                int n = sscanf(buff,"%d %d %f %f %f %f %f %f %f %f %f",
                               &index, &ts, &tx, &ty, &tz,
                               &tv0, &tv1, &tv2, &tv3, &tv4, &tv5);
                nVars = n - 5;
            }
        }
    }

    // don't read the meta data more than once
    metaDataRead = true;

    // resize our atom arrays
    s.resize(nTimeSteps);
    x.resize(nTimeSteps);
    y.resize(nTimeSteps);
    z.resize(nTimeSteps);
    for (int i=0; i<MAX_LAMMPS_DUMP_VARS; i++)
    {
        v[i].resize(nTimeSteps);
    }
    
}



// ****************************************************************************
//  Method:  avtLAMMPSDumpFileFormat::FileExtensionIdentify
//
//  Purpose:
//    Return true if the file given is an LAMMPS Dump file,
//    based on its file name.
//
//  Arguments:
//    filename   the filename
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  9, 2009
//
// ****************************************************************************
bool
avtLAMMPSDumpFileFormat::FileExtensionIdentify(const std::string &filename)
{
    int pos = filename.length()-1;
    while (pos>=0 && filename[pos]!='/' && filename[pos]!='\\')
        pos--;

    std::string fn;
    if (pos >= 0)
        fn = filename.substr(pos+1);
    else
        fn = filename;

    for (unsigned int i=0; i<fn.size(); i++)
    {
        if (fn[i]>='a' && fn[i]<='z')
            fn[i] = fn[i] + ('A'-'a');
    }

    if (fn.length()>=6 && fn.substr(fn.length()-5,5) == ".DUMP")
        return true;

    return false;
}

// ****************************************************************************
//  Method:  avtLAMMPSDumpFileFormat::FileContentsdentify
//
//  Purpose:
//    Return true if the file given is an LAMMPS Dump file.
//    based on its initial contents.  If the file starts with "ITEM:",
//    there's a good chance it's a dump file.
//
//  Arguments:
//    filename   the filename
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  9, 2009
//
// ****************************************************************************
bool
avtLAMMPSDumpFileFormat::FileContentsIdentify(const std::string &filename)
{
    ifstream in(filename.c_str());
    char buff[1000];
    in.getline(buff, 1000);
    in.close();
    if (strncmp(buff, "ITEM:", 5) == 0)
        return true;
    return false;
}

// ****************************************************************************
//  Method:  avtLAMMPSDumpFileFormat::CreateInterface
//
//  Purpose:
//    Create a file format interface from this reader.
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  9, 2009
//
// ****************************************************************************
avtFileFormatInterface *
avtLAMMPSDumpFileFormat::CreateInterface(const char *const *list,
                                     int nList, int)
{
    avtMTSDFileFormat **ffl = new avtMTSDFileFormat*[nList];
    for (int i = 0 ; i < nList ; i++)
    {
        ffl[i] = new avtLAMMPSDumpFileFormat(list[i]);
    }
    return new avtMTSDFileFormatInterface(ffl, nList);
}
