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
//                             TimeSequence.C                                //
// ************************************************************************* //

#include <visitstream.h> 
#include <silo.h>
#include <stdlib.h>
#include <string.h>

#include <SiloObjLib.h>
#include <TimeSequence.h>


// 
// Initialization of static consts.  They are not allowed as initializers in
// class definitions.
//

char * const TimeSequence::SILO_TYPE                        = "TimeSequence";
char * const TimeSequence::SILO_OBJ_NAME                    = "time";
char * const TimeSequence::SILO_N_STATES_NAME               = "NStates";
char * const TimeSequence::SILO_TIMES_NAME                  = "Times";
char * const TimeSequence::SILO_CYCLES_NAME                 = "Cycles";
char * const TimeSequence::SILO_STATES_NAME                 = "States";
char * const TimeSequence::SILO_N_FIELDS_NAME               = "NFields";
char * const TimeSequence::SILO_FIELD_NAMES_NAME            = "FieldNames";
char * const TimeSequence::SILO_FIELD_TYPES_NAME            = "FieldTypes";
char * const TimeSequence::SILO_FIELD_INTERVAL_TREE_NAME    = "IntervalTrees";
char * const TimeSequence::SILO_FIELD_DIMENSION_NAME        = "FieldDimension";
int    const TimeSequence::SILO_NUM_COMPONENTS              = 9;


// ****************************************************************************
//  Method: TimeSequence constructor
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

TimeSequence::TimeSequence()
{
    nStates            = -1;
    times              = NULL;
    cycles             = NULL;
    states             = NULL;
    
    nFields            = -1;
    fieldNames         = NULL;
    fieldTypes         = NULL;
    fieldDimension     = NULL;
    fieldIntervalTree  = NULL;
}


// ****************************************************************************
//  Method: TimeSequence destructor
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

TimeSequence::~TimeSequence()
{
    if (times != NULL)
    {
        delete [] times;
    }
    if (cycles != NULL)
    {
        delete [] cycles;
    }
    if (states != NULL)
    {
        for (int i = 0 ; i < nStates ; i++)
        {
            if (states[i] != NULL)
            {
                delete [] states[i];
            }
        }
        delete [] states;
    }
    if (fieldNames != NULL)
    {
        for (int i = 0 ; i < nFields ; i++)
        {
            if (fieldNames[i] != NULL)
            {
                delete [] fieldNames[i];
            }
        }
        delete [] fieldNames;
    }
    if (fieldTypes != NULL)
    {
        delete [] fieldTypes;
    }
    if (fieldDimension != NULL)
    {
        delete [] fieldDimension;
    }
    if (fieldIntervalTree != NULL)
    {
        for (int i = 0 ; i < nFields ; i++)
        {
            if (fieldIntervalTree[i] != NULL)
            {
                delete [] fieldIntervalTree[i];
            }
        }
        delete [] fieldIntervalTree;
    }
}


// ****************************************************************************
//  Method: TimeSequence::Write
//
//  Purpose:
//      Writes the time sequence as a SILO object to the file specified.
//
//  Arguments:
//      db_file  -  This is a pointer to the `visit' DBfile.
//
//  Note:       The type field for the DBobject, siloObj, must have its memory
//              allocated through a malloc, so that when it is freed, we do
//              not get a free memory mismatch from purify.
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

