/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                            avtPuReMDFileFormat.C                           //
// ************************************************************************* //

#include <avtPuReMDFileFormat.h>

#include <string>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCellArray.h>
#include <vtkDoubleArray.h>
#include <vtkPolyData.h>

#include <avtDatabaseMetaData.h>
#include <AtomicProperties.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>
#include <vtkTriangulationTables.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>


using std::string;
using std::pair;
using std::set;


// ****************************************************************************
//  Method: avtPuReMDFileFormat constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 16, 2010
//
// ****************************************************************************

avtPuReMDFileFormat::avtPuReMDFileFormat(const char *fn)
    : avtMTSDFileFormat(&fn, 1)
{
    filename = fn;
    OpenFileAtBeginning();

    metaDataRead = false;

    nAtoms = 0;
    nTimeSteps = 0;
    currentTimestep = -1;
}


// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 16, 2010
//
// ****************************************************************************

int
avtPuReMDFileFormat::GetNTimesteps(void)
{
    ReadAllMetaData();
    return nTimeSteps;
}


// ****************************************************************************
//  Method: avtPuReMDFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 16, 2010
//
// ****************************************************************************

void
avtPuReMDFileFormat::FreeUpResources(void)
{
    x.clear();
    y.clear();
    z.clear();
    vx.clear();
    vy.clear();
    vz.clear();
    fx.clear();
    fy.clear();
    fz.clear();
    q.clear();
    bonds.clear();
    currentTimestep = -1;
}


// ****************************************************************************
//  Method: avtPuReMDFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 16, 2010
//
// ****************************************************************************

void
avtPuReMDFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int ts)
{
    ReadAllMetaData();

    avtMeshMetaData *mmd = new avtMeshMetaData("mesh", 1, 0,0,0,
                                               3, 0,
                                               AVT_POINT_MESH);
    mmd->nodesAreCritical = true;
    if (unitCell.size() > (size_t)ts)
    {
        for (int i=0; i<9; i++)
        {
            mmd->unitCellVectors[i] = unitCell[ts][i/3][i%3];
        }
    }
    md->Add(mmd);

    avtMeshMetaData *mmd_bbox = new avtMeshMetaData("unitCell", 1, 0,0,0,
                                                    3, 1,
                                                    AVT_UNSTRUCTURED_MESH);
    if (unitCell.size() > (size_t)ts)
    {
        for (int i=0; i<9; i++)
        {
            mmd_bbox->unitCellVectors[i] = unitCell[ts][i/3][i%3];
        }
    }
    md->Add(mmd_bbox);


    if (atomFormat == AF_POS_VEL_Q || atomFormat == AF_POS_VEL_FORCE_Q)
    {
        AddScalarVarToMetaData(md, "vx", "mesh", AVT_NODECENT);
        AddScalarVarToMetaData(md, "vy", "mesh", AVT_NODECENT);
        AddScalarVarToMetaData(md, "vz", "mesh", AVT_NODECENT);
        Expression velocity_expr;
        velocity_expr.SetName("velocity");
        velocity_expr.SetDefinition("{vx, vy, vz}");
        velocity_expr.SetType(Expression::VectorMeshVar);
        md->AddExpression(&velocity_expr);
    }
    if (atomFormat == AF_POS_FORCE_Q || atomFormat == AF_POS_VEL_FORCE_Q)
    {
        AddScalarVarToMetaData(md, "fx", "mesh", AVT_NODECENT);
        AddScalarVarToMetaData(md, "fy", "mesh", AVT_NODECENT);
        AddScalarVarToMetaData(md, "fz", "mesh", AVT_NODECENT);
        Expression force_expr;
        force_expr.SetName("force");
        force_expr.SetDefinition("{fx, fy, fz}");
        force_expr.SetType(Expression::VectorMeshVar);
        md->AddExpression(&force_expr);
    }

    AddScalarVarToMetaData(md, "q", "mesh", AVT_NODECENT);

    avtScalarMetaData *el_smd =
        new avtScalarMetaData("element", "mesh", AVT_NODECENT);
    el_smd->SetEnumerationType(avtScalarMetaData::ByValue);
    for (set<int>::iterator it = existingElements.begin();
         it != existingElements.end(); it++)
    {
        el_smd->AddEnumNameValue(element_names[*it], *it);
    }
    md->Add(el_smd);

    AddScalarVarToMetaData(md, "species", "mesh", AVT_NODECENT);
    AddScalarVarToMetaData(md, "weight", "mesh", AVT_NODECENT);
}


