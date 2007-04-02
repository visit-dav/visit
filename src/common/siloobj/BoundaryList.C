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
//                            BoundaryList.C                                 //
// ************************************************************************* //

#include <stdlib.h>
#include <string.h>

#include "BoundaryList.h"


//
// Declaration of static constants.  They are not allowed as initializers in
// class definitions.
//

char * const   BoundaryList::SILO_LOCATION                = "/";
int    const   BoundaryList::SILO_NUM_COMPONENTS          = 9;
char * const   BoundaryList::SILO_TYPE                    = "BoundaryList";
char * const   BoundaryList::SILO_N_MATERIAL_NUMBERS_NAME = "NMaterialNumbers";
char * const   BoundaryList::SILO_MATERIAL_NUMBERS_NAME   = "MaterialNumbers";
char * const   BoundaryList::SILO_N_SHAPES_NAME           = "NShapes";
char * const   BoundaryList::SILO_SHAPE_CNT_NAME          = "ShapeCnt";
char * const   BoundaryList::SILO_SHAPE_SIZE_NAME         = "ShapeSize";
char * const   BoundaryList::SILO_N_OPPOSITE_MATERIAL_NAME  
                                          = "NOppositeMaterial";
char * const   BoundaryList::SILO_OPPOSITE_MATERIAL_CNT_NAME 
                                          = "OppositeMaterialCnt";
char * const   BoundaryList::SILO_OPPOSITE_MATERIAL_NUMBER_NAME  
                                          = "OppositeMaterialNumber";
char * const   BoundaryList::SILO_NODE_LIST_NAME          = "NodeList";


// ****************************************************************************
//  Method: BoundaryList constructor
//
//  Programmer: Hank Childs
//  Creation:   January 7, 2000
//
// ****************************************************************************

BoundaryList::BoundaryList()
{
    name                    = NULL;
    nMaterialNumbers        = 0;
    materialNumbers         = NULL;
    nShapes                 = NULL;
    totalShapes             = 0;
    shapeCnt                = NULL;
    shapeSize               = NULL;
    nOppositeMaterial       = NULL;
    totalOpposite           = 0;
    oppositeMaterialCnt     = NULL;
    oppositeMaterialNumber  = NULL;
    nodeList                = NULL;
}


// ****************************************************************************
//  Method: BoundaryList destructor
//
//  Programmer: Hank Childs
//  Creation:   January 7, 2000
//
// ****************************************************************************

BoundaryList::~BoundaryList()
{
    if (name != NULL)
    {
        delete [] name;
    }
    if (materialNumbers != NULL)
    {
        delete [] materialNumbers;
    }
    if (nShapes != NULL)
    {
        delete [] nShapes;
    }
    if (shapeCnt != NULL)
    {
        delete [] shapeCnt;
    }
    if (shapeSize != NULL)
    {
        delete [] shapeSize;
    }
    if (nOppositeMaterial != NULL)
    {
        delete [] nOppositeMaterial;
    }
    if (oppositeMaterialCnt != NULL)
    {
        delete [] oppositeMaterialCnt;
    }
    if (oppositeMaterialNumber != NULL)
    {
        delete [] oppositeMaterialNumber;
    }
    if (nodeList != NULL)
    {
        delete [] nodeList;
    }
}


// ****************************************************************************
//  Method: BoundaryList::SetName
//
//  Purpose:
//      Sets the BoundaryList object's name.
//
//  Programmer: Hank Childs
//  Creation:   January 7, 2000
//
// ****************************************************************************

void
BoundaryList::SetName(char *n)
{
    name = new char[strlen(n)+1];
    strcpy(name, n);
}


// ****************************************************************************
//  Method: BoundaryList::AddMaterial
//
//  Purpose:
//      Populates a BoundaryList object from a material and an OnionPeel.
//
//  Arguments:
//      mat        -  The material to create the BoundaryList from.
//      onionPeel  -  The object that contains the connectivity information.
//      um         -  The unstructured mesh that corresponds to this material.
//
//  Note:       BoundaryList is a friend to OnionPeel.
//
//  Programmer: Hank Childs
//  Creation:   January 7, 2000
//
// ****************************************************************************

void
BoundaryList::AddMaterial(DBmaterial *mat, OnionPeel *onionPeel, 
                          DBzonelist *zl)
{
    int   i;

    nMaterialNumbers = mat->nmat;
    materialNumbers  = new int[nMaterialNumbers];
    nShapes          = new int[nMaterialNumbers];
    for (i = 0 ; i < nMaterialNumbers ; i++)
    {
        materialNumbers[i] = mat->matnos[i];
        nShapes[i]         = 0;
    }

    // 
    // Every material may not have every shape, but this makes calculations
    // a lot easier and doesn't take up too much memory.
    //
    for (i = 0 ; i < zl->nzones ; i++)
    {

    }
}


// ****************************************************************************
//  Method: BoundaryList::Write
//
//  Purpose:
//      Outputs a BoundaryList objects to a SILO file.
//
//  Note:       This object is domain based and can be written by any 
//              processor.
//
//  Note:       The type field for the DBobject, siloObj, must have its memory
//              allocated through a malloc, so that when it is freed, we do
//              not get a free memory mismatch from purify.
//
//  Programmer: Hank Childs
//  Creation:   January 7, 2000
//
// ****************************************************************************

void
BoundaryList::Write(DBfile *dbfile)
{
    //
    // Set the directory in the file to be correct.
    //
    if (DBSetDir(dbfile, SILO_LOCATION) < 0)
    {
        cerr << "Unable to change directories in the silo file to "
             << SILO_LOCATION << "." << endl;
        exit(EXIT_FAILURE);
    }

    //
    // Create an object to be written into the SILO file.
    //
    DBobject  *siloObj = DBMakeObject(name, DB_USERDEF, SILO_NUM_COMPONENTS);
    if (siloObj == NULL)
    {
        cerr << "Unable to create a silo object for the mesh." << endl;
        exit(EXIT_FAILURE);
    }
    
    //
    // We must make a copy of the string, because the "destructor" deletes this
    // field and we can't have it delete statics.
    //
    siloObj->type = static_cast< char * >
                      (calloc(strlen(SILO_TYPE)+1, sizeof(char)));
    strcpy(siloObj->type, SILO_TYPE);

    DBWriteObject(dbfile, siloObj, 0);
    DBFreeObject(siloObj);
}


