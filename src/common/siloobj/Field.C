// ************************************************************************* //
//                                Field.C                                    //
// ************************************************************************* //

#include <iostream.h>
#include <stdlib.h>
#include <string.h>

#include <Field.h>
#include <SiloObjLib.h>


// 
// Declaration of static consts.  They are not allowed as initializers in 
// class definitions.
//

char * const Field::DIMS_NAME                = "Dims";
char * const Field::MIXED_VALUES_NAME        = "Mixed";
int    const Field::SILO_NUM_COMPONENTS      = 12;
char * const Field::SILO_TYPE                = "Field";
char * const Field::SILO_CENTERING_NAME      = "Centering";
char * const Field::SILO_DATA_TYPE_NAME      = "DataType";
char * const Field::SILO_DIMS_NAME           = "Dims";
char * const Field::SILO_INTERVAL_TREE_NAME  = "IntervalTree";
char * const Field::SILO_MESH_NAME           = "Mesh";
char * const Field::SILO_MIXED_MATERIAL_NAME = "MixedMaterial";
char * const Field::SILO_MIXED_VALUES_NAME   = "MixedValues";
char * const Field::SILO_N_DOMAINS_NAME      = "NDomains";
char * const Field::SILO_N_MIXED_VALUES_NAME = "NMixedValues";
char * const Field::SILO_N_VALUES_NAME       = "NValues";
char * const Field::SILO_UNITS_NAME          = "Units";
char * const Field::SILO_VALUES_NAME         = "Values";


// ****************************************************************************
//  Method: Field constructor
//
//  Purpose:
//      Creates a normally constructed Field object.
//
//  Note:       See the discussion for the other Field constructor for why two
//              constructors are necessary.
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

Field::Field()
{
    RealConstructor();
    CreateValues();
}


// ****************************************************************************
//  Method: Field Constructor
//
//  Purpose:
//      Creates a Field object that is incomplete, because the derived types
//      will fill in the Value objects.  We would like the Value objects to be 
//      fully created at the time of instantiation, so subsequent method 
//      invocations can be in confidence that the object is fully constructed.
//      Unfortunately, this cannot be accomplished with virtual functions since
//      the base type (this one) cannot resolve whether or not it is the base 
//      type, except through passing an explicit argument, like the one in this
//      constructor's signature.  Virtual functions do not work in constructors
//      because they may access data members that do not exist yet.  The 
//      virtual function table when this method is called for this object is
//      that of the Field class.  If this is a derived type of Field, the
//      VFT will not be replaced with that of the derived type until the
//      constructor for that derived type is called.
//      
//  Arguments:
//      <unnamed>   An unused argument that makes the signature differ from
//                  that of the default constructor.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
// ****************************************************************************

Field::Field(FOR_DERIVED_TYPES_e)
{
    RealConstructor();
    
    //
    // Not calling CreateValues so the derived type's constructor can.
    //
}


// ****************************************************************************
//  Method: RealConstructor
//
//  Purpose:
//      Because the Value object needs to be created by derived types, there 
//      must be multiple (and similar) constructors.   This is the "real" 
//      constructor that they all can call.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 14:08:17 PDT 2000
//      Added initialization of dims and dimsObject.
//
// ****************************************************************************

void
Field::RealConstructor(void)
{
    mesh               = NULL;
    intervalTree       = NULL;
    centering          = -1;
    dataType           = -1;
    mixedMaterial      = NULL;
    units              = NULL;
    nValues            = 0;
    nMixedValues       = 0;
    nDomains           = 0;
    values             = NULL;
    mixedValues        = NULL;
    dims               = NULL;

    name               = NULL;
 
    valueObject        = NULL;
    mixedValueObject   = NULL;
    intervalTreeObject = NULL;
    dimsObject         = NULL;
}


// ****************************************************************************
//  Method: Field::CreateValues
//
//  Purpose:
//      Creates the Value objects for the Field.  
//
//  Note:       The necessity of this method is discussed in the constructor.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 14:08:17 PDT 2000
//      Added construction of dimsObject.
//
// ****************************************************************************

void
Field::CreateValues(void)
{
    valueObject        = new Value();
    mixedValueObject   = new Value();
    dimsObject         = new Value();
    intervalTreeObject = new IntervalTree();
}


// ****************************************************************************
//  Method: Field destructor
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
//  Modifications:
//      
//     Hank Childs, Mon Jun 12 14:08:17 PDT 2000
//     Added memory cleanup for dims and dimsObject.
//
// ****************************************************************************

