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
//                       UnstructuredTopology_Prep.C                         //
// ************************************************************************* //

#include <stdlib.h>
#include <string.h>

#include <UnstructuredTopology_Prep.h>


//
// Initialization of class-scoped constants (statics) is not allowed in the
// class definition, so do that here.
//

char * const  UnstructuredTopology_Prep::SILO_LOCATION         = "/";


// ****************************************************************************
//  Method: UnstructuredTopology_Prep constructor
//
//  Programmer: Hank Childs
//  Creation:   January 28, 2000
//
// ****************************************************************************

UnstructuredTopology_Prep::UnstructuredTopology_Prep() 
    : UnstructuredTopology(DERIVED_TYPE_CONSTRUCTOR)
{
    nDomains = -1;

    CreateValues();
}


// ****************************************************************************
//  Method: UnstructuredTopology_Prep destructor
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

UnstructuredTopology_Prep::~UnstructuredTopology_Prep()
{
    //
    // Do not delete any of the aliased Values since they will be deleted by
    // the base constructor.
    //
}


// ****************************************************************************
//  Method: UnstructuredTopology_Prep::CreateValues
//
//  Purpose:
//      Creates Value_Prep objects and aliases them to the data members in the
//      base class.
//
//  Programmer: Hank Childs
//  Creation:   January 28, 2000
//
// ****************************************************************************

void
UnstructuredTopology_Prep::CreateValues(void)
{
    aliasedNodeListValue    = new Value_Prep;
    aliasedNodeListValue->SetResource(MESH_FILE);
    aliasedShapeCountValue  = new Value_Prep;
    aliasedShapeCountValue->SetResource(MESH_FILE);
    aliasedShapeTypeValue   = new Value_Prep;
    aliasedShapeTypeValue->SetResource(MESH_FILE);
    aliasedShapeSizeValue   = new Value_Prep;
    aliasedShapeSizeValue->SetResource(MESH_FILE);

    //
    // We have created the value objects we would like to be our Values.  We
    // cannot use the aliased Value_Prep objects because they are the wrong
    // type.  Alias them here so that we can use the methods for a Value_Prep,
    // but the base methods for Field will still have access to the same
    // object.
    //
    nodeListValue    = aliasedNodeListValue;
    shapeCountValue  = aliasedShapeCountValue;
    shapeTypeValue   = aliasedShapeTypeValue;
    shapeSizeValue   = aliasedShapeSizeValue;
}


// ****************************************************************************
//  Method: UnstructuredTopology_Prep::SetName
//
//  Purpose:
//      Sets the names of the objects that are contained by the unstructured
//      topology object.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2000
//
// ****************************************************************************

void
UnstructuredTopology_Prep::SetName(char *n)
{
    char  qualifiedName[LONG_STRING];

    name = CXX_strdup(n);

    sprintf(qualifiedName, "%s%s", n, SILO_NODE_LIST_NAME);
    aliasedNodeListValue->SetFieldName(qualifiedName);
    sprintf(qualifiedName, "%s%s%s", n, SILO_NODE_LIST_NAME, Value::NAME);
    nodeList = CXX_strdup(qualifiedName);

    sprintf(qualifiedName, "%s%s", n, SILO_SHAPE_COUNT_NAME);
    aliasedShapeCountValue->SetFieldName(qualifiedName);
    sprintf(qualifiedName, "%s%s%s", n, SILO_SHAPE_COUNT_NAME, Value::NAME);
    shapeCount = CXX_strdup(qualifiedName);

    sprintf(qualifiedName, "%s%s", n, SILO_SHAPE_TYPE_NAME);
    aliasedShapeTypeValue->SetFieldName(qualifiedName);
    sprintf(qualifiedName, "%s%s%s", n, SILO_SHAPE_TYPE_NAME, Value::NAME);
    shapeType = CXX_strdup(qualifiedName);

    sprintf(qualifiedName, "%s%s", n, SILO_SHAPE_SIZE_NAME);
    aliasedShapeSizeValue->SetFieldName(qualifiedName);
    sprintf(qualifiedName, "%s%s%s", n, SILO_SHAPE_SIZE_NAME, Value::NAME);
    shapeSize = CXX_strdup(qualifiedName);
}


// ****************************************************************************
//  Method: UnstructuredTopology_Prep::SetDomains
//
//  Purpose:
//      Sets the number of domains for the objects that are contained by the
//      unstructured topology object.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2000
//
// ****************************************************************************

void
UnstructuredTopology_Prep::SetDomains(int n)
{
    nDomains = n;

    aliasedNodeListValue->SetNDomains(nDomains);
    aliasedNodeListValue->SetNVals(1);

    aliasedShapeCountValue->SetNDomains(nDomains);
    aliasedShapeCountValue->SetNVals(1);

    aliasedShapeTypeValue->SetNDomains(nDomains);
    aliasedShapeTypeValue->SetNVals(1);

    aliasedShapeSizeValue->SetNDomains(nDomains);
    aliasedShapeSizeValue->SetNVals(1);
}


// ****************************************************************************
//  Method: UnstructuredTopology_Prep::Consolidate
//
//  Purpose:
//      Has the object do any additional preprocessing after the initial pass
//      is completed.
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
// ****************************************************************************

