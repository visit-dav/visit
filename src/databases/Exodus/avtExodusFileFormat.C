// ************************************************************************* //
//                           avtExodusFileFormat.C                           //
// ************************************************************************* //

#include <avtExodusFileFormat.h>

#include <algorithm>

#include "vtkExodusReader.h"
#include <vtkCellData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPointData.h>

#include <avtDatabaseMetaData.h>
#include <avtVariableCache.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <InvalidVariableException.h>


using     std::string;
using     std::vector;
using     std::sort;


// ****************************************************************************
//  Method: avtExodusFileFormat constructor
//
//  Arguments:
//      name    The name of the exodus file.
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Apr  8 10:58:18 PDT 2003
//    Defer reading in the file.
//
// ****************************************************************************

avtExodusFileFormat::avtExodusFileFormat(const char *name)
   : avtMTSDFileFormat(&name, 1)
{
    reader = NULL;
    cache  = NULL;
    readInFile = false;
}


// ****************************************************************************
//  Method: avtExodusFileFormat::ReadInFile
//
//  Purpose:
//      Reads in meta-data for the file.
//
//  Programmer: Hank Childs
//  Creation:   April 8, 2003
//
// ****************************************************************************

void
avtExodusFileFormat::ReadInFile(void)
{
    int  i;

    //
    // Do some initialization.
    //
    vtkExodusReader *rdr = GetReader();

    //
    // Determine what the blocks are and how many there are.
    //
    numBlocks = rdr->GetNumberOfBlocks();
    validBlock.resize(numBlocks);
    blockId.resize(numBlocks);
    for (int i = 0 ; i < numBlocks; i++)
    {
        if (rdr->GetNumberOfElementsInBlock(i) != 0)
        {
            validBlock[i] = true;
        }
        else
        {
            validBlock[i] = false;
        }
        blockId[i] = rdr->GetBlockId(i);
    }

    //
    // Determine what the variables are.
    //
    int numPointArrays = rdr->GetNumberOfPointDataArrays();
    for (i = 0 ; i < numPointArrays ; i++)
    {
        pointVars.push_back(string(rdr->GetPointDataArrayName(i)));
    }
    int numCellArrays = rdr->GetNumberOfCellDataArrays();
    for (i = 0 ; i < numCellArrays ; i++)
    {
        cellVars.push_back(string(rdr->GetCellDataArrayName(i)));
    }

    //
    // We need to maintain our own cache, since meshes do not change over
    // timesteps (but the exodus format is the only thing that knows that).
    //
    cache = new avtVariableCache;

    readInFile = true;
}


// ****************************************************************************
//  Method: avtExodusFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
// ****************************************************************************

avtExodusFileFormat::~avtExodusFileFormat()
{
    if (reader != NULL)
    {
        reader->Delete();
        reader = NULL;
    }
    if (cache != NULL)
    {
        delete cache;
        cache = NULL;
    }
}


// ****************************************************************************
//  Method: avtExodusFileFormat::FreeUpResources
//
//  Purpose:
//      Frees up resources, like memory and file descriptors.
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
// ****************************************************************************

void
avtExodusFileFormat::FreeUpResources(void)
{
/*  This is called when the timestep is changed, but we probably want to save
    that information.
 */
}


// ****************************************************************************
//  Method: avtExodusFileFormat::GetReader
//
//  Purpose:
//      Gets a reader.  This is always done through this method, so that
//      resources can be freed underneath the covers and then regained here.
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
//  Modifications:
//
//    Hank Childs, Sun Jun 27 10:31:18 PDT 2004
//    Tell the reader that it should always generate the global node id numbers
//
// ****************************************************************************

vtkExodusReader *
avtExodusFileFormat::GetReader(void)
{
    if (reader != NULL)
    {
        return reader;
    }

    reader = vtkExodusReader::New();
    reader->SetFileName(filenames[0]);
    reader->SetGenerateNodeGlobalIdArray(1);

    //
    // Everything we want will assume that at least the information has been
    // updated.
    //
    reader->GetOutput()->UpdateInformation();

    return reader;
}


// ****************************************************************************
//  Method: avtExodusFileFormat::GetCycles
//
//  Purpose:
//      The cycles aren't in the current version of the Exodus reader, so dummy
//      up a version.
//
//  Arguments:
//      cycles  A place to put the cycle numbers.
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
// ****************************************************************************

void
avtExodusFileFormat::GetCycles(vector<int> &cycles)
{
    int nTimesteps = GetNTimesteps();

    cycles.clear();
    for (int i = 0 ; i < nTimesteps ; i++)
    {
        cycles.push_back(i);
    }
}


