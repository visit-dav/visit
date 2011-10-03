/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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
//                            avtXSFFileFormat.C                           //
// ************************************************************************* //

#include <avtXSFFileFormat.h>

#include <string>

#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>

#include <AtomicProperties.h>

#include <avtDatabaseMetaData.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <NonCompliantFileException.h>

#include <vtkTriangulationTables.h>

#include <sstream>

using std::istringstream;
using std::istringstream;
using std::string;
using std::vector;
using std::pair;
using std::istream;

// ****************************************************************************
//  Method: avtXSFFileFormat constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   September 28, 2011
//
// ****************************************************************************

avtXSFFileFormat::avtXSFFileFormat(const char *fn)
    : avtMTSDFileFormat(&fn, 1)
{
    filename = fn;
    OpenFileAtBeginning();

    metadata_read = false;
    hasforces = false;
    natoms = 0;
    ntimesteps = 0;
}


// ****************************************************************************
//  Method: avtXSFFileFormat constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   October  3, 2011
//
// ****************************************************************************

avtXSFFileFormat::~avtXSFFileFormat()
{
    in.close();
}


// ****************************************************************************
//  Method: avtXSFFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 28, 2011
//
// ****************************************************************************

int
avtXSFFileFormat::GetNTimesteps(void)
{
    ReadAllMetaData();
    return ntimesteps;
}


// ****************************************************************************
//  Method: avtXSFFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 28, 2011
//
// ****************************************************************************

void
avtXSFFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtXSFFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 28, 2011
//
// ****************************************************************************

void
avtXSFFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    ReadAllMetaData();

    if (natoms > 0)
    {
        avtMeshMetaData *mmd = new avtMeshMetaData("mesh", 1, 0,0,0,
                                                   3, 0,
                                                   AVT_POINT_MESH);
        mmd->nodesAreCritical = true;
        if (unitCell.size() > 0)
        {
            int index = 0;
            if (timeState < unitCell.size())
                index = timeState;
            for (int i=0; i<9; i++)
                mmd->unitCellVectors[i] = unitCell[index][i/3][i%3];
        }
        md->Add(mmd);

        // Add the element scalars
        avtScalarMetaData *el_smd =
            new avtScalarMetaData("element", "mesh", AVT_NODECENT);
        el_smd->SetEnumerationType(avtScalarMetaData::ByValue);
        for (int a=0; a<=MAX_ELEMENT_NUMBER; a++)
            el_smd->AddEnumNameValue(element_names[a], a);
        md->Add(el_smd);

        // If we had forces, add metadata for them, too
        if (hasforces)
        {
            AddScalarVarToMetaData(md, "fx", "mesh", AVT_NODECENT);
            AddScalarVarToMetaData(md, "fy", "mesh", AVT_NODECENT);
            AddScalarVarToMetaData(md, "fz", "mesh", AVT_NODECENT);

            Expression forcevec_expr;
            forcevec_expr.SetName("force");
            forcevec_expr.SetDefinition("{fx, fy, fz}");
            forcevec_expr.SetType(Expression::VectorMeshVar);
            md->AddExpression(&forcevec_expr);
        }
    }

    if (unitCell.size() > 0)
    {
        avtMeshMetaData *mmd_bbox = new avtMeshMetaData("unitCell", 1, 0,0,0,
                                                        3, 1,
                                                        AVT_UNSTRUCTURED_MESH);
        int index = 0;
        if (timeState < unitCell.size())
            index = timeState;
        for (int i=0; i<9; i++)
            mmd_bbox->unitCellVectors[i] = unitCell[index][i/3][i%3];
        md->Add(mmd_bbox);
    }


    for (int meshid = 0; meshid < allMeshes.size(); meshid++)
    {
        Mesh &m = allMeshes[meshid];
        avtMeshMetaData *mmd_grid = new avtMeshMetaData(m.name,
                                                        1, // 1 block
                                                        0,0,0,
                                                        m.dim, m.dim,
                                                        AVT_RECTILINEAR_MESH);

        // need unit cell vectors, origin
        for (int i=0; i<9; i++)
            mmd_grid->unitCellVectors[i] = m.ucv[i/3][i%3];
        for (int i=0; i<3; i++)
            mmd_grid->unitCellOrigin[i] = m.origin[i];

        // it's a transformed regular grid; set up the matrix similarly
        double *t = mmd_grid->rectilinearGridTransform;
        t[ 0]=m.ucv[0][0];t[ 1]=m.ucv[1][0];t[ 2]=m.ucv[2][0];t[ 3]=m.origin[0];
        t[ 4]=m.ucv[0][1];t[ 5]=m.ucv[1][1];t[ 6]=m.ucv[2][1];t[ 7]=m.origin[1];
        t[ 8]=m.ucv[0][2];t[ 9]=m.ucv[1][2];t[10]=m.ucv[2][2];t[11]=m.origin[2];
        t[12]=0;          t[13]=0;          t[14]=0;          t[15]=1;
        mmd_grid->rectilinearGridHasTransform = true;

        // need explicit extents, too
        double extents[6] = { DBL_MAX,-DBL_MAX,DBL_MAX,-DBL_MAX,DBL_MAX,-DBL_MAX };
        for (int i=0; i<=1; i++)
        {
            for (int j=0; j<=1; j++)
            {
                for (int k=0; k<=1; k++)
                {
                    for (int axis=0; axis<3; axis++)
                    {
                        double v = (m.origin[axis] +
                                    m.ucv[0][axis] * float(i) +
                                    m.ucv[1][axis] * float(j) +
                                    m.ucv[2][axis] * float(k));
                        if (extents[2*axis] > v)
                            extents[2*axis] = v;
                        if (extents[2*axis+1] < v)
                            extents[2*axis+1] = v;
                    }
                }
            }
        }
        mmd_grid->SetExtents(extents);

        // phew!  okay, add it now
        md->Add(mmd_grid);

        // and add all the variables
        for (int v = 0; v < m.var_names.size(); v++)
        {
            AddScalarVarToMetaData(md, m.var_names[v], m.name, AVT_NODECENT);
        }
    }
}


