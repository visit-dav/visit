// ************************************************************************* //
//                             InputRepository.C                             //
// ************************************************************************* //

#include <visitstream.h>
#include <stdlib.h>
#include <string.h>

#include <InputRepository.h>
#include <SiloObjLib.h>
#include <MeshtvPrepLib.h>


// ****************************************************************************
//  Method:  InputRepository constructor
//
//  Programmer: Hank Childs
//  Creation:   December 8, 1999
//
//  Modifications:
// 
//    Hank Childs, Tue Apr  4 14:26:22 PDT 2000
//    Removed meshesID.
//
// ****************************************************************************

InputRepository::InputRepository()
{
    nMeshes               = -1;
    meshNDomains          = NULL;
    meshesName            = NULL;
    meshDomainListing     = NULL;

    nVars                 = -1;
    varNDomains           = NULL;
    varsName              = NULL;
    varDomainListing      = NULL;

    nMaterials            = -1;
    materialsName         = NULL;
    materialDomainListing = NULL;

    readInMesh            = false;
    ptmeshes              = NULL;
    qmeshes               = NULL;
    ucdmeshes             = NULL;
    ptmeshnames           = NULL;
    qmeshnames            = NULL;
    ucdmeshnames          = NULL;
    ucdmeshesN            = 0;
    qmeshesN              = 0;
    ptmeshesN             = 0;
}


// ****************************************************************************
//  Method:  InputRepository destructor
//
//  Programmer: Hank Childs
//  Creation:   December 8, 1999
//
//  Modifications:
// 
//    Hank Childs, Tue Apr  4 14:26:22 PDT 2000
//    Removed meshesID.
//
// ****************************************************************************

InputRepository::~InputRepository()
{
    DeleteMeshes();

    if (meshesName != NULL)
    {
        for (int j = 0 ; j < nMeshes ; j++)
        {
            if (meshesName[j] != NULL)
            {
                delete [] meshesName[j];
            }
        }
        delete [] meshesName;
    }
    if (materialsName != NULL)
    {
        for (int j = 0 ; j < nMaterials ; j++)
        {
            if (materialsName[j] != NULL)
            {
                delete [] materialsName[j];
            }
        }
        delete [] materialsName;
    }
    if (varsName != NULL)
    {
        for (int j = 0 ; j < nVars ; j++)
        {
            if (varsName[j] != NULL)
            {
                delete [] varsName[j];
            }
        }
        delete [] varsName;
    }
    if (meshDomainListing != NULL)
    {
        for (int i = 0 ; i < nMeshes ; i++)
        {
            if (meshDomainListing[i] != NULL)
            {
                for (int j = 0 ; j < meshNDomains[i] ; j++)
                {
                    if (meshDomainListing[i][j] != NULL)
                    {
                        delete [] meshDomainListing[i][j];
                    }
                }
                delete [] meshDomainListing[i];
            }
        }
        delete [] meshDomainListing;
    }
    if (varDomainListing != NULL)
    {
        for (int i = 0 ; i < nVars ; i++)
        {
            if (varDomainListing[i] != NULL)
            {
                for (int j = 0 ; j < varNDomains[i] ; j++)
                {
                    if (varDomainListing[i][j] != NULL)
                    {
                        delete [] varDomainListing[i][j];
                    }
                }
                delete [] varDomainListing[i];
            }
        }
        delete [] varDomainListing;
    }
    if (materialDomainListing != NULL)
    {
        for (int i = 0 ; i < nMaterials ; i++)
        {
            if (materialDomainListing[i] != NULL)
            {
                for (int j = 0 ; j < materialNDomains[i] ; j++)
                {
                    if (materialDomainListing[i][j] != NULL)
                    {
                        delete [] materialDomainListing[i][j];
                    }
                }
                delete [] materialDomainListing[i];
            }
        }
        delete [] materialDomainListing;
    }
    if (meshNDomains != NULL)
    {
        delete [] meshNDomains;
    }
    if (varNDomains != NULL)
    {
        delete [] varNDomains;
    }
    if (materialNDomains != NULL)
    {
        delete [] materialNDomains;
    }
}


// ****************************************************************************
//  Method: InputRepository::ReadRoot
//
//  Purpose: 
//      Reads in the information that is supposed to go in the repository from
//      the root file.  
//
//  Arguments:
//      dbfile    The open root file.
//      toc       The table of contents for the open root file.
//
//  Programmer: Hank Childs
//  Creation:   December 8, 1999
//
// ****************************************************************************

void
InputRepository::ReadRoot(DBfile *dbfile, DBtoc *toc)
{
    ReadMeshesFromRoot(dbfile, toc);
    ReadVarsFromRoot(dbfile, toc);
    ReadMaterialsFromRoot(dbfile, toc);
}


