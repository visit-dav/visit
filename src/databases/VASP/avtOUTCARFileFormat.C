//                          avtOUTCARFileFormat.C                       //
// ************************************************************************* //

#include <avtOUTCARFileFormat.h>

#include <string>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>

#include <avtDatabaseMetaData.h>
#include <AtomicProperties.h>
#include <avtMTSDFileFormatInterface.h>

#include <Expression.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <sstream>

#include <vtkTriangulationTables.h>

#include <map>

using std::istringstream;
using std::string;
using std::map;
using std::vector;

// ****************************************************************************
//  Method: avtOUTCAR constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
// ****************************************************************************

avtOUTCARFileFormat::avtOUTCARFileFormat(const char *fn)
    : avtMTSDFileFormat(&fn, 1)
{
    filename = fn;
    OpenFileAtBeginning();

    metadata_read = false;

    natoms = 0;
    ntimesteps = 0;
    unitCell[0][0] = 1;    unitCell[0][1] = 0;    unitCell[0][2] = 0;
    unitCell[1][0] = 0;    unitCell[1][1] = 1;    unitCell[1][2] = 0;
    unitCell[2][0] = 0;    unitCell[2][1] = 0;    unitCell[2][2] = 1;
}


// ****************************************************************************
//  Method: avtOUTCARFileFormat::FreeUpResources
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
avtOUTCARFileFormat::FreeUpResources(void)
{
}

// ****************************************************************************
//  Method:  avtOUTCARFileFormat::OpenFileAtBeginning
//
//  Purpose:
//    Opens the file, or else seeks to the beginning.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2006
//
// ****************************************************************************

void
avtOUTCARFileFormat::OpenFileAtBeginning()
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
//  Method: avtOUTCARFileFormat::PopulateDatabaseMetaData
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
avtOUTCARFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int ts)
{
    ReadAllMetaData();

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

    AddScalarVarToMetaData(md, "element", "mesh", AVT_NODECENT);
    AddScalarVarToMetaData(md, "fx", "mesh", AVT_NODECENT);
    AddScalarVarToMetaData(md, "fy", "mesh", AVT_NODECENT);
    AddScalarVarToMetaData(md, "fz", "mesh", AVT_NODECENT);
    //md->Add(new avtLabelMetaData("elementname", "mesh", AVT_NODECENT));

    Expression forcevec_expr;
    forcevec_expr.SetName("force");
    forcevec_expr.SetDefinition("{fx, fy, fz}");
    forcevec_expr.SetType(Expression::VectorMeshVar);
    md->AddExpression(&forcevec_expr);

    avtCurveMetaData *cmd1 = new avtCurveMetaData("curves/full/energy");
    md->Add(cmd1);
    avtCurveMetaData *cmd2 = new avtCurveMetaData("curves/partial/energy");
    md->Add(cmd2);
}


// ****************************************************************************
//  Method: avtOUTCARFileFormat::GetMesh
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
avtOUTCARFileFormat::GetMesh(int ts, const char *name)
{
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

        ReadAllMetaData();
        ReadAtomsForTimestep(ts);

        vector<Atom> &atoms = allatoms[ts];

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
    else // curves
    {
        if (meshname == "curves/full/energy" ||
            meshname == "curves/partial/energy")
        {
            bool partial = meshname.substr(0,15)=="curves/partial/";

            vtkPolyData *pd  = vtkPolyData::New();
            vtkPoints   *pts = vtkPoints::New();

            int npts = partial ? ts+1 : ntimesteps;

            pts->SetNumberOfPoints(npts);
            pd->SetPoints(pts);
            pts->Delete();
            for (int j = 0 ; j < npts ; j++)
            {
                pts->SetPoint(j,
                              j, free_energy[j], 0);
            }
 
            vtkCellArray *verts = vtkCellArray::New();
            pd->SetVerts(verts);
            verts->Delete();
            for (int k = 1 ; k < npts ; k++)
            {
                verts->InsertNextCell(2);
                verts->InsertCellPoint(k-1);
                verts->InsertCellPoint(k);
            }

            return pd;
        }
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtOUTCARFileFormat::GetVar
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
avtOUTCARFileFormat::GetVar(int ts, const char *varname)
{
    ReadAllMetaData();
    ReadAtomsForTimestep(ts);

    vector<Atom> &atoms = allatoms[ts];

    if (string(varname) == "element")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(atoms.size());
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<natoms; i++)
        {
            ptr[i] = element_types[atoms[i].elementtype_index];
        }
        return scalars;
    }

    if (string(varname) == "fx")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(atoms.size());
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<natoms; i++)
        {
            ptr[i] = atoms[i].fx;
        }
        return scalars;
    }

    if (string(varname) == "fy")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(atoms.size());
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<natoms; i++)
        {
            ptr[i] = atoms[i].fy;
        }
        return scalars;
    }

    if (string(varname) == "fz")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(atoms.size());
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<natoms; i++)
        {
            ptr[i] = atoms[i].fz;
        }
        return scalars;
    }

    /*
    if (string(varname) == "elementname")
    {
        vtkUnsignedCharArray *labels = vtkUnsignedCharArray::New();
        labels->SetNumberOfComponents(3);
        labels->SetNumberOfTuples(atoms.size());
        char *cptr = (char *)labels->GetVoidPointer(0);
        for (int i=0; i<natoms; i++)
        {
            // NO WONT WORK IT'S A STRING: memcpy(cptr, element_names[atoms[i].elementtype_index].c_str(), 3);
            cptr += 3;
        }
        return labels;       
    }
    */

    return NULL;
}