void
TimeSequence::Write(DBfile *dbfile)
{
    int    amount;
    char  *condensed;
    char   absoluteName[LONG_STRING];

    //
    // Create an object to be written into the SILO file.
    //
    DBobject  *siloObj = DBMakeObject(SILO_OBJ_NAME, DB_USERDEF, 
                                      SILO_NUM_COMPONENTS);
    if (siloObj == NULL)
    {
        cerr << "Unable to create a silo object for the time sequence." 
             << endl;
        exit(EXIT_FAILURE);
    }

    //
    // We must make a copy of the string, because the "destructor" deletes this
    // field and we can't have it delete statics.
    //
    siloObj->type = C_strdup(SILO_TYPE);

    //
    // Add strings and numbers directly to the object.  Write the rest of
    // the components to the file and then add a var component to the siloObj,
    // which will add a link to it.
    //
    DBAddIntComponent(siloObj, SILO_N_STATES_NAME, nStates);
    DBAddIntComponent(siloObj, SILO_N_FIELDS_NAME, nFields);

    sprintf(absoluteName, "%s%s", SILO_OBJ_NAME, SILO_TIMES_NAME);
    DBWrite(dbfile, absoluteName, times, &nStates, 1, DB_DOUBLE);
    DBAddVarComponent(siloObj, SILO_TIMES_NAME, absoluteName);

    sprintf(absoluteName, "%s%s", SILO_OBJ_NAME, SILO_CYCLES_NAME);
    DBWrite(dbfile, absoluteName, cycles, &nStates, 1, DB_INT);
    DBAddVarComponent(siloObj, SILO_CYCLES_NAME, absoluteName);

    sprintf(absoluteName, "%s%s", SILO_OBJ_NAME, SILO_STATES_NAME);
    condensed = CondenseStringArray(states, nStates, &amount);
    DBWrite(dbfile, absoluteName, condensed, &amount, 1, DB_CHAR);
    delete [] condensed;
    DBAddVarComponent(siloObj, SILO_STATES_NAME, absoluteName);

    sprintf(absoluteName, "%s%s", SILO_OBJ_NAME, SILO_FIELD_NAMES_NAME);
    condensed = CondenseStringArray(fieldNames, nFields, &amount);
    DBWrite(dbfile, absoluteName, condensed, &amount, 1, DB_CHAR);
    delete [] condensed;
    DBAddVarComponent(siloObj, SILO_FIELD_NAMES_NAME, absoluteName);


    sprintf(absoluteName, "%s%s", SILO_OBJ_NAME, SILO_FIELD_TYPES_NAME);
    DBWrite(dbfile, absoluteName, fieldTypes, &nFields, 1, DB_INT);
    DBAddVarComponent(siloObj, SILO_FIELD_TYPES_NAME, absoluteName);

    sprintf(absoluteName, "%s%s", SILO_OBJ_NAME, SILO_FIELD_DIMENSION_NAME);
    DBWrite(dbfile, absoluteName, fieldDimension, &nFields, 1, DB_INT);
    DBAddVarComponent(siloObj, SILO_FIELD_DIMENSION_NAME, absoluteName);
    
    sprintf(absoluteName, "%s%s", SILO_OBJ_NAME,SILO_FIELD_INTERVAL_TREE_NAME);
    condensed = CondenseStringArray(fieldIntervalTree, nFields, &amount);
    DBWrite(dbfile, absoluteName, condensed, &amount, 1, DB_CHAR);
    delete [] condensed;
    DBAddVarComponent(siloObj, SILO_FIELD_INTERVAL_TREE_NAME, absoluteName);

    DBWriteObject(dbfile, siloObj, 0);
    DBFreeObject(siloObj);
}


// ****************************************************************************
//  Method: TimeSequence::PrintSelf
//
//  Purpose:
//      Prints a TimeSequence object.  For debugging purposes only.
//
//  Programmer: Hank Childs
//  Creation:   December 3, 1999
//
// ****************************************************************************

void
TimeSequence::PrintSelf(ostream &out)
{
    out << "Time Sequence: " << endl;
    out << "\tnStates = " << nStates << "\n";

    out << "\tcycles = {";
    if (cycles == NULL)
    {
       out << "nil";
    }
    else
    {
        for (int i = 0 ; i < nStates ; i++)
            out << cycles[i] << ", ";
    }
    out << "}\n";

    out << "\ttimes = {";
    if (times == NULL)
    {
        out << "nil";
    }
    else
    {
       for (int i = 0 ; i < nStates ; i++)
           out << times[i] << ", ";
    }
    out << "}\n";
    
    out << "\tstates = {";
    if (states == NULL)
    {
        out << "nil" ;
    }
    else
    {
        out << endl;
        for (int i = 0 ; i < nStates ; i++)
            out << "\t\t" << states[i] << endl;
        out << "\t";
    }
    out << "}\n";

    out << "\tnFields = " << nFields << endl;
    
    out << "\tfieldNames = {" ;
    if (fieldNames == NULL)
    {
        out << "nil";
    }
    else
    {
        out << endl;
        for (int i = 0 ; i < nFields ; i++)
            out << "\t\t" << fieldNames[i] << endl;
        out << "\t";
    }
    out << "}\n";
   
    out << "\tfieldTypes = {" ;
    if (fieldTypes == NULL)
    {
        out << "nil";
    }
    else
    {
        for (int i = 0 ; i < nFields ; i++)
            out << fieldTypes[i] << ", ";
    }
    out << "}\n";

    out << "\tfieldDimension = {" ;
    if (fieldDimension == NULL)
    {
        out << "nil";
    }
    else
    {
        for (int i = 0 ; i < nFields ; i++)
            out << fieldDimension[i] << ", ";
    }
    out << "}\n";

    out << "\tfieldIntervalTree = {" ;
    if (fieldIntervalTree == NULL)
    {
        out << "nil";
    }
    else
    {
        out << endl;
        for (int i = 0 ; i < nFields ; i++)
            out << "\t\t" << fieldIntervalTree[i] << endl;
        out << "\t";
    }
    out << "}\n";
}


// ****************************************************************************
//  Method: TimeSequence::Read
//
//  Purpose:
//      Reads in a TimeSequence object from a DBobject.
//  
//  Programmer: Hank Childs
//  Creation:   January 31, 2000
//
// ****************************************************************************

