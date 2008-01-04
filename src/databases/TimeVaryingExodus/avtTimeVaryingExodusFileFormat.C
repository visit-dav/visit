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
//                       avtTimeVaryingExodusFileFormat.C                    //
// ************************************************************************* //

#include <avtTimeVaryingExodusFileFormat.h>

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
//  Method: avtTimeVaryingExodusFileFormat constructor
//
//  Arguments:
//      name    The name of the exodus file.
//
//  Programmer: Hank Childs
//  Creation:   February 15, 2002
//
// ****************************************************************************

avtTimeVaryingExodusFileFormat::avtTimeVaryingExodusFileFormat(
                                                              const char *name)
   : avtSTSDFileFormat(name)
{
    //
    // Out GetReader routine will be lots happier if this is initialized.
    //
    reader = NULL;

    readInDataset = false;

    //
    // We need to maintain our own cache, since meshes do not change over
    // timesteps (but the exodus format is the only thing that knows that).
    //
    cache = new avtVariableCache;
}


// ****************************************************************************
//  Method: avtTimeVaryingExodusFileFormat::ReadInDataset
//
//  Purpose:
//      Reads in the dataset.
//
//  Programmer: Hank Childs
//  Creation:   April 7, 2003
//
// ****************************************************************************

void
avtTimeVaryingExodusFileFormat::ReadInDataset(void)
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

    readInDataset = true;
}


// ****************************************************************************
//  Method: avtTimeVaryingExodusFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   February 15, 2002
//
// ****************************************************************************

avtTimeVaryingExodusFileFormat::~avtTimeVaryingExodusFileFormat()
{
    FreeUpResources();
}


// ****************************************************************************
//  Method: avtTimeVaryingExodusFileFormat::FreeUpResources
//
//  Purpose:
//      Frees up resources, like memory and file descriptors.
//
//  Programmer: Hank Childs
//  Creation:   February 15, 2002
//
// ****************************************************************************

void
avtTimeVaryingExodusFileFormat::FreeUpResources(void)
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
//  Method: avtTimeVaryingExodusFileFormat::GetReader
//
//  Purpose:
//      Gets a reader.  This is always done through this method, so that
//      resources can be freed underneath the covers and then regained here.
//
//  Programmer: Hank Childs
//  Creation:   February 15, 2002
//
// ****************************************************************************

vtkExodusReader *
avtTimeVaryingExodusFileFormat::GetReader(void)
{
    if (reader != NULL)
    {
        return reader;
    }

    reader = vtkExodusReader::New();
    reader->SetFileName(filename);

    //
    // Everything we want will assume that at least the information has been
    // updated.
    //
    reader->GetOutput()->UpdateInformation();
    reader->SetTimeStep(1);

    return reader;
}


// ****************************************************************************
//  Method: avtTimeVaryingExodusFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta-data from the Exodus file.
//
//  Programmer: Hank Childs
//  Creation:   February 15, 2002
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
//    Hank Childs, Mon Apr  7 18:10:10 PDT 2003
//    Do not assume that the dataset has already been read in.
//
// ****************************************************************************

void
avtTimeVaryingExodusFileFormat::PopulateDatabaseMetaData(
                                                       avtDatabaseMetaData *md)
{
    if (!readInDataset)
    {
        ReadInDataset();
    }

    int   i;

    vtkExodusReader *rdr = GetReader();

    string meshName = "Mesh";
    int spatialDimension = rdr->GetDimensionality();

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
    md->Add(mesh);

    //
    // Sort the block ids so the user thinks we know what we are doing.
    //
    vector<int> sortedBlockIds = blockId;
    sort(sortedBlockIds.begin(), sortedBlockIds.end());

    //
    // Set up the material.
    //
    string materialName = "Material";
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
//  Method: avtTimeVaryingExodusFileFormat::GetMesh
//
//  Purpose:
//      Returns the mesh for a specific timestep.
//
//  Arguments:
//      mesh    The name of the mesh of interest.
//
//  Returns:    The mesh.  Note that the "material" can be set beforehand and
//              that will alter how the mesh is read in.
//
//  Programmer: Hank Childs
//  Creation:   February 15, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 16:38:37 PST 2002
//    Use NewInstance instead of MakeObject, in order to match vtk's new api.
//
//    Hank Childs, Mon Apr  7 18:10:10 PDT 2003
//    Do not assume that the dataset has already been read in.
//
// ****************************************************************************

vtkDataSet *
avtTimeVaryingExodusFileFormat::GetMesh(const char *mesh)
{
    if (!readInDataset)
    {
        ReadInDataset();
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
//  Method: avtTimeVaryingExodusFileFormat::GetVar
//
//  Purpose:
//      Gets the scalar variable for the specified timestep and variable.
//
//  Arguments:
//      mesh    The name of the mesh of interest.
//
//  Returns:    The variable.  Note that the "material" can be set beforehand
//              and that will alter how the variable is read in.
//
//  Programmer: Hank Childs
//  Creation:   February 15, 2002
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar  6 08:20:17 PST 2002 
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Mon Apr  7 18:10:10 PDT 2003
//    Do not assume that the dataset has already been read in.
//
// ****************************************************************************

vtkDataArray *
avtTimeVaryingExodusFileFormat::GetVar(const char *var)
{
    if (!readInDataset)
    {
        ReadInDataset();
    }

    vtkDataArray *rv = NULL;

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
//  Method: avtTimeVaryingExodusFileFormat::GetVectorVar
//
//  Purpose:
//      Gets the vector variable for the specified timestep and variable.
//
//  Arguments:
//      mesh    The name of the mesh of interest.
//
//  Returns:    The variable.  Note that the "material" can be set beforehand
//              and that will alter how the variable is read in.
//
//  Programmer: Hank Childs
//  Creation:   February 15, 2002
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar  6 08:20:17 PST 2002 
//    vtkVectors has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Mon Apr  7 18:10:10 PDT 2003
//    Do not assume that the dataset has already been read in.
//
// ****************************************************************************

vtkDataArray *
avtTimeVaryingExodusFileFormat::GetVectorVar(const char *var)
{
    if (!readInDataset)
    {
        ReadInDataset();
    }

    vtkDataArray *rv = NULL;

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
//  Method: avtTimeVaryingExodusFileFormat::GetBlockInformation
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
//  Creation:   February 15, 2002
//
// ****************************************************************************

bool
avtTimeVaryingExodusFileFormat::GetBlockInformation(int &bIdx)
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
//  Method: avtTimeVaryingExodusFileFormat::LoadVariable
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
avtTimeVaryingExodusFileFormat::LoadVariable(vtkExodusReader *rdr,
                                             const char *name)
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
//  Method: avtTimeVaryingExodusFileFormat::ForceRead
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
avtTimeVaryingExodusFileFormat::ForceRead(const char *var)
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

