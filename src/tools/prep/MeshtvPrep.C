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
//                              MeshtvPrep.C                                 //
// ************************************************************************* //

//
// Includes
//

#include <visitstream.h>
#include <stdlib.h>
#include <string.h>

#include <silo.h>

#include "MeshtvPrep.h"
#include "BoundaryList.h"
#include "MeshtvPrepLib.h"
#include "OnionPeel.h"


// 
// Definition of static consts.  
//

char * const   MeshtvPrep::COORDS_NAME = "Coords";
char * const   MeshtvPrep::LOW_RES_DIR = "/lowres";
char * const   MeshtvPrep::MED_RES_DIR = "/medres";


// 
// Definition of global variables.  Poor form, but easiest way to have 
// current state accessed by all of the Value objects.
//

int                  currentState = 0;
NamingConvention    *stateNameMaker = NULL;


//
// Definition of static functions.
//

float  *GetExtents(DBfile *, char *meshname, char *component);


// ****************************************************************************
//  Method: MeshtvPrep constructor
//
//  Programmer: Hank Childs
//  Creation:   December 3, 1999
//
// ****************************************************************************

MeshtvPrep::MeshtvPrep()
{
    code                  = NULL;
    fields                = NULL;
    fieldsN               = 0;
    coords                = NULL;
    meshes                = NULL;
    medres                = NULL;
    lowres                = NULL;
    meshesN               = 0;
    materials             = NULL;
    repository            = NULL;
    structuredTopology    = NULL;
    structuredTopologyN   = 0;
    unstructuredTopology  = NULL;
    unstructuredTopologyN = 0;
}


// ****************************************************************************
//  Method: MeshtvPrep destructor
//
//  Programmer: Hank Childs
//  Creation:   December 3, 1999
//
// ****************************************************************************

MeshtvPrep::~MeshtvPrep()
{
    if (code != NULL)
    {
        delete code;
    }
    if (fields != NULL)
    {
        delete [] fields;
    }
    if (coords != NULL)
    {
        delete [] coords;
    }
    if (lowres != NULL)
    {
        delete [] lowres;
    }
    if (medres != NULL)
    {
        delete [] medres;
    }
    if (meshes != NULL)
    {
        delete [] meshes;
    }
    if (materials != NULL)
    {
        delete [] materials;
    }
    if (repository != NULL)
    {
        delete [] repository;
    }
    if (structuredTopology != NULL)
    {
        delete [] structuredTopology;
    }
    if (unstructuredTopology != NULL)
    {
        delete [] unstructuredTopology;
    }

    // validVars is owned by another object
    //if (0) 
    //{
    //  delete [] validVars;
    //}
}


// ****************************************************************************
//  Method: MeshtvPrep::PreProcess
//
//  Purpose:
//      Reads in all of the SILO files specified on the command line and
//      performs several operations on them, again depending on the command
//      line.  These include resampling, creating a binary space partition,
//      creating a connectivity list, and storing information about material
//      interfaces.  It outputs SILO files in the new SILO format.
//
//  Programmer: Hank Childs
//  Creation:   December 3, 1999
//
//  Modifications:
//
//    Hank Childs, Thu Jun 15 17:10:24 PDT 2000
//    Changed arguments to some function calls to no longer include the list
//    of files from the command line.  Also removed unused automatic variables.
//
//    Hank Childs, Fri Jun 16 07:54:22 PDT 2000
//    Only called write out mesh files if state is 0.
//
//    Hank Childs, Thu Sep  7 16:58:34 PDT 2000
//    Call routine to initialize and populate objects that exist across states.
//
// ****************************************************************************

void
MeshtvPrep::PreProcess(CommandLine &cl)
{
    Initialize(cl);

    for (int state = 0 ; state < statesN ; state++)
    {
        //
        // Used by some auxillary routines to create output files with the
        // correct name. 
        //
        currentState = state;

        //
        // Initializes all of the state dependent fields.
        //
        InitializeState();

        //
        // Reading in the root files will allow us to know how many objects we
        // need to process the files.
        //
        ReadRootFiles(state);

        //
        // The initial pass allows us to know the size of some of the objects
        // we are going to process.
        //
        MakeInitialPass(state);

        //
        // Now that sizes and such have been read in in the initial pass, let
        // the objects know that the initial pass is done.
        //
        ConsolidateObjects();

        ReadNormalFiles(state);

        //
        // Allow the objects that do additional preprocessing after all of 
        // the information has been read in.
        //
        WrapUpObjects();

        //
        // We have written out the large arrays as we have gone, but now
        // write out the objects that we have generated.
        //
        if (state == 0)
        {
            WriteOutMeshFiles();
        }
        WriteOutStateFiles();

        //
        // Make root interval trees that reflect what the bounds are for each
        // state.
        //
        if (doIntervalTrees)
        {
            if (state == 0)
            {
                CreateRootIntervalTrees();
            }
            PopulateRootIntervalTrees(state);
        }

        //
        // Destroy all of the state dependent objects.
        //
        CleanupState();
    }

    WriteOutTimeInvariantFiles();
}


// ****************************************************************************
//  Method: MeshtvPrep::ReadRootFiles
//
//  Purpose:
//      Reads in a root file for the given state and initializes objects that 
//      contain data in that file.
//      
//  Arguments:
//      state     The current state.
//
//  Programmer: Hank Childs
//  Creation:   December 6, 1999
//
//  Modifications:
//
//    Hank Childs, Thu Jun 15 16:35:29 PDT 2000
//    Changed arguments in call to GetRootFile and removed arguments list
//    and listSize.
//
// ****************************************************************************

void 
MeshtvPrep::ReadRootFiles(int state)
{
    char *filename = code->GetRootFile(state);
    cerr << "Processing root file " << filename << endl;

    //
    // Open the root file for the current state.
    //
    DBfile *dbfile = DBOpen(filename, DB_UNKNOWN, DB_READ);
    if (dbfile == NULL)
    {
        cerr << "Error: Unable to open file \"" << filename << "\"." << endl;
        exit(EXIT_FAILURE);
    }
    DBtoc  *toc = DBGetToc(dbfile);
    if (toc == NULL)
    {
        cerr << "Error: Unable to get a toc from \"" << filename << "\"."
             << endl;
        exit(EXIT_FAILURE);
    }    
   
    //
    // Some information is needed by many objects.  This is the place to
    // read in that information so that it can be fed to them later.  This
    // prevents SILO objects from being read multiple times when not
    // necessary.
    //
    repository->ReadRoot(dbfile, toc);

    //
    // Create the variably sized objects from the information in the
    // root file.
    //
    CreateObjects(dbfile, toc);

    //
    // Give those objects that need to read information from the root
    // file a chance to do so.
    //
    ObjectReadRootFiles(dbfile, toc);

    DBClose(dbfile);
}


// ****************************************************************************
//  Method:  MeshtvPrep::ObjectReadRootFiles
//
//  Arguments:
//      dbfile    The root file to read in from.
//      toc       The current table of contents from the root file.
//
//  Purpose:
//      Allows the objects that need direct access to the root file a chance
//      to read from it.  The only object that currently needs to read from
//      the root file is the TimeSequence object.
//
//  Programmer: Hank Childs
//  Creation:   December 8, 1999
//
//  Modifications:
//
//    Hank Childs, Fri Jun 16 13:35:29 PDT 2000
//    Made time object not read in the root file if we're not doing to the Silo
//    object conversion.
//
// ****************************************************************************

void
MeshtvPrep::ObjectReadRootFiles(DBfile *dbfile, DBtoc *toc)
{
    if (doSiloConversion)
    {
        time.ReadFromRootFile(dbfile, toc);
    }
}


// ****************************************************************************
//  Method: MeshtvPrep::ReadNormalFiles
//
//  Purpose:
//      Reads in the data from all of the normal files.
// 
//  Arguments:
//      state   The current state.
//
//  Programmer: Hank Childs
//  Creation:   December 6, 1999
//
//  Modifications:
//
//    Hank Childs, Thu Jun 15 16:35:29 PDT 2000
//    Changed arguments in call to GetFile and removed arguments list and
//    listSize.
//
// ****************************************************************************

void
MeshtvPrep::ReadNormalFiles(int state)
{
    //
    // Create the output files for this state.
    // 
    CreateOutputFiles(state);

    //
    // Pick a file to open.
    //
    int     fileCount   = 0;
    char   *currentFile = NULL;
    while ( (currentFile = code->GetFile(state, fileCount++)) 
             != NULL)
    {
        cerr << "Working on " << currentFile << endl;

        //
        // Open the file for input.
        //
        DBfile *dbfile = DBOpen(currentFile, DB_UNKNOWN, DB_READ);
        if (dbfile == NULL)
        {
            cerr << "Error: Unable to open file \"" << currentFile << "\"." 
                 << endl;
            exit(EXIT_FAILURE);
        }
        DBtoc  *toc    = DBGetToc(dbfile);
        if (toc == NULL)
        {
            cerr << "Error: Unable to get a toc from \"" << currentFile 
                 << "\"." << endl;
            exit(EXIT_FAILURE);
        }    

        //
        // Have iterate dirs do all the work for reading through the
        // subdirectories and have it invoke ReadDir each time.
        //
        IterateDirs(&MeshtvPrep::ReadDir, dbfile, toc, currentFile, "/");

        //
        // Close the file and pick another.
        //
        DBClose(dbfile);

    }  // end 'while' over files for a state
}


// ****************************************************************************
//  Method: MeshtvPrep::MakeInitialPass
//
//  Purpose:
//      Makes an initial pass to figure out information needed to create 
//      objects.
//
//  Arguments:
//      state   The current state.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 1999
//
//  Modifications:
//
//    Hank Childs, Thu Jun 15 16:35:29 PDT 2000
//    Changed arguments in call to GetFile and removed arguments list and
//    listSize.
//
// ****************************************************************************

void
MeshtvPrep::MakeInitialPass(int state)
{
    int     fileI     = 0;
    char   *curFile = NULL;
    while ((curFile = code->GetFile(state, fileI++)) != NULL)
    {
        cerr << "Performing initial pass on \"" << curFile << "\"." << endl;

        DBfile *dbfile = DBOpen(curFile, DB_UNKNOWN, DB_READ);
        if (dbfile == NULL)
        {
            cerr << "Error: Unable to open file \"" << curFile << "\"." <<endl;
            exit(EXIT_FAILURE);
        }
        DBtoc   *toc = DBGetToc(dbfile);
 
        IterateDirs(&MeshtvPrep::InitialPass, dbfile, toc, curFile, "/");

        DBClose(dbfile);
    }
}    


// ****************************************************************************
//  Method: MeshtvPrep::InitialPass
//
//  Purpose:
//      Combine the two previous initial passes, DetermineSizes and
//      DetermineExtents into one routine to avoid reading in the tocs, files
//      two times.
//
//  Programmer: Hank Childs
//  Creation:   February 15, 2000
//
// ****************************************************************************

void
MeshtvPrep::InitialPass(DBfile *dbfile, DBtoc *toc, char *file, char *dir)
{
    //
    // Determine the sizes of arrays for writing later.
    //
    DetermineFieldSizes(dbfile, toc, file, dir);
    DetermineMeshSizes(dbfile, toc, file, dir);
    DetermineMaterialSizes(dbfile, toc, file, dir);
    DetermineTopologySizes(dbfile, toc, file, dir);

    //
    // We only need the mesh extents if resampling is going to be done.
    // If not, return.
    //
    if (doLowRes || doMedRes)
    {
        DetermineMeshExtents(dbfile, toc, file, dir);
    }
}


