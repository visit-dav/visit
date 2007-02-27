/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                         UnstructuredTopology.C                            //
// ************************************************************************* //

#include <stdlib.h>
#include <string.h>

#include <UnstructuredTopology.h>


//
// Initialization of class-scoped constants (statics) is not allowed in the
// class definition, so do that here.
//

char * const  UnstructuredTopology::NAME                  = "Topology";
char * const  UnstructuredTopology::SILO_NAME             = "Topology";
int    const  UnstructuredTopology::SILO_NUM_COMPONENTS   = 4;
char * const  UnstructuredTopology::SILO_TYPE   = "UnstructuredTopology";
char * const  UnstructuredTopology::SILO_NODE_LIST_NAME   = "NodeList";
char * const  UnstructuredTopology::SILO_SHAPE_COUNT_NAME = "ShapeCount";
char * const  UnstructuredTopology::SILO_SHAPE_TYPE_NAME  = "ShapeType";
char * const  UnstructuredTopology::SILO_SHAPE_SIZE_NAME  = "ShapeSize";


// ****************************************************************************
//  Method: UnstructuredTopology constructor
//
//  Purpose:
//      Creates a normally constructed UnstructuredTopology object.
//
//  Note:       See the discussion for the other UnstructuredTopology 
//              constructor for why two constructors are necessary.
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

UnstructuredTopology::UnstructuredTopology()
{
    RealConstructor();
    CreateValues();
}


// ****************************************************************************
//  Method: UnstructuredTopology constructor
//
//      Creates a UnstructuredTopology object that is incomplete, because the 
//      derived types will fill in the Value objects.  We would like the Value 
//      objects to be fully created at the time of instantiation, so subsequent
//      method invocations can be in confidence that the object is fully 
//      constructed.  Unfortunately, this cannot be accomplished with virtual 
//      functions since the base type (this one) cannot resolve whether or not 
//      it is the base type, except through passing an explicit argument, like 
//      the one in this constructor's signature.  Virtual functions do not work
//      in constructors because they may access data members that do not exist 
//      yet.  The virtual function table when this method is called for this 
//      object is that of the UnstructuredTopology class.  If this is a derived
//      type of Field, the VFT will not be replaced with that of the derived 
//      type until the constructor for that derived type is called.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
// ****************************************************************************

UnstructuredTopology::UnstructuredTopology(FOR_DERIVED_TYPES_e)
{
    RealConstructor();

    //
    // Not calling CreatesValues so the derived type's constructor can.
    //
}


// ****************************************************************************
//  Method: UnstructuredTopology::RealConstructor
//
//  Purpose:
//      Because the UnstructuredTopology object needs to be created by derived
//      types, there must be multiple (and similar) constructors.  This is the
//      "real" constructor that they all can call.
//
//  Programmer: Hank Childs
//  Creation:   January 28, 2000
//
// ****************************************************************************

void
UnstructuredTopology::RealConstructor(void)
{
    nodeList         = NULL;
    shapeCount       = NULL;
    shapeType        = NULL;
    shapeSize        = NULL;
 
    name             = NULL;

    nodeListValue    = NULL;
    shapeCountValue  = NULL;
    shapeTypeValue   = NULL;
    shapeSizeValue   = NULL;
}


// ****************************************************************************
//  Method: UnstructuredTopology::CreateValues
//
//  Purpose:
//      Creates the Value objects for the UnstructuredTopology.
//
//  Note:       The necessity of this method is discussed in the constructor.
//
//  Programmer: Hank Childs
//  Creation:   January 28, 2000
//
// ****************************************************************************

void
UnstructuredTopology::CreateValues(void)
{
    nodeListValue    = new Value;
    shapeCountValue  = new Value;
    shapeTypeValue   = new Value;
    shapeSizeValue   = new Value;
}


// ****************************************************************************
//  Method: UnstructuredTopology destructor
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

UnstructuredTopology::~UnstructuredTopology()
{
    if (nodeList != NULL)
    {
        delete [] nodeList;
    }
    if (shapeCount != NULL)
    {
        delete [] shapeCount;
    }
    if (shapeType != NULL)
    {
        delete [] shapeType;
    }
    if (shapeSize != NULL)
    {
        delete [] shapeSize;
    }
    if (name != NULL)
    {
        delete [] name;
    }
    if (nodeListValue != NULL)
    {
        delete nodeListValue;
    }
    if (shapeCountValue != NULL)
    {
        delete shapeCountValue;
    }
    if (shapeTypeValue != NULL)
    {
        delete shapeTypeValue;
    }
    if (shapeSizeValue != NULL)
    {
        delete shapeSizeValue;
    }
}


