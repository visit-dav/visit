// **************************************************************************//
//  Reader: Boxlib3D
// 
//  Current format for a .boxlib3D file:
//   <relative or absolute path to plt directories>
//   <number of cycles>
//    <cycle0> <cycle1> <cycle2> ... 
// 
//  Notes:                             
//     There are certain assumptions that this reader currently makes that
//  should be noted if this reader is extended in the future to support
//  the more general Boxlib framework.
//
//   -> Ghost zones are currently not supported, and a warning is issued
//      if a dataset is found to have ghost zones. This is due to lack
//      of sample datasets, and should be removed once appropriate data
//      is avaliable.
//   -> The number of levels and patches are assumed to be uniform over
//      each timestep (identified by the first timestep). This is until
//      proper AMR support is in place to allow for such issues.
//   -> The names and relative locations of the variable MultiFab files
//      are assumed to be the same over all timesteps. The index into
//      a file for a variable is assumed to be the same over all timesteps
//      and over all levels.
//
//   Code is currently in place to check these assumptions where possible,
//   and issue warnings or throw exceptions as appropriate.
//                              
// **************************************************************************//

#include <avtBoxlib3DFileFormat.h>

#include <vector>
#include <string>

#include <ctype.h>
#include <stdlib.h>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnsignedCharArray.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>
#include <avtMaterial.h>

#include <BadDomainException.h>
#include <BadIndexException.h>
#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <InvalidDBTypeException.h>

#include <VisMF.H>

using std::vector;
using std::string;

static string GetDirName(const char *path);
static string GetNameLevel(const char *name_level_str, int &level);
static void   EatUpWhiteSpace(ifstream &in);
static int    VSSearch(const vector<string> &, const string &); 

// ****************************************************************************
//  Constructor:  avtBoxlib3DFileFormat::avtBoxlib3DFileFormat
//
//  Arguments:
//    fname      the file name of the .boxlib file
//
//  Programmer:  Akira Haddox
//  Creation:    July 25, 2003
//
// ****************************************************************************