// ****************************************************************************
//  Method: MeshtvPrep::CreateObjects
//
//  Purpose:
//      Looks at the table of contents and determines which objects need to
//      be created to store the information in this file.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
// ****************************************************************************

void
MeshtvPrep::CreateObjects(DBfile *dbfile, DBtoc *toc)
{
    CreateFields(toc);
    CreateMeshes(toc);
    CreateCoords(toc);
    CreateTopologies(dbfile, toc);
    CreateMaterials(toc);
}


// ****************************************************************************
//  Method: MeshtvPrep::CreateCoords
//
//  Purpose:
//      Creates the coord Field objects, which contains the Value objects and
//      IntervalTree objects.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
// ****************************************************************************

void
MeshtvPrep::CreateCoords(DBtoc *toc)
{
    int   nMesh = toc->nmultimesh + toc->nqmesh + toc->nucdmesh + toc->nptmesh;
    coords         = new Field_Prep[nMesh];

    int   last = 0;
    int   i;

    for (i = 0 ; i < toc->nmultimesh ; i++)
    {
        coords[last].SetName(toc->multimesh_names[i], COORDS_NAME);

        int nDoms = repository->GetMeshNDomains(toc->multimesh_names[i]);
        coords[last].SetNDomains(nDoms);

        last++;
    }

    for (i = 0 ; i < toc->nucdmesh ; i++)
    {
        coords[last].SetName(toc->ucdmesh_names[i], COORDS_NAME);
        coords[last].SetNDomains(1);

        last++;
    }

    for (i = 0 ; i < toc->nqmesh ; i++)
    {
        coords[last].SetName(toc->qmesh_names[i], COORDS_NAME);
        coords[last].SetNDomains(1);

        last++;
    }

    for (i = 0 ; i < toc->nptmesh ; i++)
    {
        coords[last].SetName(toc->ptmesh_names[i], COORDS_NAME);
        coords[last].SetNDomains(1);

        last++;
    }
}


// ****************************************************************************
//  Method: MeshtvPrep::CreateTopologies
//
//  Purpose:
//      Looks at the table of contents and determines how many topology objects
//      need to be created.  Also populates the topologies with a small amount
//      of information.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2000
//
// ****************************************************************************

void
MeshtvPrep::CreateTopologies(DBfile *dbfile, DBtoc *toc)
{
    int   i, j, sTopoI = 0, uTopoI = 0;

    // 
    // Note that these are both overestimates since the multimeshes will
    // (most likely) not be both structured and unstructured.
    //
    structuredTopologyN   = toc->nmultimesh + toc->nqmesh;
    structuredTopology    = new StructuredTopology_Prep[structuredTopologyN];
    unstructuredTopologyN = toc->nmultimesh + toc->nucdmesh;
    unstructuredTopology  
                 = new UnstructuredTopology_Prep[unstructuredTopologyN];

    for (i = 0 ; i < toc->nmultimesh ; i++)
    {
        DBmultimesh *mm = DBGetMultimesh(dbfile, toc->multimesh_names[i]);

        //
        // Check to see if there are unstructured meshes in the multimesh.
        //
        for (j = 0 ; j < mm->nblocks ; j++)
        {
            if (mm->meshtypes[j] == DB_UCDMESH)
            {
                unstructuredTopology[uTopoI].SetName(toc->multimesh_names[i]);
                unstructuredTopology[uTopoI].SetDomains(mm->nblocks);
                uTopoI++;
                break;
            }
        }

        //
        // Check to see if there are structured meshes in the multimesh.
        //
        for (j = 0 ; j < mm->nblocks ; j++)
        {
            if (mm->meshtypes[j] == DB_QUADMESH)
            {
                structuredTopology[sTopoI].SetName(toc->multimesh_names[i]);
                structuredTopology[sTopoI].SetDomains(mm->nblocks);
                sTopoI++;
                break;
            }
        }

        DBFreeMultimesh(mm);
    }

    for (i = 0 ; i < toc->nucdmesh ; i++)
    {
        unstructuredTopology[uTopoI].SetName(toc->ucdmesh_names[i]);
        unstructuredTopology[uTopoI].SetDomains(1);
        uTopoI++;
    }

    for (i = 0 ; i < toc->nqmesh ; i++)
    {
        structuredTopology[sTopoI].SetName(toc->qmesh_names[i]);
        structuredTopology[sTopoI].SetDomains(1);
        sTopoI++;
    }

    // 
    // No processing related to topology for point meshes.
    //

    //
    // Because we have allocated the topologies as blocks of objects via 
    // arrays, we can free them through the 'delete []' operator.  Therefore
    // we don't need to keep track of how many are allocated and can bump
    // the 'N' counts to be how many are valid.
    //
    structuredTopologyN   = sTopoI;
    unstructuredTopologyN = uTopoI;
}


// ****************************************************************************
//  Method: MeshtvPrep::CreateMeshes
//
//  Purpose:
//      Looks at the the table of contents and determines how many meshes
//      need to be created.  Also creates the resampling objects (if specified)
//      
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
// ****************************************************************************

void
MeshtvPrep::CreateMeshes(DBtoc *toc)
{
    int   i, meshesI = 0;

    meshesN = toc->nmultimesh + toc->nqmesh + toc->nucdmesh + toc->nptmesh;
    meshes = new Mesh_Prep[meshesN];

    if (doLowRes)
    {
        lowres = new Resample[meshesN];
        for (i = 0 ; i < meshesN ; i++) 
        {
            lowres[i].SetResolution(lowresN, LOW_RES_DIR);
        }
    }
    if (doMedRes)
    {
        medres = new Resample[meshesN];
        for (i = 0 ; i < meshesN ; i++) 
        {
            medres[i].SetResolution(medresN, MED_RES_DIR);
        }
    }
 
    for (i = 0 ; i < toc->nmultimesh ; i++)
    {
        meshes[meshesI].SetName(toc->multimesh_names[i]);

        int nDoms = repository->GetMeshNDomains(toc->multimesh_names[i]);
        meshes[meshesI].SetDomains(nDoms);

        meshesI++;
    }

    for (i = 0 ; i < toc->nucdmesh ; i++)
    {
        meshes[meshesI].SetName(toc->ucdmesh_names[i]);

        int nDoms = repository->GetMeshNDomains(toc->ucdmesh_names[i]);
        meshes[meshesI].SetDomains(nDoms);

        meshesI++;
    }

    for (i = 0 ; i < toc->nqmesh ; i++)
    {
        meshes[meshesI].SetName(toc->qmesh_names[i]);

        int nDoms = repository->GetMeshNDomains(toc->qmesh_names[i]);
        meshes[meshesI].SetDomains(nDoms);

        meshesI++;
    }

    for (i = 0 ; i < toc->nptmesh ; i++)
    {
        meshes[meshesI].SetName(toc->ptmesh_names[i]);

        int nDoms = repository->GetMeshNDomains(toc->ptmesh_names[i]);
        meshes[meshesI].SetDomains(nDoms);

        meshesI++;
    }
}


// ****************************************************************************
//  Method: MeshtvPrep::CreateFields
//
//  Purpose: 
//      Creates a field for each field in the toc.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
// ****************************************************************************

void
MeshtvPrep::CreateFields(DBtoc *toc)
{
    //
    // Keep track of max fields because not every field may actually get
    // added because of the command line var list.  This is a little bloated 
    // (to allocate a bigger pointer array than needed), but by far the 
    // easiest.
    //
    int   maxField = toc->nmultivar + toc->nqvar + toc->nucdvar + toc->nptvar;
    fields         = new Field_Prep[maxField];
    fieldsN = 0;
 
    int   i;
    for (i = 0 ; i < toc->nmultivar ; i++)
    {
        if (ValidName(toc->multivar_names[i]) == 0)
        {
            continue;
        }

        fields[fieldsN].SetName("", toc->multivar_names[i]);

        int nDoms = repository->GetVarNDomains(toc->multivar_names[i]);
        fields[fieldsN].SetNDomains(nDoms);

        fieldsN++;
    }

    for (i = 0 ; i < toc->nucdvar ; i++)
    {
        if (ValidName(toc->ucdvar_names[i]) == 0)
        {
            continue;
        }

        fields[fieldsN].SetName("", toc->ucdvar_names[i]);
        fields[fieldsN].SetNDomains(1);

        fieldsN++;
    }

    for (i = 0 ; i < toc->nqvar ; i++)
    {
        if (ValidName(toc->qvar_names[i]) == 0)
        {
            continue;
        }

        fields[fieldsN].SetName("", toc->qvar_names[i]);
        fields[fieldsN].SetNDomains(1);

        fieldsN++;
    }

    for (i = 0 ; i < toc->nptvar ; i++)
    {
        if (ValidName(toc->ptvar_names[i]) == 0)
        {
            continue;
        }

        fields[fieldsN].SetName("", toc->ptvar_names[i]);
        fields[fieldsN].SetNDomains(1);

        fieldsN++;
    }
}


// ****************************************************************************
//  Method: MeshtvPrep::CreateMaterials
//
//  Purpose:
//      Looks at a root file and determines how many Material objects need
//      to be created.
//
//  Note:       The first argument, the root file, and the third argument, 
//              the state, are unused.
//
//  Programmer: Hank Childs
//  Creation:   January 7, 2000
//
// ****************************************************************************

void
MeshtvPrep::CreateMaterials(DBtoc *toc)
{
    int  i;
    int  materialsI = 0;

    materialsN = toc->nmat + toc->nmultimat;
    materials  = new Material_Prep[materialsN];

    for (i = 0 ; i < toc->nmultimat ; i++)
    {
        materials[materialsI].SetName(toc->multimat_names[i]);
        int  nDoms = repository->GetMaterialNDomains(toc->multimat_names[i]);
        materials[materialsI].SetNDomains(nDoms);

        materialsI++;
    }

    for (i = 0 ; i < toc->nmat ; i++)
    {
        materials[materialsI].SetName(toc->mat_names[i]);
        materials[materialsI].SetNDomains(1);
      
        materialsI++;
    }
}


// ****************************************************************************
//  Method: MeshtvPrep::ReadDir
//
//  Purpose:
//      Reads in all of the information for a directory.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
// ****************************************************************************

void
MeshtvPrep::ReadDir(DBfile *dbfile, DBtoc *toc, char *file, char *dir)
{
    //
    // The meshes must be read in before the vars so that resampling on vars
    // can take place with a reference to a valid mesh.
    //
    ReadInMeshes(dbfile, toc);

    ReadMeshes(dbfile, toc, file, dir);
    ReadVars(dbfile, toc, file, dir);
    ReadMaterials(dbfile, toc, file, dir);

    //
    // We now have used the meshes for everything we want to, so delete them.
    //
    DeleteMeshes();
}


// ****************************************************************************
//  Method: MeshtvPrep::ReadMaterials
//
//  Purpose:
//      Reads in the materials and processes them by creating the Material
//      and BoundaryList objects.
//
//  Programmer: Hank Childs
//  Creation:   January 7, 2000
//
//  Modifications:
// 
//      Hank Childs, Tue Apr  4 15:35:17 PDT 2000
//      Changed mechanism for getting mesh name to safer Silo call instead of
//      manually going through the components.
//
//      Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//      Did not populate material if switch doSiloConversion is off.
//
// ****************************************************************************