void
TimeSequence::Read(DBobject *siloObj, DBfile *dbfile)
{
    if (strcmp(siloObj->type, SILO_TYPE) != 0)
    {
        //
        // A mistake was made in trying to create this object.
        //
        cerr << "Cannot create a TimeSequence object from a " << siloObj->type
             << " object." << endl;
        exit(EXIT_FAILURE);
    }

    // 
    // Populate the object by going through the components.
    //
    char  *timesName             = NULL;
    char  *cyclesName            = NULL;
    char  *statesName            = NULL;
    char  *fieldNamesName        = NULL;
    char  *fieldTypesName        = NULL;
    char  *fieldDimensionName    = NULL; 
    char  *fieldIntervalTreeName = NULL;
    for (int i = 0 ; i < siloObj->ncomponents ; i++)
    {
        //
        // Rely on SILO obj library routines to get the value from the pdb
        // name once the component has been identified.
        //
        if (strcmp(siloObj->comp_names[i], SILO_N_STATES_NAME) == 0)
        {
            nStates = ParsePDBNameInt(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_TIMES_NAME) == 0)
        {
            timesName = siloObj->pdb_names[i];
        }
        else if (strcmp(siloObj->comp_names[i], SILO_CYCLES_NAME) == 0)
        {
            cyclesName = siloObj->pdb_names[i];
        }
        else if (strcmp(siloObj->comp_names[i], SILO_STATES_NAME) == 0)
        {
            statesName = siloObj->pdb_names[i];
        }
        else if (strcmp(siloObj->comp_names[i], SILO_N_FIELDS_NAME) == 0)
        {
            nFields = ParsePDBNameInt(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_FIELD_NAMES_NAME) == 0)
        {
            fieldNamesName = siloObj->pdb_names[i];
        }
        else if (strcmp(siloObj->comp_names[i], SILO_FIELD_TYPES_NAME) == 0)
        {
            fieldTypesName = siloObj->pdb_names[i];
        }
        else if (strcmp(siloObj->comp_names[i],SILO_FIELD_DIMENSION_NAME) == 0)
        {
            fieldDimensionName = siloObj->pdb_names[i];
        }
        else if (strcmp(siloObj->comp_names[i], SILO_FIELD_INTERVAL_TREE_NAME)
                 == 0)
        {
            fieldIntervalTreeName = siloObj->pdb_names[i];
        }
        else
        {
            cerr << "Invalid component " << siloObj->comp_names[i] 
                 << " for a TimeSequence." << endl;
            exit(EXIT_FAILURE);
        }
    }   // End 'for' over all the components in the DBobject.

    //
    // Read in all of the associated arrays.  Allocate our own memory so that
    // purify will not us any memory mismatches from C-alloc/C++-dealloc.
    //
    char  *condensedString;
    void  *rv;

    if (nStates <= 0 || nFields <= 0)
    {
        cerr << "Did not read in necessary int fields." << endl;
        exit(EXIT_FAILURE);
    }
    
    if (timesName == NULL)
    {
        cerr << "Never read in times name, cannot get array." << endl;
        exit(EXIT_FAILURE);
    }
    times = new double[nStates];
    DBReadVar(dbfile, timesName, times);

    if (cyclesName == NULL)
    {
        cerr << "Never read in cycles name, cannot get array." << endl;
        exit(EXIT_FAILURE);
    }
    cycles = new int[nStates];
    DBReadVar(dbfile, cyclesName, cycles);

    if (statesName == NULL)
    {
        cerr << "Never read in states name, cannot get array." << endl;
        exit(EXIT_FAILURE);
    }
    rv = DBGetVar(dbfile, statesName);
    condensedString = static_cast< char * >(rv);
    states = new char*[nStates];
    UncondenseStringArray(states, nStates, condensedString);

    if (fieldNamesName == NULL)
    {
        cerr << "Never read in field names, cannot get array." << endl;
        exit(EXIT_FAILURE);
    }
    rv = DBGetVar(dbfile, fieldNamesName);
    condensedString = static_cast< char * >(rv);
    fieldNames = new char*[nFields];
    UncondenseStringArray(fieldNames, nFields, condensedString);

    if (fieldTypesName == NULL)
    {
        cerr << "Never read in field types name, cannot get array." << endl;
        exit(EXIT_FAILURE);
    }
    fieldTypes = new int[nFields];
    DBReadVar(dbfile, fieldTypesName, fieldTypes);

    if (fieldDimensionName == NULL)
    {
        cerr << "Never read in field dim name, cannot get array." << endl;
        exit(EXIT_FAILURE);
    }
    fieldDimension = new int[nFields];
    DBReadVar(dbfile, fieldDimensionName, fieldDimension);

    if (fieldIntervalTreeName == NULL)
    {
        cerr << "Never read in field interval tree, cannot get array." << endl;
        exit(EXIT_FAILURE);
    }
    rv = DBGetVar(dbfile, fieldIntervalTreeName);
    condensedString = static_cast< char * >(rv);
    fieldIntervalTree = new char*[nFields];
    UncondenseStringArray(fieldIntervalTree, nFields, condensedString);
}