// ****************************************************************************
//  Method: avtPuReMDFileFormat::GetMesh
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
//  Creation:   August 16, 2010
//
// ****************************************************************************

vtkDataSet *
avtPuReMDFileFormat::GetMesh(int ts, const char *name)
{
    ReadTimeStep(ts);

    string meshname(name);
    if (meshname == "unitCell")
    {
        vtkPolyData *pd  = vtkPolyData::New();
        vtkPoints   *pts = vtkPoints::New();

        pts->SetNumberOfPoints(8);
        pd->SetPoints(pts);
        pts->Delete();
        for (int j = 0 ; j < 8 ; j++)
        {
            float x=0,y=0,z=0;
            for (int axis=0; axis<3; axis++)
            {
                if (j & (1<<axis))
                {
                    x += unitCell[ts][axis][0];
                    y += unitCell[ts][axis][1];
                    z += unitCell[ts][axis][2];
                }
            }
            pts->SetPoint(j, x,y,z);
        }
 
        vtkCellArray *lines = vtkCellArray::New();
        pd->SetLines(lines);
        lines->Delete();
        for (int k = 0 ; k < 12 ; k++)
        {
            lines->InsertNextCell(2);
            lines->InsertCellPoint(voxVerticesFromEdges[k][0]);
            lines->InsertCellPoint(voxVerticesFromEdges[k][1]);
        }

        return pd;
    }
    else if (meshname == "mesh")
    {
        vtkPolyData *pd  = vtkPolyData::New();
        vtkPoints   *pts = vtkPoints::New();

        pts->SetNumberOfPoints(nAtoms);
        pd->SetPoints(pts);
        pts->Delete();
        for (int j = 0 ; j < nAtoms ; j++)
        {
            pts->SetPoint(j, x[j], y[j], z[j]);
        }
 
        vtkCellArray *verts = vtkCellArray::New();
        pd->SetVerts(verts);
        verts->Delete();
        for (int k = 0 ; k < nAtoms ; k++)
        {
            verts->InsertNextCell(1);
            verts->InsertCellPoint(k);
        }

        vtkCellArray *lines = vtkCellArray::New();
        pd->SetLines(lines);
        int nb = bonds.size();
        for (int k = 0 ; k < nb ; k++)
        {
            lines->InsertNextCell(2);
            lines->InsertCellPoint(bonds[k].first);
            lines->InsertCellPoint(bonds[k].second);
        }

        return pd;
    }
    
    return NULL;
}


// ****************************************************************************
//  Method: avtPuReMDFileFormat::GetVar
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
//  Creation:   August 16, 2010
//
// ****************************************************************************