void
MeshtvPrep::ReadMaterials(DBfile *dbfile, DBtoc *toc, char *file, char *dir)
{
    // 
    // Note that multi-materials are ignored since they have nothing to
    // add at this level.
    //

    for (int i = 0 ; i < toc->nmat ; i++)
    {
        DBmaterial  *mat = DBGetMaterial(dbfile, toc->mat_names[i]);
        
        //
        // Get the material index number.
        //
        int   mIndex = MaterialIndex(toc->mat_names[i]);
        if (mIndex < 0)
        {
            //
            //  This material was not in the root file.  Ignore.
            //
            continue;
        }

        char meshname[LONG_STRING];
        DBInqMeshname(dbfile, toc->mat_names[i], meshname);

        //
        // Resolve the number of zones in the mesh.
        //
        int nZones = repository->GetMeshNZones(meshname);
        if (nZones < 0)
        {
            cerr << "Could not resolve how many zones there were for \""
                 << meshname << "\"." << endl;
            exit(EXIT_FAILURE);
        }
       
        //
        // Determine the current domain number.
        //
        int domain = repository->GetMaterialDomain(toc->mat_names[i], 
                                                   file, dir);
        int nDoms = repository->GetMaterialNDomains(toc->mat_names[i]);

        //
        // Good spot for BoundaryList.
        //

        //
        // Add this material to our Material object and all of the Value
        // objects associated with materials.
        //
        if (doSiloConversion)
        {
            DBfile *outFile = GetOutputFile(STATE_FILE, domain, nDoms);
            materials[mIndex].AddMaterial(mat, meshname, nZones, domain, 
                                          outFile);
            RelinquishOutputFile(STATE_FILE, outFile);
        }

        DBFreeMaterial(mat);
    }
}


// ***************************************************************************
//  Method: MeshtvPrep::ReadMeshes
//
//  Purpose:
//      Reads in all of the information pertaining to meshes for a directory.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
// ****************************************************************************

void
MeshtvPrep::ReadMeshes(DBfile *dbfile, DBtoc *toc, char *file, char *dir)
{
    //
    // Multimeshes are ignored here since they have nothing to add at this 
    // level.
    //

    int   i;

    for (i = 0 ; i < toc->nucdmesh ; i++)
    {
        ProcessUcdmesh(dbfile, toc, file, dir, i);
    }

    for (i = 0 ; i < toc->nqmesh ; i++)
    {
        ProcessQuadmesh(dbfile, toc, file, dir, i);
    }

    for (i = 0 ; i < toc->nptmesh ; i++)
    {
        ProcessPointmesh(dbfile, toc, file, dir, i);
    }
}


// ****************************************************************************
//  Method: MeshtvPrep::ProcessUcdmesh
//
//  Purpose:
//      Does all of the processing on Ucd meshes.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//      Put in logic not to populate the Mesh_Prep's if the silo conversion
//      switch is off and to do the interval trees separately.
//
// ****************************************************************************

void
MeshtvPrep::ProcessUcdmesh(DBfile *, DBtoc *toc, char *file, char *dir, int i)
{
    //
    // Determine if this is a mesh we are interested in.
    //
    int index = MeshIndex(toc->ucdmesh_names[i]);
    if (index < 0)
    {
        return;
    }

    //
    // Retrieve the mesh from the repository.
    //
    DBucdmesh  *um = repository->GetUcdmesh(toc->ucdmesh_names[i]);
    int domain = repository->GetMeshDomain(toc->ucdmesh_names[i], file, dir);
    int nDoms  = repository->GetMeshNDomains(toc->ucdmesh_names[i]);

    //
    // Assign this mesh to its Resample object so that it can resample vars.
    //
    if (doLowRes)
    {
        lowres[index].AddMesh(um);
    }
    if (doMedRes)
    {
        medres[index].AddMesh(um);
    }

    if (doOnionPeel)
    {
        OnionPeel onionPeel;
        onionPeel.AddMesh(um, domain);

        //
        // Get control of the mesh file to write the onion peel 
        // information.
        //
        DBfile *outFile = GetOutputFile(MESH_FILE, domain, nDoms);
        onionPeel.Write(outFile);
        RelinquishOutputFile(MESH_FILE, outFile);
    }

    if (doIntervalTrees)
    {
        coords[index].IntervalTreeReadVar(um, domain);
    }

    if (doSiloConversion)
    {
        //
        // Read the mesh into the mesh object.
        //
        meshes[index].ReadMesh(um);

        //
        // Get control of the output file for meshes.
        //
        DBfile *meshFile = GetOutputFile(MESH_FILE, domain, nDoms);
     
        //
        // Write the arrays associated with the unstructured topology.
        //
        int  uIndex = UnstructuredTopologyIndex(toc->ucdmesh_names[i]);
        unstructuredTopology[uIndex].WriteArrays(meshFile, um, domain);

        //
        // Relinquish control of the output file for meshes.
        //
        RelinquishOutputFile(MESH_FILE, meshFile);

        //
        // Get control of the output file for states.
        //
        DBfile *outFile = GetOutputFile(STATE_FILE, domain, nDoms);
    
        // 
        // Have the coordinates Field object read the coordinates and write 
        // out its arrays.
        //
        coords[index].ReadVarAndWriteArrays(outFile, um, domain);

        //
        // Relinquish control of the output file for states.
        //
        RelinquishOutputFile(STATE_FILE, outFile);
    }
}


// ****************************************************************************
//  Method: MeshtvPrep::ProcessQuadmesh
//
//  Purpose:
//      Does all of the processing on Quad meshes.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//      Put in logic not to populate the Mesh_Prep's if the silo conversion
//      switch is off and to do the interval trees separately.
//
// ****************************************************************************

void
MeshtvPrep::ProcessQuadmesh(DBfile *, DBtoc *toc, char *file, char *dir, int i)
{
    //
    // Determine if this is a mesh we are interested in.
    //
    int index = MeshIndex(toc->qmesh_names[i]);
    if (index < 0)
    {
        return;
    }

    //
    // Retrieve the mesh from the repository.
    //
    DBquadmesh  *qm = repository->GetQuadmesh(toc->qmesh_names[i]);
    int domain = repository->GetMeshDomain(toc->qmesh_names[i], file, dir);
    int nDoms  = repository->GetMeshNDomains(toc->qmesh_names[i]);

    //
    // Assign this mesh to its Resample object so that it can resample vars.
    //
    if (doLowRes)
    {
        lowres[index].AddMesh(qm);
    }
    if (doMedRes)
    {
        medres[index].AddMesh(qm);
    }

    if (doIntervalTrees)
    {
        coords[index].IntervalTreeReadVar(qm, domain);
    }

    if (doSiloConversion)
    {
        //
        // Read the mesh into the mesh object.
        //
        meshes[index].ReadMesh(qm);

        //
        // Have the structured topology object read the mesh.
        // Note that it does not need to write out an array like an 
        // unstructured topology does because it can keep track of its own 
        // (small) array.
        //
        int  sIndex = StructuredTopologyIndex(toc->qmesh_names[i]);
        structuredTopology[sIndex].ReadMesh(qm, domain);

        //
        // Get control of the output file.
        //
        DBfile *outFile = GetOutputFile(STATE_FILE, domain, nDoms);
    
        // 
        // Have the coordinates Field object read the mesh.
        //
        coords[index].ReadVarAndWriteArrays(outFile, qm, domain);

        //
        // Relinquish control of the output file.
        //
        RelinquishOutputFile(STATE_FILE, outFile);
    }
}


// ****************************************************************************
//  Method: MeshtvPrep::ProcessPointmesh
//
//  Purpose:
//      Does all of the processing on Point meshes.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//      Put in logic not to populate the Mesh_Prep's if the silo conversion
//      switch is off and to do the interval trees separately.
//
// ****************************************************************************

void
MeshtvPrep::ProcessPointmesh(DBfile *, DBtoc *toc, char *file, char *dir,int i)
{
    //
    // Determine if this is a mesh we are interested in.
    //
    int index = MeshIndex(toc->ptmesh_names[i]);
    if (index < 0)
    {
        return;
    }

    //
    // Retrieve the mesh from the repository.
    //
    DBpointmesh  *pm = repository->GetPointmesh(toc->ptmesh_names[i]);
    int domain = repository->GetMeshDomain(toc->ptmesh_names[i], file, dir);
    int nDoms  = repository->GetMeshNDomains(toc->ptmesh_names[i]);

    if (doIntervalTrees)
    {
        coords[index].IntervalTreeReadVar(pm, domain);
    }

    if (doSiloConversion)
    {
        //
        // Read the mesh into the mesh object.
        //
        meshes[index].ReadMesh(pm);
    
        //
        // Get control of the output file.
        //
        DBfile *outFile = GetOutputFile(STATE_FILE, domain, nDoms);

        // 
        // Have the coordinates Field object read the mesh.
        //
        coords[index].ReadVarAndWriteArrays(outFile, pm, domain);

        //
        // Relinquish control of the output file.
        //
        RelinquishOutputFile(STATE_FILE, outFile);
    }
}


// ****************************************************************************
//  Method: MeshtvPrep::ReadInMeshes
//
//  Purpose:
//      Reads the meshes into the repository so that other processing can
//      access the meshes (ie resampling vars).
//
//  Note:       This routine allocates many meshes which must be deleted later
//              by a call to delete meshes.
//
//  Creation:   December 20, 1999
//
// ****************************************************************************

void
MeshtvPrep::ReadInMeshes(DBfile *dbfile, DBtoc *toc)
{
    int   i, index;

    //
    // Multi-meshes are not added since they have nothing to add at this level.
    //

    repository->SetNUcdmesh(toc->nucdmesh);
    for (i = 0 ; i < toc->nucdmesh ; i++)
    {
        index = MeshIndex(toc->ucdmesh_names[i]);
        if (index < 0)
        {
            continue;
        }
        DBucdmesh  *um = DBGetUcdmesh(dbfile, toc->ucdmesh_names[i]);
        repository->AddMesh(um, toc->ucdmesh_names[i]);
    }   

    repository->SetNQuadmesh(toc->nqmesh);
    for (i = 0 ; i < toc->nqmesh ; i++)
    {
        index = MeshIndex(toc->qmesh_names[i]);
        if (index < 0)
        {
            continue;
        }
        DBquadmesh  *qm = DBGetQuadmesh(dbfile, toc->qmesh_names[i]);
        repository->AddMesh(qm, toc->qmesh_names[i]);
    }   

    repository->SetNPointmesh(toc->nptmesh);
    for (i = 0 ; i < toc->nptmesh ; i++)
    {
        index = MeshIndex(toc->ptmesh_names[i]);
        if (index < 0)
        {
            continue;
        }
        DBpointmesh  *pm = DBGetPointmesh(dbfile, toc->ptmesh_names[i]);
        repository->AddMesh(pm, toc->ptmesh_names[i]);
    }   
}


// ****************************************************************************
//  Method: MeshtvPrep::DeleteMeshes
//
//  Purpose:
//      Now that everything that needs to make use of the meshes is done, free
//      the memory.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
// ****************************************************************************

void
MeshtvPrep::DeleteMeshes()
{
    repository->DeleteMeshes();
}


