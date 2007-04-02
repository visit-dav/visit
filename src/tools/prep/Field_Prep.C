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
//                              Field_Prep.C                                 //
// ************************************************************************* //

#include <visitstream.h>
#include <stdlib.h>
#include <string.h>

#include <Field_Prep.h>
#include <SiloObjLib.h>


// 
// Declaration of static consts.  They are not allowed as initializers in 
// class definitions.
//

int    const  Field_Prep::N_DIMS_LIMIT             = 20;
char * const  Field_Prep::SILO_LOCATION            = "/";


// ****************************************************************************
//  Method: Field_Prep constructor
//
//  Note:       Because this is a derived type of Field and because it would
//              like the data members Value to be a derived type of Value,
//              a special constructor for Field is called that indicates that
//              the Values should not be created.  For reasons why this can't
//              be done through a virtual function, see the note in the Field
//              constructor.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
// ****************************************************************************

Field_Prep::Field_Prep() : Field(DERIVED_TYPE_CONSTRUCTOR)
{
    readCentering  = false;
    readDataType   = false;
    readMeshName   = false;
    readUnits      = false;
 
    CreateValues();
}


// ****************************************************************************
//  Method: Field_Prep destructor
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
// ****************************************************************************

Field_Prep::~Field_Prep()
{
    //
    // Do not delete the aliased Value objects because they are aliased to
    // the normal Value objects and will be deleted in the base class's 
    // destructor.
    //
}


// ****************************************************************************
//  Method: Field_Prep::CreateValues
//
//  Purpose:
//      Creates the derived type of the Value objects appropriate for this 
//      class.  That type is Value_Prep.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 14:21:56 PDT 2000
//      Added initialization of aliasedDimsObject.
//
// ****************************************************************************

void
Field_Prep::CreateValues(void)
{
    aliasedValueObject        = new Value_Prep;
    aliasedValueObject->SetResource(STATE_FILE);
    aliasedMixedValueObject   = new Value_Prep;
    aliasedMixedValueObject->SetResource(STATE_FILE);
    aliasedDimsObject         = new Value_Prep;
    aliasedDimsObject->SetResource(STATE_FILE);
    aliasedIntervalTreeObject = new IntervalTree_Prep;

    //
    // We have created the value objects we would like to be our Values.  We
    // cannot use valueObject and mixedValueObject because they are the wrong 
    // type.  Alias them here so that we can use the methods for a Value_Prep,
    // but the base methods for Field will still have access to the same
    // object.
    //
    valueObject        = aliasedValueObject;
    mixedValueObject   = aliasedMixedValueObject;
    dimsObject         = aliasedDimsObject;
    intervalTreeObject = aliasedIntervalTreeObject;
}


// ****************************************************************************
//  Method: Field_Prep::SetNDomains
//
//  Purpose:
//      Sets the number of domains for itself and the objects it contains.
//
//  Arguments:
//      n       The number of domains.
//
//  Programmer: Hank Childs
//  Creation:   January 11, 2000
//
//  Modifications:
//
//      Hank Childs,  Mon Jun 12 14:21:56 PDT 2000
//      Added code to set domains for aliasedDimsObject.
//
// ****************************************************************************

void
Field_Prep::SetNDomains(int n)
{
    nDomains = n;
    aliasedValueObject->SetNDomains(nDomains);
    aliasedMixedValueObject->SetNDomains(nDomains);
    aliasedDimsObject->SetNDomains(nDomains);
    aliasedIntervalTreeObject->SetNDomains(nDomains);
}


// ****************************************************************************
//  Method: Field_Prep::SetName
//
//  Purpose:
//      Sets the name of the field.  When we know the fields name, we can
//      also set the data members which have defined names.  These include
//      intervalTree and values.
//
//  Arguments:
//      meshName  -  The name of the associated mesh.  All field names are
//                   prepended with this when written into a file to guarantee
//                   uniqueness.
//      name      -  The name of the field.
//
//  Note:       Field_Preps that are created from variables do not have access
//              to their mesh's names when they are created, so they send in
//              a "" for meshName.  This is ok, since meshName should only be
//              prepended to the Field_Prep's name if it is coordinates, to 
//              avoid collisions.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 14:21:56 PDT 2000
//      Added code to initialize dims and to set the field name for the
//      aliasedDimsObject.
//
// ****************************************************************************

