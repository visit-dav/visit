//                          avtOUTCARFileFormat.C                       //
// ************************************************************************* //

#include <avtOUTCARFileFormat.h>

#include <string>

#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVisItUtility.h>

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
//  Modifications:
//    Jeremy Meredith, Fri Apr 20 15:01:36 EDT 2007
//    Added support for magnetization fields.
//
//    Jeremy Meredith, Tue Mar 10 17:42:52 EDT 2009
//    Initialize potim.
//
// ****************************************************************************

avtOUTCARFileFormat::avtOUTCARFileFormat(const char *fn)
    : avtMTSDFileFormat(&fn, 1)
{
    filename = fn;
    OpenFileAtBeginning();

    metadata_read = false;
    has_magnetization = false;

    natoms = 0;
    ntimesteps = 0;
    potim = 1.0; // delta-t per timestep
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
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 13:31:51 EST 2007
//    If ntimesteps is zero, that means this is an intial-conditions-only
//    file and has no force variables.
//
//    Jeremy Meredith, Fri Apr 20 15:01:36 EDT 2007
//    Added support for magnetization fields.
//
//    Jeremy Meredith, Tue Feb 12 14:09:24 EST 2008
//    Support element types as an enumerated scalar.
//
//    Mark C. Miller, Mon Apr 14 15:41:21 PDT 2008
//    Changed interface to enum scalars
//
//    Jeremy Meredith, Tue Mar 10 17:43:05 EDT 2009
//    Added cycles and times.
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

    avtScalarMetaData *el_smd =
        new avtScalarMetaData("element", "mesh", AVT_NODECENT);
    el_smd->SetEnumerationType(avtScalarMetaData::ByValue);
    for (int i=0; i<element_types.size(); i++)
        el_smd->AddEnumNameValue(element_names[i], element_types[i]);
    md->Add(el_smd);

    if (ntimesteps != 0)
    {
        // If only initial consitions are saved, then we don't
        // have any force variables
        AddScalarVarToMetaData(md, "fx", "mesh", AVT_NODECENT);
        AddScalarVarToMetaData(md, "fy", "mesh", AVT_NODECENT);
        AddScalarVarToMetaData(md, "fz", "mesh", AVT_NODECENT);
    }
    if (has_magnetization)
    {
        AddScalarVarToMetaData(md, "mags", "mesh", AVT_NODECENT);
        AddScalarVarToMetaData(md, "magp", "mesh", AVT_NODECENT);
        AddScalarVarToMetaData(md, "magd", "mesh", AVT_NODECENT);
        AddScalarVarToMetaData(md, "magtot", "mesh", AVT_NODECENT);
    }
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

    vector<int> cycles;
    for (int i=0; i<ntimesteps; i++)
        cycles.push_back(i);
    md->SetCycles(cycles);
    md->SetCyclesAreAccurate(true);

    vector<double> times;
    for (int i=0; i<ntimesteps; i++)
        times.push_back(double(i) * potim);
    md->SetTimes(times);
    md->SetTimesAreAccurate(true);
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
//  Modifications:
//    Kathleen Bonnell, Mon Jul 14 16:01:32 PDT 2008
//    Specify curves as 1D rectilinear grids with y values stored in point data.
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


            int npts = partial ? ts+1 : ntimesteps;
            vtkRectilinearGrid *rg = 
                vtkVisItUtility::Create1DRGrid(npts, VTK_FLOAT);

            vtkFloatArray *xc = 
                vtkFloatArray::SafeDownCast(rg->GetXCoordinates());
            vtkFloatArray *yv = vtkFloatArray::New();
            yv->SetNumberOfComponents(1);
            yv->SetNumberOfTuples(npts);
            yv->SetName(meshname.c_str());
            for (int j = 0 ; j < npts ; j++)
            {
                xc->SetValue(j, (float) j);
                yv->SetValue(j, free_energy[j]);
            }
            rg->GetPointData()->SetScalars(yv);
            yv->Delete(); 
            return rg;
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
//  Modifications:
//    Jeremy Meredith, Fri Apr 20 15:01:36 EDT 2007
//    Added support for magnetization fields.
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

    if (has_magnetization && string(varname) == "mags")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(atoms.size());
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<natoms; i++)
        {
            ptr[i] = mags[i];
        }
        return scalars;
    }

    if (has_magnetization && string(varname) == "magp")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(atoms.size());
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<natoms; i++)
        {
            ptr[i] = magp[i];
        }
        return scalars;
    }

    if (has_magnetization && string(varname) == "magd")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(atoms.size());
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<natoms; i++)
        {
            ptr[i] = magd[i];
        }
        return scalars;
    }

    if (has_magnetization && string(varname) == "magtot")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(atoms.size());
        float *ptr = (float *) scalars->GetVoidPointer(0);
        for (int i=0; i<natoms; i++)
        {
            ptr[i] = magtot[i];
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
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 13:31:51 EST 2007
//    If ntimesteps is zero, that means this is an intial-conditions-only
//    file -- fake that ntimesteps is actually "1" when needed.
//
// ****************************************************************************
int
avtOUTCARFileFormat::GetNTimesteps(void)
{
    ReadAllMetaData();
    if (ntimesteps==0)
        return 1;
    else
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
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 13:31:51 EST 2007
//    If ntimesteps is zero, that means this is an intial-conditions-only
//    file -- fake that ntimesteps is actually "1" when needed.
//
//    Jeremy Meredith, Fri Feb 23 15:22:37 EST 2007
//    Added support for seeking directly to preset timesteps.
//    Sped up parsing by using C string functions.
//
//    Jeremy Meredith, Fri Apr 20 15:01:36 EDT 2007
//    Added support for magnetization fields.  These technically
//    could be calculated once per time step, but they are only
//    written out once as a final result.  As a further complexity
//    if they *are* written out more than once, we only want to
//    keep the last one.
//
//    Jeremy Meredith, Tue Feb  5 11:19:07 EST 2008
//    Add support for single-digit elements in TITEL line with underscored
//    suffixes.
//
//    Jeremy Meredith, Thu Jan 22 15:33:50 EST 2009
//    Changed the parsing of ions per type to allow for the fact that VASP
//    seems to use a 4-char fixed-width field for the number.  So, a line
//    like "ions per type =  40 5006000" should be parsed as resulting in
//    ion counts of 40, 500, and 6000.
//
//    Jeremy Meredith, Tue Mar 10 17:29:47 EDT 2009
//    Changed it to determine the width of the "ions per type" field
//    instead of assuming it's 4.  Starts with whitespace parsing now,
//    double-checks the count against the "NIONS" listed in the file, and
//    if that fails, goes to fixed-width.  If that still fails, it just
//    assumes all atoms are of the first species.
//    Also, read "POTIM" field used to calculate time values.
//
//    Jeremy Meredith, Tue Dec 29 13:47:35 EST 2009
//    Added some error checks.
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
    int nions_doublecheck = -1;

    ntimesteps = 0;
    while (in)
    {
        //string s(line);
        if (!strncmp(line,"  FREE ENERGIE OF THE ION-ELECTRON SYSTEM (eV)",46))
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
        else if (!strncmp(line," POSITION",9))
        {
            file_positions.push_back(in.tellg());
            ntimesteps++;
        }
        /*
          NOT SURE WHY, BUT THESE ARE IN THE WRONG ORDER AND NEGATIVE
          ... instead, use the version below this one
        else if (!strncmp(line,"  Lattice vectors:",18))
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
                 !strncmp(line,"      direct lattice vectors",28))
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
        else if (!strncmp(line," magnetization (x)",18))
        {
            // NOTE: we may hit this a number of times, but that's
            // okay, because we only want to keep the last one.

            // skip three lines
            in.getline(line, 132);
            in.getline(line, 132);
            in.getline(line, 132);

            int ion;
            mags.resize(natoms);
            magp.resize(natoms);
            magd.resize(natoms);
            magtot.resize(natoms);
            for (int i=0; i<natoms; i++)
            {
                in >> ion >> mags[i] >> magp[i] >> magd[i] >> magtot[i];
            }
            has_magnetization = true;
        }
        else if (!all_ions_read && !strncmp(line,"   TITEL",8))
        {
            istringstream sin(line);
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
                if (arg4[1] == '_')
                {
                    element[0] = arg4[0];
                    element[1] = 0;
                }
                else if (arg4[2] == '_')
                {
                    element[0] = arg4[0];
                    element[1] = arg4[1];
                    element[2] = 0;
                }
                else
                {
                    cerr << "ERROR: expected '_' in second or third position\n"
                         << "when parsing species name longer than 2 chars\n"
                         << "from TITEL line.\n";
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
        else if (!strncmp(line+58,"NIONS =",7))
        {
            istringstream sin(line+65);
            string arg1;
            sin >> arg1;
            nions_doublecheck = atoi(arg1.c_str());
        }
        else if (!all_ions_read && !strncmp(line,"   ions per type =",18))
        {
            all_ions_read = true;

            // Try the whitespace-delimited approach first
            natoms = 0;
            element_counts.clear();
            istringstream sin(&(line[18]));
            int n;
            sin >> n;
            while (sin)
            {
                natoms += n;
                element_counts.push_back(n);
                sin >> n;
            }

            // If that fails, try fixed-width
            if (nions_doublecheck>0 && natoms != nions_doublecheck)
            {
                cerr << "Warning: Got natoms="<<natoms<<" and nions="<<nions_doublecheck<<endl;
                cerr << "  Attempting fixed-width ions-per-type parsing.\n";
                natoms = 0;
                element_counts.clear();
                int index = 30;
                int len = strlen(line) - index;
                int count = element_types.size();
                int perSpecies = len/element_types.size();
                char tmp[100];
                for (int i=0; i<count; i++)
                {
                    int j = 0;
                    for ( ; j<perSpecies; j++)
                    {
                        tmp[j] = line[index];
                        index++;
                    }
                    tmp[j] = '\0';
                    int n = atoi(tmp);
                    natoms += n;
                    element_counts.push_back(n);
                }
            }

            // If that still fails, just fix it as best we can
            if (nions_doublecheck>0 && natoms != nions_doublecheck)
            {
                cerr << "Error: Got natoms="<<natoms<<" and nions="<<nions_doublecheck<<endl;
                cerr << "  Incorrectly assuming all atoms are of first species to fix it.\n";
                natoms = nions_doublecheck;
                element_counts.clear();
                element_counts.resize(element_types.size(), 0);
                element_counts[0] = natoms;
            }
        }
        else if (!strncmp(line,"   POTIM",8))
        {
            istringstream sin(line);
            string arg1,arg2,arg3;
            sin >> arg1 >> arg2 >> arg3;
            potim = strtod(arg3.c_str(), NULL);
        }


        in.getline(line, 132);
    }

    // error check
    if (natoms == 0)
        EXCEPTION2(InvalidFilesException, filename.c_str(),
                 "Got zero atoms; assuming it's not an OUTCAR format.");

    allatoms.resize(ntimesteps>0 ? ntimesteps : 1);
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
//  Modifications:
//
//    Hank Childs, Thu Aug 31 21:36:35 PDT 2006
//    AIX XLC compiler is finicky ... change variable name to make it happy.
//    It thinks variables declared in a for loop are at the same level as the
//    variables defined inside its block.  So if you declare var "v1" in the
//    for loop and inside the corresponding block, it claims "v1" was declared
//    twice.
//
//    Jeremy Meredith, Thu Feb 15 13:31:51 EST 2007
//    If ntimesteps is zero, that means this is an intial-conditions-only
//    file.  Read the atoms from a different location in the file.
//
//    Jeremy Meredith, Fri Feb 23 15:22:37 EST 2007
//    Added support for seeking directly to preset timesteps.
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

    if (ntimesteps > 0)
    {
        if (file_positions.size() > timestep)
        {
            in.seekg(file_positions[timestep]);
            in.getline(line, 132); // skip the separator
        }
        else
        {
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
        }

        atoms.resize(natoms);

        int index = 0;
        for (int et_index = 0; et_index < element_counts.size(); et_index++)
        {
            for (int a2=0; a2<element_counts[et_index]; a2++)
            {
                Atom &a = atoms[index];
                a.elementtype_index = et_index;
                in >> a.x  >> a.y  >> a.z;
                in >> a.fx >> a.fy >> a.fz;
                index++;
            }
        }
    }
    else
    {
        while (in)
        {
            in.getline(line, 132);
            string s(line);
            if (s.substr(0,42) == " position of ions in cartesian coordinates")
            {
                break;
            }
        }
        if (!in)
        {
            EXCEPTION1(InvalidFilesException, filename.c_str());            
        }

        atoms.resize(natoms);

        int index = 0;
        for (int et_index = 0; et_index < element_counts.size(); et_index++)
        {
            for (int a2=0; a2<element_counts[et_index]; a2++)
            {
                Atom &a = atoms[index];
                a.elementtype_index = et_index;
                in >> a.x  >> a.y  >> a.z;
                a.fx = a.fy = a.fz = 0.;
                index++;
            }
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