// ****************************************************************************
//  Method: MeshtvPrep::ReadVars
//
//  Purpose:
//      Reads in all of the information pertaining to variables for a 
//      directory.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
// ****************************************************************************

void
MeshtvPrep::ReadVars(DBfile *dbfile, DBtoc *toc, char *file, 
                     char *dir)
{
    int    i;
  
    // 
    // multivars are ignored here since they have nothing to add at this level.
    //

    for (i = 0 ; i < toc->nucdvar ; i++)
    {
        ProcessUcdvar(dbfile, toc, file, dir, i);
    }

    for (i = 0 ; i < toc->nqvar ; i++)
    {
        ProcessQuadvar(dbfile, toc, file, dir, i);
    }

    for (i = 0 ; i < toc->nptvar ; i++)
    {
        ProcessMeshvar(dbfile, toc, file, dir, i);
    }
}


// ****************************************************************************
//  Method: MeshtvPrep::ProcessUcdvar
//
//  Purpose:
//      Processes a Ucdvar.  This includes adding it to the TimeSequence
//      and Field objects, resampling it, and writing out the data array to the
//      output file.
//
//  Arguments:
//      dbfile  -  The currently open file to read the variable from.
//      toc     -  The table of contents object from the dbfile.
//      file    -  The name of the currently open file.  Used for figuring out
//                 which domain is being read.
//      dir     -  The name of the directory in the currently open file.  Used 
//                 for figuring which domain is being read.
//      i       -  The index into the table of contents for the var that is
//                 supposed to be read.
//
//  Programmer: Hank Childs
//  Creation:   December 15, 1999
//
//  Modifications:
// 
//      Hank Childs, Tue Apr  4 15:35:17 PDT 2000
//      Changed mechanism for getting mesh name to safer Silo call from old
//      way of asking the repository.
//
//      Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//      Put in logic not to populate the Field_Prep's if the silo conversion
//      switch is off and to do the interval trees separately.
//
// ****************************************************************************

void
MeshtvPrep::ProcessUcdvar(DBfile *dbfile, DBtoc *toc, char *file, char *dir, 
                          int i)
{
    //
    // Check to see if this is a variable that was specified in the var list.
    //
    int index = FieldIndex(toc->ucdvar_names[i]);
    if (index < 0)
    {
        return;
    }

    DBucdvar *uv = DBGetUcdvar(dbfile, toc->ucdvar_names[i]);

    //
    // Calculate prepatory information for ucdvar.
    //
    char   meshName[LONG_STRING];
    DBInqMeshname(dbfile, toc->ucdvar_names[i], meshName);
    fields[index].SetMeshName(meshName);
    int    domain = repository->GetVarDomain(toc->ucdvar_names[i], file, dir);
    int    nDoms  = repository->GetVarNDomains(toc->ucdvar_names[i]);

    //
    // Resample Variable
    //
    if (doLowRes || doMedRes)
    {
        int  meshIndex = MeshIndex(fields[index].GetMeshName());
        if (meshIndex < 0)
        {
            cerr << "Invalid mesh for resampling." << endl;
            exit(EXIT_FAILURE);
        }
        if (doLowRes)
        {
            lowres[meshIndex].AddVar(uv, index);
        }
        if (doMedRes)
        {
            medres[meshIndex].AddVar(uv, index);
        }
    }

    if (doIntervalTrees)
    {
        fields[index].IntervalTreeReadVar(uv, domain);
    }

    if (doSiloConversion)
    {
        //
        // Add Ucdvar to TimeSequence.
        //
        time.ReadVar(toc->ucdvar_names[i], uv);

        //
        // Get control of the output file.
        //
        DBfile *outFile = GetOutputFile(STATE_FILE, domain, nDoms);
    
        //
        // Read in the var and write out its arrays.
        //
        fields[index].ReadVarAndWriteArrays(outFile, uv, domain);

        //
        // Relinquish control of the output file.
        //
        RelinquishOutputFile(STATE_FILE, outFile);

    }
    DBFreeUcdvar(uv);
}


// ****************************************************************************
//  Method: MeshtvPrep::ProcessQuadvar
//
//  Purpose:
//      Processes a Quadvar.  This includes adding it to the TimeSequence
//      and Field objects, resampling it, and writing out the data arrays to 
//      the output file.
//
//  Arguments:
//      dbfile  -  The currently open file to read the variable from.
//      toc     -  The table of contents object from the dbfile.
//      file    -  The name of the currently open file.  Used for figuring out
//                 which domain is being read.
//      dir     -  The name of the directory in the currently open file.  Used 
//                 for figuring which domain is being read.
//      i       -  The index into the table of contents for the var that is
//                 supposed to be read.
//
//  Programmer: Hank Childs
//  Creation:   December 15, 1999
//
//  Modifications:
// 
//      Hank Childs, Tue Apr  4 15:35:17 PDT 2000
//      Changed mechanism for getting mesh name to safer Silo call from old
//      way of asking the repository.
//
//      Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//      Put in logic not to populate the Field_Prep's if the silo conversion
//      switch is off and to do the interval trees separately.
//
// ****************************************************************************

void
MeshtvPrep::ProcessQuadvar(DBfile *dbfile, DBtoc *toc, char *file, char *dir, 
                           int i)
{
    //
    // Check to see if this is a variable that was specified in the var list.
    //
    int index = FieldIndex(toc->qvar_names[i]);
    if (index < 0)
    {
        return;
    }

    DBquadvar *qv = DBGetQuadvar(dbfile, toc->qvar_names[i]);

    //
    // Calculate prepatory information for the quadvar.
    //
    char   meshName[LONG_STRING];
    DBInqMeshname(dbfile, toc->qvar_names[i], meshName);
    fields[index].SetMeshName(meshName);
    int    domain = repository->GetVarDomain(toc->qvar_names[i], file, dir);
    int    nDoms  = repository->GetVarNDomains(toc->qvar_names[i]);

    //
    // Resample Variable
    //
    if (doLowRes || doMedRes)
    {
        int  meshIndex = MeshIndex(fields[index].GetMeshName());
        if (meshIndex < 0)
        {
            cerr << "Invalid mesh for resampling." << endl;
            exit(EXIT_FAILURE);
        }
        if (doLowRes)
        {
            lowres[meshIndex].AddVar(qv, index);
        }
        if (doMedRes)
        {
            medres[meshIndex].AddVar(qv, index);
        }
    }

    if (doIntervalTrees)
    {
        fields[index].IntervalTreeReadVar(qv, domain);
    }

    if (doSiloConversion)
    {
        //
        // Add Quadvar to TimeSequence.
        //
        time.ReadVar(toc->qvar_names[i], qv);

        //
        // Get control of the output file.
        //
        DBfile *outFile = GetOutputFile(STATE_FILE, domain, nDoms);

        //
        // Read in the quad var and write out its arrays.
        //
        fields[index].ReadVarAndWriteArrays(outFile, qv, domain);
    
        //
        // Relinquish control of the output file.
        //
        RelinquishOutputFile(STATE_FILE, outFile);
    }

    DBFreeQuadvar(qv);
}


// ****************************************************************************
//  Method: MeshtvPrep::ProcessMeshvar
//
//  Purpose:
//      Processes a Meshvar.  This includes adding it to the TimeSequence
//      and Field objects, resampling it, and writing out the data arrays to 
//      the output file.
//
//  Arguments:
//      dbfile  -  The currently open file to read the variable from.
//      toc     -  The table of contents object from the dbfile.
//      file    -  The name of the currently open file.  Used for figuring out
//                 which domain is being read.
//      dir     -  The name of the directory in the currently open file.  Used 
//                 for figuring which domain is being read.
//      i       -  The index into the table of contents for the var that is
//                 supposed to be read.
//
//  Programmer: Hank Childs
//  Creation:   December 15, 1999
//
//  Modifications:
// 
//      Hank Childs, Tue Apr  4 15:35:17 PDT 2000
//      Changed mechanism for getting mesh name to safer Silo call from old
//      way of asking the repository.
//
//      Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//      Put in logic not to populate the Field_Prep's if the silo conversion
//      switch is off and to do the interval trees separately.
//
// ****************************************************************************

void
MeshtvPrep::ProcessMeshvar(DBfile *dbfile, DBtoc *toc, char *file, char *dir, 
                           int i)
{
    //
    // Check to see if this is a variable that was specified in the var list.
    //
    int index = FieldIndex(toc->ptvar_names[i]);
    if (index < 0)
    {
        return;
    }

    DBmeshvar *mv = DBGetPointvar(dbfile, toc->ptvar_names[i]);

    // 
    // Calculate the prepatory information for the mesh var.
    //
    char   meshName[LONG_STRING];
    DBInqMeshname(dbfile, toc->ptvar_names[i], meshName);
    fields[index].SetMeshName(meshName);
    int domain = repository->GetVarDomain(toc->ptvar_names[i], file, dir);
    int nDoms  = repository->GetVarNDomains(toc->ptvar_names[i]);

    //
    // No resampling for meshvars, but this would be the place to do it.
    //

    if (doIntervalTrees)
    {
        fields[index].IntervalTreeReadVar(mv, domain);
    }

    if (doSiloConversion)
    {
        //
        // Add Meshvar to TimeSequence.
        //
        time.ReadVar(toc->ptvar_names[i], mv);

        //
        // Get control of the output file.
        //
        DBfile *outFile = GetOutputFile(STATE_FILE, domain, nDoms);
    
        //
        // Read in the mesh var and write out its arrays.
        //
        fields[index].ReadVarAndWriteArrays(outFile, mv, domain);

        //
        // Relinquish control of the output file.
        //
        RelinquishOutputFile(STATE_FILE, outFile);
    }

    DBFreeMeshvar(mv);
}


// ****************************************************************************
//  Method: MeshtvPrep::StructuredTopologyIndex
//
//  Purpose:
//      Gets the index of the structured topology that has the name given as 
//      an argument.
//
//  Returns:    The index of the structured topology.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2000
//
// ****************************************************************************

int
MeshtvPrep::StructuredTopologyIndex(char *name)
{
    for (int i = 0 ; i < structuredTopologyN ; i++)
    {
        if (strcmp(name, structuredTopology[i].GetName()) == 0)
        {
            return i;
        }
    }
 
    //
    // No match.
    //
    return -1;
}
 

// ****************************************************************************
//  Method: MeshtvPrep::UntructuredTopologyIndex
//
//  Purpose:
//      Gets the index of the unstructured topology that has the name given as 
//      an argument.
//
//  Returns:    The index of the unstructured topology.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2000
//
// ****************************************************************************

int
MeshtvPrep::UnstructuredTopologyIndex(char *name)
{
    for (int i = 0 ; i < unstructuredTopologyN ; i++)
    {
        if (strcmp(name, unstructuredTopology[i].GetName()) == 0)
        {
            return i;
        }
    }
 
    //
    // No match.
    //
    return -1;
}
 

// ****************************************************************************
//  Method: MeshtvPrep::MaterialIndex
//
//  Purpose:
//      Gets the index of a material in the 'materials' data member.
//
//  Returns:    The index in 'materials' or -1 if the material does not exist.
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
// ****************************************************************************

int
MeshtvPrep::MaterialIndex(const char *name)
{
    for (int i = 0 ; i < materialsN ; i++)
    {
        if (strcmp(name, materials[i].GetName()) == 0)
        {
            return i;
        }
    }
 
    //
    // No match.
    //
    return -1;
}


