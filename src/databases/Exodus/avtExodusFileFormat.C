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
//                           avtExodusFileFormat.C                           //
// ************************************************************************* //

#include <avtExodusFileFormat.h>

#include <algorithm>

#include "vtkVisItExodusReader.h"
#include <vtkCellData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPointData.h>

#include <avtDatabaseMetaData.h>
#include <avtMaterial.h>
#include <avtVariableCache.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <InvalidVariableException.h>


using     std::string;
using     std::vector;
using     std::sort;


vector< vector<string> >   *avtExodusFileFormat::globalFileLists = NULL;


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
//    Hank Childs, Thu Jul 22 14:28:10 PDT 2004
//    Add support for registering file lists.
//
//    Hank Childs, Mon Aug 23 13:41:54 PDT 2004
//    Rename cache object to avoid namespace conflict with base class.
//
// ****************************************************************************

avtExodusFileFormat::avtExodusFileFormat(const char *name)
   : avtMTSDFileFormat(&name, 1)
{
    reader = NULL;
    exodusCache  = NULL;
    readInFile = false;
    fileList = -1;
}


// ****************************************************************************
//  Method: avtExodusFileFormat::RegisterFileList
//
//  Purpose:
//      Registers a file list with this format.  The file list is the "real"
//      list of file names in the Nemesis file.
//
//  Programmer: Hank Childs
//  Creation:   July 22, 2004
//
// ****************************************************************************

