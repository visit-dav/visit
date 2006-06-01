/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
//                           StateTableOfContents.C                          //
// ************************************************************************* //

#include <visitstream.h>
#include <stdlib.h>
#include <string.h>

#include <SiloObjLib.h>
#include <StateTableOfContents.h>


// ****************************************************************************
//  Method: StateTableOfContents constructor
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
// ****************************************************************************

StateTableOfContents::StateTableOfContents()
{
    fields       = NULL;
    fieldsN      = 0;
    trees        = NULL;
    treesN       = 0;
    mats         = NULL;
    matsN        = 0;
    meshes       = NULL;
    meshesN      = 0;
    values       = NULL;
    valuesN      = 0;
}


// ****************************************************************************
//  Method: StateTableOfContents destructor
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
// ****************************************************************************

StateTableOfContents::~StateTableOfContents()
{
    if (fields != NULL)
    {
        for (int i = 0 ; i < fieldsN ; i++)
        {
            if (fields[i] != NULL)
            {
                delete fields[i];
            }
        }
        delete [] fields;
    }
    if (trees != NULL)
    {
        for (int i = 0 ; i < treesN ; i++)
        {
            if (trees[i] != NULL)
            {
                delete trees[i];
            }
        }
        delete [] trees;
    }
    if (mats != NULL)
    {
        delete [] mats;
    }
    if (meshes != NULL)
    {
        delete [] meshes;
    }
    if (values != NULL)
    {
        for (int i = 0 ; i < valuesN ; i++)
        {
            if (values[i] != NULL)
            {
                delete values[i];
            }
        }
        delete [] values;
    }
}


// ****************************************************************************
//  Method: StateTableOfContents::ReadFile
//
//  Purpose:
//      Reads in the DBobjects from a state file.
//
//  Arguments:
//      filename     The name of the file to read.
//
//  Programmer: Hank Childs
//  Creation:   January 31, 2000
//
// ****************************************************************************

void
StateTableOfContents::ReadFile(const char *filename)
{
    int   i;

    DBfile *dbfile = GetSILOFile(filename);
    DBtoc  *toc    = DBGetToc(dbfile);

    meshes  = new Mesh_VTK[toc->nobj];
    fields  = new Field_VTK*[toc->nobj];
    values  = new Value_VTK*[toc->nobj];
    trees   = new IntervalTree_VTK*[toc->nobj];
    mats    = new Material[toc->nobj];

    for (i = 0 ; i < toc->nobj ; i++)
    {
        values[i] = NULL;
        trees[i]  = NULL;
    }

    for (i = 0 ; i < toc->nobj ; i++)
    {
        DBobject  *siloObj = DBGetObject(dbfile, toc->obj_names[i]);

        if (strcmp(siloObj->type, Mesh::SILO_TYPE) == 0)
        {
            meshes[meshesN].Read(siloObj);
            meshesN++;
        }
        else if (strcmp(siloObj->type, Field::SILO_TYPE) == 0)
        {
            fields[fieldsN] = new Field_VTK;
            fields[fieldsN]->Read(siloObj);
            fieldsN++;
        }
        else if (strcmp(siloObj->type, Value::SILO_TYPE) == 0)
        {
            values[valuesN] = new Value_VTK;
            values[valuesN]->Read(siloObj, dbfile);
            valuesN++;
        }
        else if (strcmp(siloObj->type, IntervalTree::SILO_TYPE) == 0)
        {
            trees[treesN] = new IntervalTree_VTK;
            trees[treesN]->Read(siloObj, dbfile);
            treesN++;
        }
        else if (strcmp(siloObj->type, Material::SILO_TYPE) == 0)
        {
            mats[matsN].Read(siloObj, dbfile);
            matsN++;
        }
 
        DBFreeObject(siloObj);
    }

    //
    // Do not need to close the SILO file, since TOC will manage it.
    //
}