// ****************************************************************************
//  Method: InputRepository::ReadMeshesFromRoot
//
//  Purpose:
//      Reads in the information pertaining to meshes from the root file.
//
//  Arguments:
//      dbfile    The open root file.
//      toc       The table of contents for the open root file.
//
//  Programmer: Hank Childs
//  Creation:   January 24, 2000
//
//  Modifications:
// 
//    Hank Childs, Tue Apr  4 14:26:22 PDT 2000
//    Removed all processing of a mesh's id.
//
// ****************************************************************************

void
InputRepository::ReadMeshesFromRoot(DBfile *dbfile, DBtoc *toc)
{
    int   i, meshI = 0;

    nMeshes = toc->nmultimesh + toc->nqmesh + toc->nucdmesh + toc->nptmesh;

    // 
    // Initialize fields that are dependent on the number of meshes for a 
    // given state.
    //
    meshesName     = new char*[nMeshes];
    meshNDomains   = new int[nMeshes];
    for (i = 0 ; i < nMeshes ; i++)
    {
        meshesName[i]     = NULL;
        meshNDomains[i]   = 0;
    }
    meshDomainListing = new char**[nMeshes];
    for (i = 0 ; i < nMeshes ; i++)
    {
        meshDomainListing[i] = NULL;
    }

    //
    // Go through all of the multimeshes.
    //
    for (i = 0 ; i < toc->nmultimesh ; i++)
    {
        DBmultimesh *mm = DBGetMultimesh(dbfile, toc->multimesh_names[i]);
        if (mm == NULL)
        {
            cerr << "Could not read in multimesh \"" << toc->multimesh_names[i]
                 << "\"." << endl;
            exit(EXIT_FAILURE);
        }
        AddMeshEntry(meshI, mm->nblocks, toc->multimesh_names[i], 
                     mm->meshnames);
        meshI++;
        DBFreeMultimesh(mm);
    }

    //
    // Go through all of the quadmeshes.
    //
    for (i = 0 ; i < toc->nqmesh ; i++)
    {
        AddMeshEntry(meshI, 1, toc->qmesh_names[i], NULL);
        meshI++;
    }

    //
    // Go through all of the ucdmeshes.
    //
    for (i = 0 ; i < toc->nucdmesh ; i++)
    {
        AddMeshEntry(meshI, 1, toc->ucdmesh_names[i], NULL);
        meshI++;
    }

    //
    // Go through all of the pointmeshes.
    //
    for (i = 0 ; i < toc->nptmesh ; i++)
    {
        AddMeshEntry(meshI, 1, toc->ucdmesh_names[i], NULL);
        meshI++;
    }
}


// ****************************************************************************
//  Method:  InputRepository::ReadVarsFromRoot
//
//  Purpose:
//      Read in all of the information about the vars from the root file.
//
//  Arguments:
//      dbfile    The open root file.
//      toc       The table of contents for the open root file.
//
//  Programmer: Hank Childs
//  Creation:   January 24, 2000
//
// ****************************************************************************

void
InputRepository::ReadVarsFromRoot(DBfile *dbfile, DBtoc *toc)
{
    int   i, varI = 0;

    nVars = toc->nmultivar + toc->nqvar + toc->nucdvar + toc->nptvar;

    // 
    // Initialize fields that are dependent on the number of Vars for a 
    // given state.
    //
    varsName    = new char*[nVars];
    varNDomains = new int[nVars];
    for (i = 0 ; i < nVars ; i++)
    {
        varsName[i]    = NULL;
        varNDomains[i] = 0;
    }
    varDomainListing = new char**[nVars];
    for (i = 0 ; i < nVars ; i++)
    {
        varDomainListing[i] = NULL;
    }

    //
    // Go through all of the multivars.
    //
    for (i = 0 ; i < toc->nmultivar ; i++)
    {
        DBmultivar *mv = DBGetMultivar(dbfile, toc->multivar_names[i]);
        if (mv == NULL)
        {
            cerr << "Could not read in multivar \"" << toc->multivar_names[i]
                 << "\"." << endl;
            exit(EXIT_FAILURE);
        }
        AddVarEntry(varI, mv->nvars, toc->multivar_names[i], 
                     mv->varnames);
        varI++;
        DBFreeMultivar(mv);
    }

    //
    // Go through all of the quadvars.
    //
    for (i = 0 ; i < toc->nqvar ; i++)
    {
        AddVarEntry(varI, 1, toc->qvar_names[i], NULL);
        varI++;
    }

    //
    // Go through all of the ucdvars.
    //
    for (i = 0 ; i < toc->nucdvar ; i++)
    {
        AddVarEntry(varI, 1, toc->ucdvar_names[i], NULL);
        varI++;
    }

    //
    // Go through all of the meshvars.
    //
    for (i = 0 ; i < toc->nptvar ; i++)
    {
        AddVarEntry(varI, 1, toc->ucdvar_names[i], NULL);
        varI++;
    }
}