void
Field_Prep::SetName(char *meshName, char *fieldName)
{
    SetMeshName(meshName);
 
    char  s[LONG_STRING];

    sprintf(s, "%s%s", meshName, fieldName);
    name = CXX_strdup(s);

    sprintf(s, "%s%s", name, IntervalTree::NAME);
    intervalTree = CXX_strdup(s);

    sprintf(s, "%s%s", name, Value::NAME);
    values = CXX_strdup(s);

    sprintf(s, "%s%s%s", name, DIMS_NAME, Value::NAME);
    dims = CXX_strdup(s);

    sprintf(s, "%s%s%s", name, MIXED_VALUES_NAME, Value::NAME);
    mixedValues = CXX_strdup(s);

    aliasedValueObject->SetFieldName(name);
    sprintf(s, "%s%s", name, MIXED_VALUES_NAME);
    aliasedMixedValueObject->SetFieldName(s);
    sprintf(s, "%s%s", name, DIMS_NAME);
    aliasedDimsObject->SetFieldName(s);
    aliasedIntervalTreeObject->SetFieldName(name);
}


// ****************************************************************************
//  Method: Field_Prep::SetMeshName
//
//  Purpose:
//      Field_Preps that are created from vars do not have access to its mesh's 
//      name when they are first created (if they are a multivar).  When
//      they do have access, they can call this function and it will set the
//      mesh's name if it has not already been set.  
//      This function is called by SetName, which will get a dummy argument
//      of "" if the mesh name is not available.  Do not use the dummy
//      argument if it is sent in as an argument.
//
//  Programmer: Hank Childs
//  Creation:   January 13, 2000
//
// ****************************************************************************

void
Field_Prep::SetMeshName(char *meshName)
{
    if (readMeshName == false || strcmp(meshName, "") != 0)
    {
        mesh = CXX_strdup(meshName);
        readMeshName = true;
    }
}


// ****************************************************************************
//  Method: Field_Prep::ReadVarAndWriteArrays (DBucdvar)
//
//  Purpose:
//      Reads in the information related to the field object from a DBucdvar.
//
//  Arguments:
//      dbfile    The Silo file to write the objects to.
//      uv        A pointer to an unstructured var.
//      domain    The domain corresponding to the unstructured mesh.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
//  Modifications:
//
//      Hank Childs, Tue Jun 13 09:36:18 PDT 2000
//      Removed the interval tree processing from this routine since Silo
//      object conversion may now be turned off and interval tree processing
//      can be turned on.
//
// ****************************************************************************

void
Field_Prep::ReadVarAndWriteArrays(DBfile *dbfile, DBucdvar *uv, int domain)
{
    ReadVar(uv->units, uv->datatype, uv->centering);

    aliasedValueObject->WriteArray(dbfile, (void **) uv->vals, dataType, 
                                   domain);

    //
    // Do not worry about the dims value object since this is an unstructured
    // var.
    //

    aliasedMixedValueObject->WriteArray(dbfile, (void **) uv->mixvals, 
                                        dataType, domain);
}


// ****************************************************************************
//  Method: Field_Prep::IntervalTreeReadVar
//
//  Purpose:
//      Gives the Field_Prep's IntervalTree object a chance to process the 
//      current domain's var.
//
//  Arguments:
//     uv       The ucdvar from the current domain.
//     domain   The domain number.
//
//  Programmer: Hank Childs
//  Creation:   June 13, 2000
//
// ****************************************************************************

void
Field_Prep::IntervalTreeReadVar(DBucdvar *uv, int domain)
{    
    aliasedIntervalTreeObject->ReadVar(uv, domain);
}


// ****************************************************************************
//  Method: Field_Prep::ReadVarAndWriteArrays (DBquadvar)
//
//  Purpose:
//      Reads in the information related to the field object from a DBquadvar.
//
//  Arguments:
//      dbfile    The Silo file to write the objects to.
//      qv        A pointer to a quad var.
//      domain    The domain corresponding to the unstructured mesh.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 14:21:56 PDT 2000
//      Added code for the aliasedDimsObject.
//
//      Hank Childs, Tue Jun 13 09:36:18 PDT 2000
//      Removed the interval tree processing from this routine since Silo
//      object conversion may now be turned off and interval tree processing
//      can be turned on.
//
// ****************************************************************************

