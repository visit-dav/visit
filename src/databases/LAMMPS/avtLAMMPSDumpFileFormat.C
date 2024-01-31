// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtLAMMPSDumpFileFormat.C                           //
// ************************************************************************* //

#include <avtLAMMPSDumpFileFormat.h>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkTriangulationTables.h>
#include <avtMTSDFileFormatInterface.h>

#include <avtDatabaseMetaData.h>

#include <Expression.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <AtomicProperties.h>

#include <sstream>
#include <string>
#include <vector>

using std::istringstream;
using std::string;

static bool getline_cr(std::ifstream& istr, char* str, std::streamsize count)
{
    istr.getline(str, count);
    bool ret = static_cast<bool>(istr);
    size_t sz = strlen(str);
    if (ret && str[sz - 1] == '\r')
    {
        str[sz - 1] = '\0';
    }
    return ret;
}

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
    for (size_t i=0; i<vars.size(); i++)
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
        in.open(filename.c_str(), std::ios::binary);
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
//    Jeremy Meredith, Tue Jun  2 16:25:01 EDT 2009
//    Added support for unit cell origin (previously assumed to be 0,0,0);
//
//    Jeremy Meredith, Thu Apr 22 11:12:51 EDT 2010
//    Added unit cell bounding box mesh.
//
//    Jeremy Meredith, Fri Apr 30 10:03:17 EDT 2010
//    Added cycles to meta-data.
//
//    Jeremy Meredith, Wed Dec 18 12:24:47 EST 2013
//    Add ID as a field since we're no longer re-sorting by it.
//
// ****************************************************************************

void
avtLAMMPSDumpFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timestep)
{
    ReadAllMetaData();

    avtMeshMetaData *mmd_bbox = new avtMeshMetaData("unitCell", 1, 0,0,0,
                                                    3, 1,
                                                    AVT_POINT_MESH);
    for (int i=0; i<9; i++)
        mmd_bbox->unitCellVectors[i] = 0;
    mmd_bbox->unitCellVectors[0] = xMax - xMin;
    mmd_bbox->unitCellVectors[4] = yMax - yMin;
    mmd_bbox->unitCellVectors[8] = zMax - zMin;
    mmd_bbox->unitCellOrigin[0] = xMin;
    mmd_bbox->unitCellOrigin[1] = yMin;
    mmd_bbox->unitCellOrigin[2] = zMin;
    md->Add(mmd_bbox);

    avtMeshMetaData *mmd = new avtMeshMetaData("mesh", 1, 0,0,0,
                                               3, 1,
                                               AVT_POINT_MESH);
    mmd->nodesAreCritical = true;
    for (int i=0; i<9; i++)
        mmd->unitCellVectors[i] = 0;
    mmd->unitCellVectors[0] = xMax - xMin;
    mmd->unitCellVectors[4] = yMax - yMin;
    mmd->unitCellVectors[8] = zMax - zMin;
    mmd->unitCellOrigin[0] = xMin;
    mmd->unitCellOrigin[1] = yMin;
    mmd->unitCellOrigin[2] = zMin;
    md->Add(mmd);

    AddScalarVarToMetaData(md, "species", "mesh", AVT_NODECENT);
    AddScalarVarToMetaData(md, "id", "mesh", AVT_NODECENT);
    for (int v=0; v<nVars; v++)
    {
        if (v == idIndex || v == speciesIndex)
            continue;
        AddScalarVarToMetaData(md, varNames[v], "mesh", AVT_NODECENT);
    }

    md->SetCycles(cycles);
    md->SetCyclesAreAccurate(true);
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
//    Jeremy Meredith, Thu Apr 22 11:12:51 EDT 2010
//    Added unit cell bounding box mesh.
//
//    Jeremy Meredith, Tue Apr 27 14:41:11 EDT 2010
//    The number of atoms can now vary per timestep.
//
// ****************************************************************************

vtkDataSet *
avtLAMMPSDumpFileFormat::GetMesh(int timestep, const char *name)
{
    ReadTimeStep(timestep);

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
            float x=xMin,y=yMin,z=zMin;
            if (j & 0x01)
                x = xMax;
            if (j & 0x02)
                y = yMax;
            if (j & 0x04)
                z = zMax;
            pts->SetPoint(j, x,y,z);
        }
 
        vtkCellArray *lines = vtkCellArray::New();
        pd->SetLines(lines);
        for (int k = 0 ; k < 12 ; k++)
        {
            lines->InsertNextCell(2);
            lines->InsertCellPoint(voxVerticesFromEdges[k][0]);
            lines->InsertCellPoint(voxVerticesFromEdges[k][1]);
        }

        lines->FastDelete();
        return pd;
    }

    if (meshname != "mesh")
        return NULL;


    vtkPolyData *pd  = vtkPolyData::New();
    vtkPoints   *pts = vtkPoints::New();

    pts->SetNumberOfPoints(nAtoms[timestep]);
    for (vtkIdType j = 0 ; j < nAtoms[timestep] ; j++)
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
    pd->SetPoints(pts);
    pts->FastDelete();

    const vtkIdType numCells = nAtoms[timestep];
    const vtkIdType arrayLen = numCells * 2; // vertices
    vtkIdTypeArray *rawCellArray = vtkIdTypeArray::New();
    rawCellArray->SetNumberOfValues(arrayLen);

    vtkIdType pointId = 0;
    for (vtkIdType k = 0; k < arrayLen; k += 2, ++pointId)
    {
        rawCellArray->SetValue(k, 1);
        rawCellArray->SetValue(k + 1, pointId);
    }
 
    vtkCellArray *verts = vtkCellArray::New();
    verts->SetCells(numCells, rawCellArray);
    pd->SetVerts(verts);

    rawCellArray->FastDelete();
    verts->FastDelete();

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
//    Jeremy Meredith, Tue Apr 27 14:41:11 EDT 2010
//    The number of atoms can now vary per timestep.
//
//    Jeremy Meredith, Wed Dec 18 12:24:47 EST 2013
//    Add ID as a field since we're no longer re-sorting by it.
//
// ****************************************************************************