// ****************************************************************************
//  Method: avtExodusFileFormat::GetTimes
//
//  Purpose:
//      Get the times for each timestep.
//
//  Arguments:
//      times  A place to put the times numbers.
//
//  Programmer: Hank Childs
//  Creation:   April 17, 2004
//
// ****************************************************************************

void
avtExodusFileFormat::GetTimes(vector<double> &times)
{
    int nTimesteps = GetNTimesteps();

    times.clear();
    for (int i = 0 ; i < nTimesteps ; i++)
    {
        if (reader != NULL)
            times.push_back(reader->GetTime(i));
        else
            times.push_back(0.);
    }
}


// ****************************************************************************
//  Method: avtExodusFileFormat::GetNTimesteps
//
//  Purpose:
//      Gets the number of timesteps in the Exodus file.
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
// ****************************************************************************

int
avtExodusFileFormat::GetNTimesteps(void)
{
    int nts = GetReader()->GetNumberOfTimeSteps();
    if (nts <= 0)
    {
        return 1;
    }

    return nts;
}


// ****************************************************************************
//  Method: avtExodusFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta-data from the Exodus file.
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Mar 11 08:52:59 PST 2002
//    Renamed to PopulateDatabaseMetaData.
//
//    Hank Childs, Tue May 28 14:07:25 PDT 2002
//    Renamed materials and domains to element block and files.  This is more
//    like how Exodus users think of these terms.
//
//    Hank Childs, Tue Apr  8 10:58:18 PDT 2003
//    Make sure the file is read in before proceeding.
//
//    Hank Childs, Sun Jun 27 13:20:51 PDT 2004
//    Indicate that we have global node ids.
//
// ****************************************************************************

void
avtExodusFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (!readInFile)
    {
        ReadInFile();
    }

    int   i;

    vtkExodusReader *rdr = GetReader();

    int spatialDimension = rdr->GetDimensionality();
    string meshName = "Mesh";

    //
    // Set up the mesh.
    //
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = meshName;
    mesh->meshType = AVT_UNSTRUCTURED_MESH;
    mesh->numBlocks = 1;
    mesh->blockOrigin = 0;
    mesh->spatialDimension = spatialDimension;
    mesh->topologicalDimension = spatialDimension;
    mesh->blockTitle = "file";
    mesh->blockPieceName = "file";
    mesh->containsGlobalNodeIds = true;
    md->Add(mesh);

    //
    // Sort the block ids so the user thinks we know what we are doing.
    //
    vector<int> sortedBlockIds = blockId;
    sort(sortedBlockIds.begin(), sortedBlockIds.end());

    //
    // Set up the material.
    //
    string materialName = "ElementBlock";
    vector<string> matNames;
    for (i = 0 ; i < numBlocks ; i++)
    {
        char name[128];
        sprintf(name, "%d", sortedBlockIds[i]);
        matNames.push_back(name);
    }
    AddMaterialToMetaData(md, materialName, meshName, numBlocks, matNames);

    //
    // Set up the variables.
    //
    int numPointArrays = rdr->GetNumberOfPointDataArrays();
    for (int i = 0 ; i < numPointArrays ; i++)
    {
        int dim = rdr->GetPointDataArrayNumberOfComponents(i);
        const char *var = rdr->GetPointDataArrayName(i);
        if (dim == 1)
        {
            AddScalarVarToMetaData(md, var, meshName, AVT_NODECENT);
        }
        else
        {
            if (dim != spatialDimension)
            {
                debug1 << "Ignoring variable " << var << " since it has "
                       << "dimension " << dim << endl;
                continue;
            }
            AddVectorVarToMetaData(md, var, meshName, AVT_NODECENT, dim);
        }
    }
    int numCellArrays = rdr->GetNumberOfCellDataArrays();
    for (int i = 0 ; i < numCellArrays ; i++)
    {
        int dim = rdr->GetCellDataArrayNumberOfComponents(i);
        const char *var = rdr->GetCellDataArrayName(i);
        if (dim == 1)
        {
            AddScalarVarToMetaData(md, var, meshName, AVT_ZONECENT);
        }
        else
        {
            if (dim != spatialDimension)
            {
                debug1 << "Ignoring variable " << var << " since it has "
                       << "dimension " << dim << endl;
                continue;
            }
            AddVectorVarToMetaData(md, var, meshName, AVT_ZONECENT, dim);
        }
    }
}


// ****************************************************************************
//  Method: avtExodusFileFormat::GetMesh
//
//  Purpose:
//      Returns the mesh for a specific timestep.
//
//  Arguments:
//      ts      The timestep of interest.
//      mesh    The name of the mesh of interest.
//
//  Returns:    The mesh.  Note that the "material" can be set beforehand and
//              that will alter how the mesh is read in.
//
//  Programmer: Hank Childs
//  Creation:   October 9, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 16:31:30 PST 2002  
//    Use NewInstance instead of MakeObject in order to match vtk's new api.
//
//    Hank Childs, Tue Apr  8 10:58:18 PDT 2003
//    Make sure the file is read in before proceeding.
//
// ****************************************************************************