void
Field_Prep::ReadVarAndWriteArrays(DBfile *dbfile, DBquadvar *qv, int domain)
{
    int  centering;
    if (qv->align[0] == 0.)
    {
        centering = DB_NODECENT;
    }
    else
    {
        centering = DB_ZONECENT;
    }

    ReadVar(qv->units, qv->datatype, centering);

    aliasedValueObject->WriteArray(dbfile, (void **) qv->vals, dataType, 
                                   domain);
    void *dims[1] = { qv->dims };
    aliasedDimsObject->WriteArray(dbfile, dims, DB_INT, domain);
    aliasedMixedValueObject->WriteArray(dbfile, (void **) qv->mixvals, 
                                        dataType, domain);
}


// ****************************************************************************
//  Method: Field_Prep::IntervalTreeReadVar
//
//  Purpose:
//      Gives the Field_Prep's IntervalTree object a chance to process the 
//      current domain's var.
//
//  Arguments:
//     qv       The quadvar from the current domain.
//     domain   The domain number.
//
//  Programmer: Hank Childs
//  Creation:   June 13, 2000
//
// ****************************************************************************

void
Field_Prep::IntervalTreeReadVar(DBquadvar *qv, int domain)
{    
    aliasedIntervalTreeObject->ReadVar(qv, domain);
}


// ****************************************************************************
//  Method: Field_Prep::ReadVarAndWriteArrays (DBmeshvar)
//
//  Purpose:
//      Reads in the information related to the field object from a DBmeshvar.
//
//  Arguments:
//      dbfile    The Silo file to write the objects to.
//      mv        A pointer to a mesh var.
//      domain    The domain corresponding to the unstructured mesh.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
//  Modifications:
//
//      Hank Childs, Tue Jun 13 09:36:18 PDT 2000
//      Removed the interval tree processing from this routine since Silo
//      object conversion may now be turned off and interval tree processing
//      can be turned on.
//
// ****************************************************************************

void
Field_Prep::ReadVarAndWriteArrays(DBfile *dbfile, DBmeshvar *mv, int domain)
{
    ReadVar(mv->units, mv->datatype, mv->centering);

    aliasedValueObject->WriteArray(dbfile, (void **) mv->vals, dataType, 
                                   domain);
    
    //
    // Do not worry about the dims value object since this is a mesh var.
    //

    //
    // Do not worry about the mixed value object because this is a meshvar.
    //
}


// ****************************************************************************
//  Method: Field_Prep::IntervalTreeReadVar
//
//  Purpose:
//      Gives the Field_Prep's IntervalTree object a chance to process the 
//      current domain's var.
//
//  Arguments:
//     mv       The meshvar from the current domain.
//     domain   The domain number.
//
//  Programmer: Hank Childs
//  Creation:   June 13, 2000
//
// ****************************************************************************

void
Field_Prep::IntervalTreeReadVar(DBmeshvar *mv, int domain)
{    
    aliasedIntervalTreeObject->ReadVar(mv, domain);
}


// ****************************************************************************
//  Method: Field_Prep::ReadVarAndWriteArrays (DBpointmesh)
//
//  Purpose:
//      Reads in the information related to the field object from a 
//      DBpointmesh.
//
//  Arguments:
//      dbfile    The Silo file to write the objects to.
//      pm        A pointer to a point mesh.
//      domain    The domain corresponding to the unstructured mesh.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
//  Modifications:
//
//      Hank Childs, Tue Jun 13 09:36:18 PDT 2000
//      Removed the interval tree processing from this routine since Silo
//      object conversion may now be turned off and interval tree processing
//      can be turned on.
//
// ****************************************************************************

void
Field_Prep::ReadVarAndWriteArrays(DBfile *dbfile, DBpointmesh *pm, int domain)
{
    ReadVar(pm->units[0], pm->datatype, -1);

    aliasedValueObject->WriteArray(dbfile, (void **) pm->coords, dataType, 
                                   domain);
    
    //
    // Do not worry about the dims value object since this is a point mesh.
    //

    //
    // Do not worry about the mixed value object since these are coordinates.
    //
}


// ****************************************************************************
//  Method: Field_Prep::IntervalTreeReadVar
//
//  Purpose:
//      Gives the Field_Prep's IntervalTree object a chance to process the 
//      current domain's mesh.
//
//  Arguments:
//     pm       The point mesh from the current domain.
//     domain   The domain number.
//
//  Programmer: Hank Childs
//  Creation:   June 13, 2000
//
// ****************************************************************************

