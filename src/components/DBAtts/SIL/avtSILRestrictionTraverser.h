// ************************************************************************* //
//                        avtSILRestrictionTraverser.h                       //
// ************************************************************************* //

#ifndef AVT_SIL_RESTRICTION_TRAVERSER_H
#define AVT_SIL_RESTRICTION_TRAVERSER_H

#include <dbatts_exports.h>

#include <vector>

#include <avtSILRestriction.h>


// ****************************************************************************
//  Class: avtSILRestrictionTraverser
//
//  Purpose:
//      A module whose devoted purpose is to traverse SIL restriction objects.
//      The purpose of putting this in its own module is solely to make the
//      interfaces cleaner.
//
//  Programmer: Hank Childs
//  Creation:   November 22, 2002
//
// ****************************************************************************

class DBATTS_API avtSILRestrictionTraverser
{
  public:
                               avtSILRestrictionTraverser(avtSILRestriction_p);

    bool                       Equal(avtSILRestriction_p);

    void                       GetDomainList(std::vector<int> &);
    bool                       UsesAllData(void);
    bool                       UsesAllDomains(void);
 
    bool                       UsesData(int setId) const;
    SetState                   UsesSetData(int setId) const;

    const std::vector<std::string> &
                               GetMaterials(int, bool &);
    bool                       GetSpecies(std::vector<bool> &);


  protected:
    avtSILRestriction_p        silr;

    bool                       preparedForMaterialSearches;
    std::vector<MaterialList>  materialList;
    std::vector<int>           materialListForChunk;
    std::vector<bool>          shouldMatSelect;
 
    void                       PrepareForMaterialSearches(void);
    void                       AddMaterialList(int, MaterialList &, bool);

  private:
                               avtSILRestrictionTraverser() {;};
                               avtSILRestrictionTraverser(
                                       const avtSILRestrictionTraverser &) {;};
    const avtSILRestrictionTraverser &    
                               operator=(const avtSILRestrictionTraverser &t)
                                    { return t; };
};


#endif