void
UnstructuredTopology_Prep::Consolidate(void)
{
    aliasedNodeListValue->Consolidate();
    aliasedShapeCountValue->Consolidate();
    aliasedShapeTypeValue->Consolidate();
    aliasedShapeSizeValue->Consolidate();
}


// ****************************************************************************
//  Method: UnstructuredTopology_Prep::WrapUp
//
//  Purpose:
//      Has the object 'wrap up'.  A place to do further processing after all
//      of the information is collected.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2000
//
// ****************************************************************************

void
UnstructuredTopology_Prep::WrapUp(void)
{
    aliasedNodeListValue->WrapUp();
    aliasedShapeCountValue->WrapUp();
    aliasedShapeTypeValue->WrapUp();
    aliasedShapeSizeValue->WrapUp();
}


// ****************************************************************************
//  Method: UnstructuredTopology_Prep::Write
//
//  Purpose:
//      Writes out the unstructured topology object to a silo file.  This is
//      done by calling the base method to do this.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2000
//
// ****************************************************************************

void
UnstructuredTopology_Prep::Write(DBfile *dbfile)
{
#ifdef PARALLEL
    extern int my_rank;
    if (my_rank != 0)
    {
        return;
    }
#endif

    if (DBSetDir(dbfile, SILO_LOCATION) < 0)
    {
        cerr << "Unable to change directories in the silo file to "
             << SILO_LOCATION << "." << endl;
        exit(EXIT_FAILURE);
    }

    UnstructuredTopology::Write(dbfile);
}


// ****************************************************************************
//  Method: UnstructuredTopology_Prep::DetermineSize
//
//  Purpose:
//      Determines how large the Value objects it contains are going to be.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2000
//
// ****************************************************************************

void
UnstructuredTopology_Prep::DetermineSize(int domain, DBfile *dbfile, 
                                         char *meshName)
{
    void   *rv;

    //
    // We have to get the name of the zones component first.
    //
    rv = DBGetComponent(dbfile, meshName, "zonelist");
    char *zones = static_cast< char * >(rv);
    if (zones == NULL)
    {
        cerr << "Could not determine the component name of zones." << endl;
        exit(EXIT_FAILURE);
    }

    rv = DBGetComponent(dbfile, zones, "lnodelist");
    int *lnodelist = static_cast< int * >(rv);
    if (lnodelist == NULL)
    {
        cerr << "Could not determine the size of the nodelist." << endl;
        exit(EXIT_FAILURE);
    }
    aliasedNodeListValue->AddDomain(lnodelist, domain);

    rv = DBGetComponent(dbfile, zones, "nshapes");
    int *nshapes = static_cast< int * >(rv);
    if (nshapes == NULL)
    {
        cerr << "Could not determine the number of shapes." << endl;
        exit(EXIT_FAILURE);
    }

    //
    // We know that the Value objects that depend on nshapes have rank 1,
    // so we do not need to bother with creating an array to put the
    // nshapes into.  The nshapes value is not large enough, since ghost
    // zones are calculated when the topology is added in.  Since we don't
    // want to do all of the work, add two and live with the overestimate.
    //
    *nshapes += 2;
    aliasedShapeCountValue->AddDomain(nshapes, domain);
    aliasedShapeTypeValue->AddDomain(nshapes, domain);
    aliasedShapeSizeValue->AddDomain(nshapes, domain);
    
    free(nshapes);
    free(lnodelist);
    free(zones);
}


// ****************************************************************************
//  Method: UnstructuredTopology_Prep::WriteArrays
//
//  Purpose:
//      Has the unstructured topology object give the arrays contained in the
//      DBucdmesh to the Value objects it contains to be written out.
//
//  Programmer: Hank Childs
//  Creation:   January 24, 2000
//
// ****************************************************************************

void
UnstructuredTopology_Prep::WriteArrays(DBfile *dbfile, DBucdmesh *um, 
                                       int domain)
{
    void   *vals;

    //
    // We had to over-estimate the number of shapes before.  Update that now.
    // Because we know the rank of the Value is one, there is no need to 
    // construct an array.  We will just send in the address of the new length.
    //
    aliasedShapeCountValue->UpdateLength(domain, &(um->zones->nshapes));
    aliasedShapeTypeValue->UpdateLength(domain, &(um->zones->nshapes));
    aliasedShapeSizeValue->UpdateLength(domain, &(um->zones->nshapes));

    vals  = static_cast< void * >(um->zones->nodelist);
    aliasedNodeListValue->WriteArray(dbfile, &vals, DB_INT, domain); 

    vals = static_cast< void * >(um->zones->shapecnt);
    aliasedShapeCountValue->WriteArray(dbfile, &vals, DB_INT, domain);

    vals = static_cast< void * >(um->zones->shapetype);
    aliasedShapeTypeValue->WriteArray(dbfile, &vals, DB_INT, domain);

    vals = static_cast< void * >(um->zones->shapesize);
    aliasedShapeSizeValue->WriteArray(dbfile, &vals, DB_INT, domain);
}