vtkDataArray *
avtPuReMDFileFormat::GetVar(int timestate, const char *varname)
{
    if (string(varname) == "vx" ||
        string(varname) == "vy" ||
        string(varname) == "vz" ||
        string(varname) == "fx" ||
        string(varname) == "fy" ||
        string(varname) == "fz" ||
        string(varname) == "q" ||
        string(varname) == "element" ||
        string(varname) == "species" ||
        string(varname) == "weight")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(nAtoms);
        float *ptr = (float *) scalars->GetVoidPointer(0);
        if (string(varname) == "vx")
        {
            for (int i=0; i<nAtoms; i++)
                ptr[i] = vx[i];
        }
        else if (string(varname) == "vy")
        {
            for (int i=0; i<nAtoms; i++)
                ptr[i] = vy[i];
        }
        else if (string(varname) == "vz")
        {
            for (int i=0; i<nAtoms; i++)
                ptr[i] = vz[i];
        }
        else if (string(varname) == "fx")
        {
            for (int i=0; i<nAtoms; i++)
                ptr[i] = fx[i];
        }
        else if (string(varname) == "fy")
        {
            for (int i=0; i<nAtoms; i++)
                ptr[i] = fy[i];
        }
        else if (string(varname) == "fz")
        {
            for (int i=0; i<nAtoms; i++)
                ptr[i] = fz[i];
        }
        else if (string(varname) == "q")
        {
            for (int i=0; i<nAtoms; i++)
                ptr[i] = q[i];
        }
        else if (string(varname) == "element")
        {
            for (int i=0; i<nAtoms; i++)
                ptr[i] = atomElement[i];
        }
        else if (string(varname) == "species")
        {
            for (int i=0; i<nAtoms; i++)
                ptr[i] = atomSpecies[i];
        }
        else // (string(varname) == "weight")
        {
            for (int i=0; i<nAtoms; i++)
                ptr[i] = atomWeight[i];
        }
        return scalars;
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtPuReMDFileFormat::GetVectorVar
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
//  Creation:   August 16, 2010
//
// ****************************************************************************

vtkDataArray *
avtPuReMDFileFormat::GetVectorVar(int timestate, const char *varname)
{
    return NULL;
}



// ****************************************************************************
// Method:  avtPuReMDFileFormat::OpenFileAtBeginning
//
// Purpose:
//   Either opens the file, or resets it to the beginning position.
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    August 12, 2010
//
// ****************************************************************************
void
avtPuReMDFileFormat::OpenFileAtBeginning()
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
// Method:  avtPuReMDFileFormat::ReadAllMetaData
//
// Purpose:
//   Parse the file and read the meta-data.
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    August 16, 2010
//
// ****************************************************************************
void
avtPuReMDFileFormat::ReadAllMetaData()
{
    if (metaDataRead)
        return;

    OpenFileAtBeginning();

    char line[4096];
    string buff;

    nTimeSteps = 0;

    //
    // main file header
    //
    while (in.good() && !in.eof())
    {
        in.getline(line,4096);
        if (strncmp(line, "chars_to_skip_section:", 22) == 0)
            break;

        if (strncmp(line, "number_of_atoms:", 16) == 0)
        {
            nAtoms = atoi(line+16);
        }
        else if (strncmp(line, "atom_info:", 10) == 0)
        {
            if (strstr(line+10, "none"))
            {
                EXCEPTION2(InvalidFilesException, filename.c_str(),
                           "No atom info in file");
            }
            else if (strstr(line+10, "invalid"))
            {
                EXCEPTION2(InvalidFilesException, filename.c_str(),
                           "Atom info in file claimed to be invalid");
            }
            else if (strstr(line+10, "xyz_q"))
                atomFormat = AF_POS_Q;
            else if (strstr(line+10, "xyz_q_fxfyfz"))
                atomFormat = AF_POS_FORCE_Q;
            else if (strstr(line+10, "xyz_q_vxvyvz"))
                atomFormat = AF_POS_VEL_Q;
            else if (strstr(line+10, "detailed_atom_info"))
                atomFormat = AF_POS_VEL_FORCE_Q;
            else
            {
                EXCEPTION2(InvalidFilesException, filename.c_str(),
                           "Unknown atom info in file");
            }
        }
        else if (strncmp(line, "bond_info:", 10) == 0)
        {
            if (!strstr(line+10, "basic_bond_info"))
            {
                EXCEPTION2(InvalidFilesException, filename.c_str(),
                           "Only basic_bond_info is currently supported.");
            }
        }
    }

    //
    // main atom definitions
    //
    if (nAtoms == 0)
        EXCEPTION2(InvalidFilesException, filename.c_str(), "Found no atoms");
    atomElement.resize(nAtoms);
    atomSpecies.resize(nAtoms);
    atomWeight.resize(nAtoms);
    for (int i=0; i<nAtoms; i++)
    {
        if (!in.good() || in.eof())
            EXCEPTION2(InvalidFilesException, filename.c_str(),
                       "Premature EOF or file read error");
    
        int index;
        int species;
        string symbol;
        double weight;
        in >> index >> species >> symbol >> weight;
        int element = ElementNameToAtomicNumber(symbol.c_str());
        if (element <= 0)
        {
            cerr << "ERROR: element symbol '"<<symbol<<"' not found.\n";
            element = 0;
        }
        if (index <= 0 || index > nAtoms)
        {
            EXCEPTION2(InvalidFilesException, filename.c_str(),
                       "Bad index in atom definition section");
        }
        atomElement[index-1] = element;
        atomWeight[index-1]  = weight;
        atomSpecies[index-1] = species;
        existingElements.insert(element);
    }
    in.getline(line,4096); // skip end of last line

    //
    // each timestep
    //
    in.getline(line,4096);
    if (strncmp(line, "chars_to_skip_frame_header:", 27) != 0)
        EXCEPTION2(InvalidFilesException, filename.c_str(),
                   "Bad format, expected frame header");
    while (in.good() && !in.eof())
    {
        // frame header
        UCV ucv;
        double time_in_ps = 0;
        int step = 0;
        while (in.good() && !in.eof())
        {
            in.getline(line,4096);
            if (strncmp(line, "chars_to_skip_section:", 22) == 0)
                break;

            if (strncmp(line, "step:", 5) == 0)
                step = atoi(line+5);
            else if (strncmp(line, "time_in_ps:", 11) == 0)
                time_in_ps = strtod(line+11,NULL);
            else if (strncmp(line, "box_dimensions:", 15) == 0)
            {
                double bx, by, bz;
                if (sscanf(line+15, "%lf,%lf,%lf", &bx,&by,&bz) != 3)
                    EXCEPTION2(InvalidFilesException, filename.c_str(),
                               "Bad format, couldn't parse box dimensions");
                ucv[0][0] = bx;
                ucv[1][1] = by;
                ucv[2][2] = bz;
            }
            else if (strncmp(line, "coordinate_angles:", 18) == 0)
            {
                double a1, a2, a3;
                if (sscanf(line+18, "%lf,%lf,%lf", &a1,&a2,&a3) != 3)
                    EXCEPTION2(InvalidFilesException, filename.c_str(),
                               "Bad format, couldn't parse coordinate angles");
                if (a1 != 90 || a2 != 90 || a3 != 90)
                {
                    cerr << "Warning: angles in file were not 90,90,90.  "
                         << "This is not yet supported.  We will continue, "
                         << "but assume an orthonormal unit cell.\n";
                }
            }
        }
        times.push_back(time_in_ps);
        cycles.push_back(step);
        unitCell.push_back(ucv);

        // atoms
        filePositions.push_back(in.tellg());
        for (int i=0; i<nAtoms; i++)
        {
            in.getline(line,4096);
            if (!in.good() || in.eof())
                EXCEPTION2(InvalidFilesException, filename.c_str(),
                           "Premature EOF or file read error");
        }

        // bonds
        in.getline(line,4096);
        if (strncmp(line, "chars_to_skip_section:", 22) != 0)
            EXCEPTION2(InvalidFilesException, filename.c_str(),
                       "Bad format, expected section header");

        in.getline(line,4096);
        int nb = 0;
        while (strncmp(line, "chars_to_skip_section:", 22) != 0)
        {
            in.getline(line,4096);
            if (!in.good() || in.eof())
                EXCEPTION2(InvalidFilesException, filename.c_str(),
                           "Premature EOF or file read error");
            nb++;
        }
        nBonds.push_back(nb);

        // angles
        in.getline(line,4096);
        while (strncmp(line, "chars_to_skip_frame_header:", 22) != 0)
        {
            in.getline(line,4096);
            if (in.eof())
                break;
            if (in.bad())
                EXCEPTION2(InvalidFilesException, filename.c_str(),
                           "File read error");
        }

        nTimeSteps++;
    }

    if (nTimeSteps == 0)
        EXCEPTION2(InvalidFilesException, filename.c_str(), "Found no timesteps");

    // don't read the meta data more than once
    metaDataRead = true;
}

// ****************************************************************************
//  Method:  avtPuReMDFileFormat::ReadTimestep
//
//  Purpose:
//    Read the atoms and bonds at a given time step.
//
//  Arguments:
//    timestep   the timestep to read
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 16, 2010
//
//  Modifications:
//
// ****************************************************************************
void
avtPuReMDFileFormat::ReadTimeStep(int ts)
{
    ReadAllMetaData();

    // don't read this time step if it's already in memory
    if (currentTimestep == ts)
        return;
    currentTimestep = ts;

    OpenFileAtBeginning();
    in.seekg(filePositions[ts]);

    // make room
    x.resize(nAtoms);
    y.resize(nAtoms);
    z.resize(nAtoms);
    if (atomFormat == AF_POS_VEL_Q || atomFormat == AF_POS_VEL_FORCE_Q)
    {
        vx.resize(nAtoms);
        vy.resize(nAtoms);
        vz.resize(nAtoms);
    }
    if (atomFormat == AF_POS_FORCE_Q || atomFormat == AF_POS_VEL_FORCE_Q)
    {
        fx.resize(nAtoms);
        fy.resize(nAtoms);
        fz.resize(nAtoms);
    }
    q.resize(nAtoms);

    bonds.resize(nBonds[ts]);

    // read atoms
    for (int i=0 ; i<nAtoms; i++)
    {
        int index;
        in >> index;
        in >> x[index-1];
        in >> y[index-1];
        in >> z[index-1];
        if (atomFormat == AF_POS_VEL_Q || atomFormat == AF_POS_VEL_FORCE_Q)
        {
            in >> vx[index-1];
            in >> vy[index-1];
            in >> vz[index-1];
        }
        if (atomFormat == AF_POS_FORCE_Q || atomFormat == AF_POS_VEL_FORCE_Q)
        {
            in >> fx[index-1];
            in >> fy[index-1];
            in >> fz[index-1];
        }
        in >> q[index-1];
    }

    char line[4096];
    in.getline(line,4096); // skip rest of last atom line

    // read bonds
    in.getline(line,4096); // skip bonds section header line
    for (int i=0 ; i<nBonds[ts]; i++)
    {
        int a, b;
        double dist;
        double othervalue;
        in >> a >> b >> dist >> othervalue;
        bonds[i].first  = a-1; // 1-origin
        bonds[i].second = b-1; // 1-origin
    }

}

// ****************************************************************************
// Method:  avtPuReMDFileFormat::GetCycles
//
// Purpose:
//   Return cycle values for each timestep.
//
// Programmer:  Jeremy Meredith
// Creation:    August 16, 2010
//
// ****************************************************************************
void
avtPuReMDFileFormat::GetCycles(std::vector<int> &cycleOut)
{
    for (int i=0; i<nTimeSteps; i++)
        cycleOut.push_back(cycles[i]);
}

// ****************************************************************************
// Method:  avtPuReMDFileFormat::GetTimes
//
// Purpose:
//   Returns time values for each timestep.
//
// Programmer:  Jeremy Meredith
// Creation:    August 16, 2010
//
// ****************************************************************************
void
avtPuReMDFileFormat::GetTimes(std::vector<double> &timeOut)
{
    for (int i=0; i<nTimeSteps; i++)
        timeOut.push_back(times[i]);
}

