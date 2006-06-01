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
//                          StructuredTopology.C                             //
// ************************************************************************* //

#include <visitstream.h>
#include <stdlib.h>
#include <string.h>

#include <SiloObjLib.h>
#include <StructuredTopology.h>


//
// Class-scoped constants
//

char * const    StructuredTopology::NAME                    = "Topology";
char * const    StructuredTopology::SILO_NAME               = "Topology";
int    const    StructuredTopology::SILO_NUM_COMPONENTS     = 2;
char * const    StructuredTopology::SILO_TYPE               
                                                    = "StructuredTopology";
char * const    StructuredTopology::SILO_N_DIMENSIONS_NAME = "NDimensions";
char * const    StructuredTopology::SILO_DIMENSIONS_NAME   = "Dimensions";


// ****************************************************************************
//  Method: StructuredTopology constructor
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

StructuredTopology::StructuredTopology()
{
    nDimensions    = -1;
    dimensions     = NULL;

    name           = NULL;
    nDomains       = -1;
}


// ****************************************************************************
//  Method: StructuredTopology destructor
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

StructuredTopology::~StructuredTopology()
{
    if (dimensions != NULL)
    {
        delete [] dimensions;
    }
    if (name != NULL)
    {
        delete [] name;
    }
}


// ****************************************************************************
//  Method: StructuredTopology::Write
//
//  Purpose:
//      Writes out the structured topology object to a silo file.
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
StructuredTopology::Write(DBfile *dbfile)
{
    //
    // Create an object ot be written into the SILO file.
    //
    char   absName[LONG_STRING];
    sprintf(absName, "%s%s", name, SILO_NAME);
    DBobject *siloObj = DBMakeObject(absName, DB_USERDEF, SILO_NUM_COMPONENTS);
    if (siloObj == NULL)
    {
        cerr << "Unable to create a SILO object for the structured topology."
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
    DBAddIntComponent(siloObj, SILO_N_DIMENSIONS_NAME, nDimensions);

    sprintf(absName, "%s%s", name, SILO_DIMENSIONS_NAME);
    int totalSize = nDimensions*nDomains*2;
    DBWrite(dbfile, absName, dimensions, &totalSize, 1, DB_FLOAT);
    DBAddVarComponent(siloObj, SILO_DIMENSIONS_NAME, absName);

    // 
    // Write and free the SILO object.
    //
    DBWriteObject(dbfile, siloObj, 0);
    DBFreeObject(siloObj);
}


// ****************************************************************************
//  Method: StructuredTopology::Read
//
//  Purpose:
//      Reads in a StructuredTopology from a DBobject.
//
//  Programmer: Hank Childs
//  Creation:   January 31, 2000
//
// ****************************************************************************

void
StructuredTopology::Read(DBobject *siloObj, DBfile *dbfile)
{
    if (strcmp(siloObj->type, SILO_TYPE) != 0)
    {
        //
        // A mistake was made in trying to create this object.
        //
        cerr << "Cannot create a StructuredTopology object from a " 
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
    char  *dimensionsName = NULL;
    for (int i = 0 ; i < siloObj->ncomponents ; i++)
    {
        //
        // Rely on SILO obj library routine to get the value from the pdb
        // name once the component has been identified.
        //
        if (strcmp(siloObj->comp_names[i], SILO_N_DIMENSIONS_NAME) == 0)
        {
            nDimensions = ParsePDBNameInt(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_DIMENSIONS_NAME) == 0)
        {
            dimensionsName = siloObj->pdb_names[i];
        }
        else
        {
            cerr << "Invalid component " << siloObj->comp_names[i] << " for " 
                 << "StructuredTopology." << endl;
            exit(EXIT_FAILURE);
        }
    }

    //
    // Read in the dimensions array.
    // We will allocate the space so that purify will not give us any memory
    // mismatch from C-alloc/C++-dealloc.
    //
    if (dimensionsName == NULL)
    {
        cerr << "Never read in the dimensions name, cannot get array." << endl;
        exit(EXIT_FAILURE);
    }
    if (nDimensions <= 0)
    {
        cerr << "Never read in a valid nDimensions, cannot get array." << endl;
        exit(EXIT_FAILURE);
    }
    int  arrayLength = DBGetVarLength(dbfile, dimensionsName);
    nDomains = arrayLength / (nDimensions * 2);
    dimensions = new float[arrayLength];
    DBReadVar(dbfile, dimensionsName, dimensions);
}


// ****************************************************************************
//  Method: StructuredTopology::PrintSelf
//
//  Purpose:
//      Prints the data members of a StructuredTopology object.  For debugging
//      purposes only.
//
//  Programmer: Hank Childs
//  Creation:   January 31, 2000
//
// ****************************************************************************

void
StructuredTopology::PrintSelf(ostream &out)
{
    out << "StructuredTopology (" << name << "):" << endl;
    out << "\tnDimensions = " << nDimensions << endl;
    out << "\tnDomains    = " << nDomains << endl;
    for (int i = 0 ; i < nDomains ; i++)
    {
        cerr << "\t(" ;
        for (int j = 0 ; j < nDimensions ; j++)
        {
            cerr << "(" << dimensions[i*nDimensions*2 + 2*j] << ", "
                 << dimensions[i*nDimensions*2 + 2*j + 1] << ")";
            if (j != nDimensions - 1)
            {
                cerr << ", ";
            }
        }
        cerr << ")" << endl;
    }
}


