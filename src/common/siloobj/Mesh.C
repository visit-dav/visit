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
//                                 Mesh.C                                    //
// ************************************************************************* //

#include <stdlib.h>
#include <string.h>

#include <Mesh.h>
#include <SiloObjLib.h>


//
// Declaration of static constants.  They are not allowed as initializers in
// class definitions.
//

int    const   Mesh::SILO_NUM_COMPONENTS            = 13;
char * const   Mesh::SILO_TYPE                      = "Mesh";

char * const   Mesh::SILO_COORDINATE_DIMENSION_NAME = "CoordinateDimension";
char * const   Mesh::SILO_COORDINATES_NAME          = "Coordinates";
char * const   Mesh::SILO_DATA_TYPE_NAME            = "DataType";
char * const   Mesh::SILO_INTERVAL_TREE_NAME        = "IntervalTree";
char * const   Mesh::SILO_LABEL_0_NAME              = "Label0";
char * const   Mesh::SILO_LABEL_1_NAME              = "Label1";
char * const   Mesh::SILO_LABEL_2_NAME              = "Label2";
char * const   Mesh::SILO_MESH_TYPE_NAME            = "MeshType";
char * const   Mesh::SILO_N_DOMAINS_NAME            = "NDomains";
char * const   Mesh::SILO_TOPOLOGY_NAME             = "Topology";
char * const   Mesh::SILO_UNIT_0_NAME               = "Unit0";
char * const   Mesh::SILO_UNIT_1_NAME               = "Unit1";
char * const   Mesh::SILO_UNIT_2_NAME               = "Unit2";


// ****************************************************************************
//  Method: Mesh constructor
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
//  Modifications:
//
//      Hank Childs, Sat Mar  4 09:53:12 PST 2000
//      Added meshType.
//
// ****************************************************************************

Mesh::Mesh()
{
    topology             = NULL;
    intervalTree         = NULL;
    coordinateDimension  = -1;
    dataType             = -1;
    nDomains             = -1;
    coordinates          = NULL;
    meshType             = UNKNOWN;
    for (int i = 0 ; i < 3 ; i++)
    {
        labels[i]        = NULL;
        units[i]         = NULL;
    }

    name                 = NULL;
}


// ****************************************************************************
//  Method: Mesh destructor
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

Mesh::~Mesh()
{
    if (topology != NULL)
    {
        delete [] topology;
    }
    if (intervalTree != NULL)
    {
        delete [] intervalTree;
    }
    for (int i = 0 ; i < 3 ; i++)
    {
        if (labels[i] != NULL)
        {
            delete [] labels[i];
        }
        if (units[i] != NULL)
        {
            delete [] units[i];
        }
    }
    if (coordinates != NULL)
    {
        delete [] coordinates;
    }
    if (name != NULL)
    {
        delete [] name;
    }
}


// ****************************************************************************
//  Method:  Mesh::PrintSelf
//
//  Purpose:
//      Prints out a Mesh object.  For debugging purposes only.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
//  Modifications:
//      
//      Hank Childs, Sat Mar  4 09:53:12 PST 2000
//      Added meshType.
//
// ****************************************************************************

void
Mesh::PrintSelf(ostream &out)
{
    out << "Mesh (" << name << ")" << endl;
    
    out << "\tType:                 " << meshType << endl;

    out << "\tTopology:             " ;
    if (topology != NULL)
    {
        out << topology << endl;
    }
    else
    {
        out << "NIL" << endl;
    }

    out << "\tInterval Tree:        " ;
    if (intervalTree != NULL)
    {
        out << intervalTree << endl;
    }
    else
    {
        out << "NIL" << endl;
    }

    out << "\tCoordinate Dimension: " << coordinateDimension << endl;
    out << "\tData Type:            " << dataType << endl;

    out << "\tLabels:               " ;
    for (int i = 0 ; i < 3 ; i++)
    {
        if (labels[i] != NULL)
        {
            out << labels[i];
        }
        else
        {
            out << "NIL";
        }
        out << ", " ;
    }
    out << endl;

    out << "\tUnits:                " ;
    for (int j = 0 ; j < 3 ; j++)
    {
        if (units[j] != NULL)
        {
            out << units[j];
        }
        else
        {
            out << "NIL";
        }
        out << ", " ;
    }
    out << endl;

    out << "\tnDomains:             " << nDomains << endl;

    out << "\tCoordinates:          " ;
    if (coordinates != NULL)
    {
        out << coordinates << endl;
    }
    else
    {
        out << "NIL" << endl;
    }
}


// ****************************************************************************
//  Method: Mesh::Write
//
//  Purpose:
//      Outputs a Mesh objects to a SILO file.
//
//  Note:       The type field for the DBobject, siloObj, must have its memory
//              allocated through a malloc, so that when it is freed, we do
//              not get a free memory mismatch from purify.
//
//  Programmer: Hank Childs
//  Creation:   January 4, 2000
//
//  Modifications:
//  
//      Hank Childs, Sat Mar  4 09:53:12 PST 2000
//      Added code to write out meshType.
//
// ****************************************************************************