vtkDataSet *
avtExodusFileFormat::GetMesh(int ts, const char *mesh)
{
    if (!readInFile)
    {
        ReadInFile();
    }

    if (strcmp(mesh, "Mesh") != 0)
    {
        EXCEPTION1(InvalidVariableException, mesh);
    }

    //
    // See if we have this cached.
    //
    const char *matname = NULL;
    if (doMaterialSelection)
    {
        matname = materialName;
    }
    else
    {
        matname = "_all";
    }
    int fts = 0;
    int dom = 0;
    vtkDataSet *cmesh = (vtkDataSet *) cache->GetVTKObject(mesh,
                            avtVariableCache::DATASET_NAME, fts, dom, matname);
    if (cmesh != NULL)
    {
        // The reference count will be decremented by the generic database,
        // because it will assume it owns it.
        cmesh->Register(NULL);
        return cmesh;
    }

    SetTimestep(ts);

    vtkDataSet *ds = ForceRead("_none");

    vtkDataSet *rv = NULL;

    if (ds != NULL)
    {
        rv = (vtkDataSet *) ds->NewInstance();
        rv->ShallowCopy(ds);

        //
        // Cache the mesh back.
        //
        cache->CacheVTKObject(mesh, avtVariableCache::DATASET_NAME, fts, dom,
                              matname, rv);
    }

    return rv;
}


// ****************************************************************************
//  Method: avtExodusFileFormat::GetVar
//
//  Purpose:
//      Gets the scalar variable for the specified timestep and variable.
//
//  Arguments:
//      ts      The timestep of interest.
//      mesh    The name of the mesh of interest.
//
//  Returns:    The variable.  Note that the "material" can be set beforehand
//              and that will alter how the variable is read in.
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Tue Apr  8 10:58:18 PDT 2003
//    Make sure the file is read in before proceeding.
//
// ****************************************************************************

vtkDataArray *
avtExodusFileFormat::GetVar(int ts, const char *var)
{
    if (!readInFile)
    {
        ReadInFile();
    }

    vtkDataArray *rv = NULL;

    SetTimestep(ts);

    vtkDataSet *ds = ForceRead(var);

    //
    // This may be a block not appropriate for this file.
    //
    if (ds == NULL)
    {
        return NULL;
    }

    if (ds->GetPointData()->GetScalars())
    {
        rv = ds->GetPointData()->GetScalars();
    }
    else
    {
        if (ds->GetCellData()->GetScalars())
        {
            rv = ds->GetCellData()->GetScalars();
        }
        else
        {
            EXCEPTION1(InvalidVariableException, var);
        }
    }

    rv->Register(NULL);
    return rv;
}


// ****************************************************************************
//  Method: avtExodusFileFormat::GetVectorVar
//
//  Purpose:
//      Gets the vector variable for the specified timestep and variable.
//
//  Arguments:
//      ts      The timestep of interest.
//      mesh    The name of the mesh of interest.
//
//  Returns:    The variable.  Note that the "material" can be set beforehand
//              and that will alter how the variable is read in.
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkVectors has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Tue Apr  8 10:58:18 PDT 2003
//    Make sure the file is read in before proceeding.
//
// ****************************************************************************

vtkDataArray *
avtExodusFileFormat::GetVectorVar(int ts, const char *var)
{
    if (!readInFile)
    {
        ReadInFile();
    }

    vtkDataArray *rv = NULL;

    SetTimestep(ts);

    vtkDataSet *ds = ForceRead(var);

    //
    // This may be a block not appropriate for this file.
    //
    if (ds == NULL)
    {
        return NULL;
    }

    if (ds->GetPointData()->GetVectors())
    {
        rv = ds->GetPointData()->GetVectors();
    }
    else
    {
        if (ds->GetCellData()->GetVectors())
        {
            rv = ds->GetCellData()->GetVectors();
        }
        else
        {
            EXCEPTION1(InvalidVariableException, var);
        }
    }

    rv->Register(NULL);
    return rv;
}


// ****************************************************************************
//  Method: avtExodusFileFormat::SetTimestep
//
//  Purpose:
//      Sets the timestep of the Exodus file.
//
//  Programmer: Hank Childs
//  Creation:   October 9, 2001
//
// ****************************************************************************