avtBoxlib3DFileFormat::avtBoxlib3DFileFormat(const char *fname)
    : avtMTMDFileFormat(fname)
{
    rootPath = GetDirName(fname);

    //
    // Open the .boxlib file and read in the cycles.
    // 
    ifstream in;
    in.open(fname);
    
    if (in.fail())
        EXCEPTION1(InvalidFilesException, fname);
    
    string pth;
    in >> pth;
    if (pth[0] == '/')
        rootPath = pth;
    else
        rootPath = rootPath + pth;
    if (rootPath[rootPath.length() - 1] != '/')
        rootPath += "/";

    in >> nTimesteps;
    
    cycles.resize(nTimesteps);
    
    int i;
    for (i = 0; i < nTimesteps; ++i)
        in >> cycles[i];

    in.close();
    //
    // End reading .boxlib file.
    //

    //
    // Populate with the paths to each timestep.
    //
    timestepPaths.reserve(nTimesteps);
    for (i = 0; i < nTimesteps; ++i)
    {
        char buf[1024];
        sprintf(buf, "plt%04d", cycles[i]);
        timestepPaths.push_back(buf); 
    }

    patches.resize(nTimesteps);
    readTimeHeader.resize(nTimesteps, false); 
    
    //
    // Read in the first timestep, resizing most of our data arrays,
    // and gathering essential information.
    //
    ReadTimeHeader(0, true);

    mfReaders.resize(nTimesteps);
    for (i = 0; i < nTimesteps; ++i)
        mfReaders[i].resize(multifabFilenames.size(), NULL);
    
    //
    // Now that we have the varNames and the multifabFilenames,
    // find which variable is in which file.
    // 
    int count = 0;
    int level = 0;
    for (i = 0; i < multifabFilenames.size(); ++i)
    {
        VisMF *vmf = GetVisMF(0, i);
        int cnt = vmf->nComp();

        //
        // Determine what type of centering it is.
        //
        const IndexType &ix = vmf->boxArray()[0].ixType();
        bool isNode = true;
        bool isCell = true;

        int k;
        for (k = 0; k < dimension; ++k)
        {
            if (ix.test(k))
                isCell = false;
            else
                isNode = false;
        }

        int type = isNode ? AVT_NODECENT :
                   isCell ? AVT_ZONECENT :
                            AVT_UNKNOWN_CENT;
    
        int j;
        for (j = 0; j < cnt; ++j)
        {
            componentIds[level][j + count] = j;
            fabfileIndex[level][j + count] = i;
            if (level == 0)
                varCentering[j + count] = type;
        }

        count += cnt;
        if (count >= nVars)
        {
            count -= nVars;
            ++level;
        }
    } 

    //
    // Find any materials
    //
    nMaterials = 0;
    for (i = 0; i < nVars; ++i)
    {
        int val = 0;
        if (varNames[i].find("frac") == 0)
        {
            int j;
            for (j = 4; isdigit(varNames[i][j]); ++j)
            {
                val *= 10;
                val += varNames[i][j] - '0';
            }

            if (val > nMaterials)
                nMaterials = val;
        }
    }
    
    //
    // Find possible vectors by combining scalars with {x,y,z} prefixes.
    // 
    nVectors = 0;
    for (i = 0; i < nVars; ++i)
    {
        if (varNames[i][0] == 'x')
        {
            string needle = varNames[i];
            needle[0] = 'y';

            int id2 = VSSearch(varNames, needle);
            if (id2 == -1)
                continue;
            
            needle[0] = 'z';
            int id3 = VSSearch(varNames, needle);
            if (id3 == -1)
                continue;
            
            // Ensure they're all the same type of centering.
            if (varCentering[i] == AVT_UNKNOWN_CENT)
                continue;
            if (varCentering[i] != varCentering[id2])
                continue;
            if (varCentering[i] != varCentering[id3])
                continue;

            int index = nVectors;
            ++nVectors;
            vectorNames.push_back(needle.substr(1, needle.length() - 1));
            vectorCentering.push_back(varCentering[i]);

            vectorComponents.resize(nVectors);
            vectorComponents[index].resize(dimension);
            vectorComponents[index][0] = i;
            vectorComponents[index][1] = id2;
            vectorComponents[index][2] = id3;
        }
    }
}


// ****************************************************************************
//  Destructor:  avtBoxlib3DFileFormat::~avtBoxlib3DFileFormat
//
//  Programmer:  Akira Haddox
//  Creation:    July 25, 2003
//
// ****************************************************************************

avtBoxlib3DFileFormat::~avtBoxlib3DFileFormat()
{
    FreeUpResources();
}


// ****************************************************************************
//  Method:  avtBoxlib3DFileFormat::GetMesh
//
//  Purpose:
//    Returns the mesh with the given name for the given time step and
//    domain. 
//
//  Arguments:
//    ts         the time step
//    dom        the domain number
//    level_name the name of the mesh to read
//
//  Programmer:  Akira Haddox
//  Creation:    July 25, 2003
//
// ****************************************************************************

vtkDataSet *
avtBoxlib3DFileFormat::GetMesh(int ts, int patch, const char *level_name)
{
    int level;
    string name = GetNameLevel(level_name, level);
    if (level >= nLevels)
    {
        EXCEPTION1(InvalidVariableException, level_name);
    }

    if (patch >= patchesPerLevel[level])
    {
        EXCEPTION2(BadDomainException, patch, patchesPerLevel[level]);
    }

    //
    // Read in the time header, which generates the mesh that we need.
    //
    if (!readTimeHeader[ts])
        ReadTimeHeader(ts, false);

    // May be NULL, we don't support changing number of patches right now.
    if (patches[ts][level][patch])
        patches[ts][level][patch]->Register(NULL);
    else
        EXCEPTION1(InvalidDBTypeException, "Reader does not support changing number of patches over time.");
    return patches[ts][level][patch];
}