// ****************************************************************************
//  Method:  InputRepository::ReadMaterialsFromRoot
//
//  Purpose:
//      Read in all of the information about the materials from the root file.
//
//  Arguments:
//      dbfile    The open root file.
//      toc       The table of contents for the open root file.
//
//  Programmer: Hank Childs
//  Creation:   January 24, 2000
//
// ****************************************************************************

void
InputRepository::ReadMaterialsFromRoot(DBfile *dbfile, DBtoc *toc)
{
    int   i, materialI = 0;

    nMaterials = toc->nmultimat + toc->nmat;

    // 
    // Initialize fields that are dependent on the number of Materials for a 
    // given state.
    //
    materialsName     = new char*[nMaterials];
    materialNDomains  = new int[nMaterials];
    for (i = 0 ; i < nMaterials ; i++)
    {
        materialsName[i]     = NULL;
        materialNDomains[i]   = 0;
    }
    materialDomainListing = new char**[nMaterials];
    for (i = 0 ; i < nMaterials ; i++)
    {
        materialDomainListing[i] = NULL;
    }

    //
    // Go through all of the multimats.
    //
    for (i = 0 ; i < toc->nmultimat ; i++)
    {
        DBmultimat *mm = DBGetMultimat(dbfile, toc->multimat_names[i]);
        if (mm == NULL)
        {
            cerr << "Could not read in multimat \"" << toc->multimat_names[i]
                 << "\"." << endl;
            exit(EXIT_FAILURE);
        }
        AddMaterialEntry(materialI, mm->nmats, toc->multimat_names[i], 
                         mm->matnames);
        materialI++;
        DBFreeMultimat(mm);
    }

    //
    // Go through all of the materials.
    //
    for (i = 0 ; i < toc->nmat ; i++)
    {
        AddMaterialEntry(materialI, 1, toc->mat_names[i], NULL);
        materialI++;
    }
}


// ****************************************************************************
//  Method:  InputRepository::AddVarEntry
//
//  Purpose:
//      To have a single routine that adds a var to this object's fields.
//
//  Arguments:
//      index         The index of the new variable.
//      numDoms       The number of domains the new variable has.
//      name          The name of the new variable.
//      domainNames   The locations of each of the variable's domains.
//
//  Programmer: Hank Childs
//  Creation:   January 24, 2000
//
// ****************************************************************************

void
InputRepository::AddVarEntry(int index, int numDoms, char *name, 
                             char **domainNames)
{
    varNDomains[index] = numDoms;
    varsName[index]    = new char[strlen(name)+1];
    strcpy(varsName[index], name);

    if (domainNames != NULL)
    {
        varDomainListing[index] = new char*[varNDomains[index]];
        for (int i = 0 ; i < numDoms ; i++)
        {
            varDomainListing[index][i] = 
                     new char[strlen(domainNames[i])+1];
            strcpy(varDomainListing[index][i], domainNames[i]);
        }
    }
}


// ****************************************************************************
//  Method:  InputRepository::AddMaterialEntry
//
//  Purpose:
//      To have a single routine that adds a material to this object's fields.
//
//  Arguments:
//      index         The index of the new material.
//      numDoms       The number of domains the new material has.
//      name          The name of the new material.
//      domainNames   The locations of each of the material's domains.
//
//  Programmer: Hank Childs
//  Creation:   January 24, 2000
//
// ****************************************************************************

void
InputRepository::AddMaterialEntry(int index, int numDoms, char *name, 
                                  char **domainNames)
{
    materialNDomains[index] = numDoms;
    materialsName[index]    = new char[strlen(name)+1];
    strcpy(materialsName[index], name);

    if (domainNames != NULL)
    {
        materialDomainListing[index] = new char*[materialNDomains[index]];
        for (int i = 0 ; i < numDoms ; i++)
        {
            materialDomainListing[index][i] = 
                     new char[strlen(domainNames[i])+1];
            strcpy(materialDomainListing[index][i], domainNames[i]);
        }
    }
}


// ****************************************************************************
//  Method:  InputRepository::AddMeshEntry
//
//  Purpose:
//      To have a single routine that adds a mesh to this object's fields.
//
//  Arguments:
//      index        The index this entry should go in.
//      numDoms      The number of domains for this entry.
//      name         The name of this mesh entry.
//      domainNames  The names of all of the subdomains of this mesh.
//
//  Programmer: Hank Childs
//  Creation:   December 8, 1999
//
//  Modifications:
// 
//    Hank Childs, Tue Apr  4 14:26:22 PDT 2000
//    Removed argument id and references to meshesID.
//
// ****************************************************************************