// ****************************************************************************
//  Method: UnstructuredTopology::Write
//
//  Purpose:
//      Writes out the unstructured topology object to a silo file.
//
//  Note:       The type field for the DBobject, siloObj, must have its memory
//              allocated through a malloc, so that when it is freed, we do
//              not get a free memory mismatch from purify.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2000
//
// ****************************************************************************

void
UnstructuredTopology::Write(DBfile *dbfile)
{
    nodeListValue->Write(dbfile);
    shapeCountValue->Write(dbfile);
    shapeTypeValue->Write(dbfile);
    shapeSizeValue->Write(dbfile);

    //
    // Create an object to be written into the SILO file.
    //
    char   absName[LONG_STRING];
    sprintf(absName, "%s%s", name, SILO_NAME);
    DBobject *siloObj = DBMakeObject(absName, DB_USERDEF, SILO_NUM_COMPONENTS);
    if (siloObj == NULL)
    {
        cerr << "Unable to create a SILO object for the unstructured topology."
             << endl;
        exit(EXIT_FAILURE);
    }
 
    //
    // We must make a copy of the string, because the "destructor" deletes
    // the field and we can't have it delete statics.
    //
    siloObj->type = C_strdup(SILO_TYPE);
  
    //
    // Add data members to the siloObj.
    //
    char   *validEntry = NULL;
    char   *nullString = "";

    validEntry = (nodeList == NULL ? nullString : nodeList);
    DBAddStrComponent(siloObj, SILO_NODE_LIST_NAME, validEntry);

    validEntry = (shapeCount == NULL ? nullString : shapeCount);
    DBAddStrComponent(siloObj, SILO_SHAPE_COUNT_NAME, validEntry);
  
    validEntry = (shapeType == NULL ? nullString : shapeType);
    DBAddStrComponent(siloObj, SILO_SHAPE_TYPE_NAME, validEntry);

    validEntry = (shapeSize == NULL ? nullString : shapeSize);
    DBAddStrComponent(siloObj, SILO_SHAPE_SIZE_NAME, validEntry);

    DBWriteObject(dbfile, siloObj, 0);
    DBFreeObject(siloObj);
}


// ****************************************************************************
//  Method: UnstructuredTopology::Read
//
//  Purpose:
//      Reads in an UnstructuredTopology object from a DBobject.
//
//  Note:       The second argument, DBfile *, is unused.
//
//  Programmer: Hank Childs
//  Creation:   January 31, 2000
//
// ****************************************************************************

void 
UnstructuredTopology::Read(DBobject *siloObj, DBfile *)
{
    if (strcmp(siloObj->type, SILO_TYPE) != 0)
    {
        //
        // A mistake was made in trying to create this object.
        //
        cerr << "Cannot create an UnstructuredTopology object for a "
             << siloObj->type << " object." << endl;
        exit(EXIT_FAILURE);
    }
  
    //
    // Copy the name.
    //
    name = CXX_strdup(siloObj->name);
  
    //
    // Populate the object by going through the components.
    //
    for (int i = 0 ; i < siloObj->ncomponents ; i++)
    {
        //
        // Rely on SILO obj library routines to get the value from the PDB
        // name once the component has been identified.
        //
        if (strcmp(siloObj->comp_names[i], SILO_NODE_LIST_NAME) == 0)
        {
            nodeList = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_SHAPE_COUNT_NAME) == 0)
        {
            shapeCount = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_SHAPE_TYPE_NAME) == 0)
        {
            shapeType = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_SHAPE_SIZE_NAME) == 0)
        {
            shapeSize = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else
        {
            cerr << "Invalid component " << siloObj->comp_names[i]
                 << " for UnstructuredTopology." << endl;
            exit(EXIT_FAILURE);
        }
    }   // End 'for' over all components in DBobject.
}


// ****************************************************************************
//  Method: UnstructuredTopology::PrintSelf
//
//  Purpose:
//      Prints out the unstructured topology object.  For debugging purposes.
//
//  Programmer: Hank Childs
//  Creation:   January 31, 2000
//
// ****************************************************************************

void
UnstructuredTopology::PrintSelf(ostream &out)
{
    out << "UnstructuredTopology (" << name << "): " << endl;
    out << "\tnode list     = " << (nodeList==NULL?"NIL":nodeList) << endl;
    out << "\tshape count   = " << (shapeCount==NULL?"NIL":shapeCount) << endl;
    out << "\tshape type    = " << (shapeType==NULL?"NIL":shapeType) << endl;
    out << "\tshape size    = " << (shapeSize==NULL?"NIL":shapeSize) << endl;
}