// ****************************************************************************
//  Method: avtOUTCARFileFormat::GetVectorVar
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
avtOUTCARFileFormat::GetVectorVar(int ts, const char *varname)
{
    return NULL;
}

// ****************************************************************************
//  Method:  avtOUTCARFileFormat::GetNTimesteps
//
//  Purpose:
//    return the number of timesteps
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2006
//
// ****************************************************************************
int
avtOUTCARFileFormat::GetNTimesteps(void)
{
    ReadAllMetaData();
    return ntimesteps;
}

// ****************************************************************************
//  Method:  avtOUTCARFileFormat::ReadAllMetaData
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
avtOUTCARFileFormat::ReadAllMetaData()
{
    if (metadata_read)
        return;

    OpenFileAtBeginning();

    metadata_read = true;

    char line[132];
    in.getline(line, 132);

    bool read_lattice = false;
    bool all_ions_read = false;

    ntimesteps = 0;
    while (in)
    {
        string s(line);
        if (s.substr(0,46) ==
            "  FREE ENERGIE OF THE ION-ELECTRON SYSTEM (eV)")
        {
            string tmp = "";
            while (tmp != "=")
            {
                in >> tmp;
            }
            float energy;
            in >> energy;
            free_energy.push_back(energy);
        }
        if (s.substr(0,9) == " POSITION")
        {
            ntimesteps++;
        }
        /*
          NOT SURE WHY, BUT THESE ARE IN THE WRONG ORDER AND NEGATIVE
          ... instead, use the version below this one
        else if (s.substr(0,18) == "  Lattice vectors:")
        {
            // skip a line
            in.getline(line, 132);

            in.getline(line, 132);
            s = line;
            unitCell[0][0] = atof(s.substr( 7,15).c_str());
            unitCell[0][1] = atof(s.substr(23,15).c_str());
            unitCell[0][2] = atof(s.substr(39,15).c_str());

            in.getline(line, 132);
            s = line;
            unitCell[1][0] = atof(s.substr( 7,15).c_str());
            unitCell[1][1] = atof(s.substr(23,15).c_str());
            unitCell[1][2] = atof(s.substr(39,15).c_str());

            in.getline(line, 132);
            s = line;
            unitCell[2][0] = atof(s.substr( 7,15).c_str());
            unitCell[2][1] = atof(s.substr(23,15).c_str());
            unitCell[2][2] = atof(s.substr(39,15).c_str());
        }*/
        else if (read_lattice == false &&
                 s.substr(0,28) == "      direct lattice vectors")
        {
            float tmp;
            in >> unitCell[0][0];
            in >> unitCell[0][1];
            in >> unitCell[0][2];
            in >> tmp >> tmp >> tmp;
            in >> unitCell[1][0];
            in >> unitCell[1][1];
            in >> unitCell[1][2];
            in >> tmp >> tmp >> tmp;
            in >> unitCell[2][0];
            in >> unitCell[2][1];
            in >> unitCell[2][2];
            in >> tmp >> tmp >> tmp;
            read_lattice = true;
        }
        else if (s.substr(0,8) == "   TITEL" && !all_ions_read)
        {
            istringstream sin(s);
            string arg1,arg2,arg3,arg4;
            sin >> arg1 >> arg2 >> arg3 >> arg4;
            char element[3];
            if (arg4.length() == 1)
            {
                element[0] = arg4[0];
                element[1] = 0;
            }
            else if (arg4.length() == 2)
            {
                if (arg4[1] == '_')
                {
                    element[0] = arg4[0];
                    element[1] = 0;
                }
                else
                {
                    element[0] = arg4[0];
                    element[1] = arg4[1];
                    element[2] = 0;
                }
            }
            else if (arg4.length() > 2)
            {
                if (arg4[2] != '_')
                {
                    // ERROR: expected "_" in third position
                    cerr << "ERROR: expected '_' in third position\n";
                }
                else
                {
                    element[0] = arg4[0];
                    element[1] = arg4[1];
                    element[2] = 0;
                }
            }
            else
            {
                // ERROR: didn't read anything for element type
                cerr << "ERROR: didn't read anything for element type\n";
            }

            int number = ElementNameToAtomicNumber(element);
            if (number <= 0)
            {
                // ERROR: not a match
                cerr << "ERROR: element '"<<element<<"' didn't exist\n";
            }
            element_names.push_back(element);
            element_types.push_back(number);
        }
        else if (s.substr(0,18) == "   ions per type =" && !all_ions_read)
        {
            all_ions_read = true;
            istringstream sin(s.substr(18));
            int n;
            sin >> n;
            while (sin)
            {
                natoms += n;
                element_counts.push_back(n);
                sin >> n;
            }
        }

        in.getline(line, 132);
    }

    allatoms.resize(ntimesteps);
}