// ****************************************************************************
//  Method:  avtBoxlib3DFileFormat::ReadTimeHeader
//
//  Purpose:
//    Reads in the header information for a time step and generate
//    meshes for all patches and levels from that information.
//    Meshes are not generated if populate is set to true.
//
//  Arguments:
//    ts         the time step
//    populate   true if this is read is part of the constructor
//
//  Programmer:  Akira Haddox
//  Creation:    July 25, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Sep  8 16:04:01 PDT 2003
//    While parsing header, a string was returned on the DECs when it was
//    actually EOF, leading to a bad value in the multiFabFilenames.  I put
//    in a check for this and ignored the string in this case.
//
// ****************************************************************************

void
avtBoxlib3DFileFormat::ReadTimeHeader(int ts, bool populate)
{
    ifstream in;

    string headerFilename = rootPath + timestepPaths[ts] + "/Header";
    
    in.open(headerFilename.c_str());
    
    if (in.fail())
        EXCEPTION1(InvalidFilesException, headerFilename.c_str());

    int integer;
    char buf[1024];
    // Read in version
    in.getline(buf, 1024);
    // Read in nVars
    in >> integer;

    if (populate)
    {
        nVars = integer;
        varNames.resize(nVars);
        varCentering.resize(nVars);
    }
    
    EatUpWhiteSpace(in);
    int i;
    for (i = 0; i < nVars; ++i)
    {
        in.getline(buf, 1024); // Read in var names
        if (populate)
            varNames[i] = buf;
    }
    // Read in dimension
    in >> integer;
    if (populate && dimension != integer)
    {
        EXCEPTION1(InvalidDBTypeException,"This reader only handles 3D files.");
    }

    // Read in time
    double time;
    in >> time;
    if (!populate)
        metadata->SetTime(ts, time);

    // Read in number of levels for this timestep.
    int lev;
    in >> lev;
    ++lev;
    if (populate)
    {
        nLevels = lev;
        patchesPerLevel.resize(nLevels);
        for (i = 0; i < patches.size(); ++i)
            patches[i].resize(nLevels);

        fabfileIndex.resize(nLevels);
        componentIds.resize(nLevels);

        for (i = 0; i < nLevels; ++i)
        {
            fabfileIndex[i].resize(nVars);
            componentIds[i].resize(nVars);
        }
    }
    // Do a quick check that the levels haven't changed.
    else if (lev != nLevels)
    {
        EXCEPTION1(InvalidDBTypeException, "Reader does not support changing number of levels over time.");
    }

    // Read the problem size
    if (populate)
    {
        for (i = 0; i < dimension; ++i)
            in >> probLo[i];
        for (i = 0; i < dimension; ++i)
            in >> probHi[i];
    }
    else
    {
        float ignored;

        for (i = 0; i < 2 * dimension; ++i)
            in >> ignored;
    }

    EatUpWhiteSpace(in);

    // Read in the refinement ratio
    if (nLevels != 1)
        in.getline(buf, 1024);
    
    // Read in the problem domain for this level
    in.getline(buf, 1024);
    
    // Read in the levelsteps
    in.getline(buf, 1024);

    // For each level, read in the gridSpacing
    int levI;
    vector<vector<double> > delta(lev);
    for (levI = 0; levI < lev; ++levI)
    {
        delta[levI].resize(3);
        for (i = 0; i < dimension; ++i)
        {
            in >> delta[levI][i];
        }
    }
 
    // Read in coord system;
    in >> integer;
   
    // Read in width of boundry regions (ghost zones)
    in >> integer;
    if (integer)
    {
        avtCallback::IssueWarning("Reader does not currently support ghostzones.");
    }
    
    // For each level
    for (levI = 0; levI < lev; ++levI)
    {
        // Read in which level
        int myLevel;
        in >> myLevel;

        // Read in the number of patches
        int myNPatch;
        in >> myNPatch;
        if (populate)
        {
            patchesPerLevel[levI] = myNPatch;
        }
        // Do a quick check to make sure the patches haven't changed.
        else if (myNPatch != patchesPerLevel[levI])
        {
            EXCEPTION1(InvalidDBTypeException, "Reader does not support changing number of patches over time.");
        }

        // Resize to the maximum number of patches, init to NULL
        patches[ts][levI].resize(patchesPerLevel[levI], NULL);

        // Read in the time (again)
        in >> time;

        // Read in iLevelSteps
        in >> integer;

        // For each patch
        for (i = 0; i < myNPatch; ++i)
        {
            double lo[3];
            double hi[3];

            // Read the patch range
            int j;
            for (j = 0; j < dimension; ++j)
                in >> lo[j] >> hi[j];

            // Generate the mesh for this patch
            if (!populate)
                patches[ts][levI][i] = CreateGrid(lo, hi, &(delta[levI][0]));
        }
            
        EatUpWhiteSpace(in);
        // Read in the MultiFab files (Until we hit an int or eof)
        for (;;)
        {
            if (isdigit(in.peek()) || in.eof() || in.fail())
                break;
            in.getline(buf, 1024);
            if (strcmp(buf, "") == 0)
                continue;
            multifabFilenames.push_back(buf);
        }
    }
    
    if (!populate)
        readTimeHeader[ts] = true;
}