// ****************************************************************************
//  Method: MeshtvPrep::MeshIndex
//
//  Purpose:
//      Gets the index of a mesh in the 'meshes' data member.
//
//  Arguments:
//      name    The name of the mesh.
//      
//  Returns:    The index in 'meshes' or -1 if the mesh does not exist.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
//  Modifications:
//
//      Hank Childs, Tue Jun 13 08:45:01 PDT 2000
//      Removed logic for trying to guess the correct index first, since it
//      was confusing and there aren't that many meshes.
//
// ****************************************************************************

int
MeshtvPrep::MeshIndex(const char *name)
{
    for (int i = 0 ; i < meshesN ; i++)
    {
        if (strcmp(meshes[i].GetName(), name) == 0)
        {
            return i;
        }
    }

    //
    // No match.
    //
    return -1;
}


// ****************************************************************************
//  Method: MeshtvPrep::FieldIndex
//
//  Purpose:
//      Get the field index and make sure that it is valid (ie it is on the
//      list of valid vars.
//
//  Arguments:
//      name    The name of the field.
//      
//  Returns:    The index of the object in the 'fields' data member, provided
//              that it exists and that the var is a valid var.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
// ****************************************************************************

int
MeshtvPrep::FieldIndex(const char *name)
{
   int rv = GetFieldIndex(name);
   if (rv < 0)
   {
       return rv;
   }

   //
   // We have found the variable in our field index, so make sure that it is 
   // on the valid variable list and return it.
   //
   if (ValidName(name))
   {
       return rv;
   }

   // 
   // In our field index, but not a valid var.
   //
   return -1;
}


// ****************************************************************************
//  Method: MeshtvPrep::ValidName
//
//  Purpose:
//      Checks to see if the variable name is on the valid name list.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
// ****************************************************************************

bool
MeshtvPrep::ValidName(const char *n)
{
    if (validVars == NULL)
    {
        //
        // The user did not specify a var list on the command line.  That means 
        // that all vars are valid.
        //
        return true;
    }

    for (int i = 0 ; i < validVarsN ; i++)
    {
        if (strcmp(validVars[i], n) == 0)
        {
            return true;
        }
    }
  
    return false;
}


// ****************************************************************************
//  Method: MeshtvPrep::GetFieldIndex
//
//  Purpose: 
//      Iterates through the field objects and determines which one has a name
//      that matches the one given.
//
//  Arguments:
//      name    The name of the field.
//      
//  Returns:    The index of the object in the 'fields' data member, -1 on
//              failure.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
// 
//  Modifications:
//
//      Hank Childs, Tue Jun 13 08:45:01 PDT 2000
//      Removed logic for trying to guess the correct index first, since it
//      was confusing and there aren't that many meshes.
//
// ****************************************************************************

int
MeshtvPrep::GetFieldIndex(const char *name)
{
    //
    // Couldn't match the suggested one, so iterate through all choices.
    //
    for (int i = 0 ; i < fieldsN ; i++)
    {
        if (strcmp(fields[i].GetName(), name) == 0)
        {
            return i;
        }
    }

    //
    // No match.
    //
    return -1;
}


// ****************************************************************************
//  Method: MeshtvPrep::Initialize
//
//  Purpose:
//      Initialize those fields which are stored in the command line object,
//      determine which kind of code these files came from, and determine
//      the number of states.
// 
//  Arguments:
//      cl      An object that has parsed the command line.
//
//  Programmer: Hank Childs
//  Creation:   December 3, 1999
//
//  Modifications:
//
//    Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//    Added initialization of SiloConversion switch.
//
//    Hank Childs, Thu Jun 15 13:55:27 PDT 2000
//    Removed call to Code::IdentifyStates because that is now done in the
//    Code's constructor.
//
//    Hank Childs, Mon Jun 26 09:47:08 PDT 2000
//    Changed way resolution for resampling was obtained to allow for different
//    resolutions in each dimension.
//
// ****************************************************************************

void
MeshtvPrep::Initialize(CommandLine &cl)
{
    //
    // Seems as good of a place as any to initialize SILO stuff.
    //
    DBForceSingle(1);
    DBShowErrors(DB_ABORT, NULL);

    // 
    // Initialize the fields that are stored in the command line object.
    //
    validVars        = cl.GetVarList();
    validVarsN       = cl.GetNumVars();

    nFiles           = cl.GetNFiles();
    SetNFiles(nFiles);
    doMedRes         = cl.GetMedResSwitch();
    if (doMedRes)
    {
        cl.GetMedResolution(medresN);
    }
    doLowRes         = cl.GetLowResSwitch();
    if (doLowRes)
    {
        cl.GetLowResolution(lowresN);
    }
    doOnionPeel      = cl.GetConnectSwitch();
    doIntervalTrees  = cl.GetBSPSwitch();
    doSiloConversion = cl.GetSiloObjSwitch();

    outPrefix        = cl.GetOutputPrefix();
    char *p = outPrefix, *q = NULL;
    bool hasDir = false;
    while ( (q = strstr(p, "/")) != NULL)
    {
        hasDir = true;
        p = q+1;
    }
    if (hasDir)
    {
        char prefix[LONG_STRING];
        strncpy(prefix, outPrefix, p-outPrefix);
        prefix[p-outPrefix] = '\0';
        SetOutputDirectory(prefix);
        outPrefix = p;
    }

    //
    // The Value object needs to be initialized with the file prefix and
    // information about the number of files.
    //
    if (nFiles == 1)
    {
        Value_Prep::SetFileBase(NULL);
    }
    else
    {
        Value_Prep::SetFileBase(cl.GetOutputPrefix());
    }
    Value_Prep::SetNFiles(nFiles);
    Value_Prep::SetFileNum();

    //
    // Determine what kind of code is being processed.
    //
    char  **fileList     = cl.GetFiles();
    int     fileListSize = cl.GetNumFiles();
    DetermineGeneratingCode(fileList, fileListSize);
    SetStatesN(code->GetStatesN());
}


// ****************************************************************************
//  Method: MeshtvPrep::SetStatesN
//
//  Purpose:
//      Sets the statesN variable and gives its values to other objects that
//      need to know the number of states.  
//  
//  Arguments:
//      n     The number of states.
//
//  Programmer: Hank Childs
//  Creation:   January 11, 2000
//
// ****************************************************************************

void
MeshtvPrep::SetStatesN(int n)
{
    statesN = n;

    //
    // Initialize the objects that need to know the number of states.
    //
    time.SetStates(code);
}


// ****************************************************************************
//  Method:  MeshtvPrep::InitializeState
//
//  Purpose:
//      Create all of the objects needed for preprocessing a certain state.
//
//  Programmer: Hank Childs
//  Creation:   January 6, 2000
//
// ****************************************************************************
 
void
MeshtvPrep::InitializeState(void)
{
    fieldsN               = 0;
    fields                = NULL;
    coords                = NULL;
    meshes                = NULL;
    meshesN               = 0;
    structuredTopology    = NULL;
    structuredTopologyN   = 0;
    unstructuredTopology  = NULL;
    unstructuredTopologyN = 0;
    if (doLowRes)
    {
        lowres            = NULL;
    }
    if (doMedRes)
    {
        medres            = NULL;
    }

    // 
    // Make ourselves a new input repository
    // 
    repository = new InputRepository;

    //
    // Force control of the output file back to root processors.
    // This is the default, but it may have changed in the last state.
    //
    ForceControlToRoot();
}


// ****************************************************************************
//  Method: MeshtvPrep::CleanupState
//
//  Purpose:
//      We have now done all the preprocessing we will for the given state.
//      Clean up all objects that are dedicated to this state only.
//
//  Programmer: Hank Childs
//  Creation:   January 11, 2000
//
// ****************************************************************************

void
MeshtvPrep::CleanupState(void)
{
    if (fields != NULL)
    {
        delete [] fields;
        fields = NULL;
    }
    if (coords != NULL)
    {
        delete [] coords;
        coords = NULL;
    }
    fieldsN             = 0;

    if (meshes != NULL)
    {
        delete [] meshes;
        meshes = NULL;
    }
    meshesN             = 0;

    if (lowres != NULL)
    {
        delete [] lowres;
        lowres = NULL;
    }
    if (medres != NULL)
    {
        delete [] medres;
        medres = NULL;
    }

    if (structuredTopology != NULL)
    {
        delete [] structuredTopology;
        structuredTopology = NULL;
    }
    structuredTopologyN = 0;
    if (unstructuredTopology != NULL)
    {
        delete [] unstructuredTopology;
        unstructuredTopology = NULL;
    }
    unstructuredTopologyN = 0;

    if (repository != NULL)
    {
        delete repository;
        repository = NULL;
    }
}


// ****************************************************************************
//  Method: MeshtvPrep::DetermineGeneratingCode
//
//  Purpose: 
//      Determines what code the files came from.  
//
//  Programmer: Hank Childs
//  Creation:   December 3, 1999
//
//  Arguments:
//      list        A list of file names.
//      listSize    The number of names in list.
//
//  Modifications:
//      
//    Hank Childs, Thu Jun 15 13:39:55 PDT 2000
//    Instead of requiring knowledge of all of the derived types of Code,
//    used static method of Code to return instance of the correct derived
//    type.
//
//    Hank Childs, Wed Sep 13 10:50:57 PDT 2000
//    Changed name of Code to NamingConvention.
//
// ****************************************************************************

void
MeshtvPrep::DetermineGeneratingCode(char **list, int listSize)
{
    code = NamingConvention::DetermineNamingConvention(list, listSize);

    if (code == NULL)
    {
        //
        // An error message was already printed by Code::DetermineCode.
        //
        exit(EXIT_FAILURE);
    }

    //
    // Poor form, but Value objects need to be able to have access to the Code
    // object to get the state names, so just add a global variable.
    //
    stateNameMaker = code;
}


// ****************************************************************************
//  Method: MeshtvPrep::WriteOutTimeInvariantFiles
//
//  Purpose:
//      Iterates through all of its objects and writes out the files.
//
//  Programmer: Hank Childs
//  Creation:   December 9, 1999
//
// ****************************************************************************

void
MeshtvPrep::WriteOutTimeInvariantFiles(void)
{
    WriteOutVisit();
}


// ****************************************************************************
//  Method: MeshtvPrep::WriteOutVisit
//
//  Purpose:
//      Writes out the visit file.
//
//  Programmer: Hank Childs
//  Creation:   December 9, 1999
//
//  Modifications:
//
//    Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//    Added code to stop creation of the visit file if the silo conversion
//    switch is false.
//
//    Hank Childs, Thu Sep  7 17:25:16 PDT 2000
//    Added code to write out root interval trees.
//
// ****************************************************************************