int
avtExodusFileFormat::RegisterFileList(const char *const *list, int nlist)
{
    if (globalFileLists == NULL)
    {
        globalFileLists = new vector< vector<string> >;
    }

    vector<string> thisList(nlist);
    for (int i = 0 ; i < nlist ; i++)
        thisList[i] = list[i];

    globalFileLists->push_back(thisList);

    return globalFileLists->size()-1;
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
//  Modifications:
//
//    Hank Childs, Mon Aug 23 13:41:54 PDT 2004
//    Rename cache object to avoid namespace conflict with base class.
//
//    Eric Brugger, Fri Mar  9 14:43:07 PST 2007
//    Added support for element block names.
//
// ****************************************************************************

void
avtExodusFileFormat::ReadInFile(void)
{
    int  i;

    //
    // Do some initialization.
    //
    vtkVisItExodusReader *rdr = GetReader();

    //
    // Determine what the blocks are and how many there are.
    //
    numBlocks = rdr->GetNumberOfBlocks();
    validBlock.resize(numBlocks);
    blockId.resize(numBlocks);
    blockName.resize(numBlocks);
    for (i = 0 ; i < numBlocks; i++)
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
        blockName[i] = rdr->GetBlockName(i);
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
    exodusCache = new avtVariableCache;

    readInFile = true;
}


// ****************************************************************************
//  Method: avtExodusFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Aug 23 13:41:54 PDT 2004
//    Rename cache object to avoid namespace conflict with base class.
//
// ****************************************************************************

avtExodusFileFormat::~avtExodusFileFormat()
{
    if (reader != NULL)
    {
        reader->Delete();
        reader = NULL;
    }
    if (exodusCache != NULL)
    {
        delete exodusCache;
        exodusCache = NULL;
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
//    Hank Childs, Thu Jul 22 10:43:36 PDT 2004
//    Tell the reader to generate block ids.
//
//    Mark C. Miller, Mon Aug  9 19:12:24 PDT 2004
//    Tell the reader to generate global zone ids
//
// ****************************************************************************

vtkVisItExodusReader *
avtExodusFileFormat::GetReader(void)
{
    if (reader != NULL)
    {
        return reader;
    }

    reader = vtkVisItExodusReader::New();
    reader->SetFileName(filenames[0]);
    reader->SetGenerateNodeGlobalIdArray(1);
    reader->SetGenerateElementGlobalIdArray(1);
    reader->SetGenerateBlockIdCellArray(1);

    //
    // Everything we want will assume that at least the information has been
    // updated.
    //
    reader->GetOutput()->UpdateInformation();

    return reader;
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
//  Modifications:
//
//    Hank Childs, Wed Jul 14 07:35:25 PDT 2004
//    Explicitly tell the reader to load the times -- it turns out that this
//    is a costly operation and we should only do it when necessary -- hence
//    we explicitly tell the reader when we want them.
//
// ****************************************************************************

void
avtExodusFileFormat::GetTimes(vector<double> &times)
{
    int nTimesteps = GetNTimesteps();

    if (reader != NULL)
        reader->LoadTimes();
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
//    Hank Childs, Thu Jul 22 14:41:51 PDT 2004
//    Use the real filenames when creating the SIL.
//
//    Mark C. Miller, Mon Aug  9 19:12:24 PDT 2004
//    Removed setting of avtMeshMetadata->containsGlobalNodeIds
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added timeState arg since this is an MTXX database. Nonetheless,
//    timeState argument is ununsed
//
//    Eric Brugger, Fri Mar  9 14:43:07 PST 2007
//    Added support for element block names.
//
// ****************************************************************************

void
avtExodusFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
    int timeState)
{
    if (!readInFile)
    {
        ReadInFile();
    }

    int   i;

    vtkVisItExodusReader *rdr = GetReader();

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
    mesh->blockTitle = "File";
    if (globalFileLists != NULL && fileList >= 0 && 
        (fileList < globalFileLists->size()))
    {
        mesh->blockNames = (*globalFileLists)[fileList];
    }
    mesh->blockPieceName = "File";
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
    if (numBlocks > 0 && blockName[0] == "")
    {
        //
        // Sort the block ids so the user thinks we know what we are doing.
        //
        vector<int> sortedBlockIds = blockId;
        sort(sortedBlockIds.begin(), sortedBlockIds.end());

        for (i = 0 ; i < numBlocks ; i++)
        {
            char name[128];
            sprintf(name, "%d", sortedBlockIds[i]);
            matNames.push_back(name);
        }
    }
    else
    {
        for (i = 0 ; i < numBlocks ; i++)
        {
            matNames.push_back(blockName[i]);
        }
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
//    Mark C. Miller, Thu Aug  5 14:17:36 PDT 2004
//    Moved whole of implementation to ReadMesh
//
// ****************************************************************************

vtkDataSet *
avtExodusFileFormat::GetMesh(int ts, const char *mesh)
{
    bool okToRemoveAndCacheGlobalIds = true;
    return ReadMesh(ts, mesh, okToRemoveAndCacheGlobalIds);
}

// ****************************************************************************
//  Method: avtExodusFileFormat::ReadMesh
//
//  Purpose:
//      Returns the mesh for a specific timestep and, optionally, can remove
//      and cache global node and zone ids in AVT's VoidRefCache
//
//  Arguments:
//      ts      The timestep of interest.
//      mesh    The name of the mesh of interest.
//      ok...   Boolean indicating if remove and caching of global id
//              information is permitted.
//
//  Returns:    The mesh.  Note that the "material" can be set beforehand and
//              that will alter how the mesh is read in.
//
//  Programmer: Hank Childs
//  Creation:   October 9, 2001
//
//  Modifications:
//
//    Mark C. Miller,
//    Renamed from GetMesh
//    Added bool argument for okToRemoveAndCacheGlobalIds
//    Added code to remove and cache global node and zone ids
//
//    Hank Childs, Mon Aug 23 13:41:54 PDT 2004
//    Rename cache object to avoid namespace conflict with base class.
//    Also increment reference counts of arrays that are becoming void refs.
//
// ****************************************************************************

vtkDataSet *
avtExodusFileFormat::ReadMesh(int ts, const char *mesh,
                              bool okToRemoveAndCacheGlobalIds)
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
    const char *matname = "_all";
    int fts = 0;
    int dom = 0;
    vtkDataSet *cmesh = (vtkDataSet *) exodusCache->GetVTKObject(mesh,
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
        if (okToRemoveAndCacheGlobalIds)
        {
            //
            // We've decided that global node and zone ids are AVT_AUXILIARY
            // data and have a life of their own apart from any mesh they are
            // associated with. So, if the VTK dataset we've obtained has
            // them, here, we remove them and cache them in AVT separately.
            //
            vtkDataArray *gnodeIds =
                               ds->GetPointData()->GetArray("avtGlobalNodeId");
            if (gnodeIds != NULL)
            {
                gnodeIds->Register(NULL);
                void_ref_ptr vr = void_ref_ptr(gnodeIds, 
                                          avtVariableCache::DestructVTKObject);
                cache->CacheVoidRef(mesh, AUXILIARY_DATA_GLOBAL_NODE_IDS,
                                    -1, myDomain, vr);
            }

            vtkDataArray *gzoneIds =
                                ds->GetCellData()->GetArray("ElementGlobalId");
            if (gzoneIds != NULL)
            {
                gzoneIds->Register(NULL);
                void_ref_ptr vr = void_ref_ptr(gzoneIds,
                                          avtVariableCache::DestructVTKObject);
                cache->CacheVoidRef(mesh, AUXILIARY_DATA_GLOBAL_ZONE_IDS,
                                    -1, myDomain, vr);
            }
        }

        rv = (vtkDataSet *) ds->NewInstance();
        rv->ShallowCopy(ds);

        //
        // Cache the mesh back.
        //
        exodusCache->CacheVTKObject(mesh, avtVariableCache::DATASET_NAME, fts, 
                                    dom, matname, rv);
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
//  Method: avtExodusFileFormat::GetAuxiliaryData
//
//  Purpose:
//      Gets the auxiliary data specified.
//
//  Arguments:
//      var        The variable of interest.
//      ts         The timestep of interest.
//      type       The type of auxiliary data.
//      <unnamed>  The arguments for that type -- not used.
//      df         Destructor function.
//
//  Returns:    The auxiliary data.
//
//  Programmer: Hank Childs
//  Creation:   July 22, 2004
//
//  Modifications:
//
//    Mark C. Miller, Mon Aug  9 19:12:24 PDT 2004
//    Added code to read global node/zone ids. Unfortunately, I saw no easy
//    way to do it *without* reading the whole mesh first. In typical usage,
//    that will have already been done and the global node/zone ids cached and
//    so we'll rarely, if ever, wind up in here making an explicit request
//    for them.
//
//    Hank Childs, Mon Aug 23 13:41:54 PDT 2004
//    Rename cache object to avoid namespace conflict with base class.
//    Also increment reference counts of arrays that are becoming void refs.
//
//    Hank Childs, Tue Feb  6 15:47:22 PST 2007
//    Do not assume that global node ids exist.
//
//    Eric Brugger, Fri Mar  9 14:43:07 PST 2007
//    Added support for element block names.
//
// ****************************************************************************

void *
avtExodusFileFormat::GetAuxiliaryData(const char *var, int ts, 
                                    const char * type, void *,
                                    DestructorFunction &df)
{
    int   i;

    if (strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        if (strstr(var, "ElementBlock") != var)
            EXCEPTION1(InvalidVariableException, var);

        vtkDataSet *ds = ForceRead("_none");
        if (ds == NULL)
            return NULL;
        vtkDataArray *arr = ds->GetCellData()->GetArray("BlockId");
        if (arr == NULL)
            return NULL;

        int nzones = ds->GetNumberOfCells();

        std::vector<std::string> mats(numBlocks);
        if (numBlocks > 0 && blockName[0] == "")
        {
            vector<int> sortedBlockIds = blockId;
            sort(sortedBlockIds.begin(), sortedBlockIds.end());

            for (i = 0 ; i < numBlocks ; i++)
            {
                char num[1024];
                sprintf(num, "%d", sortedBlockIds[i]);
                mats[i] = num;
            }
        }
        else
        {
            for (i = 0 ; i < numBlocks ; i++)
            {
                mats[i] = blockName[i];
            }
        }

        int *mat_0 = new int[nzones];
        int *ptr = (int *) arr->GetVoidPointer(0);
        for (i = 0 ; i < nzones ; i++)
            mat_0[i] = ptr[i]-1;
        avtMaterial *mat = new avtMaterial(numBlocks, mats, nzones, mat_0,
                                           0, NULL, NULL, NULL, NULL);
        delete [] mat_0;
        df = avtMaterial::Destruct;
        return (void*) mat;
    }
    else if (strcmp(type, AUXILIARY_DATA_GLOBAL_NODE_IDS) == 0)
    {
        // Unfortunately, without making broad changes to vtkVisItExodusReader
        // the only way to obtain global node ids is by first asking
        // the reader for the mesh and then taking them from that if
        // they are present. Fortunately, in most situations, the
        // mesh will have already been read and the global node ids
        // cached and we'll never wind up here having to read global
        // node ids for the first time without having already read
        // the mesh
        bool okToRemoveAndCacheGlobalIds = false;
        vtkDataSet *ds = ReadMesh(ts, "Mesh", okToRemoveAndCacheGlobalIds);

        //
        // If we happend to find that there exists also global zone Ids,
        // we'll remove them from from the vtkDataSet object and stick
        // them in the AVT cache now.
        //
        vtkDataArray *gzoneIds =ds->GetCellData()->GetArray("ElementGlobalId");
        if (gzoneIds != NULL)
        {
            gzoneIds->Register(NULL);
            void_ref_ptr vr = void_ref_ptr(gzoneIds,
                                          avtVariableCache::DestructVTKObject);
            cache->CacheVoidRef("Mesh", AUXILIARY_DATA_GLOBAL_ZONE_IDS, 
                                -1, myDomain, vr);
        }

        //
        // Remove the global node ids from the vtkDataSet object but DO NOT
        // cache them as avtGenericDatabase will do that for us upon return
        // from this call
        //
        vtkDataArray *gnodeIds=ds->GetPointData()->GetArray("avtGlobalNodeId");

        //
        // Return what we came here for
        //
        if (gnodeIds != NULL)
            gnodeIds->Register(NULL);
        df = avtVariableCache::DestructVTKObject;
        return (void*) gnodeIds;
    }
    else if (strcmp(type, AUXILIARY_DATA_GLOBAL_ZONE_IDS) == 0)
    {
        //
        // See long note, above, for AUXILIARY_DATA_GLOBAL_NODE_IDS query
        //
        bool okToRemoveAndCacheGlobalIds = false;
        vtkDataSet *ds = ReadMesh(ts, "Mesh", okToRemoveAndCacheGlobalIds);

        //
        // If we happend to find that there exists also global node Ids,
        // we'll remove them from from the vtkDataSet object and stick
        // them in the AVT cache now.
        //
        vtkDataArray *gnodeIds=ds->GetPointData()->GetArray("avtGlobalNodeId");
        if (gnodeIds != NULL)
        {
            gnodeIds->Register(NULL);
            void_ref_ptr vr = void_ref_ptr(gnodeIds,
                                          avtVariableCache::DestructVTKObject);
            cache->CacheVoidRef("Mesh", AUXILIARY_DATA_GLOBAL_NODE_IDS,
                                -1, myDomain, vr);
        }

        //
        // Remove the global zone ids from the vtkDataSet object but DO NOT
        // cache them as avtGenericDatabase will do that for us upon return
        // from this call
        //
        vtkDataArray *gzoneIds =ds->GetCellData()->GetArray("ElementGlobalId");

        //
        // Return what we came here for
        //
        gzoneIds->Register(NULL);
        df = avtVariableCache::DestructVTKObject;
        return (void*) gzoneIds;
    }

    return NULL;
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

    vtkVisItExodusReader *rdr = GetReader();
    rdr->SetTimeStep(ts+1);
}


// ****************************************************************************
//  Method: avtExodusFileFormat::LoadVariable
//
//  Purpose:
//      Tells the exodus reader which variables it should load.
//
//  Arguments:
//      rdr     A vtkVisItExodusReader.
//      name    The name of the variable.
//
//  Programmer: Hank Childs
//  Creation:   October 10, 2001
//
// ****************************************************************************

void
avtExodusFileFormat::LoadVariable(vtkVisItExodusReader *rdr, const char *name)
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
    vtkVisItExodusReader *rdr = GetReader();

    vtkDataSet *ds = rdr->GetOutput();
    ds->SetUpdatePiece(0);
    ds->SetUpdateNumberOfPieces(1);

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