// ****************************************************************************
//  Method:  avtBoxlib3DFileFormat::CreateGrid
//
//  Purpose:
//    Create a rectilinear grid given bounds and a delta.
//
//  Arguments
//    lo        Lower bound
//    hi        Upper bound
//    delta     Step size in the xyz directions.
//
//  Returns: The grid as a vtkDataSet.
//
//  Programmer:  Akira Haddox
//  Creation:    July 28, 2003
//
// ****************************************************************************

vtkDataSet *
avtBoxlib3DFileFormat::CreateGrid(double lo[3], double hi[3], double delta[3]) 
{
    vtkRectilinearGrid *rg = vtkRectilinearGrid::New();

    int steps[3];
    
    // Because we're reconstructing the dimensions from the deltas, we
    // want to make sure that we don't accidently round down in integer
    // conversion, so we add an appropriate little amount.
    int i;
    for (i = 0; i < dimension; ++i)
    {
        double epsilon = delta[i] / 8.0;
        // This expands out to (hi - lo / delta) + epsilon,
        // cast to an int.
        steps[i] = (int) ((hi[i] - lo[i] + epsilon) / delta[i]);
        
        // Incriment by one for fencepost point at end.
        ++(steps[i]);
    }

    rg->SetDimensions(steps);

    vtkFloatArray  *xcoord = vtkFloatArray::New();
    vtkFloatArray  *ycoord = vtkFloatArray::New();
    vtkFloatArray  *zcoord = vtkFloatArray::New();

    xcoord->SetNumberOfTuples(steps[0]);
    ycoord->SetNumberOfTuples(steps[1]);
    zcoord->SetNumberOfTuples(steps[2]);

    float *ptr = xcoord->GetPointer(0);
    for (i = 0; i < steps[0]; ++i)
        ptr[i] = (lo[0] + i * delta[0]);

    ptr = ycoord->GetPointer(0);
    for (i = 0; i < steps[1]; ++i)
        ptr[i] = (lo[1] + i * delta[1]);

    ptr = zcoord->GetPointer(0);
    for (i = 0; i < steps[2]; ++i)
        ptr[i] = (lo[2] + i * delta[2]);

    rg->SetXCoordinates(xcoord);
    rg->SetYCoordinates(ycoord);
    rg->SetZCoordinates(zcoord);

    xcoord->Delete();
    ycoord->Delete();
    zcoord->Delete();

    return rg;
}


// ****************************************************************************
//  Method:  avtBoxlib3DFileFormat::GetVar
//
//  Purpose:
//    Reads the variable with the given name for the given time step and
//    domain.
//
//  Arguments:
//    ts         the time step
//    patch      the domain number
//    name       the name of the variable to read
//
//  Programmer:  Akira Haddox
//  Creation:    July 25, 2003
//
// ****************************************************************************