// ****************************************************************************
//  Method:  avtOUTCARFileFormat::ReadAtomsForTimestep
//
//  Purpose:
//    Read the atoms at a given time step.
//
//  Arguments:
//    timestep   the timestep to read
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2006
//
// ****************************************************************************
void
avtOUTCARFileFormat::ReadAtomsForTimestep(int timestep)
{
    OpenFileAtBeginning();

    if (allatoms[timestep].size() > 0)
        return;

    vector<Atom> &atoms = allatoms[timestep];    

    char line[132];
    in.getline(line, 132);

    int curtime = -1;
    while (in && curtime < timestep)
    {
        string s(line);
        if (s.substr(0,9) == " POSITION")
        {
            curtime++;
        }
        in.getline(line, 132);
    }

    // skip the separator

    atoms.resize(natoms);

    int index = 0;
    for (int et_index = 0; et_index < element_counts.size(); et_index++)
    {
        for (int a=0; a<element_counts[et_index]; a++)
        {
            Atom &a = atoms[index];
            a.elementtype_index = et_index;
            in >> a.x  >> a.y  >> a.z;
            in >> a.fx >> a.fy >> a.fz;
            index++;
        }
    }
}

// ****************************************************************************
//  Method:  avtOUTCARFileFormat::Identify
//
//  Purpose:
//    Return true if the file given is an OUTCAR VASP file.
//    So far, only check based on the filename.
//
//  Arguments:
//    filename   the filename
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2006
//
// ****************************************************************************
bool
avtOUTCARFileFormat::Identify(const std::string &filename)
{
    int pos = filename.length()-1;
    while (pos>=0 && filename[pos]!='/' && filename[pos]!='\\')
        pos--;

    std::string fn;
    if (pos >= 0)
        fn = filename.substr(pos+1);
    else
        fn = filename;

    for (int i=0; i<fn.size(); i++)
    {
        if (fn[i]>='a' && fn[i]<='z')
            fn[i] = fn[i] + ('A'-'a');
    }

    if (fn.length()>=6 && fn.substr(0,6) == "OUTCAR")
        return true;

    return false;
}


// ****************************************************************************
//  Method:  avtOUTCARFileFormat::CreateInterface
//
//  Purpose:
//    Create a file format interface from this reader.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2006
//
// ****************************************************************************
avtFileFormatInterface *
avtOUTCARFileFormat::CreateInterface(const char *const *list,
                                     int nList, int)
{
    avtMTSDFileFormat **ffl = new avtMTSDFileFormat*[nList];
    for (int i = 0 ; i < nList ; i++)
    {
        ffl[i] = new avtOUTCARFileFormat(list[i]);
    }
    return new avtMTSDFileFormatInterface(ffl, nList);
}
