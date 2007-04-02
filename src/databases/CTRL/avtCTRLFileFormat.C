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
//                             avtCTRLFileFormat.C                           //
// ************************************************************************* //

#include <avtCTRLFileFormat.h>

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

#include <map>

using std::string;
using std::map;


// ****************************************************************************
//  Method: avtCTRL constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
// ****************************************************************************

avtCTRLFileFormat::avtCTRLFileFormat(const char *fn)
    : avtSTSDFileFormat(fn)
{
    filename = fn;
    OpenFileAtBeginning();

    metadata_read = false;

    alat = 1.0;
    unitCell[0][0] = 1;    unitCell[0][1] = 0;    unitCell[0][2] = 0;
    unitCell[1][0] = 0;    unitCell[1][1] = 1;    unitCell[1][2] = 0;
    unitCell[2][0] = 0;    unitCell[2][1] = 0;    unitCell[2][2] = 1;
}


// ****************************************************************************
//  Method: avtCTRLFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
// ****************************************************************************

void
avtCTRLFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method:  avtCTRLFileFormat::OpenFileAtBeginning
//
//  Purpose:
//    Either open the file or seek to the beginning
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2006
//
// ****************************************************************************
void
avtCTRLFileFormat::OpenFileAtBeginning()
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
//  Method: avtCTRLFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
// ****************************************************************************

void
avtCTRLFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    ReadAllMetaData();

    avtMeshMetaData *mmd = new avtMeshMetaData("mesh", 1, 0,0,0,
                                               3, 1,
                                               AVT_POINT_MESH);
    mmd->nodesAreCritical = true;
    for (int i=0; i<9; i++)
    {
        mmd->unitCellVectors[i] = unitCell[i/3][i%3];
    }
    md->Add(mmd);

    AddScalarVarToMetaData(md, "element", "mesh", AVT_NODECENT);
    md->Add(new avtLabelMetaData("elementname", "mesh", AVT_NODECENT));
}


// ****************************************************************************
//  Method: avtCTRLFileFormat::GetMesh
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
//  Creation:   August 29, 2006
//
// ****************************************************************************

vtkDataSet *
avtCTRLFileFormat::GetMesh(const char *meshname)
{
    ReadAllMetaData();
    ReadAtoms();

    int natoms = atoms.size();

    vtkPolyData *pd  = vtkPolyData::New();
    vtkPoints   *pts = vtkPoints::New();

    pts->SetNumberOfPoints(atoms.size());
    pd->SetPoints(pts);
    pts->Delete();
    for (int j = 0 ; j < atoms.size() ; j++)
    {
        pts->SetPoint(j,
                      atoms[j].x,
                      atoms[j].y,
                      atoms[j].z);
    }
 
    vtkCellArray *verts = vtkCellArray::New();
    pd->SetVerts(verts);
    verts->Delete();
    for (int k = 0 ; k < atoms.size() ; k++)
    {
        verts->InsertNextCell(1);
        verts->InsertCellPoint(k);
    }


    return pd;
}


// ****************************************************************************
//  Method: avtCTRLFileFormat::GetVar
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
//  Creation:   August 29, 2006
//
// ****************************************************************************

vtkDataArray *
avtCTRLFileFormat::GetVar(const char *varname)
{
    ReadAllMetaData();


    if (string(varname) == "element")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(atoms.size());
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<atoms.size(); i++)
        {
            ptr[i] = atoms[i].atomicnumber;
        }
        return scalars;
    }

    if (string(varname) == "elementname")
    {
        vtkUnsignedCharArray *labels = vtkUnsignedCharArray::New();
        labels->SetNumberOfComponents(3);
        labels->SetNumberOfTuples(atoms.size());
        char *cptr = (char *)labels->GetVoidPointer(0);
        for (int i=0; i<atoms.size(); i++)
        {
            memcpy(cptr, atoms[i].element, 3);
            cptr += 3;
        }
        return labels;       
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtCTRLFileFormat::GetVectorVar
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
//  Creation:   August 29, 2006
//
// ****************************************************************************

vtkDataArray *
avtCTRLFileFormat::GetVectorVar(const char *varname)
{
    return NULL;
}


// ****************************************************************************
//  Method:  avtCTRLFileFormat::ReadAllMetaData
//
//  Purpose:
//    scan the file, looking for and parsing out metadata
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2006
//
// ****************************************************************************
void
avtCTRLFileFormat::ReadAllMetaData()
{
    if (metadata_read)
        return;

    OpenFileAtBeginning();

    metadata_read = true;

    char line[132];
    in.getline(line, 132);

    while (in && string(line).substr(0,5) != string("STRUC"))
    {
        in.getline(line, 132);
    }
    if (!in)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }
    while (in)
    {
        string s(line);
        string n = s.substr(10,4);
        if (n == "ALAT")
        {
            alat = atof(s.substr(15).c_str());
        }
        else if (n == "PLAT")
        {
            unitCell[0][0] = alat*atof(s.substr(15,10).c_str());
            unitCell[0][1] = alat*atof(s.substr(26,10).c_str());
            unitCell[0][2] = alat*atof(s.substr(37,10).c_str());

            in.getline(line, 132);
            s = line;
            unitCell[1][0] = alat*atof(s.substr(15,10).c_str());
            unitCell[1][1] = alat*atof(s.substr(26,10).c_str());
            unitCell[1][2] = alat*atof(s.substr(37,10).c_str());

            in.getline(line, 132);
            s = line;
            unitCell[2][0] = alat*atof(s.substr(15,10).c_str());
            unitCell[2][1] = alat*atof(s.substr(26,10).c_str());
            unitCell[2][2] = alat*atof(s.substr(37,10).c_str());
        }

        in.getline(line, 132);
        if (line[0] != ' ')
            break;
    }
}


// ****************************************************************************
//  Method:  avtCTRLFileFormat::ReadAtoms
//
//  Purpose:
//    Read the atoms from the file
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2006
//
// ****************************************************************************
void
avtCTRLFileFormat::ReadAtoms()
{
    OpenFileAtBeginning();

    if (atoms.size() > 0)
        return;

    char line[132];
    in.getline(line, 132);

    while (in && string(line).substr(0,5) != string("CLASS"))
    {
        in.getline(line, 132);
    }
    if (!in)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    map<string, int> atommap;
    while (in)
    {
        string s(line);
        string e = s.substr(15,2);
        string z = s.substr(20,2);
        cerr << "'"<<e<<"' => '"<<atoi(z.c_str())<<"'\n";
        atommap[e] = atoi(z.c_str());

        in.getline(line, 132);
        if (line[0] != ' ')
            break;
    }

    while (in && string(line).substr(0,4) != string("SITE"))
    {
        in.getline(line, 132);
    }
    if (!in)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    while (in)
    {
        string s(line);
        string e = s.substr(15,2);
        string x = s.substr(22,11);
        string y = s.substr(34,11);
        string z = s.substr(46,11);

        Atom a;
        a.atomicnumber = atommap[e];
        if (a.atomicnumber != 0)
        {
            a.atomicnumber--;
            a.element[0] = e[0];
            a.element[1] = e[1];
            a.element[2] = '\0';
            a.x = atof(x.c_str()) * alat;
            a.y = atof(y.c_str()) * alat;
            a.z = atof(z.c_str()) * alat;
            atoms.push_back(a);
        }

        in.getline(line, 132);
        if (line[0] != ' ')
            break;
    }
}