Field::~Field()
{
    if (mesh != NULL)
    {
        delete [] mesh;
    }
    if (intervalTree != NULL)
    {
        delete [] intervalTree;
    }
    if (mixedMaterial != NULL)
    {
        delete [] mixedMaterial;
    }
    if (values != NULL)
    {
        delete [] values;
    }
    if (mixedValues != NULL)
    {
        delete [] mixedValues;
    }
    if (dims != NULL)
    {
        delete [] dims;
    }
    if (name != NULL)
    {
        delete [] name;
    }
    if (units != NULL)
    {
        delete [] units;
    }
    if (valueObject != NULL)
    {
        delete valueObject;
    }
    if (mixedValueObject != NULL)
    {
        delete mixedValueObject;
    }
    if (intervalTreeObject != NULL)
    {
        delete intervalTreeObject;
    }
    if (dimsObject != NULL)
    {
        delete dimsObject;
    }
}


// ****************************************************************************
//  Method: Field::PrintSelf
//
//  Purpose:
//      Prints the data members of a Field object.  For debugging purposes
//      only.
//
//  Arguments:
//      out     An ostream to print the object out to.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 1999
//
// ****************************************************************************

void
Field::PrintSelf(ostream &out)
{
    out << "Field (" << name << "):" << endl;

    out << "\tMesh:          " ;
    if (mesh != NULL)
    {
        out << mesh << endl;
    }
    else
    {
        out << "nil" << endl;
    }

    out << "\tIntervalTree:  " ;
    if (intervalTree != NULL)
    {
        out << intervalTree << endl;
    }
    else
    {
        out << "nil" << endl;
    }
  
    out << "\tCentering:     " << centering << endl;
    out << "\tdataType:      " << dataType << endl;
  
    out << "\tmixedMaterial: " ;
    if (mixedMaterial != NULL)
    {
        out << mixedMaterial << endl;
    }
    else
    {
        out << "nil" << endl;
    }

    out << "\tunits:         " ;
    if (units != NULL)
    {
        out << units << endl;
    }
    else
    {
        out << "nil" << endl;
    }

    out << "\tnValues:       " << nValues << endl;
    out << "\tnMixedValues:  " << nMixedValues << endl;
    out << "\tnDomains:      " << nDomains << endl;

    out << "\tvalues:        " ;
    if (values != NULL)
    {
        out << values << endl;
    }
    else
    {
        out << "nil" << endl;
    }

    out << "\tmixedValues:   " ;
    if (mixedValues != NULL)
    {
        out << mixedValues << endl;
    }
    else
    {
        out << "nil" << endl;
    }

    out << "\tdims:   " ;
    if (dims != NULL)
    {
        out << dims << endl;
    }
    else
    {
        out << "nil" << endl;
    }
}


// ****************************************************************************
//  Method: Field::Write
//
//  Purpose:
//      Write out a Field to a SILO file.
//
//  Arguments:
//      dbfile    The Silo file to write the Field object into.
//
//  Note:       The type field for the DBobject, siloObj, must have its memory
//              allocated through a malloc, so that when it is freed, we do
//              not get a free memory mismatch from purify.
//
//  Programmer: Hank Childs
//  Creation:   December 9, 1999
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 14:08:17 PDT 2000
//      Wrote out dims and dimsObject.
//
//      Hank Childs, Tue Jun 13 08:45:01 PDT 2000
//      Removed writing of interval tree and put in separate function.
//
// ****************************************************************************