// ****************************************************************************
//  Method: StateTableOfContents::UpdateReferences
//
//  Purpose:
//      Give each object a reference to the TOC (this) so that they can
//      update the pointers to SILO objects they have as data members.  They
//      also need a reference to this so that they can request files.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2000
//
// ****************************************************************************

void
StateTableOfContents::UpdateReferences(void)
{
    int  i = 0;

    //
    // Fields must be updated before meshes since some fields will be taken out
    // of the contents list and be contained by a Mesh.  If this is the case,
    // then it must have a chance to update references before it is removed.
    //
    for (i = 0 ; i < valuesN ; i++)
    {
        values[i]->UpdateReferences(this);
    }
    for (i = 0 ; i < fieldsN ; i++)
    { 
        fields[i]->UpdateReferences(this);
    }
    for (i = 0 ; i < meshesN ; i++)
    { 
        meshes[i].UpdateReferences(this);
    }
}


// ****************************************************************************
//  Method: StateTableOfContents::GetIntervalTree
//   
//  Purpose:
//      Goes through the interval tree objects and returns the one with the
//      correct name.  Meant for contained objects when UpdateReferences is
//      called.
//
//  Arguments:
//      name    The name of the interval tree.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2000
//
// ****************************************************************************

IntervalTree_VTK *
StateTableOfContents::GetIntervalTree(const char *name)
{
    IntervalTree_VTK *rv = NULL;
    int i;
    for (i = 0 ; i < treesN ; i++)
    {
        if (strcmp(name, trees[i]->GetName()) == 0)
        {
            rv = trees[i];
            trees[i] = NULL;
            break;
        }
    }

    //
    // Take the interval tree out of the list and guarantee that there are not
    // any NULL entries lying in wait.
    //
    for (int j = i ; j < treesN ; j++)
    {
        trees[j] = trees[j+1];
    }
    if (i < treesN)
    {
        treesN--;
    }

    return rv;
}


// ****************************************************************************
//  Method: StateTableOfContents::GetField
//   
//  Purpose:
//      Goes through the field objects and returns the one with the correct 
//      name.  Meant for contained objects when UpdateReferences is called.
//
//  Arguments: 
//      name    The name of the field.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2000
//
// ****************************************************************************

Field_VTK *
StateTableOfContents::GetField(const char *name)
{
    Field_VTK *rv = NULL;
    int i;
    for (i = 0 ; i < fieldsN ; i++)
    {
        if (strcmp(name, fields[i]->GetName()) == 0)
        {
            rv = fields[i];
            fields[i] = NULL;
            break;
        }
    }

    //
    // Take the Field out of the list and guarantee that there are not any
    // NULL entries lying in wait.
    //
    for (int j = i ; j < fieldsN ; j++)
    {
        fields[j] = fields[j+1];
    }
    if (i < fieldsN)
    {
        fieldsN--;
    }

    return rv;
}


// ****************************************************************************
//  Method: StateTableOfContents::GetValue
//   
//  Purpose:
//      Goes through the value objects and returns the one with the correct 
//      name.  Meant for contained objects when UpdateReferences is called.
//
//  Arguments:
//      name    The name of the Value object.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2000
//
// ****************************************************************************

Value_VTK *
StateTableOfContents::GetValue(const char *name)
{
    Value_VTK *rv = NULL;
    int i;
    for (i = 0 ; i < valuesN ; i++)
    {
        if (strcmp(name, values[i]->GetName()) == 0)
        {
            rv = values[i];
            values[i] = NULL;
            break;
        }
    }

    //
    // Take the Value out of the list and guarantee that there are not any
    // NULL entries lying in wait.
    //
    for (int j = i ; j < valuesN ; j++)
    {
        values[j] = values[j+1];
    }
    if (i < valuesN)
    {
        valuesN--;
    }

    return rv;
}


// ****************************************************************************
//  Method: StateTableOfContents::GetCoords
//
//  Purpose:
//      Gets the coordinates for an unstructured mesh.
//
//  Arguments:
//      name          The name of the mesh.
//      domainList    The list of domains of interest.
//      listN         The number of domains in domainList.
//      ugrid         The unstructured grid where the coordinates will be put.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2000
//
// ****************************************************************************