void
Field_Prep::IntervalTreeReadVar(DBpointmesh *pm, int domain)
{    
    aliasedIntervalTreeObject->ReadVar(pm, domain);
}


// ****************************************************************************
//  Method: Field_Prep::ReadVarAndWriteArrays (DBquadmesh)
//
//  Purpose:
//      Reads in the information related to the field object from a 
//      DBquadmesh.
//
//  Arguments:
//      dbfile    The Silo file to write the objects to.
//      qm        A pointer to a quad mesh.
//      domain    The domain corresponding to the unstructured mesh.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 14:21:56 PDT 2000
//      Added code for the aliasedDimsObject.
//
//      Hank Childs, Tue Jun 13 09:36:18 PDT 2000
//      Removed the interval tree processing from this routine since Silo
//      object conversion may now be turned off and interval tree processing
//      can be turned on.
//
// ****************************************************************************

void
Field_Prep::ReadVarAndWriteArrays(DBfile *dbfile, DBquadmesh *qm, int domain)
{
    ReadVar(qm->units[0], qm->datatype, -1);

    aliasedValueObject->WriteArray(dbfile, (void **) qm->coords, dataType, 
                                   domain);
    
    void *dims[1] = { qm->dims };
    aliasedDimsObject->WriteArray(dbfile, dims, DB_INT, 
                                  domain);

    //
    // Do not worry about the mixed value object since these are coordinates.
    //
}


// ****************************************************************************
//  Method: Field_Prep::IntervalTreeReadVar
//
//  Purpose:
//      Gives the Field_Prep's IntervalTree object a chance to process the 
//      current domain's var.
//
//  Arguments:
//     qm       The quadmesh from the current domain.
//     domain   The domain number.
//
//  Programmer: Hank Childs
//  Creation:   June 13, 2000
//
// ****************************************************************************

void
Field_Prep::IntervalTreeReadVar(DBquadmesh *qm, int domain)
{    
    aliasedIntervalTreeObject->ReadVar(qm, domain);
}


// ****************************************************************************
//  Method: Field_Prep::ReadVarAndWriteArrays (DBucdmesh)
//
//  Purpose:
//      Reads in the information related to the field object from a 
//      DBucdmesh.
//
//  Arguments:
//      dbfile    The Silo file to write the objects to.
//      um        A pointer to an unstructured mesh.
//      domain    The domain corresponding to the unstructured mesh.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
//  Modifications:
//
//      Hank Childs, Tue Jun 13 09:36:18 PDT 2000
//      Removed the interval tree processing from this routine since Silo
//      object conversion may now be turned off and interval tree processing
//      can be turned on.
//
// ****************************************************************************

void
Field_Prep::ReadVarAndWriteArrays(DBfile *dbfile, DBucdmesh *um, int domain)
{
    ReadVar(um->units[0], um->datatype, -1);

    aliasedValueObject->WriteArray(dbfile, (void **) um->coords, dataType, 
                                   domain);
    
    //
    // Do not worry about the dims value object since this is a ucd mesh.
    //

    //
    // Do not worry about the mixed value object since these are coordinates.
    //
}


// ****************************************************************************
//  Method: Field_Prep::IntervalTreeReadVar
//
//  Purpose:
//      Gives the Field_Prep's IntervalTree object a chance to process the 
//      current domain's var.
//
//  Arguments:
//     um       The ucdmesh from the current domain.
//     domain   The domain number.
//
//  Programmer: Hank Childs
//  Creation:   June 13, 2000
//
// ****************************************************************************

void
Field_Prep::IntervalTreeReadVar(DBucdmesh *um, int domain)
{    
    aliasedIntervalTreeObject->ReadVar(um, domain);
}


// ****************************************************************************
//  Method: Field_Prep::DetermineUcdMeshSize
//
//  Purpose:
//      Gets the size of a ucdmesh without having to read in the ucd mesh
//      and gives it to its Value object.
//
//  Arguments:
//      domain    The domain corresponding to the current directory in the
//                Silo file.
//      dbfile    A handle to a Silo file, its current directory is the domain
//                of interest.
//      meshName  The name of the mesh.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2000
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 14:21:56 PDT 2000
//      Added code for the aliasedDimsObject.
//
// ****************************************************************************

