// ************************************************************************* //
//                                 Value.C                                   //
// ************************************************************************* //

#include <visitstream.h>
#include <stdlib.h>
#include <string.h>

#include <SiloObjLib.h>
#include <Value.h>


//
// Initialization of static consts.  They are not allowed as initializers in
// class definitions.
//

char * const Value::ARRAY_STRING           = "Data";
int    const Value::SILO_NUM_COMPONENTS    = 5;
char * const Value::SILO_OBJ_NAME          = "Value";
char * const Value::SILO_TYPE              = "Value";

char * const Value::SILO_N_DOMAINS_NAME    = "NDomains";
char * const Value::SILO_N_VALS_NAME       = "NVals";
char * const Value::SILO_LENGTHS_NAME      = "Lengths";
char * const Value::SILO_ENTRY_NAMES_NAME  = "Names";
char * const Value::SILO_OFFSETS_NAME      = "Offsets";

char * const Value::NAME                   = Value::SILO_OBJ_NAME;


// ****************************************************************************
//  Method: Value constructor
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

Value::Value()
{
    nDomains        = -1;
    nVals           = -1;
    totalEntries    = 0;
    entryNames      = NULL;
    offsets         = NULL;
    lengths         = NULL;
    name            = NULL;
}


// ****************************************************************************
//  Method: Value destructor
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

Value::~Value()
{
    if (entryNames != NULL)
    {
        for (int i = 0 ; i < nDomains ; i++)
        {
            if (entryNames[i] != NULL)
            {
                delete [] entryNames[i];
            }
        }
        delete [] entryNames;
    }
    if (offsets != NULL)
    {
        delete [] offsets;
    }
    if (lengths != NULL)
    {
        delete [] lengths;
    }
    if (name != NULL)
    {
        delete [] name;
    }
}


// ****************************************************************************
//  Method:  Value::Write
//
//  Purpose:
//      Writes the Value object to a SILO file.  
//
//  Note:       The type field for the DBobject, siloObj, must have its memory
//              allocated through a malloc, so that when it is freed, we do
//              not get a free memory mismatch from purify.
//
//  Programmer: Hank Childs
//  Creation:   December 13, 1999
//
// ****************************************************************************

void
Value::Write(DBfile *dbfile)
{
    char   absName[LONG_STRING];
    int    amount;
    char  *condensed;

    //
    // Create an object to be written into the SILO file.
    //
    DBobject *siloObj = DBMakeObject(name, DB_USERDEF, SILO_NUM_COMPONENTS);
    if (siloObj == NULL)
    {
        cerr << "Was not able to create a SILO object for Value." << endl;
        exit(EXIT_FAILURE);
    }

    //
    // We must make a copy of the string, because the "destructor" deletes this
    // field and we can't have it delete statics.
    //
    siloObj->type = C_strdup(SILO_TYPE);

    //
    // Create a NULL character delimited list of strings and write that to the
    // file.  Then create a component in the DBobject linking it in.
    //
    condensed = CondenseStringArray(entryNames, nDomains, &amount);
    sprintf(absName, "%s%s", name, SILO_ENTRY_NAMES_NAME);
    DBWrite(dbfile, absName, condensed, &amount, 1, DB_CHAR);
    DBAddVarComponent(siloObj, SILO_ENTRY_NAMES_NAME, absName);
    delete [] condensed;
    condensed = NULL;
  
    sprintf(absName, "%s%s", name, SILO_OFFSETS_NAME);
    DBWrite(dbfile, absName, offsets, &totalEntries, 1, DB_INT);
    DBAddVarComponent(siloObj, SILO_OFFSETS_NAME, absName);
    
    sprintf(absName, "%s%s", name, SILO_LENGTHS_NAME);
    DBWrite(dbfile, absName, lengths, &totalEntries, 1, DB_INT);
    DBAddVarComponent(siloObj, SILO_LENGTHS_NAME, absName);

    DBAddIntComponent(siloObj, SILO_N_DOMAINS_NAME, nDomains);
    DBAddIntComponent(siloObj, SILO_N_VALS_NAME, nVals);

    DBWriteObject(dbfile, siloObj, 0);
    DBFreeObject(siloObj);
}