void
StateTableOfContents::GetCoords(const char *name, const int *domainList, 
                                int listN, vtkUnstructuredGrid **ugrid)
{
    Mesh_VTK  *mesh = ResolveMesh(name);
    if (mesh == NULL)
    {
        cerr << "Unable to resolve meshname " << name << endl;
        // throw
        return;
    }

    mesh->GetCoords(domainList, listN, ugrid);
}
     

// ****************************************************************************
//  Method: StateTableOfContents::GetCoords
//
//  Purpose:
//      Gets the coordinates for a rectilinear mesh.
//
//  Arguments:
//      name          The name of the mesh.
//      domainList    The list of domains of interest.
//      listN         The number of domains in domainList.
//      rgrid         The rectilinear grid where the coordinates will be put.
//
//  Programmer: Hank Childs
//  Creation:   March 4, 2000
//
// ****************************************************************************

void
StateTableOfContents::GetCoords(const char * name, const int * domainList, 
                                int listN, vtkRectilinearGrid **rgrid)
{
    Mesh_VTK  *mesh = ResolveMesh(name);
    if (mesh == NULL)
    {
        cerr << "Unable to resolve meshname " << name << endl;
        // throw
        return;
    }

    mesh->GetCoords(domainList, listN, rgrid);   
}
     

// ****************************************************************************
//  Method: StateTableOfContents::GetCoords
//
//  Purpose:
//      Gets the coordinates for a curvilinear mesh.
//
//  Arguments:
//      name          The name of the mesh.
//      domainList    A list of the domains.
//      listN         The size of domainList.
//      sgrid         The structured grid to put the coordinates in.
//
//  Programmer: Hank Childs
//  Creation:   April 3, 2000
//
// ****************************************************************************

void
StateTableOfContents::GetCoords(const char * name, const int * domainList, 
                                int listN, vtkStructuredGrid **sgrid)
{
    Mesh_VTK  *mesh = ResolveMesh(name);
    if (mesh == NULL)
    {
        cerr << "Unable to resolve meshname " << name << endl;
        // throw
        return;
    }

    mesh->GetCoords(domainList, listN, sgrid);   
}
     

// ****************************************************************************
//  Method: StateTableOfContents::GetVar
//
//  Purpose:
//      Gets the variable specified for the current mesh.
//
//  Arguments:
//      varName      The name of the variable.
//      domainList   The list of domains of interest.
//      listSize     The size of domainList.
//      scalars      The vtk object to put the scalars in.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2000
//
// ****************************************************************************

void
StateTableOfContents::GetVar(const char *varName, const int *domainList, 
                             int listSize, vtkScalars **scalars)
{
    Field_VTK  *field = ResolveField(varName);
    if (field == NULL)
    {
        cerr << "Unable to resolve var " << varName << endl;
        // throw
        return;
    }

    field->GetVar(domainList, listSize, scalars);
}


// ****************************************************************************
//  Method: StateTableOfContents::GetMetaData
//
//  Purpose:
//      Gets the IntervalTree for the variable or mesh specified.
//
//  Arguments:  
//      name    The name of the mesh or var to find the meta data for.
//
//  Returns:    A pointer to a constant interval tree.
//
//  Programmer: Hank Childs
//  Creation:   April 12, 2000
//
// ****************************************************************************

const IntervalTree_VTK *
StateTableOfContents::GetMetaData(const char *name)
{
    Mesh_VTK  *mesh = ResolveMesh(name);
    if (mesh != NULL)
    {
        return mesh->GetMetaData();
    }

    Field_VTK *field = ResolveField(name);
    if (field != NULL)
    {
        return field->GetMetaData();
    }

    //
    // Not a Field or a Mesh, what to do?
    //
    cerr << "Unable to resolve type of " << name << endl;
    // throw
    return NULL;
}