void
Field_Prep::DetermineUcdMeshSize(int domain, DBfile *dbfile, char *meshName)
{
    void  *rv;

    //
    // This is the first opportunity we have to set the number of dimensions
    // for the Value object.
    //
    if (aliasedValueObject->IsSetNVals() == false)
    {
        rv = DBGetComponent(dbfile, meshName, "ndims");
        int  *ndims = static_cast< int * >(rv);
        aliasedValueObject->SetNVals(*ndims);
        aliasedDimsObject->SetNVals(1);
        free(ndims);
    }

    rv = DBGetComponent(dbfile, meshName, "nnodes");
    int  *nnodes = static_cast< int * >(rv);

    //
    // Make an array that has every element equal to nnodes.  The Value object
    // is a coordinates array.  For a ucdmesh, if the x-coords has nnodes
    // elements, then the y-coords and z-coords will too.
    //
    int    nnodesA[N_DIMS_LIMIT];
    for (int i = 0 ; i < N_DIMS_LIMIT ; i++)
    {
        nnodesA[i] = *nnodes;
    }

    aliasedValueObject->AddDomain(nnodesA, domain);

    //
    // Mesh vars don't really have a dims, so don't worry about the 
    // aliasedDimsObject.
    //

    //
    // We are reading in a mesh, so this is a coordinate field.  Do not
    // worry about the aliasedMixedValueObject.
    //

    //
    // Clean up memory.
    //
    free(nnodes);
}


// ****************************************************************************
//  Method: Field_Prep::DeterminePointMeshSize
//
//  Purpose:
//      Gets the size of a pointmesh without having to read in the point mesh
//      and gives it to its Value object.
//
//  Arguments:
//      domain    The domain corresponding to the current directory in the
//                Silo file.
//      dbfile    A handle to a Silo file, its current directory is the domain
//                of interest.
//      meshName  The name of the mesh.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2000
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 14:21:56 PDT 2000
//      Added code for the aliasedDimsObject.
//
// ****************************************************************************

void
Field_Prep::DeterminePointMeshSize(int domain, DBfile *dbfile, char *meshName)
{
    void   *rv;

    //
    // This is the first opportunity we have to set the number of dimensions
    // for the Value object.
    //
    if (aliasedValueObject->IsSetNVals() == false)
    {
        rv = DBGetComponent(dbfile, meshName, "ndims");
        int  *ndims = static_cast< int * >(rv);
        aliasedValueObject->SetNVals(*ndims);
        aliasedDimsObject->SetNVals(1);
        free(ndims);
    }

    rv   = DBGetComponent(dbfile, meshName, "nels");
    int   *nels = static_cast< int * >(rv);

    //
    // Make an array that has every element equal to nels.  The Value object
    // is a coordinates array.  For a pointmesh, if the x-coords has nels
    // elements, then the y-coords and z-coords will too.
    //
    int    nelsA[N_DIMS_LIMIT];
    for (int i = 0 ; i < N_DIMS_LIMIT ; i++)
    {
        nelsA[i] = *nels;
    }

    aliasedValueObject->AddDomain(nelsA, domain);

    //
    // Point meshes don't really have a dims, so don't worry about the 
    // aliasedDimsObject.
    //

    //
    // We are reading in a mesh, so this is a coordinate field.  Do not
    // worry about the aliasedMixedValueObject.
    //

    //
    // Clean up memory.
    //
    free(nels);
}


// ****************************************************************************
//  Method: Field_Prep::DetermineQuadMeshSize
//
//  Purpose:
//      Gets the size of a Quadmesh without having to read in the Quad mesh
//      and gives it to its Value object.
//
//  Arguments:
//      domain    The domain corresponding to the current directory in the
//                Silo file.
//      dbfile    A handle to a Silo file, its current directory is the domain
//                of interest.
//      meshName  The name of the mesh.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2000
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 14:21:56 PDT 2000
//      Added code for the aliasedDimsObject.
//
// ****************************************************************************

