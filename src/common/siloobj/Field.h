// ************************************************************************* //
//                                 Field.h                                   //
// ************************************************************************* //

#ifndef FIELD_H
#define FIELD_H
#include <siloobj_exports.h>

#include <visitstream.h>
#include <silo.h>

#include <Value.h>
#include <SiloObjLib.h>
#include <IntervalTree.h>


// ****************************************************************************
//  Class: Field
// 
//  Purpose:
//      Keeps information about a field.
//
//  Data Members:
//      mesh              The name of the mesh associated with this field.
//      intervalTree      The name of the interval tree associated with this
//                        field.
//      centering         The centering of the variable.
//      dataType          The data type of the field.
//      mixedMaterial     The name of the material object associated with this
//                        field.
//      units             The units associated with the variable.
//      nValues           The total number of values in the field.
//      nMixedValues      The number of mixed values in the field.
//      nDomains          The number of domains.
//      values            The name of the value object assocated with this
//                        field.
//      mixedValues       The name of the value object associated with the
//                        mixed values.
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
//  Modifications:
//
//      Hank Childs,   Mon Jun 12 14:08:17 PDT 2000
//      Added fields dims, dimsObject, and SILO_DIMS_NAME.
//
// ****************************************************************************

class SILOOBJ_API Field
{
  public:
                      Field();
                      Field(FOR_DERIVED_TYPES_e);
    virtual          ~Field();

    char             *GetName(void)       { return name; };
    int               GetCentering(void)  { return centering; };

    void              Read(DBobject *);
    virtual void      Write(DBfile *);
    virtual void      WriteIntervalTree(DBfile *);

    void              PrintSelf(ostream &);

  protected:
    char             *mesh;
    char             *intervalTree;
    int               centering;
    int               dataType;
    char             *mixedMaterial;
    char             *units;
    int               nValues;
    int               nMixedValues;
    int               nDomains;
    char             *dims;
    char             *values;
    char             *mixedValues;

    char             *name;

    Value            *dimsObject;
    Value            *valueObject;
    Value            *mixedValueObject;
    IntervalTree     *intervalTreeObject;

    // Protected methods
    void              CreateValues(void);
    void              RealConstructor(void);
     
    // Constants
  public:
    static char * const   SILO_TYPE;
  protected:
    static char * const   DIMS_NAME;
    static char * const   MIXED_VALUES_NAME;
    static int    const   SILO_NUM_COMPONENTS;
    static char * const   SILO_CENTERING_NAME;
    static char * const   SILO_DATA_TYPE_NAME;
    static char * const   SILO_DIMS_NAME;
    static char * const   SILO_INTERVAL_TREE_NAME;
    static char * const   SILO_MESH_NAME;
    static char * const   SILO_MIXED_MATERIAL_NAME;
    static char * const   SILO_MIXED_VALUES_NAME;
    static char * const   SILO_N_DOMAINS_NAME;
    static char * const   SILO_N_MIXED_VALUES_NAME;
    static char * const   SILO_N_VALUES_NAME;
    static char * const   SILO_UNITS_NAME;
    static char * const   SILO_VALUES_NAME;
};


#endif


