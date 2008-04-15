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
//                            avtPOSCARFileFormat.C                           //
// ************************************************************************* //

#include <avtPOSCARFileFormat.h>

#include <float.h>
#include <string>

#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>

#include <avtDatabaseMetaData.h>
#include <avtSTSDFileFormatInterface.h>
#include <AtomicProperties.h>

#include <Expression.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <sstream>

#include <vtkTriangulationTables.h>

using     std::string;

// ****************************************************************************
//  Method: avtPOSCAR constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   January  8, 2008
//
//  Modifications:
//
// ****************************************************************************

avtPOSCARFileFormat::avtPOSCARFileFormat(const char *fn)
    : avtSTSDFileFormat(fn)
{
    filename = fn;
    OpenFileAtBeginning();

    file_read = false;

    natoms = 0;

    unitCell[0][0] = 1;    unitCell[0][1] = 0;    unitCell[0][2] = 0;
    unitCell[1][0] = 0;    unitCell[1][1] = 1;    unitCell[1][2] = 0;
    unitCell[2][0] = 0;    unitCell[2][1] = 0;    unitCell[2][2] = 1;
}


// ****************************************************************************
//  Method: avtPOSCARFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Jeremy Meredith
//  Creation:   January  8, 2008
//
//  Modifications:
//
// ****************************************************************************

void
avtPOSCARFileFormat::FreeUpResources(void)
{
    x.clear();
    y.clear();
    z.clear();
    species.clear();
    element_map.clear();
    file_read = false;
}


// ****************************************************************************
//  Method: avtPOSCARFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Jeremy Meredith
//  Creation:   January  8, 2008
//
//  Modifications:
//    Jeremy Meredith, Tue Feb 12 14:09:24 EST 2008
//    Support element types as an enumerated scalar.
//
//    Mark C. Miller, Mon Apr 14 15:41:21 PDT 2008
//    Changed interface to enum scalars
// ****************************************************************************

void
avtPOSCARFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    ReadFile();

    avtMeshMetaData *mmd = new avtMeshMetaData("mesh", 1, 0,0,0,
                                               3, 0,
                                               AVT_POINT_MESH);
    mmd->nodesAreCritical = true;
    for (int i=0; i<9; i++)
    {
        mmd->unitCellVectors[i] = unitCell[i/3][i%3];
    }
    md->Add(mmd);

    avtMeshMetaData *mmd_bbox = new avtMeshMetaData("unitCell", 1, 0,0,0,
                                                    3, 1,
                                                    AVT_UNSTRUCTURED_MESH);
    for (int i=0; i<9; i++)
    {
        mmd_bbox->unitCellVectors[i] = unitCell[i/3][i%3];
    }
    md->Add(mmd_bbox);

    AddScalarVarToMetaData(md, "species", "mesh", AVT_NODECENT);
    if (element_map.size() > 0)
    {
        avtScalarMetaData *el_smd =
            new avtScalarMetaData("element", "mesh", AVT_NODECENT);
        el_smd->SetEnumerationType(avtScalarMetaData::ByValue);
        for (int i=0; i<element_map.size(); i++)
            el_smd->AddEnumNameValue(element_names[element_map[i]-1],element_map[i]);
        md->Add(el_smd);
    }
    if (cx.size() > 0)
        AddScalarVarToMetaData(md, "cx", "mesh", AVT_NODECENT);
    if (cy.size() > 0)
        AddScalarVarToMetaData(md, "cy", "mesh", AVT_NODECENT);
    if (cz.size() > 0)
        AddScalarVarToMetaData(md, "cz", "mesh", AVT_NODECENT);
}