void
Field_Prep::DetermineQuadMeshSize(int domain, DBfile *dbfile, char *meshName)
{
    int    i;
    void  *rv; 

    //
    // This is the first opportunity we have to set the number of dimensions
    // for the Value object.
    //
    if (aliasedValueObject->IsSetNVals() == false)
    {
        rv = DBGetComponent(dbfile, meshName, "ndims");
        int  *ndims = static_cast< int * >(rv);
        aliasedValueObject->SetNVals(*ndims);
        aliasedDimsObject->SetNVals(1);
        free(ndims);
    }

    //
    // The quadmeshes are handled differently depending on the coordtype.
    // 
    rv = DBGetComponent(dbfile, meshName, "coordtype");
    int  *coordtype = static_cast< int * >(rv);

    if (*coordtype == DB_COLLINEAR)
    {
        //
        // This is the one case where the variables in a vector may have a 
        // different number of values.  The number of points in the x-direction
        // may be different than the y or z.  The "dims" field covers how
        // many nodes are in each direction, so pass this straight to the
        // Value object.
        //
        rv = DBGetComponent(dbfile, meshName, "dims");
        int   *dims = static_cast< int * >(rv);
        aliasedValueObject->AddDomain(dims, domain);
        free(dims);
    }
    else
    {
        //
        // The curvilinear case is just like the other meshes.  If you have
        // an x-component, then you must have a matching y-component.
        //
        rv = DBGetComponent(dbfile, meshName, "nnodes");
        int   *nnodes = static_cast< int * >(rv);

        //
        // Make an array that has every element equal to nnodes.  
        //
        int    nnodesA[N_DIMS_LIMIT];
        for (i = 0 ; i < N_DIMS_LIMIT ; i++)
        {
            nnodesA[i] = *nnodes;
        }

        aliasedValueObject->AddDomain(nnodesA, domain);
        free(nnodes);
    }

    //
    // The size of the value object will be 1 for each domain for each 
    // dimension.
    //
    int   dims = aliasedValueObject->GetNVals();
    aliasedDimsObject->AddDomain(&dims, domain);

    //
    // We are reading in a mesh, so this is a coordinate field.  Do not
    // worry about the aliasedMixedValueObject.
    //

    //
    // Clean up memory.
    //
    free(coordtype);
}


// ****************************************************************************
//  Method: Field_Prep::DetermineUcdvarSize
//
//  Purpose:
//      Gets the size of a Ucdvar without having to read in the ucd var
//      and gives it to its Value object.
//
//  Arguments:
//      domain    The domain corresponding to the current directory in the
//                Silo file.
//      dbfile    A handle to a Silo file, its current directory is the domain
//                of interest.
//      varName   The name of a mesh variable.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2000
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 14:21:56 PDT 2000
//      Added code for the aliasedDimsObject.
//
//      Brad Whitlock, Fri Jun 23 16:53:00 PST 2000
//      Fixed a small for-loop scope problem rejected by some compilers.
//
// ****************************************************************************

void
Field_Prep::DetermineUcdvarSize(int domain, DBfile *dbfile, char *varName)
{
    void  *rv; 

    //
    // This is the first opportunity we have to set the number of dimensions
    // for the Value object.
    //
    if (aliasedValueObject->IsSetNVals() == false)
    {
        rv = DBGetComponent(dbfile, varName, "nvals");
        int  *nvals = static_cast< int * >(rv);
        aliasedValueObject->SetNVals(*nvals);
        aliasedMixedValueObject->SetNVals(*nvals);
        aliasedDimsObject->SetNVals(1);
        free(nvals);
    }

    rv = DBGetComponent(dbfile, varName, "nels");
    int  *nels = static_cast< int * >(rv);

    //
    // Make an array that has every element equal to nels. 
    // If this is a vector variable, then each var in the vector will have
    // the same sized array (namely nels).
    //
    int    nelsA[N_DIMS_LIMIT];
    for (int i = 0 ; i < N_DIMS_LIMIT ; i++)
    {
        nelsA[i] = *nels;
    }
    free(nels);

    aliasedValueObject->AddDomain(nelsA, domain);

    //
    // Ucd vars don't really have a dims, so don't worry about the 
    // aliasedDimsObject.
    //

    rv = DBGetComponent(dbfile, varName, "mixlen");
    int  *mixlen = static_cast< int * >(rv);
    int    mixlenA[N_DIMS_LIMIT];
    for (int i = 0 ; i < N_DIMS_LIMIT ; i++)
    {
        mixlenA[i] = *mixlen;
    }
    free(mixlen);
   
    aliasedMixedValueObject->AddDomain(mixlenA, domain);
}