void
MeshtvPrep::WriteOutVisit(void)
{
    int  i, j;

    char visitName[LONG_STRING];
    sprintf(visitName, "%s.visit", outPrefix);
    DBfile *dbfile = GetSiloOutputFile(visitName);

    if (doSiloConversion)
    {
        //
        // Have the time object write itself out.
        //
        time.Write(dbfile);
    }

    if (doIntervalTrees)
    {
        //
        // Write out the interval trees for the whole extents over time.
        //
        for (i = 0 ; i < fieldITN ; i++)
        {
            fieldIT[i].WrapUp();
            fieldIT[i].Write(dbfile);
        }
        for (i = 0 ; i < meshITN ; i++)
        {
            meshIT[i].WrapUp();
            meshIT[i].Write(dbfile);
        }
    }

    //
    // Put the names of the mesh and state files in the file.
    //
    int     nMesh                  = nFiles;
    char  **meshname = new char*[nMesh];
    for (i = 0 ; i < nMesh ; i++)
    {
        meshname[i] = new char[512];
        GetOutputFilename(MESH_FILE, i, meshname[i], 0);
    }
    int    *nStateFiles = new int[statesN];
    char ***states      = new char**[statesN];
    for (i = 0 ; i < statesN ; i++)
    {
        nStateFiles[i] = nFiles;
        states[i] = new char *[nFiles];
        for (j = 0 ; j < nFiles ; j++)
        {
             states[i][j] = new char[512];
             GetOutputFilename(STATE_FILE, j, states[i][j], i);
        }
    }

    int    length;
    char  *fl = MakeVisitFileList(nMesh, meshname, statesN, nStateFiles,
                                   states, &length);
    DBWrite(dbfile, "_files", fl, &length, 1, DB_CHAR);

    for (i = 0 ; i < nFiles ; i++)
    {
        delete [] meshname[i];
    }
    delete [] meshname;
    for (i = 0 ; i < statesN ; i++)
    {
        for (j = 0 ; j < nFiles ; j++)
        {
            delete [] states[i][j];
        }
        delete [] states[i];
    }
    delete [] states;

    DBClose(dbfile);
}
    

// ****************************************************************************
//  Method: MeshtvPrep::WriteOutMeshFiles
//
//  Purpose:
//      Writes out all of the mesh files for the current state.
//
//  Programmer: Hank Childs
//  Creation:   January 22, 2000
//
// ****************************************************************************

void
MeshtvPrep::WriteOutMeshFiles(void)
{
    //
    // We must make sure that we are doing Silo object conversion or an
    // onione peel, or else the file won't exist.
    //
    if (doSiloConversion || doOnionPeel)
    {
        //
        // Open the file.  We just want the first one, so feed in 0 and 1.
        //
        DBfile *dbfile = GetOutputFile(MESH_FILE, 0, 1);
    
        WriteTopologies(dbfile);
    
        RelinquishOutputFile(MESH_FILE, dbfile);
    }
}


// ****************************************************************************
//  Method: MeshtvPrep::WriteOutStateFiles
//
//  Purpose:
//      Writes out all of the state files for the current state.
//
//  Programmer: Hank Childs
//  Creation:   December 9, 1999
//
// ****************************************************************************

void
MeshtvPrep::WriteOutStateFiles(void)
{
    //
    // Open the file.  We just want the first one, so feed in 0 and 1.
    //
    DBfile *dbfile = GetOutputFile(STATE_FILE, 0, 1);
    
    CreateDirectoryStructure(dbfile);

    //
    // The actual work.
    //
    WriteMeshes(dbfile);
    WriteFields(dbfile);
    WriteMaterials(dbfile);

    RelinquishOutputFile(STATE_FILE, dbfile);
}


// ****************************************************************************
//  Method: MeshtvPrep::CreateDirectoryStructure
//
//  Purpose:
//      Creates the subdirectories from main needed for some of the objects
//      to write into.
//
//  Programmer: Hank Childs
//  Creation:   January 6, 1999
//
// ****************************************************************************

void
MeshtvPrep::CreateDirectoryStructure(DBfile *dbfile)
{
#ifdef PARALLEL
    extern int my_rank;
    if (my_rank != 0)
    {
        return;
    }
#endif
    if (doLowRes)
    {
        DBMkDir(dbfile, LOW_RES_DIR);
    }
    if (doMedRes)
    {
        DBMkDir(dbfile, MED_RES_DIR);
    }
}


// ****************************************************************************
//  Method:  MeshtvPrep::WriteFields
//
//  Purpose:
//      Writes the fields out to a file.  The field will then also write out
//      its Value object and IntervalTree object (if appropriate).
//
//  Arguments:
//      dbfile -  the file handle.
//
//  Programmer: Hank Childs
//  Creation:   December 10, 1999
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//      Added code to stop writing the Field_Preps if the silo conversion
//      switch is false and to do interval trees separately.
//
// ****************************************************************************

void
MeshtvPrep::WriteFields(DBfile *dbfile)
{
    if (doIntervalTrees)
    {
        for (int i = 0 ; i < fieldsN ; i++)
        {
            fields[i].WriteIntervalTree(dbfile);
        }
    }
    if (doSiloConversion)
    {
        for (int i = 0 ; i < fieldsN ; i++)
        {
            fields[i].Write(dbfile);
        }
    }
}


// ****************************************************************************
//  Method: MeshtvPrep::WriteMeshes
//
//  Purpose:
//      Writes out the mesh objects, including the coordinates.  The 
//      coordinates object is a Field, so it will write a Value and 
//      IntervalTree (if appropriate).
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//      Added code to stop writing the Mesh_Preps if the silo conversion
//      switch is false and to do interval trees separately.
//
// ****************************************************************************

void
MeshtvPrep::WriteMeshes(DBfile *dbfile)
{
    int    i;

    if (doIntervalTrees)
    {
        for (i = 0 ; i < meshesN ; i++)
        {
            coords[i].WriteIntervalTree(dbfile);
        }
    }

    if (doSiloConversion)
    {
        for (i = 0 ; i < meshesN ; i++)
        {
            meshes[i].Write(dbfile);
            coords[i].Write(dbfile);
        }
    }

    for (i = 0 ; i < meshesN ; i++)
    {
        if (doLowRes)
        {
            lowres[i].Write(dbfile);
        }
        if (doMedRes)
        {
            medres[i].Write(dbfile);
        }
    }
}


// ****************************************************************************
//  Method: MeshtvPrep::WriteMaterials
//
//  Purpose:
//      Writes out the Material object.
//
//  Programmer: Hank Childs
//  Creation:   January 10, 2000
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//      Added code to stop writing the Materials if the silo conversion
//      switch is false.
//
// ****************************************************************************

void
MeshtvPrep::WriteMaterials(DBfile *dbfile)
{
    if (doSiloConversion)
    {
        for (int i = 0 ; i < materialsN ; i++)
        {
            materials[i].Write(dbfile);
        }
    }
}


// ****************************************************************************
//  Method: MeshtvPrep::WriteTopologies
//
//  Purpose:
//      Writes out the Topology objects.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2000
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//      Added code to stop writing the Topologies if the silo conversion
//      switch is false.
//
// ****************************************************************************

void
MeshtvPrep::WriteTopologies(DBfile *dbfile)
{
    int   i;
    if (doSiloConversion)
    {
        for (i = 0 ; i < structuredTopologyN ; i++)
        {
            structuredTopology[i].Write(dbfile);
        }
        for (i = 0 ; i < unstructuredTopologyN ; i++)
        {
            unstructuredTopology[i].Write(dbfile);
        }
    }
}


// ****************************************************************************
//  Method: MeshtvPrep::ConsolidateObjects
//
//  Purpose:
//      Tell the objects that the initial pass is over, allowing them to do
//      additional processing that can only be done when all of the meta-data
//      has been read in from each domain.
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//      Added code to stop consolidating the _Prep objects if the silo 
//      conversion switch is false.
//
// ****************************************************************************

void
MeshtvPrep::ConsolidateObjects(void)
{
    //
    // Consolidate the objects that were getting information from the 
    // DetermineExtents portion of the initial pass.
    //
    for (int mesh = 0 ; mesh < meshesN ; mesh++)
    {
        if (doLowRes)
        {
            lowres[mesh].ConsolidateExtents();
        }
        if (doMedRes)
        {
            medres[mesh].ConsolidateExtents();
        }
    }

    //
    // Consolidate the objects that were getting information from the
    // size information from the initial pass.
    //
    if (doSiloConversion)
    {
        int   i;
        for (i = 0 ; i < fieldsN ; i++)
        {
            fields[i].Consolidate();
        }
        for (i = 0 ; i < meshesN ; i++)
        {
            meshes[i].Consolidate();
            coords[i].Consolidate();
        }
        for (i = 0 ; i < materialsN ; i++)
        {
            materials[i].Consolidate();
        }
        for (i = 0 ; i < structuredTopologyN ; i++)
        {
            structuredTopology[i].Consolidate();
        }
        for (i = 0 ; i < unstructuredTopologyN ; i++)
        {
            unstructuredTopology[i].Consolidate();
        }
    }
}


// ****************************************************************************
//  Method: MeshtvPrep::WrapUpObjects
//
//  Purpose:
//      Gives the objects a chance to wrap up now that all of the files for 
//      this state have been read in.
//
//  Programmer: Hank Childs
//  Creation:   December 13, 1999
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//      Added code to stop wrapping up the _Prep objects if the silo 
//      conversion switch is false and to do the interval trees separately.
//
// ****************************************************************************

void
MeshtvPrep::WrapUpObjects(void)
{
    //
    // Force control of the output file back to root processors.
    //
    ForceControlToRoot();

    //
    // Some of the WrapUp routines have to interact with other processors.  
    // Make sure everyone is on the same page.  This also makes debugging
    // easier.
    //
#ifdef PARALLEL
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    int    i;

    if (doLowRes)
    {
        for (i = 0 ; i < meshesN ; i++)
        {
            lowres[i].WrapUp();
        }
    }

    if (doMedRes)
    {
        for (i = 0 ; i < meshesN ; i++)
        {
            medres[i].WrapUp();
        }
    }

    if (doIntervalTrees)
    {
        for (i = 0 ; i < fieldsN ; i++)
        {
            fields[i].WrapUpIntervalTree();
        }
        for (i = 0 ; i < meshesN ; i++)
        {
            coords[i].WrapUpIntervalTree();
        }
    }

    if (doSiloConversion)
    {
        for (i = 0 ; i < fieldsN ; i++)
        {
            fields[i].WrapUp();
        }
        for (i = 0 ; i < meshesN ; i++)
        {
            coords[i].WrapUp();
            meshes[i].WrapUp();
        }
        for (i = 0 ; i < materialsN ; i++)
        {
            materials[i].WrapUp();
        }
        for (i = 0 ; i < structuredTopologyN ; i++)
        {
            structuredTopology[i].WrapUp();
        }
        for (i = 0 ; i < unstructuredTopologyN ; i++)
        {
            unstructuredTopology[i].WrapUp();
        }
    }
}


// ****************************************************************************
//  Method: MeshtvPrep::GetOutputFile
//
//  Purpose:
//      Gets a handle to the file to write out to.  Buffers the other 
//      functions from needing to know about waiting for the file to be 
//      released.
//
//  Note:       Any changes to this routine should probably also take place
//              in CreateOutputFiles as well.
//
//  Returns:    A handle to the DBfile.
//
//  Programmer: Hank Childs
//  Creation:   December 14, 1999
//
// **************************************************************************** 

DBfile *
MeshtvPrep::GetOutputFile(RESOURCE_e resource, int domain, int nDoms)
{
    //
    // Get control of the output file from the resource manager.
    //
    ObtainResource(resource);

    //
    // Create the output file name from the type of resource requested.
    //
    char  fileName[LONG_STRING];
    int   fileNum = GetOutputFileNum(domain, nDoms);
    GetOutputFilename(resource, fileNum, fileName, currentState);

    //
    // Now that we have the go ahead, open up the file.
    //
    DBfile *dbfile = GetSiloOutputFile(fileName);
  
    return dbfile;
}


