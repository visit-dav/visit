// ************************************************************************* //
//                                Material.h                                 //
// ************************************************************************* //

#ifndef MATERIAL_H
#define MATERIAL_H
#include <siloobj_exports.h>

#include <iostream.h>

#include <silo.h>

#include <SiloObjLib.h>
#include <Value.h>


// ****************************************************************************
//  Class : Material
//
//  Purpose:
//      Contains information about materials.
//
//  Data Members:
//      name                       The name of this material.
//      mesh                       The name of the mesh associated with this
//                                 material field.
//      dataType                   The data type of the volume fractions.
//      nMaterialNumbers           The number of material numbers. 
//      materialNumbers            The material numbers.  This is an array of
//                                 size nMaterialNumbers.
//      nValues                    The total number of values in the material
//                                 field.
//      nMixedValues               The total number of mixed values in the
//                                 material field.
//      nDomains                   The total number of domains.
//      nDomainMaterialNumbers     The number of material numbers for each
//                                 domain.  This is an array of size nDomains.
//      domainMaterialNumbers      The material numbers for each domain.  This
//                                 is an array of size nDomains and the ith
//                                 entry contain an array of size 
//                                 nDomainMaterialNumbers[i].
//      materials                  The name of the value object associated 
//                                 with the material numbers.
//      mixedVF                    The name of the value object with the mixed
//                                 material volume fractions.
//      mixedNext                  The name of the value object with the mixed
//                                 material next indices.
//      mixedMaterial              The name of the value object with the mixed
//                                 material numbers.
//      nDomainMaterials#s1D       The size of the domainMaterialNumbers if
//                                 it was compressed to be a 1D array.
//      domainMaterial#s1D         An array that contains the domainMat#s 
//                                 compressed to a 1D array.  This is 
//                                 important for writing to SILO files.
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

class SILOOBJ_API Material
{
  public:
                       Material();
                       Material(FOR_DERIVED_TYPES_e);
    virtual           ~Material();
    
    char              *GetName(void) { return name; };

    void               PrintSelf(ostream &);
    void               Read(DBobject *, DBfile *);
    virtual void       Write(DBfile *);

    int                GetNMaterialNumbers(void) { return nMaterialNumbers; };
    const int         *GetMaterialNumbers(void)  { return materialNumbers;  };
    int                GetNDomains(void)         { return nDomains; };
    const char        *GetMesh(void)             { return mesh; };

  protected:
    char              *name;

    char              *mesh;
    int                dataType;
    int                nMaterialNumbers;
    int               *materialNumbers;
    int                nValues;
    int                nMixedValues;
    int                nDomains;
    int               *nDomainMaterialNumbers;
    int              **domainMaterialNumbers;
    char              *materials;
    char              *mixedVF;
    char              *mixedNext;
    char              *mixedMaterial;
    
    Value             *materialsValue;
    Value             *mixedVFValue;
    Value             *mixedNextValue;
    Value             *mixedMaterialValue;
 
    int                nDomainMaterialNumbers1D;
    int               *domainMaterialNumbers1D;

    // Protected methods
    virtual void       CreateValues(void);
    void               RealConstructor(void);

    // Class-scoped constants
  public:
    static char * const   SILO_TYPE;
  protected:
    static int    const   SILO_NUM_COMPONENTS;
    static char * const   SILO_DATA_TYPE_NAME;
    static char * const   SILO_DOMAIN_MATERIAL_NUMBERS_NAME;
    static char * const   SILO_MATERIAL_NUMBERS_NAME;
    static char * const   SILO_MATERIALS_NAME;
    static char * const   SILO_MESH_NAME;
    static char * const   SILO_MIXED_VF_NAME;
    static char * const   SILO_MIXED_NEXT_NAME;
    static char * const   SILO_MIXED_MATERIAL_NAME;
    static char * const   SILO_N_MATERIAL_NUMBERS_NAME;
    static char * const   SILO_N_MIXED_VALUES_NAME;
    static char * const   SILO_N_DOMAINS_NAME;
    static char * const   SILO_N_DOMAIN_MATERIAL_NUMBERS_NAME;
    static char * const   SILO_N_VALUES_NAME;
};


#endif