// ****************************************************************************
//  Method: Field_Prep::DetermineQuadvarSize
//
//  Purpose:
//      Gets the size of a quadvar without having to read in the quad var
//      and gives it to its Value object.
//
//  Arguments:
//      domain    The domain corresponding to the current directory in the
//                Silo file.
//      dbfile    A handle to a Silo file, its current directory is the domain
//                of interest.
//      varName   The name of a quad variable.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2000
//
//  Modification:
//
//      Hank Childs, Mon Jun 12 14:21:56 PDT 2000
//      Added code for the aliasedDimsObject.
//
// ****************************************************************************

void
Field_Prep::DetermineQuadvarSize(int domain, DBfile *dbfile, char *varName)
{
    void  *rv; 
    int    i;

    //
    // This is the first opportunity we have to set the number of dimensions
    // for the Value object.
    //
    if (aliasedValueObject->IsSetNVals() == false)
    {
        rv = DBGetComponent(dbfile, varName, "nvals");
        int  *nvals = static_cast< int * >(rv);
        aliasedValueObject->SetNVals(*nvals);
        aliasedMixedValueObject->SetNVals(*nvals);
        aliasedDimsObject->SetNVals(1);
        free(nvals);
    }

    rv = DBGetComponent(dbfile, varName, "nels");
    int  *nels = static_cast< int * >(rv);

    //
    // Make an array that has every element equal to nels. 
    // If this is a vector variable, then each var in the vector will have
    // the same sized array (namely nels).
    //
    int    nelsA[N_DIMS_LIMIT];
    for (i = 0 ; i < N_DIMS_LIMIT ; i++)
    {
        nelsA[i] = *nels;
    }
    free(nels);
    aliasedValueObject->AddDomain(nelsA, domain);

    //
    // The size of the value object will be 1 for each domain for each 
    // dimension.
    //
    int   dims = aliasedValueObject->GetNVals();
    aliasedDimsObject->AddDomain(&dims, domain);

    rv = DBGetComponent(dbfile, varName, "mixlen");
    int  *mixlen = static_cast< int * >(rv);
    int   mixlenA[N_DIMS_LIMIT];
    for (i = 0 ; i < N_DIMS_LIMIT ; i++)
    {
        mixlenA[i] = *mixlen;
    }
    free(mixlen);
   
    aliasedMixedValueObject->AddDomain(mixlenA, domain);
}


// ****************************************************************************
//  Method: Field_Prep::DetermineMeshvarSize
//
//  Purpose:
//      Gets the size of a Meshvar without having to read in the Mesh var
//      and gives it to its Value object.
//
//  Arguments:
//      domain    The domain corresponding to the current directory in the
//                Silo file.
//      dbfile    A handle to a Silo file, its current directory is the domain
//                of interest.
//      varName   The name of a mesh variable.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2000
//
//  Modification:
//
//      Hank Childs, Mon Jun 12 14:21:56 PDT 2000
//      Added code for the aliasedDimsObject.
//
// ****************************************************************************

void
Field_Prep::DetermineMeshvarSize(int domain, DBfile *dbfile, char *varName)
{
    void  *rv; 

    //
    // This is the first opportunity we have to set the number of dimensions
    // for the Value object.
    //
    if (aliasedValueObject->IsSetNVals() == false)
    {
        rv = DBGetComponent(dbfile, varName, "nvals");
        int  *nvals = static_cast< int * >(rv);
        aliasedValueObject->SetNVals(*nvals);
        aliasedDimsObject->SetNVals(1);
        aliasedMixedValueObject->SetNVals(*nvals);
        free(nvals);
    }

    rv = DBGetComponent(dbfile, varName, "nels");
    int  *nels = static_cast< int * >(rv);

    //
    // Make an array that has every element equal to nels. 
    // If this is a vector variable, then each var in the vector will have
    // the same sized array (namely nels).
    //
    int    nelsA[N_DIMS_LIMIT];
    for (int i = 0 ; i < N_DIMS_LIMIT ; i++)
    {
        nelsA[i] = *nels;
    }
    free(nels);

    aliasedValueObject->AddDomain(nelsA, domain);

    //
    // Mesh vars don't really have a dims, so don't worry about the 
    // aliasedDimsObject.
    //

    //
    // Mesh vars don't have a mixlen field, so don't worry about the 
    // aliasedMixedValueObject.
    //
}