void
avtExodusFileFormat::SetTimestep(int ts)
{
    int nTimesteps = GetNTimesteps();
    if (ts < 0 || ts >= nTimesteps)
    {
        EXCEPTION2(BadIndexException, ts, nTimesteps);
    }

    vtkExodusReader *rdr = GetReader();
    rdr->SetTimeStep(ts+1);
}


// ****************************************************************************
//  Method: avtExodusFileFormat::GetBlockInformation
//
//  Purpose:
//      Determine which block we should read in.
//
//  Arguments:
//      bIdx    A location to place a block index.
//
//  Returns:    True if we should read in only one block, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   October 9, 2001
//
// ****************************************************************************

bool
avtExodusFileFormat::GetBlockInformation(int &bIdx)
{
    if (!doMaterialSelection)
    {
        return false;
    }

    bool gotBlock = false;
    for (int i = 0 ; i < numBlocks ; i++)
    {
        char name[1024];
        sprintf(name, "%d", blockId[i]);
        if (strcmp(name, materialName) == 0)
        {
            bIdx = i;
            gotBlock = true;
            break;
        }
    }

    if (!gotBlock)
    {
        debug1 << "Exodus file format could not find block "
               << materialName << endl;
        return false;
    }

    return true;
}


// ****************************************************************************
//  Method: avtExodusFileFormat::LoadVariable
//
//  Purpose:
//      Tells the exodus reader which variables it should load.
//
//  Arguments:
//      rdr     A vtkExodusReader.
//      name    The name of the variable.
//
//  Programmer: Hank Childs
//  Creation:   October 10, 2001
//
// ****************************************************************************

void
avtExodusFileFormat::LoadVariable(vtkExodusReader *rdr, const char *name)
{
    int   i;

    int   nPtsVars  = pointVars.size();
    int   nCellVars = cellVars.size();

    //
    // Start off by telling the reader that it should not load any variables.
    //
    for (i = 0 ; i < nPtsVars ; i++)
    {
        rdr->SetPointDataArrayLoadFlag(i, 0);
    }
    for (i = 0 ; i < nCellVars ; i++)
    {
        rdr->SetCellDataArrayLoadFlag(i, 0);
    }

    //
    // We may not want to load any variables (like for a mesh plot).
    //
    if (strcmp(name, "_none") == 0)
    {
        return;
    }

    //
    // Now try to identify if we want to load a variable and tell the reader
    // about it.
    //
    for (i = 0 ; i < nPtsVars ; i++)
    {
        if (strcmp(pointVars[i].c_str(), name) == 0)
        {
            rdr->SetPointDataArrayLoadFlag(i, 1);
        }
    }
    for (i = 0 ; i < nCellVars ; i++)
    {
        if (strcmp(cellVars[i].c_str(), name) == 0)
        {
            rdr->SetCellDataArrayLoadFlag(i, 1);
        }
    }
}


// ****************************************************************************
//  Method: avtExodusFileFormat::ForceRead
//
//  Purpose:
//      This forces the exodus file to read in what we have requested.  Note
//      that the reader is smart and caches the geometry, so getting a dataset
//      when all we want is a variable isn't as bad as it sounds.
//
//  Arguments:
//      var     The variable to read in (may be "_none").
//
//  Returns:    The mesh (or piece of the mesh if we are doing material
//              selection).
//
//  Programmer: Hank Childs
//  Creation:   October 10, 2001
//
// ****************************************************************************

vtkDataSet *
avtExodusFileFormat::ForceRead(const char *var)
{
    vtkExodusReader *rdr = GetReader();

    //
    // Determine which block we should read in.
    //
    int  bId;
    bool useBlockIds = GetBlockInformation(bId);

    //
    // It might be a block that is not valid for this domain (this frequently
    // happens with Exodus files).  If so, just return a dummy.
    //
    if (useBlockIds)
    {
        if (!validBlock[bId])
        {
            debug5 << "Asked Exodus file for a block that it does not have, "
                   << "returning NULL." << endl;
            return NULL;
        }
    }

    //
    // We can use the "pieces" mechanism to only read in part of the Exodus
    // file.  We can bypass it if we want to read in the whole thing -- the
    // else case.
    //
    vtkDataSet *ds = rdr->GetOutput();
    if (useBlockIds)
    {
        ds->SetUpdatePiece(bId);
        ds->SetUpdateNumberOfPieces(numBlocks);
    }
    else
    {
        ds->SetUpdatePiece(0);
        ds->SetUpdateNumberOfPieces(1);
    }

    //
    // Use a helper function to set up the reader to only read in the variable
    // we want.  (It may be "_none", which LoadVariable knows how to handle.)
    //
    LoadVariable(rdr, var);

    //
    // Of course, nothing will be valid until we do an Update.
    //
    ds->Update();

    return ds;
}