void
Mesh::Write(DBfile *dbfile)
{
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
    siloObj->type = C_strdup(SILO_TYPE);

    //
    // Add fields to the Mesh SILO object.
    //
    char   *validEntry;
    char   *nullString = "";
   
    DBAddIntComponent(siloObj, SILO_MESH_TYPE_NAME, meshType);

    validEntry = (topology == NULL ? nullString : topology);
    DBAddStrComponent(siloObj, SILO_TOPOLOGY_NAME, validEntry);
 
    validEntry = (intervalTree == NULL ? nullString : intervalTree);
    DBAddStrComponent(siloObj, SILO_INTERVAL_TREE_NAME, validEntry);

    DBAddIntComponent(siloObj, SILO_COORDINATE_DIMENSION_NAME, 
                      coordinateDimension);

    DBAddIntComponent(siloObj, SILO_DATA_TYPE_NAME, dataType);

    validEntry = (labels[0] == NULL ? nullString : labels[0]);
    DBAddStrComponent(siloObj, SILO_LABEL_0_NAME, validEntry);

    validEntry = (labels[1] == NULL ? nullString : labels[1]);
    DBAddStrComponent(siloObj, SILO_LABEL_1_NAME, validEntry);

    validEntry = (labels[2] == NULL ? nullString : labels[2]);
    DBAddStrComponent(siloObj, SILO_LABEL_2_NAME, validEntry);

    validEntry = (units[0] == NULL ? nullString : units[0]);
    DBAddStrComponent(siloObj, SILO_UNIT_0_NAME, validEntry);

    validEntry = (units[1] == NULL ? nullString : units[1]);
    DBAddStrComponent(siloObj, SILO_UNIT_1_NAME, validEntry);

    validEntry = (units[2] == NULL ? nullString : units[2]);
    DBAddStrComponent(siloObj, SILO_UNIT_2_NAME, validEntry);

    DBAddIntComponent(siloObj, SILO_N_DOMAINS_NAME, nDomains);

    validEntry = (coordinates == NULL ? nullString : coordinates);
    DBAddStrComponent(siloObj, SILO_COORDINATES_NAME, validEntry);

    DBWriteObject(dbfile, siloObj, 0);
    DBFreeObject(siloObj);
}


// ****************************************************************************
//  Method: Mesh::Read
//
//  Purpose:  
//      Reads in a Mesh object from a siloObj.  
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
//  Modifications:
//
//      Hank Childs, Sat Mar  4 09:53:12 PST 2000
//      Added code to read in meshType.
//
// ****************************************************************************

void
Mesh::Read(DBobject *siloObj)
{
    if (strcmp(siloObj->type, SILO_TYPE) != 0)
    {
        // 
        // A mistake was made in trying to create this object.
        //
        cerr << "Cannot create a Mesh object from a " << siloObj->type 
             << " object." << endl;
        exit(EXIT_FAILURE);
    }

    //
    // Copy the name.
    //
    name = CXX_strdup(siloObj->name);

    //
    // Go through each component and populate the Mesh object.
    //
    for (int i = 0 ; i < siloObj->ncomponents ; i++)
    {
        //
        // Rely on SILO obj library routine to get the value from the pdb
        // name once the component has been identified.
        //
        if (strcmp(siloObj->comp_names[i], SILO_COORDINATE_DIMENSION_NAME)==0)
        {
            coordinateDimension = ParsePDBNameInt(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_COORDINATES_NAME) == 0)
        {
            coordinates = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_DATA_TYPE_NAME) == 0)
        {
            dataType = ParsePDBNameInt(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_INTERVAL_TREE_NAME) == 0)
        {
            intervalTree = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_LABEL_0_NAME) == 0)
        {
            labels[0] = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_LABEL_1_NAME) == 0)
        {
            labels[1] = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_LABEL_2_NAME) == 0)
        {
            labels[2] = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_MESH_TYPE_NAME) == 0)
        {
            meshType = (MESH_TYPE_e) ParsePDBNameInt(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_N_DOMAINS_NAME) == 0)
        {
            nDomains = ParsePDBNameInt(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_TOPOLOGY_NAME) == 0)
        {
            topology = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_UNIT_0_NAME) == 0)
        {
            units[0] = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_UNIT_1_NAME) == 0)
        {
            units[1] = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_UNIT_2_NAME) == 0)
        {
            units[2] = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else
        {
            cerr << "Unknown component " << siloObj->comp_names[i] 
                 << " for Mesh object." << endl;
            exit(EXIT_FAILURE);
        }
    }
}