// ****************************************************************************
//  Method: avtXSFFileFormat::GetMesh
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
//  Creation:   September 28, 2011
//
// ****************************************************************************

vtkDataSet *
avtXSFFileFormat::GetMesh(int timestate, const char *name)
{
    ReadAllMetaData();

    string meshname(name);
    if (meshname == "unitCell")
    {
        // simple unit cell; create the edges of it as a (deformed) cube
        vtkPolyData *pd  = vtkPolyData::New();
        vtkPoints   *pts = vtkPoints::New();

        int index = 0;
        if (unitCell.size() > timestate)
            index = timestate;

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
                    x += unitCell[index][axis][0];
                    y += unitCell[index][axis][1];
                    z += unitCell[index][axis][2];
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
        // the mesh for our atoms
        ReadAtomsForTimestep(timestate);

        vtkPolyData *pd  = vtkPolyData::New();
        vtkPoints   *pts = vtkPoints::New();

        pts->SetNumberOfPoints(currentAtoms.size());
        pd->SetPoints(pts);
        pts->Delete();
        for (int j = 0 ; j < currentAtoms.size() ; j++)
        {
            pts->SetPoint(j,
                          currentAtoms[j].x,
                          currentAtoms[j].y,
                          currentAtoms[j].z);
        }
 
        vtkCellArray *verts = vtkCellArray::New();
        pd->SetVerts(verts);
        verts->Delete();
        for (int k = 0 ; k < currentAtoms.size() ; k++)
        {
            verts->InsertNextCell(1);
            verts->InsertCellPoint(k);
        }

        return pd;
    }
    else
    {
        // one of the grids; identify it in the list by name
        for (int m = 0 ; m < allMeshes.size(); m++)
        {
            Mesh &mesh = allMeshes[m];
            if (meshname == string(mesh.name))
            {
                vtkRectilinearGrid   *rgrid   = vtkRectilinearGrid::New(); 

                vtkFloatArray   *coords[3];
                for (int i = 0 ; i < 3 ; i++)
                {
                    coords[i] = vtkFloatArray::New();
                    coords[i]->SetNumberOfTuples(mesh.numnodes[i]);

                    for (int j = 0 ; j < mesh.numnodes[i] ; j++)
                    {
                        coords[i]->SetComponent(j, 0,
                                        float(j) / float(mesh.numnodes[i]-1) );
                    }
                }
                rgrid->SetDimensions(mesh.numnodes);
                rgrid->SetXCoordinates(coords[0]);
                rgrid->SetYCoordinates(coords[1]);
                rgrid->SetZCoordinates(coords[2]);
                coords[0]->Delete();
                coords[1]->Delete();
                coords[2]->Delete();

                return rgrid;
            }
        }
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtXSFFileFormat::GetVar
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
//  Creation:   September 28, 2011
//
// ****************************************************************************

vtkDataArray *
avtXSFFileFormat::GetVar(int timestate, const char *varname)
{
    ReadAllMetaData();
    ReadAtomsForTimestep(timestate);

    if (string(varname) == "element" ||
        string(varname) == "fx" ||
        string(varname) == "fy" ||
        string(varname) == "fz")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(natoms);
        float *ptr = (float *) scalars->GetVoidPointer(0);
        if (string(varname) == "element")
        {
            for (int i=0; i<natoms; i++)
                ptr[i] = currentAtoms[i].e;
        }
        else if (string(varname) == "fx")
        {
            for (int i=0; i<natoms; i++)
                ptr[i] = currentAtoms[i].fx;
        }
        else if (string(varname) == "fy")
        {
            for (int i=0; i<natoms; i++)
                ptr[i] = currentAtoms[i].fy;
        }
        else if (string(varname) == "fz")
        {
            for (int i=0; i<natoms; i++)
                ptr[i] = currentAtoms[i].fz;
        }

        return scalars;
    }

    for (int m = 0 ; m < allMeshes.size(); m++)
    {
        Mesh &mesh = allMeshes[m];
        for (int v = 0 ; v < mesh.var_names.size(); v++)
        {
            if (mesh.var_names[v] == varname)
            {
                int nvals = (mesh.numnodes[0] *
                             mesh.numnodes[1] *
                             mesh.numnodes[2]);
                vtkFloatArray *scalars = vtkFloatArray::New();
                scalars->SetNumberOfTuples(nvals);
                float *ptr = (float *) scalars->GetVoidPointer(0);

                // read it directly here
                OpenFileAtBeginning();
                in.seekg(mesh.var_filepos[v]);
                for (int i=0; i<nvals; i++)
                    in >> ptr[i];

                return scalars;
            }
        }
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtXSFFileFormat::GetVectorVar
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
//  Creation:   September 28, 2011
//
// ****************************************************************************

vtkDataArray *
avtXSFFileFormat::GetVectorVar(int timestate, const char *varname)
{
    ReadAllMetaData();

    return NULL;
}

// ****************************************************************************
//  Method:  avtXSFFileFormat::OpenFileAtBeginning
//
//  Purpose:
//    Opens the file, or else seeks to the beginning.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 28, 2011
//
// ****************************************************************************

void
avtXSFFileFormat::OpenFileAtBeginning()
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
//  Method:  avtXSFFileFormat::ReadAtomsForTimestep
//
//  Purpose:
//    read the atoms for the given time step
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 28, 2011
//
//  Modifications:
// ****************************************************************************
void
avtXSFFileFormat::ReadAtomsForTimestep(int ts)
{
    OpenFileAtBeginning();
    currentAtoms.clear();
    in.seekg(atom_filepos[ts]);
    currentAtoms.resize(natoms);
    for (int i=0; i<natoms; i++)
    {
        Atom &a = currentAtoms[i];
        string species;
        in >> species;
        a.e = atoi(species.c_str());
        if (a.e == 0)
            a.e = ElementNameToAtomicNumber(species.c_str());
        in >> a.x >> a.y >> a.z;
        if (hasforces)
            in >> a.fx >> a.fy >> a.fz;
    }
}

// ****************************************************************************
//  Method:  avtXSFFileFormat::ReadAllMetaData
//
//  Purpose:
//    scan the file, looking for and parsing out metadata
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 28, 2011
//
//  Modifications:
// ****************************************************************************
void
avtXSFFileFormat::ReadAllMetaData()
{
    if (metadata_read)
        return;

    metadata_read = true;
    OpenFileAtBeginning();

    // parsing data grids requires state:
    Mesh mesh;
    mesh.clear();

    // okay, let's see what we've got!
    char line[4096];
    in.getline(line, 4096);
    while (in)
    {
        istringstream sin(line);
        string keyword;
        sin >> keyword;
        if (keyword == "ATOMS")
        {
            EXCEPTION2(NonCompliantFileException, "XSF",
                       "Simple molecules (i.e. using the ATOMS keyword) "
                       "are not yet supported.  Please contact the "
                       "VisIt developers to request this feature.");
        }
        else if (keyword == "ANIMSTEPS")
        {
            EXCEPTION2(NonCompliantFileException, "XSF",
                       "Animation (i.e. using the ANIMSTEPS keyword) "
                       "is not yet supported.  Please contact the "
                       "VisIt developers to request this feature.");
        }
        else if (keyword == "CRYSTAL" ||
                 keyword == "SLAB" ||
                 keyword == "POLYMER" ||
                 keyword == "MOLECULE")
        {
            // Great!  
            //
            // We don't do anything with this newfound knowledge about
            // the periodicity of the unit cell.
            // (I'm pretty sure: crystal  = all 3 axes periodic
            //                   slab     = 2 periodic axes
            //                   polymer  = 1 periodic axis
            //                   molecule = no periodic axes)
            //
            // It also lets us know to expect e.g. PRIMVEC+PRIMCOORD
            // instead of ATOMS, but we'll discover that fact on our own
            // shortly.
            //
            // In other words, do nothing here.
        }
        else if (keyword == "PRIMVEC")
        {
            // "primary" lattice vectors; the spec says it will
            // generally use these, not the "conventional ones"
            UCV ucv;
            in >> ucv[0][0] >> ucv[0][1] >> ucv[0][2];
            in >> ucv[1][0] >> ucv[1][1] >> ucv[1][2];
            in >> ucv[2][0] >> ucv[2][1] >> ucv[2][2];
            unitCell.push_back(ucv);
            in.getline(line,4096); // get the remainder of that line
        }
        else if (keyword == "PRIMCOORD")
        {
            // and the "primary" coordinates to go with our 
            // "primary" lattice vectors
            in.getline(line, 4096);
            int must_be_one;
            int n = sscanf(line, "%d %d", &natoms, &must_be_one);
            if (n != 2)
                EXCEPTION2(NonCompliantFileException, "XSF",
                           "The two values on the line following PRIMCOORD "
                           "are expected to be the number of atoms, then 1. "
                           "We did not parse two integers here.");
            if (natoms <= 0)
                EXCEPTION2(NonCompliantFileException, "XSF",
                           "The two values on the line following PRIMCOORD "
                           "are expected to be the number of atoms, then 1. "
                           "We did not get a positive number of atoms.");
            if (must_be_one != 1)
                EXCEPTION2(NonCompliantFileException, "XSF",
                           "The two values on the line following PRIMCOORD "
                           "are expected to be the number of atoms, then 1. "
                           "The second of these two values was not 1.");

            // record the file positions of the start of the actual atom data
            atom_filepos.push_back(in.tellg());

            // figure out if we have forces in our atom lines
            in.getline(line,4096);
            char buff[4096];
            float x,y,z,fx,fy,fz;
            n = sscanf(line,"%s %f %f %f %f %f %f",buff,&x,&y,&z,&fx,&fy,&fz);
            if (n != 4 && n != 7)
                EXCEPTION2(NonCompliantFileException, "XSF",
                           "The first atom line did not parse into "
                           "either 4 (species + coord) "
                           "or 7 (species + coord + force) values.");
            bool forces_in_first_atom = (n==7) ? true : false;
            if (atom_filepos.size() == 1)
            {
                // first one we encounter; set the hasforces for the file
                hasforces = forces_in_first_atom;
            }
            else
            {
                // others: make sure it's consisitent throughout the file
                if (hasforces != forces_in_first_atom)
                    EXCEPTION2(NonCompliantFileException, "XSF",
                               "Some atom lines had forces, and some did not. "
                               "This must be consistent throughout the file.");
            }
        }
        else if (keyword == "CONVVEC")
        {
            // We ignore any CONVVEC and CONVCOORD information:
            // we don't use the conventional lattice vectors, and
            // the format tells us you have to specify PRIMCOORD
            // (not CONVCOORD) to make their code happy, so
            // it seems safe to ignore any CONVCOORD as well.
        }
        else if (keyword == "CONVCOORD")
        {
            // Ignore here, too -- see the above comment about CONVVEC.
        }
        else if (keyword.substr(0, 21) == "BEGIN_BLOCK_DATAGRID_")
        {
            // parse the dimension; the next chars will be "2D" or "3D"
            mesh.dim = int(line[21]) - int('0');
            if (mesh.dim < 2 || mesh.dim > 3)
            {
                EXCEPTION2(NonCompliantFileException, "XSF",
                           "Encountered a block datagrid with "
                           "dimensions other than 2 or 3.");                
            }
            in >> mesh.name;
            in.getline(line,4096); // skip remainder of mesh name line
        }
        else if (keyword.substr(0, 18) == "BEGIN_DATAGRID_2D_" ||
                 keyword.substr(0, 18) == "BEGIN_DATAGRID_3D_" ||
                 keyword.substr(0, 12) == "DATAGRID_2D_" ||
                 keyword.substr(0, 12) == "DATAGRID_3D_")
        {
            // The variable name follows the final underscore above.

            // Note: the spec says you must have "BEGIN_" here,
            // but PWSCF/ESPRESSO didn't think it was necessary.
            // Be nice and parse it either way....
            char *varname;
            if (keyword.substr(0,6) == "BEGIN_")
                varname = line+18;
            else
                varname = line+12;
            // All meshes have the same dims, origin, etc., but
            // they are specified for each variable.  So just
            // overwrite them whenever we get another variable.

            // read dims
            mesh.numnodes[2] = 1;
            if (mesh.dim == 2)
                in >> mesh.numnodes[0] >> mesh.numnodes[1];
            else
                in >> mesh.numnodes[0] >> mesh.numnodes[1] >> mesh.numnodes[2];
            // read origin
            in >> mesh.origin[0] >> mesh.origin[1] >> mesh.origin[2];
            // read i vector
            in >> mesh.ucv[0][0] >> mesh.ucv[0][1] >> mesh.ucv[0][2];
            // read j vector
            in >> mesh.ucv[1][0] >> mesh.ucv[1][1] >> mesh.ucv[1][2];
            if (mesh.dim == 2)
            {
                mesh.ucv[2][0] = 0;
                mesh.ucv[2][1] = 0;
                mesh.ucv[2][2] = 1;
            }
            else
            {
                // read k vector
                in >> mesh.ucv[2][0] >> mesh.ucv[2][1] >> mesh.ucv[2][2];
            }

            // save the name
            mesh.var_names.push_back(varname);
            // skip remainder of k vector line
            in.getline(line,4096);
            // save file position
            mesh.var_filepos.push_back(in.tellg());
        }
        else if (keyword.substr(0, 19) == "END_BLOCK_DATAGRID_")
        {
            // end the current block
            // add it to the list
            allMeshes.push_back(mesh);
            // clear out its data
            mesh.clear();
        }
        else
        {
            // Anything else, assume it's a comment and do nothing.
            // Sure, it would be nice if these all had hash marks
            // starting the line like the specification requires,
            // but the output from PWSCF/ESPRESSO seems to ignore that
            // little detail....
        }

        in.getline(line, 4096);
    }

    ntimesteps = atom_filepos.size();
}