vtkDataArray *
avtBoxlib3DFileFormat::GetVar(int ts, int patch, const char *visit_var_name)
{
    int level;
    string var_name = GetNameLevel(visit_var_name, level);    

    if (level >= nLevels)
        EXCEPTION1(InvalidVariableException, visit_var_name);

    if (!readTimeHeader[ts])
        ReadTimeHeader(ts, false);
    
    if (patch >= patchesPerLevel[level])
        EXCEPTION2(BadDomainException, patch, patchesPerLevel[level]);

    int varIndex;
    for (varIndex = 0; varIndex < varNames.size(); ++varIndex)
        if (varNames[varIndex] == var_name)
            break;

    if (varIndex > varNames.size())
        EXCEPTION1(InvalidVariableException, visit_var_name);
    

    vtkRectilinearGrid *rg = (vtkRectilinearGrid *)(patches[ts][level][patch]);
    // If no patch, we need no array.
    // Not sure if we need to do this check. This was here when
    // we assumed that different timesteps could have different
    // number of patches.
    if (!rg)
        return NULL;

    int mfIndex = fabfileIndex[level][varIndex];
    int compId = componentIds[level][varIndex];
    
    VisMF *vmf = GetVisMF(ts, mfIndex);
    
    // Get the data (an FArrayBox)
    FArrayBox fab = vmf->GetFab(patch, compId);

    int dims[3];
    rg->GetDimensions(dims);
    
    // Cell based variable. Shift the dimensions.
    if (varCentering[varIndex] == AVT_ZONECENT)
    {
        --dims[0];
        --dims[1];
        --dims[2];
    }
    
    vtkFloatArray *farr = vtkFloatArray::New();
    farr->SetNumberOfTuples(dims[0] * dims[1] * dims[2]);

    float *fptr = farr->GetPointer(0);
    
    // SLOW Implementation for now, but to get it working...
    IntVect pos;

    // We need to index in the same box that the data is defined on,
    // so we need to offset all of our indexes by the lower box corner. 
    const int * offset = fab.box().loVect();
    
    int x, y, z;
    for (z = 0; z < dims[2]; ++z)
    {
        for (y = 0; y < dims[1]; ++y)
        {
            for (x = 0; x < dims[0]; ++x)
            {
                pos[0] = x + offset[0];
                pos[1] = y + offset[1];
                pos[2] = z + offset[2];
                // Always index component 0, since this fab only
                // holds one (not a multifab).
                *(fptr++) = fab(pos, 0);
            }
        }
    }
    
    return farr;
}


// ****************************************************************************
//  Method:  avtBoxlib3DFileFormat::GetVectorVar
//
//  Purpose:
//    Reads the vector variable with the given name for the given domain.
//
//  Note:  The Generic Database (which calls this function) already handles
//         caching for a single time step.
//
//  Arguments:
//    patch                the patch number
//    visit_var_name       the name of the variable to read
//
//  Programmer:  Akira Haddox
//  Creation:    July 25, 2003
//
// ****************************************************************************

vtkDataArray *
avtBoxlib3DFileFormat::GetVectorVar(int ts, int patch, 
                                  const char *visit_var_name)
{
    int level;
    string var_name = GetNameLevel(visit_var_name, level);    

    if (level >= nLevels)
        EXCEPTION1(InvalidVariableException, visit_var_name);

    if (!readTimeHeader[ts])
        ReadTimeHeader(ts, false);

    if (patch >= patchesPerLevel[level])
        EXCEPTION2(BadDomainException, patch, patchesPerLevel[level]);

    int vectIndex;
    for (vectIndex = 0; vectIndex < nVectors; ++vectIndex)
        if (vectorNames[vectIndex] == var_name)
            break;

    if (vectIndex > nVectors)
        EXCEPTION1(InvalidVariableException, visit_var_name);


    vtkRectilinearGrid *rg = (vtkRectilinearGrid *)(patches[ts][level][patch]);
    // If no patch, we need no array.
    // Not sure if we need to do this check. This was here when
    // we assumed that different timesteps could have different
    // number of patches.
    if (!rg)
        return NULL;

    // Get the data for the components (in FArrayBoxes).
    vector<FArrayBox *> fab(dimension);
    
    int i;
    for (i = 0; i < dimension; ++i)
    {
        int varIndex = vectorComponents[vectIndex][i];
        int mfIndex = fabfileIndex[level][varIndex];
        int compId = componentIds[level][varIndex];
        
        VisMF *vmf = GetVisMF(ts, mfIndex); 
        fab[i] = new FArrayBox(vmf->GetFab(patch, compId));
    }
    
    int dims[3];
    rg->GetDimensions(dims);
    
    // Cell based variable.
    if (vectorCentering[vectIndex] == AVT_ZONECENT)
    {
        --dims[0];
        --dims[1];
        --dims[2];
    }
    
    vtkFloatArray *farr = vtkFloatArray::New();
    farr->SetNumberOfComponents(3);
    farr->SetNumberOfTuples(dims[0] * dims[1] * dims[2]);

    float *fptr = farr->GetPointer(0);
    
    // SLOW Implementation for now, but to get it working...
    IntVect pos;

    // We need to index in the same box that the data is defined on,
    // so we need to offset all of our indexes by the lower box corner. 
    //
    // We also know the fabs are all defined on the same box, so we
    // only need one offset.
    const int * offset = fab[0]->box().loVect();

    int x, y, z;
    for (z = 0; z < dims[2]; ++z)
    {
        for (y = 0; y < dims[1]; ++y)
        {
            for (x = 0; x < dims[0]; ++x)
            {
                pos[0] = x + offset[0];
                pos[1] = y + offset[1];
                pos[2] = z + offset[2];

                *(fptr++) = (*(fab[0]))(pos, 0);
                *(fptr++) = (*(fab[1]))(pos, 0);
                *(fptr++) = (*(fab[2]))(pos, 0);
            }
        }
    }
    
    for (i = 0; i < dimension; ++i)
        delete fab[i];

    return farr;
}