void 
Field::Write(DBfile *dbfile)
{
    //
    // Write out the objects the Field object contains.
    //
    valueObject->Write(dbfile);
    dimsObject->Write(dbfile);

    //
    // Do not write the mixedValueObject if it has no values.  This is 
    // common, esp. because coordinates are Fields and they have no mixed
    // values, so this is primarily to prevent coordinate mixed values
    // (which doesn't really make sense) from being written.
    //
    if (nMixedValues > 0)
    {
        mixedValueObject->Write(dbfile);
    }

    //
    // Create an object to be written into the SILO file.
    //
    DBobject *siloObj = DBMakeObject(name, DB_USERDEF, SILO_NUM_COMPONENTS);
    if (siloObj == NULL)
    {
        cerr << "Unable to create a silo object for the field." << endl;
        exit(EXIT_FAILURE);
    }

    //
    // We must make a copy of the string, because the "destructor" deletes this
    // field and we can't have it delete statics.
    //
    siloObj->type = C_strdup(SILO_TYPE);
    
    //
    // Add fields to the Field SILO object.
    //
    char  *validEntry;
    char  *nullString = "\0";

    validEntry = (mesh == NULL ? nullString : mesh);
    DBAddStrComponent(siloObj, SILO_MESH_NAME, validEntry);
 
    validEntry = (intervalTree == NULL ? nullString : intervalTree);
    DBAddStrComponent(siloObj, SILO_INTERVAL_TREE_NAME, validEntry);

    DBAddIntComponent(siloObj, SILO_CENTERING_NAME, centering);

    DBAddIntComponent(siloObj, SILO_DATA_TYPE_NAME, dataType);

    validEntry = (mixedMaterial == NULL ? nullString : mixedMaterial);
    DBAddStrComponent(siloObj, SILO_MIXED_MATERIAL_NAME, validEntry);

    validEntry = (units == NULL ? nullString : units);
    DBAddStrComponent(siloObj, SILO_UNITS_NAME, validEntry);

    DBAddIntComponent(siloObj, SILO_N_VALUES_NAME, nValues);

    DBAddIntComponent(siloObj, SILO_N_MIXED_VALUES_NAME, nMixedValues);

    DBAddIntComponent(siloObj, SILO_N_DOMAINS_NAME, nDomains);

    validEntry = (values == NULL ? nullString : values);
    DBAddStrComponent(siloObj, SILO_VALUES_NAME, validEntry);
  
    validEntry = (dims == NULL ? nullString : dims);
    DBAddStrComponent(siloObj, SILO_DIMS_NAME, validEntry);

    if (nMixedValues > 0)
    {
        validEntry = (mixedValues == NULL ? nullString : mixedValues);
    }
    else
    {
        //
        // We already decided not to write the mixedValueObject earlier, so
        // don't put in a reference to a non-existent object.
        //
        validEntry = nullString;
    }
    DBAddStrComponent(siloObj, SILO_MIXED_VALUES_NAME, validEntry);

    DBWriteObject(dbfile, siloObj, 0);
    DBFreeObject(siloObj);
}


// ****************************************************************************
//  Method: Field::WriteIntervalTree
//
//  Purpose:
//      Write out the IntervalTree for a Field to a SILO file.
//
//  Arguments:
//      dbfile    The Silo file to write the Field object into.
//
//  Programmer: Hank Childs
//  Creation:   June 13, 2000
//
// ****************************************************************************

void
Field::WriteIntervalTree(DBfile *dbfile)
{
    intervalTreeObject->Write(dbfile);
}


// ****************************************************************************
//  Method: Field::Read
//
//  Purpose:
//      Reads in a Field from a DBobject.
//
//  Arguments:
//      siloObj    The Silo object that the Field should be populated from.
//  
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
//  Modification:
//
//      Hank Childs, Mon Jun 12 14:08:17 PDT 2000
//      Added reading of the name of the dims object.
//
// ****************************************************************************

void
Field::Read(DBobject *siloObj)
{
    if (strcmp(siloObj->type, SILO_TYPE) != 0)
    {
        //
        // A mistake was made in trying to create this object.
        //
        cerr << "Cannot create a Field object from a " << siloObj->type
             << " object." << endl;
        exit(EXIT_FAILURE);
    }

    //
    // Copy the name.
    //
    name = CXX_strdup(siloObj->name);

    //
    // Go through each component and populate the Field object.
    //
    for (int i = 0 ; i < siloObj->ncomponents ; i++)
    {
        //
        // Rely on SILO obj library routines to get the value from the pdb
        // name once the component has been identified.
        //
        if (strcmp(siloObj->comp_names[i], SILO_CENTERING_NAME) == 0)
        {
            centering = ParsePDBNameInt(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_DATA_TYPE_NAME) == 0)
        {
            dataType = ParsePDBNameInt(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_DIMS_NAME) == 0)
        {
            dims = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_INTERVAL_TREE_NAME) == 0)
        {
            intervalTree = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_MESH_NAME) == 0)
        {
            mesh = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_MIXED_MATERIAL_NAME) == 0)
        {
            mixedMaterial = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_MIXED_VALUES_NAME) == 0)
        {
            mixedValues = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_N_DOMAINS_NAME) == 0)
        {
            nDomains = ParsePDBNameInt(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_N_MIXED_VALUES_NAME) == 0)
        {
            nMixedValues = ParsePDBNameInt(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_N_VALUES_NAME) == 0)
        {
            nValues = ParsePDBNameInt(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_UNITS_NAME) == 0)
        {
            units = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_VALUES_NAME) == 0)
        {
            values = ParsePDBNameString(siloObj->pdb_names[i]);
        }
    }   // End 'for' over all components
}