void
InputRepository::AddMeshEntry(int index, int numDoms, char *name, 
                              char **domainNames)
{
    meshNDomains[index] = numDoms;
    meshesName[index]   = new char[strlen(name)+1];
    strcpy(meshesName[index], name);

    if (domainNames != NULL)
    {
        meshDomainListing[index] = new char*[meshNDomains[index]];
        for (int i = 0 ; i < numDoms ; i++)
        {
            meshDomainListing[index][i] = 
                     new char[strlen(domainNames[i])+1];
            strcpy(meshDomainListing[index][i], domainNames[i]);
        }
    }
}
    

// ****************************************************************************
//  Method:  InputRepository::GetMeshDomain
//
//  Purpose: 
//      Identifies what the current domain is for a given file, directory,
//      and mesh.
//
//  Arguments:
//      meshname    The name of the mesh.
//      filename    The file the domain of this mesh lies in.
//      dirname     The directory the domain of this mesh lies in.
//
//  Returns:    The domain number, -1 on failure.
//
//  Programmer: Hank Childs
//  Creation:   December 9, 1999
//
// ****************************************************************************

int
InputRepository::GetMeshDomain(char *meshname, char *filename, char *dirname) 
{
    //
    // Determine which index this mesh corresponds to.
    //
    int  i, index = -1;
    for (i = 0 ; i < nMeshes ; i++)
    {
        if (strcmp(meshesName[i], meshname) == 0)
        {
            index = i;
            break;
        }
    }
 
    //
    // Check to make sure a matching meshname was found.
    //
    if (index == -1)
    {
        return -1;
    }

    //
    // There is only one domain is the mesh is not a multimesh and the 
    // domain listings are invalid.
    //
    if (meshNDomains[index] == 1)
    {
        return 0;
    }

    //
    // Create the string 'filename:dirname/meshname' for 
    //  multi-file, multi-block.
    //
    char  s[LONG_STRING];
    sprintf(s, "%s:%s/%s", filename, dirname, meshname);

    for (i = 0 ; i < meshNDomains[index] ; i++)
    {
        if (strcmp(meshDomainListing[index][i], s) == 0)
        {
            return i;
        }
    }

    //
    // Create the string 'dirname/meshname' for single-file, multi-block.
    //
    sprintf(s, "%s/%s", dirname, meshname);
    for (i = 0 ; i < meshNDomains[index] ; i++)
    {
        if (strcmp(meshDomainListing[index][i], s) == 0)
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
//  Method:  InputRepository::GetVarDomain
//
//  Purpose: 
//      Identifies what the current domain is for a given file, directory,
//      and var.
//
//  Arguments:
//      varname     The name of the variable.
//      filename    The file the domain of this variable lies in.
//      dirname     The directory the domain of this variable lies in.
//
//  Returns:    The domain number, -1 on failure.
//
//  Programmer: Hank Childs
//  Creation:   January 24, 2000
//
// ****************************************************************************

int
InputRepository::GetVarDomain(char *varname, char *filename, char *dirname) 
{
    //
    // Determine which index this var corresponds to.
    //
    int  i, index = -1;
    for (i = 0 ; i < nVars ; i++)
    {
        if (strcmp(varsName[i], varname) == 0)
        {
            index = i;
            break;
        }
    }
 
    //
    // Check to make sure a matching varname was found.
    //
    if (index == -1)
    {
        return -1;
    }

    //
    // There is only one domain is the var is not a multivar and the 
    // domain listings are invalid.
    //
    if (varNDomains[index] == 1)
    {
        return 0;
    }

    //
    // Create the string 'filename:dirname/varname' for 
    //  multi-file, multi-block.
    //
    char  s[LONG_STRING];
    sprintf(s, "%s:%s/%s", filename, dirname, varname);

    for (i = 0 ; i < varNDomains[index] ; i++)
    {
        if (strcmp(varDomainListing[index][i], s) == 0)
        {
            return i;
        }
    }

    //
    // Create the string 'dirname/varname' for single-file, multi-block.
    //
    sprintf(s, "%s/%s", dirname, varname);
    for (i = 0 ; i < varNDomains[index] ; i++)
    {
        if (strcmp(varDomainListing[index][i], s) == 0)
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
//  Method:  InputRepository::GetMaterialDomain
//
//  Purpose: 
//      Identifies what the current domain is for a given file, directory,
//      and Material.
//
//  Arguments:
//      materialname    The name of the material.
//      filename        The file the domain of this material lies in.
//      dirname         The directory the domain of this material lies in.
//
//  Returns:    The domain number, -1 on failure.
//
//  Programmer: Hank Childs
//  Creation:   January 24, 2000
//
// ****************************************************************************

int
InputRepository::GetMaterialDomain(char *materialname, char *filename, 
                                   char *dirname) 
{
    //
    // Determine which index this material corresponds to.
    //
    int  i, index = -1;
    for (i = 0 ; i < nMaterials ; i++)
    {
        if (strcmp(materialsName[i], materialname) == 0)
        {
            index = i;
            break;
        }
    }
 
    //
    // Check to make sure a matching materialname was found.
    //
    if (index == -1)
    {
        return -1;
    }

    //
    // There is only one domain is the material is not a multimat and the 
    // domain listings are invalid.
    //
    if (materialNDomains[index] == 1)
    {
        return 0;
    }

    //
    // Create the string 'filename:dirname/materialname' for 
    //  multi-file, multi-block.
    //
    char  s[LONG_STRING];
    sprintf(s, "%s:%s/%s", filename, dirname, materialname);

    for (i = 0 ; i < materialNDomains[index] ; i++)
    {
        if (strcmp(materialDomainListing[index][i], s) == 0)
        {
            return i;
        }
    }

    //
    // Create the string 'dirname/materialname' for single-file, multi-block.
    //
    sprintf(s, "%s/%s", dirname, materialname);
    for (i = 0 ; i < materialNDomains[index] ; i++)
    {
        if (strcmp(materialDomainListing[index][i], s) == 0)
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
//  Method:  InputRepository::GetMeshNDomains
//
//  Purpose:
//      Finds the number of domains corresping to a mesh name.
//
//  Arguments:
//      name    The name of the mesh.
//
//  Returns:    The number of domains, -1 if the name is not valid. 
//
//  Programmer: Hank Childs
//  Creation:   December 8, 1999
//
// ****************************************************************************

int
InputRepository::GetMeshNDomains(char *name)
{
    for (int i = 0 ; i < nMeshes ; i++)
    {
        if (strcmp(name, meshesName[i]) == 0)
        {
            return meshNDomains[i];
        }
    }

    //
    // No match.
    //
    return -1;
}


// ****************************************************************************
//  Method:  InputRepository::GetVarNDomains
//
//  Purpose:
//      Finds the number of domains corresping to a Var name.
//
//  Arguments:
//      name    The name of the variable.
//
//  Returns:    The number of domains, -1 if the name is not valid. 
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2000
//
// ****************************************************************************

int
InputRepository::GetVarNDomains(char *name)
{
    for (int i = 0 ; i < nVars ; i++)
    {
        if (strcmp(name, varsName[i]) == 0)
        {
            return varNDomains[i];
        }
    }

    //
    // No match.
    //
    return -1;
}


// ****************************************************************************
//  Method:  InputRepository::GetMaterialNDomains
//
//  Purpose:
//      Finds the number of domains corresping to a Material name.
//
//  Arguments:
//      name    The name of the material.
//
//  Returns:    The number of domains, -1 if the name is not valid. 
//
//  Programmer: Hank Childs
//  Creation:   December 8, 1999
//
// ****************************************************************************

int
InputRepository::GetMaterialNDomains(char *name)
{
    for (int i = 0 ; i < nMaterials ; i++)
    {
        if (strcmp(name, materialsName[i]) == 0)
        {
            return materialNDomains[i];
        }
    }

    //
    // No match.
    //
    return -1;
}


// ****************************************************************************
//  Method: InputRepository::GetPointmesh
//
//  Purpose:
//      Gets a point mesh previously stashed in the repository.
//
//  Arguments:
//      name    The name of the point mesh.
//
//  Returns:    A pointer to a point mesh, NULL if no match is found.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
// ****************************************************************************

DBpointmesh *
InputRepository::GetPointmesh(char *name)
{
    for (int i = 0 ; i < ptmeshesN ; i++)
    {
        if (ptmeshnames[i] == NULL)
        {
            //
            // We were promised ptmeshesN, but got less than that.
            //
            break;
        }
        if (strcmp(name, ptmeshnames[i]) == 0)
        {
            return ptmeshes[i];
        }
    }

    return NULL;
}


// ****************************************************************************
//  Method: InputRepository::GetQuadmesh
//
//  Purpose:
//      Gets a quad mesh previously stashed in the repository.
//
//  Arguments:
//      name    The name of the quad mesh.
//
//  Returns:    A pointer to a quad mesh, NULL if no match is found.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
// ****************************************************************************

DBquadmesh *
InputRepository::GetQuadmesh(char *name)
{
    for (int i = 0 ; i < qmeshesN ; i++)
    {
        if (qmeshnames[i] == NULL)
        {
            //
            // We were promised qmeshesN, but got less than that.
            //
            break;
        }
        if (strcmp(name, qmeshnames[i]) == 0)
        {
            return qmeshes[i];
        }
    }

    return NULL;
}


// ****************************************************************************
//  Method: InputRepository::GetUcdmesh
//
//  Purpose:
//      Gets a ucd mesh previously stashed in the repository.
//
//  Arguments:
//      name    The name of the ucd mesh.
//
//  Returns:    A pointer to a ucd mesh, NULL if no match is found.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
// ****************************************************************************

DBucdmesh *
InputRepository::GetUcdmesh(char *name)
{
    for (int i = 0 ; i < ucdmeshesN ; i++)
    {
        if (ucdmeshnames[i] == NULL)
        {
            //
            // We were promised quadmeshesN, but got less than that.
            //
            break;
        }
        if (strcmp(name, ucdmeshnames[i]) == 0)
        {
            return ucdmeshes[i];
        }
    }

    return NULL;
}


// ****************************************************************************
//  Method: AddMesh (DBquadmesh *)
//
//  Purpose:
//      Adds a quad mesh to the repository.
//
//  Arguments:
//      qm     One domain of a quadmesh.
//      name   The name of that quadmesh.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
// ****************************************************************************

void
InputRepository::AddMesh(DBquadmesh *qm, char *name)
{
    //
    // To prevent having an extra data member for last quad mesh, just search
    // through the names and see what the last NULL entry is.
    //
    int   i;
    for (i = 0 ; i < qmeshesN ; i++)
    {
        if (qmeshnames[i] == NULL)
        {
            break;
        }
    }

    if (i == qmeshesN)
    {
        //
        // Trying to add more than what was quoted. 
        //
        cerr << "Cannot add more quadmeshes than were specified." << endl;
    }

    //
    // Put the quad mesh in entry i.
    //
    qmeshnames[i] = new char[strlen(name)+1];
    strcpy(qmeshnames[i], name);
    qmeshes[i] = qm;
}


// ****************************************************************************
//  Method: AddMesh (DBpointmesh *)
//
//  Purpose:
//      Adds a point mesh to the repository.
//
//  Arguments:
//      pm     One domain of a pointmesh.
//      name   The name of that pointmesh.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
// ****************************************************************************

void
InputRepository::AddMesh(DBpointmesh *pm, char *name)
{
    //
    // To prevent having an extra data member for last pt mesh, just search
    // through the names and see what the last NULL entry is.
    //
    int   i;
    for (i = 0 ; i < ptmeshesN ; i++)
    {
        if (ptmeshnames[i] == NULL)
        {
            break;
        }
    }

    if (i == ptmeshesN)
    {
        //
        // Trying to add more than what was quoted. 
        //
        cerr << "Cannot add more ptmeshes than were specified." << endl;
    }

    //
    // Put the point mesh in entry i.
    //
    ptmeshnames[i] = new char[strlen(name)+1];
    strcpy(ptmeshnames[i], name);
    ptmeshes[i] = pm;
}


// ****************************************************************************
//  Method: AddMesh (DBucdmesh *)
//
//  Purpose:
//      Adds a ucd mesh to the repository.
//
//  Arguments:
//      um     One domain of a ucdmesh.
//      name   The name of that ucdmesh.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
// ****************************************************************************

void
InputRepository::AddMesh(DBucdmesh *um, char *name)
{
    //
    // To prevent having an extra data member for last ucd mesh, just search
    // through the names and see what the last NULL entry is.
    //
    int   i;
    for (i = 0 ; i < ucdmeshesN ; i++)
    {
        if (ucdmeshnames[i] == NULL)
        {
            break;
        }
    }

    if (i == ucdmeshesN)
    {
        //
        // Trying to add more than what was quoted. 
        //
        cerr << "Cannot add more ucdmeshes than were specified." << endl;
    }

    //
    // Put the ucd mesh in entry i.
    //
    ucdmeshnames[i] = new char[strlen(name)+1];
    strcpy(ucdmeshnames[i], name);
    ucdmeshes[i] = um;
}


// ****************************************************************************
//  Method: InputRepository::SetNPtmesh
//
//  Purpose:
//      Specifies how many point meshes are going to be added.
//
//  Arguments:
//      n     The number of points meshes that will be added later.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
// ****************************************************************************

void
InputRepository::SetNPointmesh(int n)
{
    if (n == 0)
    {
        return;
    }

    ptmeshesN   = n;
    ptmeshes    = new DBpointmesh *[ptmeshesN];
    ptmeshnames = new char *[ptmeshesN];
    for (int i = 0 ; i < ptmeshesN ; i++)
    {
        ptmeshes[i]    = NULL;
        ptmeshnames[i] = NULL;
    }
}


// ****************************************************************************
//  Method: InputRepository::SetNUcdmesh
//
//  Purpose:
//      Specifies how many ucd meshes are going to be added.
//
//  Arguments:
//      n     The number of ucd meshes that will be added later.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
// ****************************************************************************

void
InputRepository::SetNUcdmesh(int n)
{
    if (n == 0)
    {
        return;
    }

    ucdmeshesN   = n;
    ucdmeshes    = new DBucdmesh *[ucdmeshesN];
    ucdmeshnames = new char *[ucdmeshesN];
    for (int i = 0 ; i < ucdmeshesN ; i++)
    {
        ucdmeshes[i]    = NULL;
        ucdmeshnames[i] = NULL;
    }
}


// ****************************************************************************
//  Method: InputRepository::SetNQuadmesh
//
//  Purpose:
//      Specifies how many quad meshes are going to be added.
//
//  Arguments:
//      n     The number of quad meshes that will be added later.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
// ****************************************************************************

void
InputRepository::SetNQuadmesh(int n)
{
    if (n == 0)
    {
        return;
    }

    qmeshesN   = n;
    qmeshes    = new DBquadmesh *[qmeshesN];
    qmeshnames = new char *[qmeshesN];
    for (int i = 0 ; i < qmeshesN ; i++)
    {
        qmeshes[i]    = NULL;
        qmeshnames[i] = NULL;
    }
}


// ****************************************************************************
//  Method: InputRepository::DeleteMeshes
//
//  Purpose:
//      Frees the memory associated with the meshes.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
// ****************************************************************************

void
InputRepository::DeleteMeshes(void)
{
    if (ptmeshnames != NULL)
    {
        for (int i = 0 ; i < ptmeshesN ; i++)
        {
            if (ptmeshnames[i] != NULL)
            {
                delete [] ptmeshnames[i];
                ptmeshnames[i] = NULL;
            }
        }
        delete [] ptmeshnames;
        ptmeshnames = NULL;
    }
    if (ptmeshes != NULL)
    {
        for (int i = 0 ; i < ptmeshesN ; i++)
        {
            if (ptmeshes[i] != NULL)
            {
                DBFreePointmesh(ptmeshes[i]);
                ptmeshes[i] = NULL;
            }
        }
        delete [] ptmeshes;
        ptmeshes = NULL;
    }
    ptmeshesN = 0;

    if (qmeshnames != NULL)
    {
        for (int i = 0 ; i < qmeshesN ; i++)
        {
            if (qmeshnames[i] != NULL)
            {
                delete [] qmeshnames[i];
                qmeshnames[i] = NULL;
            }
        }
        delete [] qmeshnames;
        qmeshnames = NULL;
    }
    if (qmeshes != NULL)
    {
        for (int i = 0 ; i < qmeshesN ; i++)
        {
            if (qmeshes[i] != NULL)
            {
                DBFreeQuadmesh(qmeshes[i]);
                qmeshes[i] = NULL;
            }
        }
        delete [] qmeshes;
        qmeshes = NULL;
    }
    qmeshesN = 0;

    if (ucdmeshnames != NULL)
    {
        for (int i = 0 ; i < ucdmeshesN ; i++)
        {
            if (ucdmeshnames[i] != NULL)
            {
                delete [] ucdmeshnames[i];
                ucdmeshnames[i] = NULL;
            }
        }
        delete [] ucdmeshnames;
        ucdmeshnames = NULL;
    }
    if (ucdmeshes != NULL)
    {
        for (int i = 0 ; i < ucdmeshesN ; i++)
        {
            if (ucdmeshes[i] != NULL)
            {
                DBFreeUcdmesh(ucdmeshes[i]);
                ucdmeshes[i] = NULL;
            }
        }
        delete [] ucdmeshes;
        ucdmeshes = NULL;
    }
    ucdmeshesN = 0;
}


// ****************************************************************************
//  Method: InputRepository::GetMeshNZones
//
//  Purpose:
//      Returns the number of zones for a mesh.
//
//  Arguments:
//      meshname     The name of the mesh.
//
//  Returns:    The number of zones for the mesh specified, -1 on failure.
//
//  Programmer: Hank Childs
//  Creation:   January 7, 2000
//
// ****************************************************************************

int
InputRepository::GetMeshNZones(char *meshname)
{
    int   i;
    
    for (i = 0 ; i < ucdmeshesN ; i++)
    {
        if (strcmp(meshname, ucdmeshnames[i]) == 0)
        {
            return ucdmeshes[i]->zones->nzones;
        }
    }
    for (i = 0 ; i < qmeshesN ; i++)
    {
        if (strcmp(meshname, qmeshnames[i]) == 0)
        {
            int  zones = 1;
            for (int j = 0 ; j < qmeshes[i]->ndims ; j++)
            {
                zones *= qmeshes[i]->dims[j];
            }
            return zones;
        }
    }
    for (i = 0 ; i < ptmeshesN ; i++)
    {
        cerr << "Do not know how to calculate the number of zone for point "
             << "meshes." << endl;
        exit(EXIT_FAILURE);
        /* NOTREACHED */ return -1;
    }
   
    cerr << "Could not match mesh to find the number of zones." << endl;
    exit(EXIT_FAILURE);
    /* NOTREACHED */ return -1;
}


// ****************************************************************************
//  Method: InputRepository::ContainsDomain
//
//  Purpose:
//      Determines if there is a domain in the filename or directory (or one
//      of its subdirectories) that is in the domain list specified.
//
//  Arguments:
//      filename     The name of the file.
//      dirname      The name of the dir.
//
//  Returns:      true if there is a domain, false otherwise
//
//  Programmer: Hank Childs
//  Creation:   February 9, 2000
//
// ****************************************************************************

bool
InputRepository::ContainsDomain(char *filename, char *dirname)
{
    bool  rv = false;

    //
    // Check to see if there is a domain for a mesh in this directory.
    //
    rv = ContainsDomainForEntry(filename, dirname, nMeshes, meshNDomains, 
                                meshDomainListing);
    if (rv == true)
    {
        return rv;
    }

    //
    // Check to see if there is a domain for a var in this directory.
    //
    rv = ContainsDomainForEntry(filename, dirname, nVars, varNDomains, 
                                varDomainListing);
    if (rv == true)
    {
        return rv;
    }

    //
    // Check to see if there is a domain for a material in this directory.
    //
    rv = ContainsDomainForEntry(filename, dirname, nMaterials, 
                                materialNDomains, materialDomainListing);
    if (rv == true)
    {
        return rv;
    }

    return false;
}


// ****************************************************************************
//  Method: InputRepository::ContainsDomainForEntry
//
//  Purpose:
//      Checks through the arrays for a specific kind of entry (mesh, var,
//      material) to see if there is a domain in the list that is in the dir.
//     
//  Arguments:  
//      filename          The name of the file.
//      dirname           The name of the directory.
//      list              A sorted list of domains we are interested in.
//      listN             The size of list.
//      entryN            The number of entries to check.
//      entryNDomains     The number of domains for each of the entries.
//      entryListings     The location of the DB object in SILO format,
//                        ie filename:dirname/object or dirname/object
//
//  Returns:    true if this entry has a domain we are interested in
//              in the directory specified for the file specified.
//
//  Programmer: Hank Childs
//  Creation:   February 9, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Jun 15 17:59:48 PDT 2000
//    Fixed problem where a strstr has its arguments switched when there
//    were multiple files.
//
// ****************************************************************************

bool
InputRepository::ContainsDomainForEntry(char *filename, char *dirname, 
                       int entryN, int *entryNDomains, char ***entryListing)
{
    int  i, j;

    //
    // Create the strings used for comparisons
    // Put in the trailing '/' for the dirname.
    //
    char  filedir[LONG_STRING];
    sprintf(filedir, "%s:%s/", filename, dirname);
    char  dironly[LONG_STRING];
    sprintf(dironly, "%s/", dirname);

    int   minDomain, maxDomain;
    for (i = 0 ; i < entryN ; i++)
    {
        GetDomainRange(entryNDomains[i], &minDomain, &maxDomain);
        for (j = minDomain ; j < maxDomain ; j++)
        {
            //
            // Find out if the domain is in this directory.
            //
            if (strstr(entryListing[i][j], ":") != NULL)
            {
                //
                // The domain listing has a colon, so it has the prepended
                // file name.  Use filedir for comparison.
                //
                if (strstr(entryListing[i][j], filedir) != NULL)
                {
                    return true;
                }
            }
            else
            {
                //
                // The domain listing does not have a colon, so this is a
                // single file.  Use dironly for comparison.
                //
                if (strstr(entryListing[i][j], dironly) != NULL)
                {
                    return true;
                }
            }
        }
    }

    //
    // For the domains we are interested in, there is no match.
    //
    return false;
}


// ****************************************************************************
//  Method: InputRepository::GetDomainRange
//
//  Purpose:
//      Decided what the minimum and maximum domains this processor should
//      process are.
//
//  Arguments:
//      min     The minimum domain number to consider.
//      max     The first domain not to consider (ie '<' not '<=').
//
//  Programmer: Hank Childs
//  Creation:   February 9, 2000
//
// ****************************************************************************

void
InputRepository::GetDomainRange(int nDomains, int *min, int *max)
{
    SplitAmongProcessors(nDomains, min, max);
}


