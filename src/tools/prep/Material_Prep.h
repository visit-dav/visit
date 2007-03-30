// ************************************************************************* //
//                            Material_Prep.h                                //
// ************************************************************************* //

#ifndef MATERIAL_PREP_H
#define MATERIAL_PREP_H


#include <Material.h>
#include <Value_Prep.h>


// ****************************************************************************
//  Class : Material_Prep
//
//  Purpose:
//      A derived type of Material, this is meant exclusively for meshtvprep.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
// ****************************************************************************

class Material_Prep : public Material
{
  public:
                       Material_Prep();
    virtual           ~Material_Prep();
    
    void               AddMaterial(DBmaterial *,char *, int, int, DBfile *);
  
    void               SetName(char *);
    void               SetNDomains(int);

    void               DetermineSize(int, DBfile *, char *);

    void               Consolidate(void);
    void               WrapUp(void);
    virtual void       Write(DBfile *);

  protected:
    bool               setOnceFields;
    int                nMaterialNumbersAlloc;

    Value_Prep        *aliasedMaterialsValue;
    Value_Prep        *aliasedMixedVFValue;
    Value_Prep        *aliasedMixedNextValue;
    Value_Prep        *aliasedMixedMaterialValue;
 
    // Protected methods
    void               AllocMaterialNumbers(int);
    void               AddToMaterialList(int, int *);
    virtual void       CreateValues(void);
    void               SetOnceFields(int, char *);

    // Class-scoped constants
    static int    const   MATERIAL_NUMBERS_SIZE;
    static char * const   SILO_LOCATION;
};


#endif