// ****************************************************************************
//  Method: avtPOSCARFileFormat::GetMesh
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
//  Programmer: Jeremy Meredith
//  Creation:   January  8, 2008
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtPOSCARFileFormat::GetMesh(const char *name)
{
    ReadFile();

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
                    x += unitCell[axis][0];
                    y += unitCell[axis][1];
                    z += unitCell[axis][2];
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

        pts->SetNumberOfPoints(natoms);
        pd->SetPoints(pts);
        pts->Delete();
        for (int j = 0 ; j < natoms ; j++)
        {
            pts->SetPoint(j, x[j], y[j], z[j]);
        }
 
        vtkCellArray *verts = vtkCellArray::New();
        pd->SetVerts(verts);
        verts->Delete();
        for (int k = 0 ; k < natoms ; k++)
        {
            verts->InsertNextCell(1);
            verts->InsertCellPoint(k);
        }

        return pd;
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtPOSCARFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   January  8, 2008
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtPOSCARFileFormat::GetVar(const char *varname)
{
    ReadFile();

    if (string(varname) == "species")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(natoms);
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<natoms; i++)
        {
            ptr[i] = species[i];
        }
        return scalars;
    }

    if (string(varname) == "element")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(natoms);
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<natoms; i++)
        {
            ptr[i] = element_map[species[i]];
        }
        return scalars;
    }

    if (string(varname) == "cx")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(natoms);
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<natoms; i++)
        {
            ptr[i] = cx[i];
        }
        return scalars;
    }

    if (string(varname) == "cy")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(natoms);
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<natoms; i++)
        {
            ptr[i] = cy[i];
        }
        return scalars;
    }

    if (string(varname) == "cz")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(natoms);
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<natoms; i++)
        {
            ptr[i] = cz[i];
        }
        return scalars;
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtPOSCARFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   January  8, 2008
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtPOSCARFileFormat::GetVectorVar(const char *varname)
{
    return NULL;
}


// ****************************************************************************
//  Method:  avtPOSCARFileFormat::ReadFile
//
//  Purpose:
//    Read the metadata and atoms.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  8, 2008
//
//  Modifications:
//
// ****************************************************************************
void
avtPOSCARFileFormat::ReadFile()
{
    if (file_read)
        return;

    file_read = true;

    char line[132];

    // this line is the comment line; we can scan it for
    // an equals sign and attempt to populate the element map
    // with the values following it.
    in.getline(line, 132);
    int equalspos = -1;
    for (int p=0; p<strlen(line); p++)
    {
        if (line[p] == '=')
            equalspos = p;
    }
    if (equalspos >= 0)
    {
        string elem_map_line(&(line[equalspos+1]));
        std::istringstream elem_map_in(elem_map_line);
        int tmp;
        while (elem_map_in >> tmp)
        {
            element_map.push_back(tmp);
        }
    }

    // Read the scale and lattice vectors
    double scale;
    double lat[3][3];
    in >> scale;
    in >> lat[0][0] >> lat[0][1] >> lat[0][2];
    in >> lat[1][0] >> lat[1][1] >> lat[1][2];
    in >> lat[2][0] >> lat[2][1] >> lat[2][2];

    for (int i=0; i<3; i++)
        for (int j=0; j<3; j++)
            unitCell[i][j] = scale*lat[i][j];

    in.getline(line, 132); // skip rest of the last lattice line

    // get atom counts
    in.getline(line, 132);
    string atomcountline(line);

    natoms = 0;
    int tmp;
    std::istringstream count_in(atomcountline);
    while (count_in >> tmp)
    {
        species_counts.push_back(tmp);
        natoms += tmp;
    }

    // next line is either Selective dynamics or Cartesian/Direct
    bool selective_dynamics = false;
    in.getline(line, 132);
    if (line[0] == 's' || line[0] == 'S')
    {
        selective_dynamics = true;
        // if it was Selective dynamics, then Cartesian/Direct is the next one
        in.getline(line, 132);
    }

    bool cartesian = false;
    if (line[0] == 'c' || line[0] == 'C' || line[0] == 'k' || line[0] == 'K')
    {
        cartesian = true;
    }

    // read the atoms
    x.resize(natoms);
    y.resize(natoms);
    z.resize(natoms);
    species.resize(natoms);
    if (selective_dynamics)
    {
        cx.resize(natoms);
        cy.resize(natoms);
        cz.resize(natoms);
    }

    int species_index = 0;
    int species_count = 0;
    for (int i=0; i<natoms; i++)
    {
        in.getline(line,132);
        string atomline(line);
        std::istringstream atom_in(atomline);

        double tx, ty, tz;
        atom_in >> tx >> ty >> tz;
        if (cartesian)
        {
            x[i] = tx * scale;
            y[i] = ty * scale;
            z[i] = tz * scale;
        }
        else
        {
            x[i] = tx*unitCell[0][0] + ty*unitCell[1][0] + tz*unitCell[2][0];
            y[i] = tx*unitCell[0][1] + ty*unitCell[1][1] + tz*unitCell[2][1];
            z[i] = tx*unitCell[0][2] + ty*unitCell[1][2] + tz*unitCell[2][2];
        }
        if (selective_dynamics)
        {
            char tmp;
            atom_in >> tmp;
            cx[i] = (tmp=='t' || tmp=='T');
            atom_in >> tmp;
            cy[i] = (tmp=='t' || tmp=='T');
            atom_in >> tmp;
            cz[i] = (tmp=='t' || tmp=='T');
        }
        species[i] = species_index;
        species_count++;
        if (species_count >= species_counts[species_index])
        {
            species_index++;
            species_count = 0;
        }
    }

    // If we tried to create an element map, but it did not have the
    // same number of values as the species counts, it is invalid.
    if (element_map.size() > 0 && element_map.size() != species_counts.size())
    {
        element_map.clear();
    }

    in.close();
}