// ****************************************************************************
//  Method: MeshtvPrep::GetOutputFilename
//
//  Purpose:
//      Determines the correct name for the file with the given current state
//      and resource.
//
//  Arguments:
//      resource    The kind of resource the filename should reflect.
//      fileNum     The number in the set for this state.
//      fileName    A place to copy the filename.
//      state       The state for this file.
// 
//  Programmer:     Hank Childs
//  Creation:       September 8, 2000
//
// ****************************************************************************

void
MeshtvPrep::GetOutputFilename(RESOURCE_e resource, int fileNum, char *fileName,
                              int state)
{
    char  fileExtension[LONG_STRING], stateName[LONG_STRING];
    NumToString(fileExtension, fileNum, nFiles, 2);
    switch (resource)
    {
        case STATE_FILE:
            code->GetState(stateName, state);
            sprintf(fileName, "%s%s.%s", outPrefix, stateName, fileExtension);
            break;

        case MESH_FILE:
            sprintf(fileName, "%s.mesh.%s", outPrefix, fileExtension);
            break;

        default:
            cerr << "Unable to determine resource type." << endl;
            exit(EXIT_FAILURE);
            /* NOTREACHED */ break;
    }
}


// ****************************************************************************
//  Method: CreateOutputFiles
//
//  Purpose:
//      Creates all of the SILO files to be used in that state.  Only has the 
//      root processor for a group create the file.  The root processor should
//      be root for the mesh file and state file, but I didn't want someone
//      to get caught if the scheme is changed.
//
//  Arguments:
//      state   The index of the current state (0-nStates-1).
//      
//  Programmer: Hank Childs
//  Creation:   January 10, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Jun 13 08:45:01 PDT 2000
//    Added logic to not create the mesh files and some state files unless
//    we are doing the silo object conversion.
//
//    Hank Childs, Fri Jun 16 07:54:22 PDT 2000
//    Only create mesh file if this is the initial state. Added argument state.
//
//    Hank Childs, Fri Sep  8 15:16:53 PDT 2000
//    Create .visit file here.
//
// ****************************************************************************

void
MeshtvPrep::CreateOutputFiles(int state)
{
    int   minNFile, maxNFile;
    GetNFileRange(&minNFile, &maxNFile);
    for (int i = minNFile ; i < maxNFile ; i++)
    {
        char  fileExtension[LONG_STRING], stateName[LONG_STRING], 
              fileName[LONG_STRING];
        NumToString(fileExtension, i, nFiles, 2);

        if (RootWithinGroup(STATE_FILE))
        {
            //
            // If we are doing the Silo object conversion, we need all of the
            // state files.  Otherwise we need only the first one if we are
            // doing resampling.
            //
            if (doSiloConversion || 
                (i == 0 && (doLowRes || doMedRes || doIntervalTrees)))
            {
                //
                // Create the state files.
                //
                code->GetState(stateName, state);
                sprintf(fileName, "%s%s.%s", outPrefix, stateName, 
                                             fileExtension);
                CreateSiloOutputFile(fileName);
            }
        }
    
        if (RootWithinGroup(MESH_FILE))
        {
            if (doSiloConversion || doOnionPeel)
            {
                if (state == 0)
                {
                    //
                    // Create the mesh files.
                    //
                    sprintf(fileName, "%s.mesh.%s", outPrefix, fileExtension);
                    CreateSiloOutputFile(fileName);
                }
            }
        }
    }
    if (state == 0)
    {
        char  fileName[LONG_STRING];
        sprintf(fileName, "%s.visit", outPrefix);
        CreateSiloOutputFile(fileName);
    }
}


// ****************************************************************************
//  Method: MeshtvPrep::RelinquishOutputFile
//
//  Purpose:
//      Relinquishes control of the output file and closes it.
//
//  Programmer: Hank Childs
//  Creation:   December 17, 1999
//
// ****************************************************************************

void
MeshtvPrep::RelinquishOutputFile(RESOURCE_e resource, DBfile *dbfile)
{
    //
    // Close the file before relinquishing the resource so that another
    // process does not try to open an incomplete file.
    //
    DBClose(dbfile);

    RelinquishResource(resource);
}


// ****************************************************************************
//  Method:  MeshtvPrep::DetermineMeshExtents
//
//  Purpose:
//      Determines the Mesh Extents for a given directory.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 1999
//
//  Modifications:
//
//    Hank Childs, Fri Jun 16 10:35:10 PDT 2000
//    Pushed code to get extents to its own function since it was getting
//    more complicated.
//
//    Hank Childs, Wed Aug 30 14:37:42 PDT 2000
//    Add code to take advantage of Resample's new ability to handle bad
//    input gracefully.
//
// ****************************************************************************

void
MeshtvPrep::DetermineMeshExtents(DBfile *dbfile, DBtoc *toc, char *, char *)
{
    if (!doLowRes && !doMedRes)
    {
        //
        // No reason to determine mesh extents.
        //
        return;
    }

    int   i;

    //
    // Ignore multimeshes.  They have nothing to add at this level.
    // Point meshes are also ignored since they can't be resampled and 
    // resampling is the only operation that the mesh extents are needed for
    // currently.
    //

    for (i = 0 ; i < toc->nqmesh ; i++)
    {
        int  ind   = MeshIndex(toc->qmesh_names[i]);
        if (ind < 0)
        {
            continue;
        }

        bool doThisLowRes = doLowRes && lowres[ind].ValidObject();
        bool doThisMedRes = doMedRes && medres[ind].ValidObject();
        if (!doThisLowRes && !doThisMedRes)
        {
            continue;
        }

        float  *minExtents = GetExtents(dbfile, toc->qmesh_names[i], 
                                        "min_extents");
        if (doLowRes)
        {
            lowres[ind].AddExtents(minExtents);
        }
        if (doMedRes)
        {
            medres[ind].AddExtents(minExtents);
        }
        delete [] minExtents;

        float  *maxExtents = GetExtents(dbfile, toc->qmesh_names[i], 
                                        "max_extents");
        if (doLowRes)
        {
            lowres[ind].AddExtents(maxExtents);
        }
        if (doMedRes)
        {
            medres[ind].AddExtents(maxExtents);
        }
        delete [] maxExtents;
    }

    for (i = 0 ; i < toc->nucdmesh ; i++)
    {
        int  ind   = MeshIndex(toc->ucdmesh_names[i]);
        if (ind < 0)
        {
            continue;
        }

        bool doThisLowRes = doLowRes && lowres[ind].ValidObject();
        bool doThisMedRes = doMedRes && medres[ind].ValidObject();
        if (!doThisLowRes && !doThisMedRes)
        {
            continue;
        }

        float  *minExtents = GetExtents(dbfile, toc->ucdmesh_names[i], 
                                        "min_extents");
        if (doLowRes)
        {
            lowres[ind].AddExtents(minExtents);
        }
        if (doMedRes)
        {
            medres[ind].AddExtents(minExtents);
        }
        delete [] minExtents;

        float  *maxExtents = GetExtents(dbfile, toc->ucdmesh_names[i], 
                                        "max_extents");
        if (doLowRes)
        {
            lowres[ind].AddExtents(maxExtents);
        }
        if (doMedRes)
        {
            medres[ind].AddExtents(maxExtents);
        }
        delete [] maxExtents;
    }
}


// ****************************************************************************
//  Method: MeshtvPrep::DetermineTopologySizes
//
//  Purpose:
//      Determines the size of the node list, shape count, shape type, and
//      shape size arrays, which are used for the unstructured topology object.
//
//  Note:       This routine invalidates the toc argument by changing dirs.
//
//  Note:       The free function should be called instead of delete for the
//              return value of DBGetComponent because it was allocated using
//              a C construct (malloc or calloc) and purify will give a 
//              Free Memory Mismatch warning.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2000
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//      Added code to not determine the Topology sizes if the silo 
//      conversion switch is false.
//
// ****************************************************************************

void
MeshtvPrep::DetermineTopologySizes(DBfile *dbfile, DBtoc *toc, char *filename, 
                                   char *dirname)
{
    //
    // Note that only unstructured meshes will have array information of which
    // we want to keep track.
    //
    if (doSiloConversion)
    {
        for (int i = 0 ; i < toc->nucdmesh ; i++)
        {
            int    uIndex = UnstructuredTopologyIndex(toc->ucdmesh_names[i]);
            if (uIndex < 0)
            {
                cerr << "Invalid unstructured topology name." << endl;
                exit(EXIT_FAILURE);
            }

            //
            // Get the domain number for the Value object.
            //
            int  domain = repository->GetMeshDomain(toc->ucdmesh_names[i], 
                                                    filename, dirname);

            unstructuredTopology[uIndex].DetermineSize(domain, dbfile, 
                                                       toc->ucdmesh_names[i]);
        }
    }
}


// ****************************************************************************
//  Method: MeshtvPrep::DetermineMaterialSizes
//
//  Purpose:
//      Determines the number of zones in the DBmaterial's matlist, and the
//      number of mixed zones and passes that information to the Value
//      objects stored by the Material object.
//
//  Note:       This routine invalidates the toc argument by changing dirs.
//
//  Note:       The free function should be called instead of delete for the
//              return value of DBGetComponent because it was allocated using
//              a C construct (malloc or calloc) and purify will give a 
//              Free Memory Mismatch.
//            
//  Programmer: Hank Childs
//  Creation:   January 10, 2000
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//      Added code to not determine the Material sizes if the silo 
//      conversion switch is false.
//
// ****************************************************************************

void
MeshtvPrep::DetermineMaterialSizes(DBfile *dbfile, DBtoc *toc, char *filename, 
                                   char *dirname)
{
    //
    // Ignore multimats, they have nothing to add at this level.
    //

    if (doSiloConversion)
    {
        for (int i = 0 ; i < toc->nmat ; i++)
        {
            //
            // Get the domain number for the Value object.
            //
            int  domain = repository->GetMaterialDomain(toc->mat_names[i], 
                                                        filename, dirname);
            materials[i].DetermineSize(domain, dbfile, toc->mat_names[i]);
        }
    }
}


// ****************************************************************************
//  Method:  MeshtvPrep::DetermineFieldSizes
//
//  Purpose:
//      Determines the size of the Fields.  We are just incrementing a counter,
//      so domain numbers are not important to us.
//
//  Note:       This does not do a good job of accounting for 
//              type (ie float vs. double).
// 
//  Note:       This routine invalidates the toc argument by changing dirs.
//
//  Note:       The free function should be called instead of delete for the
//              return value of DBGetComponent because it was allocated using
//              a C construct (malloc or calloc) and purify will give a 
//              Free Memory Mismatch.
//            
//  Programmer: Hank Childs
//  Creation:   December 17, 1999
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//      Added code to not determine the Field sizes if the silo 
//      conversion switch is false.
//
// ****************************************************************************