// ****************************************************************************
//  Method: Value::PrintSelf
//
//  Purpose:
//      Prints out a Value object.  (Intended for debugging only)
//
//  Programmer: Hank Childs
//  Creation:   December 13, 1999
//
// ****************************************************************************

void
Value::PrintSelf(ostream &out)
{
    int  i;

    out << "Value: " << name << endl;
    for (i = 0 ; i < nDomains ; i++)
    {
        out << "\tDomain = " << i << "\tname = " << entryNames[i] << endl;
    }
    for (i = 0 ; i < totalEntries ; i++)
    {
        out << "\toffset = " << offsets[i] << "\tlength = " 
            << lengths[i] << endl;
    }
}


// ****************************************************************************
//  Method: Value::Read
//
//  Purpose:
//      Populates the Value object from the DBobject.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
// ****************************************************************************

void
Value::Read(DBobject *siloObj, DBfile *dbfile)
{
    if (strcmp(siloObj->type, SILO_TYPE) != 0)
    {
        //
        // A mistake was made in trying to create this object.
        //
        cerr << "Cannot create a Value object from a " << siloObj->type
             << " object." << endl;
        exit(EXIT_FAILURE);
    }
   
    //
    // Copy the name.
    //
    name = CXX_strdup(siloObj->name);

    //
    // Populate the object by going through the components.
    //
    char  *entryNamesName   = NULL;
    char  *offsetsName      = NULL;
    char  *lengthsName      = NULL;
    for (int i = 0 ; i < siloObj->ncomponents ; i++)
    {
        //
        // Rely on SILO obj library routines to get the value from the pdb
        // name once the component has been identified.
        //
        if (strcmp(siloObj->comp_names[i], SILO_N_DOMAINS_NAME) == 0)
        {
            nDomains = ParsePDBNameInt(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_N_VALS_NAME) == 0)
        {
            nVals = ParsePDBNameInt(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_ENTRY_NAMES_NAME) == 0)
        {
            entryNamesName = siloObj->pdb_names[i];
        }
        else if (strcmp(siloObj->comp_names[i], SILO_OFFSETS_NAME) == 0)
        {
            offsetsName = siloObj->pdb_names[i];
        }
        else if (strcmp(siloObj->comp_names[i], SILO_LENGTHS_NAME) == 0)
        {
            lengthsName = siloObj->pdb_names[i];
        }
    }   // End 'for' over components in DBobject

    if (nDomains <= 0 || nVals <= 0)
    {
        cerr << "Did not read in nDomains or nVals, cannot calculate total "
             << "entries." << endl;
        exit(EXIT_FAILURE);
    }
    totalEntries = nDomains*nVals;
   
    // 
    // Read in the entry names.
    //
    if (entryNamesName == NULL)
    {
        cerr << "Could not establish the name of the entry names." << endl;
        exit(EXIT_FAILURE);
    }
    void  *rv = DBGetVar(dbfile, entryNamesName);
    char  *condensedString = static_cast< char * >(rv);
    entryNames = new char*[nDomains];
    UncondenseStringArray(entryNames, nDomains, condensedString);

    //
    // Use C-construct to free memory allocated from SILO.
    //
    free(rv);

    //
    // Read in the lengths.
    //
    if (lengthsName == NULL)
    {
        cerr << "Could not establish the name of the lengths variable." 
             << endl;
        exit(EXIT_FAILURE);
    }
    lengths = new int[totalEntries];
    DBReadVar(dbfile, lengthsName, lengths);

    // 
    // Read in the offsets.
    //
    if (offsetsName == NULL)
    {
        cerr << "Could not establish the name of the offsets variable." 
             << endl;
        exit(EXIT_FAILURE);
    }
    offsets = new int[totalEntries];
    DBReadVar(dbfile, offsetsName, offsets);
}