// ****************************************************************************
//  Method:  avtPOSCARFileFormat::Identify
//
//  Purpose:
//    Return true if the file given is a POSCAR VASP file.
//    So far, only check based on the filename.
//
//  Arguments:
//    filename   the filename
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  8, 2008
//
// ****************************************************************************
bool
avtPOSCARFileFormat::Identify(const std::string &filename)
{
    // strip off all leading paths
    int pos = filename.length()-1;
    while (pos>=0 && filename[pos]!='/' && filename[pos]!='\\')
        pos--;

    std::string fn;
    if (pos >= 0)
        fn = filename.substr(pos+1);
    else
        fn = filename;

    // uppercase it
    for (int i=0; i<fn.size(); i++)
    {
        if (fn[i]>='a' && fn[i]<='z')
            fn[i] = fn[i] + ('A'-'a');
    }

    for (int i=0; i<=fn.length()-3; i++)
    {
        if (fn.substr(i,3) == "POS")
            return true;
    }

    return false;
}

// ****************************************************************************
//  Method:  avtPOSCARFileFormat::CreateInterface
//
//  Purpose:
//    Create a file format interface from this reader.
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  8, 2008
//
//  Modifications:
//
// ****************************************************************************
avtFileFormatInterface *
avtPOSCARFileFormat::CreateInterface(const char *const *list,
                                     int nList, int nBlock)
{
    avtSTSDFileFormat ***ffl = new avtSTSDFileFormat**[nList];
    int nTimestep = nList / nBlock;
    for (int i = 0 ; i < nTimestep ; i++)
    {
        ffl[i] = new avtSTSDFileFormat*[nBlock];
        for (int j = 0 ; j < nBlock ; j++)
        {
            ffl[i][j] = new avtPOSCARFileFormat(list[i*nBlock + j]);
        }
    }
    return new avtSTSDFileFormatInterface(ffl, nTimestep, nBlock);
}


// ****************************************************************************
//  Method:  avtPOSCARFileFormat::OpenFileAtBeginning
//
//  Purpose:
//    Opens the file, or else seeks to the beginning.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  8, 2008
//
// ****************************************************************************
void
avtPOSCARFileFormat::OpenFileAtBeginning()
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