// ****************************************************************************
//  Method:  avtBoxlib3DFileFormat::GetVisMF
//
//  Purpose:
//    Get the VisMF multifab reader for a specified timestep and file.
//    If it doesn't exist yet, create it.
//
//  Arguments:
//    ts        The timestep
//    index     The index of the multifab file (into multifabFilenames)
//
//  Returns:    A pointer to the VisMF.
//
//  Programmer:  Akira Haddox
//  Creation:    July 30, 2003
//
// ****************************************************************************

VisMF *
avtBoxlib3DFileFormat::GetVisMF(int ts, int index)
{
    if (!mfReaders[ts][index])
    {
        char filename[1024];
        sprintf(filename, "%s%s/%s", rootPath.c_str(),
                                     timestepPaths[ts].c_str(),
                                     multifabFilenames[index].c_str());
        mfReaders[ts][index] = new VisMF(filename);
    }
    return mfReaders[ts][index];
}


// ****************************************************************************
//  Method:  avtBoxlib3DFileFormat::GetCycles
//
//  Purpose:
//    Returns the actual cycle numbers for each time step.
//
//  Arguments:
//   out_cycles      the output vector of cycle numbers 
//
//  Programmer:  Akira Haddox
//  Creation:    July 25, 2003
//
// ****************************************************************************

void
avtBoxlib3DFileFormat::GetCycles(vector<int> &out_cycles)
{
    out_cycles = cycles;
}


// ****************************************************************************
//  Method:  avtBoxlib3DFileFormat::GetNTimesteps
//
//  Purpose:
//    Returns the number of timesteps
//
//  Programmer:  Akira Haddox
//  Creation:    July 25, 2003
//
// ****************************************************************************

int
avtBoxlib3DFileFormat::GetNTimesteps()
{
    return nTimesteps;
}


// ****************************************************************************
//  Method:  avtBoxlib3DFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//    Returns meta-data about the database.
//
//  Arguments:
//    md         The meta-data structure to populate
//
//  Programmer:  Akira Haddox
//  Creation:    July 25, 2003
//
// ****************************************************************************