// ****************************************************************************
//  Method: Field_Prep::ReadVar 
//
//  Purpose:
//     Called by all of the read var routines that take a SILO var, this does
//     the actual work and is a central method used by all three.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
// ****************************************************************************

void
Field_Prep::ReadVar(char *u, int d, int c)
{
    if (u != NULL && readUnits == false)
    {
        units = CXX_strdup(u);
        readUnits = true;
    }

    if (d != -1 && readDataType == false)
    {
        dataType = d;
        readDataType = true;
    }

    if (c != -1 && readCentering == false)
    {
        centering = c;
        readCentering = true;
    }
}   


// ****************************************************************************
//  Method: Field_Prep::Write
//
//  Purpose:
//      Write out a Field_Prep to a SILO file.
//
//  Arguments:
//      dbfile   A handle to the file the Field should be written to.
//
//  Programmer: Hank Childs
//  Creation:   December 9, 1999
//
// ****************************************************************************

void 
Field_Prep::Write(DBfile *dbfile)
{
#ifdef PARALLEL
    extern int my_rank;
    if (my_rank != 0)
    {
        return;
    }
#endif

    //
    // Set the directory in the file to be correct.
    //
    if (DBSetDir(dbfile, SILO_LOCATION) < 0)
    {
        cerr << "Unable to change directories in the silo file to " 
             << SILO_LOCATION << "." << endl;
        exit(EXIT_FAILURE);
    }

    Field::Write(dbfile);
}


// ****************************************************************************
//  Method: Field_Prep::WriteIntervalTree
//
//  Purpose:
//      Write out a Field_Prep's interval tree to a SILO file.
//
//  Arguments:
//      dbfile   A handle to the file the interval tree should be written to.
//
//  Programmer: Hank Childs
//  Creation:   June 13, 2000
//
// ****************************************************************************

void 
Field_Prep::WriteIntervalTree(DBfile *dbfile)
{
#ifdef PARALLEL
    extern int my_rank;
    if (my_rank != 0)
    {
        return;
    }
#endif

    //
    // Set the directory in the file to be correct.
    //
    if (DBSetDir(dbfile, SILO_LOCATION) < 0)
    {
        cerr << "Unable to change directories in the silo file to " 
             << SILO_LOCATION << "." << endl;
        exit(EXIT_FAILURE);
    }

    Field::WriteIntervalTree(dbfile);
}


// ****************************************************************************
//  Method: Field_Prep::Consolidate
//
//  Purpose:
//      Has the objects it contains 'consolidate'.  A place to do some 
//      additional pre-processing after the initial pass is completed.
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
//  Modifications:
//     
//      Hank Childs, Mon Jun 12 14:21:56 PDT 2000
//      Added code to have the aliasedDimsObject consolidate.
//
// ****************************************************************************

void
Field_Prep::Consolidate(void)
{
    aliasedValueObject->Consolidate();
    aliasedMixedValueObject->Consolidate();
    aliasedDimsObject->Consolidate();
}


// ****************************************************************************
//  Method: Field_Prep::WrapUp
//
//  Purpose:
//      Has the objects it contains 'wrap up'.  A place to do further 
//      processing after all of the information is collected.
//
//  Programmer: Hank Childs
//  Creation:   January 11, 2000
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 14:21:56 PDT 2000
//      Added code to have the aliasedDimsObject wrap up.
//
//      Hank Childs, Tue Jun 13 10:12:04 PDT 2000
//      Removed wrapping up of interval tree object so it could be in a
//      separately accessible function.
//
// ****************************************************************************

void
Field_Prep::WrapUp(void)
{
    aliasedValueObject->WrapUp();
    aliasedMixedValueObject->WrapUp();
    aliasedDimsObject->WrapUp();

    //
    // aliasedValueObject contains the number of entries and has already done 
    // the work of reducing it across all of the processors, so just steal it.
    //
    nValues      = aliasedValueObject->GetSize();
    nMixedValues = aliasedMixedValueObject->GetSize();
}


// ****************************************************************************
//  Method: Field_Prep::WrapUpIntervalTree
//
//  Purpose:
//      Has the interval tree it contains 'wrap up'.  A place to do further 
//      processing after all of the information is collected.
//
//  Programmer: Hank Childs
//  Creation:   June 13, 2000
//
// ****************************************************************************

void
Field_Prep::WrapUpIntervalTree(void)
{
    aliasedIntervalTreeObject->WrapUp();
}