void
MeshtvPrep::DetermineFieldSizes(DBfile *dbfile, DBtoc *toc, char *filename, 
                                char *dirname)
{
    int   i;

    if (! doSiloConversion)
    {
        return;
    }

    //
    //  Ignore multivars.  They have nothing to add at this level.
    //

    //
    // Get the sizes for all of the Ucdvars.
    //
    for (i = 0 ; i < toc->nucdvar ; i++)
    {
        //
        // Get the index into the fields array for this var.
        //
        int  ind = FieldIndex(toc->ucdvar_names[i]);
        if (ind < 0)
        {
            //
            // This variable was not entered in the command line.
            //
            continue;
        }

        //
        // Get the domain number for the Value object.
        //
        int   domain = repository->GetVarDomain(toc->ucdvar_names[i], filename,
                                                dirname);

        fields[ind].DetermineUcdvarSize(domain, dbfile, toc->ucdvar_names[i]);
    }    

    //
    // Get the sizes for all of the Meshvars.
    //
    for (i = 0 ; i < toc->nptvar ; i++)
    {
        //
        // Get the index into the fields array for this var.
        //
        int  ind = FieldIndex(toc->ptvar_names[i]);
        if (ind < 0)
        {
            //
            // This variable was not entered in the command line.
            //
            continue;
        }

        //
        // Get the domain number for the Value object.
        //
        int  domain = repository->GetVarDomain(toc->ptvar_names[i], filename,
                                               dirname);

        fields[ind].DetermineMeshvarSize(domain, dbfile, toc->ptvar_names[i]);
    }    

    //
    // Get the size for all of the Quadvars.
    //
    for (i = 0 ; i < toc->nqvar ; i++)
    {
        //
        // Get the index into the fields array for this var.
        //
        int  ind = FieldIndex(toc->qvar_names[i]);
        if (ind < 0)
        {
            //
            // This variable was not entered in the command line.
            //
            continue;
        }

        //
        // Get the domain number for the Value object.
        //
        int  domain = repository->GetVarDomain(toc->qvar_names[i], filename,
                                               dirname);

        fields[ind].DetermineQuadvarSize(domain, dbfile, toc->qvar_names[i]);
    }    
}


// ****************************************************************************
//  Method:  MeshtvPrep::DetermineMeshSizes
//
//  Purpose:
//      Determines the size of the Mesh fields.  We are just incrementing a 
//      counter, so domain numbers are not important to us.  This is primarily
//      concerned with the coordinates.
//
//  Note:       This does not do a good job of accounting for 
//              type (ie float vs. double).
// 
//  Note:       This routine invalidates the toc argument by changing dirs.
//
//  Note:       The free function should be called instead of delete for the
//              return value of DBGetComponent because it was allocated using
//              a C construct (malloc or calloc) and purify will give a 
//              Free Memory Mismatch.
//            
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//      Added code to not determine the Mesh sizes if the silo 
//      conversion switch is false.
//
// ****************************************************************************

void
MeshtvPrep::DetermineMeshSizes(DBfile *dbfile, DBtoc *toc, char *filename, 
                               char *dirname)
{
    int   i;

    if (! doSiloConversion)
    {
        return;
    }

    //
    //  Ignore multimeshes.  They have nothing to add at this level.
    //

    //
    // Get the sizes for all of the Ucdmeshes.
    //
    for (i = 0 ; i < toc->nucdmesh ; i++)
    {
        //
        // Get the index into the fields array for this mesh.
        //
        int  ind = MeshIndex(toc->ucdmesh_names[i]);
        if (ind < 0)
        {
            continue;
        }

        //
        // Get the domain number for the Value object.
        //
        int  domain = repository->GetMeshDomain(toc->ucdmesh_names[i], 
                                                filename, dirname);

        coords[ind].DetermineUcdMeshSize(domain, dbfile, 
                                         toc->ucdmesh_names[i]);
    }    

    //
    // Get the sizes for all of the Point meshes.
    //
    for (i = 0 ; i < toc->nptmesh ; i++)
    {
        //
        // Get the index into the fields array for this mesh.
        //
        int  ind = MeshIndex(toc->ptmesh_names[i]);
        if (ind < 0)
        {
            continue;
        }

        //
        // Get the domain number for the Value object.
        //
        int  domain = repository->GetMeshDomain(toc->ptmesh_names[i], 
                                                filename, dirname);

        coords[ind].DeterminePointMeshSize(domain, dbfile, 
                                           toc->ptmesh_names[i]);
    }    

    //
    // Get the size for all of the Quad meshes.
    //
    for (i = 0 ; i < toc->nqmesh ; i++)
    {
        //
        // Get the index into the fields array for this mesh.
        //
        int  ind = MeshIndex(toc->qmesh_names[i]);
        if (ind < 0)
        {
            continue;
        }
   
        //
        // Get the domain number for the Value object.
        //
        int  domain = repository->GetMeshDomain(toc->qmesh_names[i], 
                                                filename, dirname);
 
        coords[ind].DetermineQuadMeshSize(domain, dbfile, toc->qmesh_names[i]);
    }    
}


// ****************************************************************************
//  Method: IterateDirs
//
//  Purpose:
//      Calls a method of MeshtvPrep on a directory, then iterates through all 
//      of the subdirectories calling the same method.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 1999
//
// ****************************************************************************

void
MeshtvPrep::IterateDirs(void (MeshtvPrep::*f)(DBfile *,DBtoc *,char *, char *), 
                        DBfile *dbfile,DBtoc *toc,char *curFileName, char *dir)
{
    int     i;

    //
    // Call the function for this directory.
    //
    (this->*f)(dbfile, toc, curFileName, dir);

    //
    // Determine fully-qualified directory names.
    //
    int     numDirs = toc->ndir;
    char  **dirs    = new char*[numDirs];
    char    dirName[LONG_STRING];
    DBGetDir(dbfile, dirName);
    int     dirNameLength = strlen(dirName);
    char   *slash;
    if (dirName[dirNameLength-1] == '/')
    {
        slash = "";
    }
    else
    {
        slash = "/";
    }
    for (i = 0 ; i < numDirs ; i++)
    {
        // Assumes directory is one deep.
        char   s[LONG_STRING];
        sprintf(s, "%s%s%s", dirName, slash, toc->dir_names[i]);
        dirs[i] = CXX_strdup(s);
    }

    //
    // Unfortunately, the directories are kept in lexicographical order,
    // so xxx10 comes before xxx2.  Sort this list so that the domains are
    // sorted in the correct order.
    //
    SortListByNumber(dirs, numDirs);

    //
    // Iterate over the subdirectories.
    //
    for (i = 0 ; i < numDirs ; i++)
    {
        if (repository->ContainsDomain(curFileName, dirs[i]) == false)
        {
            //
            // Neither this director nor any of its sub-directories contain
            // any domains we are interested in.
            //
            continue;
        }

        DBSetDir(dbfile, dirs[i]);
        DBtoc *subDirToc = DBGetToc(dbfile);
        IterateDirs(f, dbfile, subDirToc, curFileName, dirs[i]);
    }

    //
    // Clean up memory.
    //
    for (i = 0 ; i < numDirs ; i++)
    {
        delete [] dirs[i];
    }
    delete [] dirs;
}


// ****************************************************************************
//  Method: MeshtvPrep::CreateRootIntervalTrees
//
//  Purpose:
//      Creates the root interval trees.  These trees keep track of the extents
//      of all of the blocks for each variable over each time step.
//
//  Programmer: Hank Childs
//  Creation:   September 7, 2000
//
// ****************************************************************************

void
MeshtvPrep::CreateRootIntervalTrees(void)
{
    int   i;
    fieldITN = repository->GetNVars();
    fieldIT  = new IntervalTree_Prep[fieldITN];
    for (i = 0 ; i < fieldITN ; i++)
    {
        char  *fieldName  = repository->GetVar(i);
        int    fieldIndex = FieldIndex(fieldName);
        IntervalTree_Prep  *it = fields[fieldIndex].GetIntervalTree();
        fieldIT[i].SetFieldName(fieldName);
        fieldIT[i].SetNDomains(statesN);
        fieldIT[i].SetNDims(it->GetNDims());
    }

    meshITN = repository->GetNMeshes();
    meshIT  = new IntervalTree_Prep[meshITN];
    for (i = 0 ; i < meshITN ; i++)
    {
        char  *meshName    = repository->GetMesh(i);
        int    meshIndex   = MeshIndex(meshName);
        IntervalTree_Prep *it = coords[meshIndex].GetIntervalTree();
        meshIT[i].SetFieldName(meshName);
        meshIT[i].SetNDomains(statesN);
        meshIT[i].SetNDims(it->GetNDims());
    }
}


// ****************************************************************************
//  Method: MeshtvPrep::PopulateRootIntervalTrees
//
//  Purpose:
//      Gets the root extents of the interval trees for the current state and
//      adds it to the root interval tree.
//
//  Arguments:
//      state    The current state.
//
//  Programmer:  Hank Childs
//  Creation:    September 8, 2000
//
// ****************************************************************************

void
MeshtvPrep::PopulateRootIntervalTrees(int state)
{
    int  i;
    for (i = 0 ; i < fieldITN ; i++)
    {
        char  *fieldName  = fieldIT[i].GetName();
        int    fieldIndex = FieldIndex(fieldName);
        IntervalTree_Prep *it = fields[fieldIndex].GetIntervalTree();
        float extents[256];  // way more than we need
        it->GetRootExtents(extents);
        fieldIT[i].AddVar(state, extents);
    }

    for (i = 0 ; i < meshITN ; i++)
    {
        char   *fieldName = meshIT[i].GetName();
        int     meshIndex = MeshIndex(fieldName);
        IntervalTree_Prep  *it = coords[meshIndex].GetIntervalTree();
        float extents[256];  // way more than we need
        it->GetRootExtents(extents);
        meshIT[i].AddVar(state, extents);
    }
}

        
// ****************************************************************************
//  Method: GetExtents
//
//  Purpose:
//      Gets the extents of a mesh.  There is an error in the DBGetComponent
//      routine that does not force the extents to be in single precision, so
//      that is accounted for here.
//
//  Arguments:
//      dbfile      The current file.
//      meshname    The name of the mesh.
//      comp        The component.  Should be "min_extents" or "max_extents".
//
//  Returns:    An array of floats.
//
//  Note:       The free function should be called instead of delete for the
//              return value of DBGetComponent because it was allocated using
//              a C construct (malloc or calloc) and purify will give a 
//              Free Memory Mismatch.
//            
//  Note:       The return value must be freed or it is leaked.
//
//  Programmer: Hank Childs
//  Creation:   June 16, 2000
//
// ****************************************************************************

float  *
GetExtents(DBfile *dbfile, char *meshname, char *comp)
{
    float  *extents = new float[3];
    void   *rv;
    
    //
    // Determine what the datatype of the mesh is.
    //
    rv = DBGetComponent(dbfile, meshname, "datatype");
    int  *irv = (int *) rv;
    int   datatype = irv[0];
    free(rv);

    //
    // Get the array.
    //
    rv = DBGetComponent(dbfile, meshname, comp);

    if (datatype == DB_DOUBLE)
    {
        //
        // This should have been casted to a float, but there is an error
        // with pdblib.
        //
        double *e = (double *) rv;
        for (int i = 0 ; i < 3 ; i++)
        {
            extents[i] = e[i];
        }
    }
    else
    {
        //
        // This is a float like it should be.
        //
        float *e = (float *) rv;
        for (int i = 0 ; i < 3 ; i++)
        {
            extents[i] = e[i];
        }
    }
    free(rv);

    return extents;
}