void
avtBoxlib3DFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    int i;
    for (i = 0; i < nLevels; ++i)
    {
        char mesh_name[32];
        avtMeshMetaData *mesh = new avtMeshMetaData;
        sprintf(mesh_name, "Level_%05d", i);
        mesh->name = mesh_name;
        mesh->meshType = AVT_RECTILINEAR_MESH;
        mesh->numBlocks = patchesPerLevel[i];
        mesh->blockOrigin = 0;
        mesh->spatialDimension = dimension;
        mesh->topologicalDimension = dimension;
        mesh->hasSpatialExtents = false;
        md->Add(mesh);

        
        int v;
        for (v = 0; v < nVars; ++v)
        {
            char var_name[1024];
            sprintf(var_name, "%s_%05d", varNames[v].c_str(), i);
            if (varCentering[v] != AVT_UNKNOWN_CENT)
                AddScalarVarToMetaData(md, var_name, mesh_name, 
                                       (avtCentering)varCentering[v]);
        }

        for (v = 0; v < nVectors; ++v)
        {
            char var_name[1024];
            sprintf(var_name, "%s_%05d", vectorNames[v].c_str(), i);
            AddVectorVarToMetaData(md, var_name, mesh_name,
                                 (avtCentering)varCentering[v], dimension);
        }


        if (nMaterials)
        {
            int m;
            vector<string> mnames(nMaterials);
            char str[32];

            string matname;
            sprintf(str, "materials_%05d", i);
            matname = str;
            
            for (m = 0; m < nMaterials; ++m)
            {
                sprintf(str, "mat%d", m);
                mnames[m] = str;
            }
            AddMaterialToMetaData(md, matname, mesh_name, nMaterials, 
                                  mnames);
        }

    }
}


// ****************************************************************************
//  Method: avtBoxlib3DFileFormat::GetAuxiliaryData
//
//  Purpose:
//      Gets the auxiliary data specified.
//
//  Arguments:
//      var        The variable of interest.
//      ts         The timestep of interest.
//      dom        The domain of interest.
//      type       The type of auxiliary data.
//      <unnamed>  The arguments for that type -- not used.
//      df         Destructor function.
//
//  Returns:    The auxiliary data.
//
//  Programmer: Akira Haddox
//  Creation:   August 5, 2003
//
// ****************************************************************************
 
void *
avtBoxlib3DFileFormat::GetAuxiliaryData(const char *var, int ts, int dom, 
                                        const char * type, void *,
                                        DestructorFunction &df) 
{
    if (strcmp(type, AUXILIARY_DATA_MATERIAL))
        return NULL;
    
    int level = 0;
    int len = strlen(var);
    const char *last = var + (len-1);
    while (*last != '_' && last > var)
    {
        last--;
    }

    level = strtol(last + 1, NULL, 0);

    if (level >= nLevels)
        EXCEPTION1(InvalidVariableException, var);

    if (dom >= patchesPerLevel[level])
        EXCEPTION2(BadDomainException, dom, patchesPerLevel[level]);

    
    int i;
    vector<string> mnames(nMaterials);
    char str[32];
    for (i = 0; i < nMaterials; ++i)
    {
        sprintf(str, "mat%d", i);
        mnames[i] = str;
    }
    
    if (!readTimeHeader[ts])
        ReadTimeHeader(ts, false);
    
    int nCells = patches[ts][level][dom]->GetNumberOfCells();

    vector<int> material_list(nCells);
    vector<int> mix_mat;
    vector<int> mix_next;
    vector<int> mix_zone;
    vector<float> mix_vf;

    // Get the material fractions
    vector<vtkFloatArray *> floatArrays(nMaterials);
    vector<float *> mats(nMaterials);
    for (i = 1; i <= nMaterials; ++i)
    {
        sprintf(str,"frac%d_%05d", i, level);
        floatArrays[i - 1] = (vtkFloatArray *)(GetVar(ts, dom, str));
        mats[i - 1] = floatArrays[i - 1]->GetPointer(0);
    }
         
    // Build the appropriate data structures
    for (i = 0; i < nCells; ++i)
    {
        int j;

        // First look for pure materials
        bool pure = false;
        for (j = 0; j < nMaterials; ++j)
        {
            if (mats[j][i] >= 1)
            {
                pure = true;
                break;
            }
            else if (mats[j][i] > 0)
                break;
        }

        if (pure)
        {
            material_list[i] = j;
            continue;
        }

        // For unpure materials, we need to add entries to the tables.  
        material_list[i] = -1 * (1 + mix_zone.size());
        for (j = 0; j < nMaterials; ++j)
        {
            if (mats[j][i] <= 0)
                continue;
            // For each material that's present, add to the tables
            mix_zone.push_back(i);
            mix_mat.push_back(j);
            mix_vf.push_back(mats[j][i]);
            mix_next.push_back(mix_zone.size() + 1);
        }

        // When we're done, the last entry is a '0' in the mix_next
        mix_next[mix_next.size() - 1] = 0;
    }
    
    
    int mixed_size = mix_zone.size();
    avtMaterial * mat = new avtMaterial(nMaterials, mnames, nCells,
                                        &(material_list[0]), mixed_size,
                                        &(mix_mat[0]), &(mix_next[0]),
                                        &(mix_zone[0]), &(mix_vf[0]));
     
    df = avtMaterial::Destruct;
    return (void*) mat;
}