vtkDataArray *
avtLAMMPSDumpFileFormat::GetVar(int timestep, const char *varname)
{
    ReadTimeStep(timestep);

    // element is a built-in variable
    if (string(varname) == "species" || string(varname) == "id")
    {
        vtkFloatArray *scalars = vtkFloatArray::New();
        scalars->SetNumberOfTuples(nAtoms[timestep]);
        float *ptr = (float *) scalars->GetVoidPointer(0);
        if (string(varname) == "species")
            for (int i=0; i<nAtoms[timestep]; i++)
                ptr[i] = speciesVar[i];
        else // (string(varname) == "id")
            for (int i=0; i<nAtoms[timestep]; i++)
                ptr[i] = idVar[i];
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
    scalars->SetNumberOfTuples(nAtoms[timestep]);
    float *ptr = (float *) scalars->GetVoidPointer(0);
    for (int i=0; i<nAtoms[timestep]; i++)
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
//    Jeremy Meredith, Fri May 15 11:32:54 EDT 2009
//    Fixed species to be 0-origin.  Also, only set it once.
//
//    Jeremy Meredith, Tue Apr 27 14:41:11 EDT 2010
//    The number of atoms can now vary per timestep.
//
//    Jeremy Meredith, Wed Dec 18 12:17:28 EST 2013
//    Don't re-sort atoms by their "id" field.  That field is NOT
//    actually guaranteed to be 1 to natoms.  (New atoms get inserted
//    with new IDs and old ones go away.)  Instead, add ID as a new field.
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

    speciesVar.resize(nAtoms[timestep]);
    idVar.resize(nAtoms[timestep]);
    for (int v=0; v<(int)vars.size(); v++)
    {
        // id and species are ints; don't bother with the float arrays for them
        if (v == idIndex || v == speciesIndex)
            continue;
        vars[v].resize(nAtoms[timestep]);
    }

    std::vector<double> tmpVars(nVars);
    int tmpID, tmpSpecies;

    char buff[1000];
    // read all the atoms
    for (int a=0; a<nAtoms[timestep]; a++)
    {
        getline_cr(in,buff,1000);
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

        int index = a;  // no longer tmpID (tmpID-1 actually); don't re-sort
        for (int v=0; v<nVars; v++)
        {
            if (v == idIndex || v == speciesIndex)
                continue;
            vars[v][index] = tmpVars[v];
        }
        speciesVar[index] = tmpSpecies - 1;
        idVar[index] = tmpID;
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
//    Jeremy Meredith, Tue Apr 27 14:41:11 EDT 2010
//    The number of atoms can now vary per timestep.
//
//    Matthew Wheeler, Fri Aug 31 15:51:00 BST 2012
//    Modified and rearranged the BOX BOUNDS code to ignore irrelevant trailing
//    boundary style fields, so data is handled as in the LAMMPS documentation.
//
//    Satheesh Maheswaran, Fri Oct 19 13:52:00 BST 2012
//    Added options for reading atom coordinates.  The reader can now handle
//    options xu,yu,zu and xsu,ysu,zsu
//
//    Matthew Wheeler, Tue May 31 09:27:45 PDT 2016
//    Corrected a typo in handling options xsu, ysu, zsu.
//
// ****************************************************************************
void
avtLAMMPSDumpFileFormat::ReadAllMetaData()
{
    if (metaDataRead)
        return;

    OpenFileAtBeginning();

    char buff[1000];

    nTimeSteps = 0;
    nVars = -1;

    while (in)
    {
        getline_cr(in,buff,1000);
        if (strncmp(buff, "ITEM:", 5) != 0)
            continue;

        string item(&buff[6]);
        if (item == "TIMESTEP")
        {
            nTimeSteps++;
            getline_cr(in,buff,1000);
            cycles.push_back(strtol(buff, NULL, 10));
        }
        else if (item.substr(0,19) == "BOX BOUNDS xy xz yz")
        {
            float xy, xz, yz;
            in >> xMin >> xMax >> xy;
            in >> yMin >> yMax >> xz;
            in >> zMin >> zMax >> yz;
            getline_cr(in,buff, 1000); // get rest of Z line
        }
        else if (item.substr(0,10) == "BOX BOUNDS")
        {
            in >> xMin >> xMax;
            in >> yMin >> yMax;
            in >> zMin >> zMax;
            getline_cr(in,buff, 1000); // get rest of Z line
        }
        else if (item == "NUMBER OF ATOMS")
        {
            getline_cr(in,buff,1000);
            int n = strtol(buff, NULL, 10);
            nAtoms.push_back(n);
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
                        idIndex = (int)varNames.size();
                    else if (varName == "type")
                        speciesIndex = (int)varNames.size();
                    else if (varName == "x" || varName == "xs" || 
                               varName == "xu" || varName == "xsu" )
                        xIndex = (int)varNames.size();
                    else if (varName == "y" || varName == "ys" ||
                               varName == "yu" || varName == "ysu" )
                        yIndex = (int)varNames.size();
                    else if (varName == "z" || varName == "zs" ||
                               varName == "zu" || varName == "zsu" )
                        zIndex = (int)varNames.size();

                    if (varName == "xs" || varName == "xsu")
                        xScaled = true;
                    if (varName == "ys" || varName == "ysu")
                        yScaled = true;
                    if (varName == "zs" || varName == "zsu")
                        zScaled = true;

                    varNames.push_back(varName);

                }
                nVars = (int)varNames.size();
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
                    nVars = (int)varNames.size();
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
    int pos = (int)filename.length()-1;
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
    ifstream in(filename.c_str(), std::ios::binary);
    char buff[1000];
    getline_cr(in,buff, 1000);
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
//  Modifications:
//    Jeremy Meredith, Thu Jan 28 12:28:07 EST 2010
//    MTSD now accepts grouping multiple files into longer sequences, so
//    its interface has changed to accept both a number of timestep groups
//    and a number of blocks.
// ****************************************************************************
avtFileFormatInterface *
avtLAMMPSDumpFileFormat::CreateInterface(const char *const *list,
                                         int nList, int nBlock)
{
    int nTimestepGroups = nList / nBlock;
    avtMTSDFileFormat ***ffl = new avtMTSDFileFormat**[nTimestepGroups];
    for (int i = 0 ; i < nTimestepGroups ; i++)
    {
        ffl[i] = new avtMTSDFileFormat*[nBlock];
        for (int j = 0 ; j < nBlock ; j++)
        {
            ffl[i][j] = new avtLAMMPSDumpFileFormat(list[i*nBlock+j]);
        }
    }
    return new avtMTSDFileFormatInterface(ffl, nTimestepGroups, nBlock);
}
