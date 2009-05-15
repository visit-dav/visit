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

#include <sstream>
using std::istringstream;


// ****************************************************************************
//  Method: avtLAMMPS constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   February  9, 2009
//
//  Modifications:
//    Jeremy Meredith, Mon May 11 16:55:53 EDT 2009
//    Added support for new, more arbitrary LAMMPS atom dump style formatting.
//    Includes bounds/unit cell, and an optional atom format string.
//
// ****************************************************************************

avtLAMMPSDumpFileFormat::avtLAMMPSDumpFileFormat(const char *fn)
    : avtMTSDFileFormat(&fn, 1)
{
    metaDataRead = false;
    filename = fn;
    xIndex = yIndex = zIndex = speciesIndex = idIndex = -1;
    xMin = xMax = yMin = yMax = zMin = zMax = 0;
    currentTimestep = -1;
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
//  Modifications:
//    Jeremy Meredith, Mon May 11 16:55:53 EDT 2009
//    Added support for new, more arbitrary LAMMPS atom dump style formatting.
//    Only keep one time step at a time to prevent memory bloat.
//
// ****************************************************************************

void
avtLAMMPSDumpFileFormat::FreeUpResources(void)
{
    for (int i=0; i<vars.size(); i++)
        vars[i].clear();
    vars.clear();
    varNames.clear();
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
//    Jeremy Meredith, Mon May 11 16:55:53 EDT 2009
//    Added support for new, more arbitrary LAMMPS atom dump style formatting.
//    Includes adding unit cell vectors.
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
    for (int i=0; i<9; i++)
        mmd->unitCellVectors[i] = 0;
    mmd->unitCellVectors[0] = xMax - xMin;
    mmd->unitCellVectors[4] = yMax - yMin;
    mmd->unitCellVectors[8] = zMax - zMin;
    md->Add(mmd);

    AddScalarVarToMetaData(md, "species", "mesh", AVT_NODECENT);
    for (int v=0; v<nVars; v++)
    {
        if (v == idIndex || v == speciesIndex)
            continue;
        AddScalarVarToMetaData(md, varNames[v], "mesh", AVT_NODECENT);
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
//    Jeremy Meredith, Mon May 11 16:55:53 EDT 2009
//    Added support for new, more arbitrary LAMMPS atom dump style formatting.
//    Includes support for scaled (unit cell) coordinates.
//
//    Jeremy Meredith, Fri May 15 11:00:49 EDT 2009
//    Fixed typo....
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
        double x = vars[xIndex][j];
        double y = vars[yIndex][j];
        double z = vars[zIndex][j];
        if (xScaled)
            x = xMin + (xMax-xMin) * x;
        if (yScaled)
            y = yMin + (yMax-yMin) * y;
        if (zScaled)
            z = zMin + (zMax-zMin) * z;
        pts->SetPoint(j, x, y, z);
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
//    Jeremy Meredith, Mon May 11 16:55:53 EDT 2009
//    Added support for new, more arbitrary LAMMPS atom dump style formatting.
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
            ptr[i] = speciesVar[i];
        }
        return scalars;
    }

    int varIndex = -1;
    for (int v=0; v<nVars; v++)
    {
        if (varNames[v] == varname)
        {
            varIndex = v;
            break;
        }
    }

    if (varIndex == -1)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    // and now create the data array for it
    vtkFloatArray *scalars = vtkFloatArray::New();
    scalars->SetNumberOfTuples(nAtoms);
    float *ptr = (float *) scalars->GetVoidPointer(0);
    for (int i=0; i<nAtoms; i++)
    {
        ptr[i] = vars[varIndex][i];
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
//  Modifications:
//    Jeremy Meredith, Mon May 11 16:55:53 EDT 2009
//    Added support for new, more arbitrary LAMMPS atom dump style formatting.
//
// ****************************************************************************
void
avtLAMMPSDumpFileFormat::ReadTimeStep(int timestep)
{
    ReadAllMetaData();

    // don't read this time step if it's already in memory
    if (currentTimestep == timestep)
        return;
    currentTimestep = timestep;

    OpenFileAtBeginning();
    in.seekg(file_positions[timestep]);

    speciesVar.resize(nAtoms);
    for (int v=0; v<vars.size(); v++)
    {
        // id and species are ints; don't bother with the float arrays for them
        if (v == idIndex || v == speciesIndex)
            continue;
        vars[v].resize(nAtoms);
    }

    vector<double> tmpVars(nVars);
    int tmpID, tmpSpecies;

    char buff[1000];
    // read all the atoms
    for (int a=0; a<nAtoms; a++)
    {
        in.getline(buff,1000);
        istringstream sin(buff);
        for (int v=0; v<nVars; v++)
        {
            if (v==speciesIndex)
                sin >> tmpSpecies;
            else if (v==idIndex)
                sin >> tmpID;
            else
                sin >> tmpVars[v];
        }
        --tmpID;  // 1-origin; we need 0-origin

        speciesVar[tmpID] = tmpSpecies;
        for (int v=0; v<nVars; v++)
        {
            if (v == idIndex || v == speciesIndex)
                continue;
            vars[v][tmpID] = tmpVars[v];
        }
        speciesVar[tmpID] = tmpSpecies;
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
//  Modifications:
//    Jeremy Meredith, Mon May 11 16:55:53 EDT 2009
//    Added support for new, more arbitrary LAMMPS atom dump style formatting.
//    Includes bounds/unit cell, and an optional atom format string.
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
        else if (item == "BOX BOUNDS")
        {
            in >> xMin >> xMax;
            in >> yMin >> yMax;
            in >> zMin >> zMax;
            in.getline(buff, 1000); // get rest of Z line
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
        else if (item.substr(0,5) == "ATOMS")
        {
            istream::pos_type current_pos = in.tellg();
            file_positions.push_back(current_pos);
            if (nVars == -1)
            {
                istringstream sin(&buff[11]);
                string varName;
                xScaled = yScaled = zScaled = false;
                while (sin >> varName)
                {
                    if (varName == "id")
                        idIndex = varNames.size();
                    else if (varName == "type")
                        speciesIndex = varNames.size();
                    else if (varName == "x" || varName == "xs")
                        xIndex = varNames.size();
                    else if (varName == "y" || varName == "ys")
                        yIndex = varNames.size();
                    else if (varName == "z" || varName == "zs")
                        zIndex = varNames.size();

                    if (varName == "xs")
                        xScaled = true;
                    if (varName == "ys")
                        yScaled = true;
                    if (varName == "zs")
                        zScaled = true;

                    varNames.push_back(varName);

                }
                nVars = varNames.size();
                if (nVars == 0)
                {
                    // OLD FORMAT: Assume "id type x y z"
                    varNames.push_back("id");
                    varNames.push_back("type");
                    varNames.push_back("x");
                    varNames.push_back("y");
                    varNames.push_back("z");
                    idIndex = 0;
                    speciesIndex = 1;
                    xIndex = 2; xScaled = false;
                    yIndex = 3; yScaled = false;
                    zIndex = 4; zScaled = false;
                    nVars = varNames.size();
                }
                vars.resize(nVars);
            }
        }
    }

    if (xIndex<0 || yIndex<0 || zIndex<0 || idIndex<0 || speciesIndex<0)
    {
        EXCEPTION2(InvalidFilesException,
                   filename,
                   "Didn't get indices for all necessary vars");
    }

    // don't read the meta data more than once
    metaDataRead = true;
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