// ****************************************************************************
//  Method:  avtBoxlib3DFileFormat::FreeUpResources
//
//  Purpose:
//    Release cached memory used by this reader.
//
//  Programmer:  Akira Haddox
//  Creation:    July 30, 2003
//
// ****************************************************************************

void
avtBoxlib3DFileFormat::FreeUpResources()
{
    int i,j,k;
    for (i = 0; i < patches.size(); ++i)
        for (j = 0; j < patches[i].size(); ++j)
            for (k = 0; k < patches[i][j].size(); ++k)
                if (patches[i][j][k])
                {
                    patches[i][j][k]->Delete();
                    patches[i][j][k] = NULL;
                }

    for (i = 0; i < nTimesteps; ++i)
    {
        readTimeHeader[i] = false;
        int j;
        for (j = 0; j < mfReaders[i].size(); ++j)
            if (mfReaders[i][j])
            {
                delete mfReaders[i][j];
                mfReaders[i][j] = NULL;
            }
    }
}


// ****************************************************************************
//  Method:  GetDirName
//
//  Purpose:
//    Returns the directory from a full path name
//
//  Arguments:
//    path       the full path name
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  7, 2003
//
// ****************************************************************************

string 
GetDirName(const char *path)
{
    string dir = "";

    int len = strlen(path);
    const char *last = path + (len-1);
    while (*last != '/' && last > path)
    {
        last--;
    }

    if (*last != '/')
    {
        return "";
    }

    char str[1024];
    strcpy(str, path);
    str[last-path+1] = '\0';

    return str;
}


// ****************************************************************************
//  Method:  GetNameLevel
//
//  Purpose:
//    Returns the 'name' string and level number from a 'name.level' string
//
//  Arguments:
//    IN:  name_level_str     the string with name and level data
//    OUT: level              the level
//
//  Programmer:  Walter Herrera Jimenez
//  Creation:    July  11, 2003
//
// ****************************************************************************

string 
GetNameLevel(const char *name_level_str, int &level)
{
    int len = strlen(name_level_str);
    const char *last = name_level_str + (len-1);
    while (*last != '_' && last > name_level_str)
    {
        last--;
    }

    if (*last != '_')
    {
        level = 0;
        return "";
    }

    char str[1024];
    strcpy(str, name_level_str);
    str[last-name_level_str] = '\0';

    level = strtol(last+1,NULL,0);
    return str;
}


// ****************************************************************************
//  Method:  EatUpWhiteSpace
//
//  Purpose:
//    Read from the filestream to skip whitespace.
//
//  Programmer:  Akira Haddox
//  Creation:    July  28, 2003
//
// ****************************************************************************

void 
EatUpWhiteSpace(ifstream &in)
{
    while (isspace(in.peek()))
            in.get();
}


// ***************************************************************************
//  Method: VSSearch
//
//  Purpose:
//    Search a vector of strings.
//
//  Arguments:
//    v     the vector
//    s     the string to search for
//
//  Returns:
//    The index of s, or -1 if not found.
//    
//  Programmer:  Akira Haddox
//  Creation:    July  28, 2003
//
// ***************************************************************************

int VSSearch(const vector<string> &v, const string &s)
{
    int i;
    for (i = 0; i < v.size(); ++i)
        if (v[i] == s)
            return i;
    return -1;
}
