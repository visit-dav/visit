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
//    Jeremy Meredith, Wed May 20 11:16:46 EDT 2009
//    nAtoms is now allowed to change at each timestep.
//
// ****************************************************************************

vtkDataSet *
avtXYZFileFormat::GetMesh(int timestep, const char *meshname)
{
    ReadTimeStep(timestep);

    vtkPolyData *pd  = vtkPolyData::New();
    vtkPoints   *pts = vtkPoints::New();

    pts->SetNumberOfPoints(nAtoms[timestep]);
    pd->SetPoints(pts);
    pts->Delete();
    for (int j = 0 ; j < nAtoms[timestep] ; j++)
    {
        pts->SetPoint(j,
                      x[timestep][j],
                      y[timestep][j],
                      z[timestep][j]);
    }
 
    vtkCellArray *verts = vtkCellArray::New();
    pd->SetVerts(verts);
    verts->Delete();
    for (int k = 0 ; k < nAtoms[timestep] ; k++)
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
//    Jeremy Meredith, Wed May 20 11:17:01 EDT 2009
//    nAtoms is now allowed to change at each timestep.
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
        scalars->SetNumberOfTuples(nAtoms[timestep]);
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<nAtoms[timestep]; i++)
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
    scalars->SetNumberOfTuples(nAtoms[timestep]);
    float *ptr = (float *) scalars->GetVoidPointer(0);
    for (int i=0; i<nAtoms[timestep]; i++)
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
//  Modifications:
//    Jeremy Meredith, Mon May 11 13:09:39 EDT 2009
//    Allow numeric first column for atomic number instead of element symbol.
//
//    Jeremy Meredith, Wed May 20 11:17:19 EDT 2009
//    Added support for CrystalMakers more "human-friendly" (but
//    less computer friendly) style of XYZ file.
//    nAtoms is now allowed to change at each timestep.
//    Added checking for invalid atomic numbers.
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

    e[timestep].resize(nAtoms[timestep]);
    x[timestep].resize(nAtoms[timestep]);
    y[timestep].resize(nAtoms[timestep]);
    z[timestep].resize(nAtoms[timestep]);
    for (int i=0; i<MAX_XYZ_VARS; i++)
    {
        v[i][timestep].resize(nAtoms[timestep]);
    }

    char buff[1000];

    // skip the header lines
    int nheaderlines = (fileStyle == Normal) ? 2 : 10;
    for (int l=0; l<nheaderlines; l++)
        in.getline(buff,1000);

    // read all the atoms
    for (int a=0; a<nAtoms[timestep]; a++)
    {
        in.getline(buff,1000);
        char element[100];
        if (MAX_XYZ_VARS != 6)
        {
            EXCEPTION1(VisItException,
                       "Internal error: avtXYZFileFormat has "
                       "MAX_XYZ_VARS mismatch.");
        }

        if (fileStyle == Normal)
        {
            sscanf(buff,"%s %f %f %f %f %f %f %f %f %f",
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
        }
        else // (fileStyle == CrystalMaker)
        {
            char label[100];
            float xs,ys,zs;
            sscanf(buff,"%s %s %f %f %f %f %f %f %f %f %f %f %f %f",
                   element,
                   label,
                   &xs,
                   &ys,
                   &zs,
                   &x[timestep][a],
                   &y[timestep][a],
                   &z[timestep][a],
                   &v[0][timestep][a],
                   &v[1][timestep][a],
                   &v[2][timestep][a],
                   &v[3][timestep][a],
                   &v[4][timestep][a],
                   &v[5][timestep][a]);
        }

        e[timestep][a] = ElementNameToAtomicNumber(element);
        if (e[timestep][a] <= 0)
            e[timestep][a] = atoi(element);
        if (e[timestep][a] < 0 || e[timestep][a] > MAX_ELEMENT_NUMBER)
            e[timestep][a] = 0; // not valid; 0==unknown
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
//  Modifications:
//    Jeremy Meredith, Wed May 20 11:17:19 EDT 2009
//    Added support for CrystalMakers more "human-friendly" (but
//    less computer friendly) style of XYZ file.
//    nAtoms is now allowed to change at each timestep.
//
// ****************************************************************************
void
avtXYZFileFormat::ReadAllMetaData()
{
    if (metaDataRead)
        return;

    OpenFileAtBeginning();

    char buff[1000];

    fileStyle = Normal;
    nTimeSteps = 0;
    nVars = 0;
    while (in)
    {
        // get the current file position
        istream::pos_type current_pos = in.tellg();

        // get the first line
        in.getline(buff,1000);

        // this is the first read to return EOF after the last timestep
        if (!in)
            break;

        // record the position as the start of this timestep
        file_positions.push_back(current_pos);

        // we expect the first line to be a number for a simple XYZ file
        int natoms_tmp = atoi(buff);

        // if that failed, it's not a simple XYZ file
        if (natoms_tmp != 0)
        {
            // Simple XYZ file: we just skip the next line (molecule name)
            // before the atoms actually start
            in.getline(buff,1000);
        }
        else
        {
            // Nope, not a simple file!
            // We'll make an exception here for CrystalMaker's files,
            // which start with:
            //   "This file generated by CrystalMaker <version> <c> etc...."
            // and then the unit cell stuff, and then more header info
            // and a different content structure.  Yuck.
            in.seekg(current_pos, ios::beg);
            in.getline(buff,1000);
            if (strncmp(buff, "This file generated by CrystalMaker ", 36) != 0)
            {
                // No, not CrystalMaker either.  Fail!
                EXCEPTION1(InvalidFilesException, filename.c_str());
            }
            fileStyle = CrystalMaker;

            // next lines: 
            in.getline(buff,1000); // blank
            in.getline(buff,1000); // unit cell header
            in.getline(buff,1000); // a/b/c unit cell values
            in.getline(buff,1000); // alpha/beta/gamma unit cell euler? angles
            in.getline(buff,1000); // blank

            // now the atoms, so conveniently buried within a string of text:
            in >> buff;            // "Total"
            in >> buff;            // "of"
            in >> natoms_tmp;      // <number>
            in.getline(buff,1000); // "atoms in range" <endline>

            // more header junk:
            in.getline(buff,1000); // blank
            in.getline(buff,1000); // first header line
            in.getline(buff,1000); // second header line

            // finally, the data follows.  Make sure we actually got a good
            // number of atoms, though!
            if (natoms_tmp == 0)
            {
                // we were deceived
                EXCEPTION1(InvalidFilesException, filename.c_str());
            }
        }

        nAtoms.push_back(natoms_tmp);

        // in the first time step, get the number of vars.
        if (nTimeSteps == 0)
        {
            in.getline(buff,1000);
            char element[100];
            char label[100]; // crystalmaker only
            float xs,ys,zs; // fractional coords; crystalmaker only
            float x,y,z,v0,v1,v2,v3,v4,v5;
            if (MAX_XYZ_VARS != 6)
            {
                EXCEPTION1(VisItException,
                           "Internal error: avtXYZFileFormat has "
                           "MAX_XYZ_VARS mismatch.");
            }

            if (fileStyle == Normal)
            {
                int n = sscanf(buff,"%s %f %f %f %f %f %f %f %f %f",
                           element,&x,&y,&z,&v0,&v1,&v2,&v3,&v4,&v5);
                nVars = n-4;
            }
            else // (fileStyle == CrystalMaker)
            {
                int n = sscanf(buff,"%s %s %f %f %f %f %f %f %f %f %f %f %f %f",
                               element,label,&xs,&ys,&zs,&x,&y,&z,
                                                   &v0,&v1,&v2,&v3,&v4,&v5);
                nVars = n-8;
            }

            // skip the remainin atoms
            for (int a=1; a<natoms_tmp; a++)
            {
                in.getline(buff,1000);
            }
        }
        else
        {
            // skip the remainin atoms
            for (int a=0; a<natoms_tmp; a++)
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