// ****************************************************************************
//  Method: StateTableOfContents::ResolveField
//
//  Purpose:
//      Determines which Field matches the name.
//
//  Arguments:
//      name   The name of the field.
//
//  Returns:   The valid Field_VTK reference.
//
//  Programmer: Hank Childs
//  Creation:   February 4, 2000
//
// ****************************************************************************

Field_VTK *
StateTableOfContents::ResolveField(const char *name)
{
    for (int i = 0 ; i < fieldsN ; i++)
    {
        if (strcmp(name, fields[i]->GetName()) == 0)
        {
            return fields[i];
        }
    }
   
    // throw ???
    return NULL;
}
    

// ****************************************************************************
//  Method: StateTableOfContents::ResolveMesh
//
//  Purpose:
//      Determines which Mesh is the current mesh and returns it.
//
//  Arguments:
//      name    The name of the mesh.
//
//  Returns:    A pointer to the Mesh, NULL if failure.
//
//  Programmer: Hank Childs
//  Creation:   February 4, 2000
//
// ****************************************************************************

Mesh_VTK *
StateTableOfContents::ResolveMesh(const char *name)
{
    for (int i = 0 ; i < meshesN ; i++)
    {
        if (strcmp(name, meshes[i].GetName()) == 0)
        {
            return &(meshes[i]);
        }
    }
   
    // throw ???
    return NULL;
}


// ****************************************************************************
//  Method: StateTableOfContents::GetMeshName
//
//  Purpose:
//      Gets the mesh name from a variable.
//
//  Returns:    The name of the mesh associated with the input variable.
//
//  Programmer: Hank Childs
//  Creation:   February 14, 2000
//
// ****************************************************************************

const char *
StateTableOfContents::GetMeshName(const char *varname)
{
    Field_VTK  *field = ResolveField(varname);
    if (field == NULL)
    {
        return NULL;
    }

    return  field->GetMeshName();
}


// ****************************************************************************
//  Method: StateTableOfContents::GetMeshType
//
//  Purpose:
//      Gets the type of the mesh.
//
//  Arguments:
//      meshname   The name of the mesh.
//
//  Returns:    The enumerated typed type of the mesh.
//
//  Programmer: Hank Childs
//  Creation:   March 4, 2000
//
// ****************************************************************************

MESH_TYPE_e
StateTableOfContents::GetMeshType(const char *meshname)
{
    Mesh_VTK *mesh = ResolveMesh(meshname);
    if (mesh == NULL)
    {
        cerr << "Unable to resolve meshname " << meshname << endl;
        // throw
        return UNKNOWN;
    }

    return mesh->GetMeshType();
}


// ****************************************************************************
//  Method: StateTableOfContents::GetNDomains
//
//  Purpose:
//      Takes in a mesh name and determines how many domains that mesh has.
//
//  Arguments:
//      meshname      The name of a mesh.
//
//  Returns:          The number of domains in meshname.
//
//  Programmer:       Hank Childs
//  Creation:         August 10, 2000
//
// ****************************************************************************

int
StateTableOfContents::GetNDomains(const char *meshname)
{
    Mesh_VTK *mesh = ResolveMesh(meshname);

    if (mesh == NULL)
    {
        return -1;
    }

    return mesh->GetNDomains();
}


// ****************************************************************************
//  Method: StateTableOfContents::GetCentering
//
//  Purpose:
//      Gets the centering of a variable.
//
//  Arguments:
//      varName    The name of the variable of interest.
//
//  Returns:       The centering of the variable in its Silo type
//                 (ie DB_NODECENT, DB_ZONECENT).
//
//  Programmer:    Hank Childs
//  Creation:      September 15, 2000
//
// ****************************************************************************

int
StateTableOfContents::GetCentering(const char *varName)
{
    Field_VTK  *field = ResolveField(varName);
    
    return (field != NULL ? field->GetCentering() : DB_NODECENT);
}


