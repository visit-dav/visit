// ************************************************************************* //
//                            avtSILCollection.h                             //
// ************************************************************************* //

#ifndef AVT_SIL_COLLECTION_H
#define AVT_SIL_COLLECTION_H
#include <dbatts_exports.h>


#include <iostream.h>

#include <string>
#include <vector>

#include <ref_ptr.h>


class     avtSILNamespace;


typedef enum
{
    SIL_TOPOLOGY        = 0,
    SIL_PROCESSOR,     /* 1 */
    SIL_BLOCK,         /* 2 */
    SIL_DOMAIN,        /* 3 */
    SIL_ASSEMBLY,      /* 4 */
    SIL_MATERIAL,      /* 5 */
    SIL_BOUNDARY,      /* 6 */
    SIL_SPECIES,       /* 7 */
    SIL_USERD          /* 8 */
} SILCategoryRole;


// ****************************************************************************
//  Class: avtSILCollection
//
//  Purpose:
//      This is the representation of a collection for a SIL.  It resembles a
//      SAF collection and draws its name from it.  A collection is a group of
//      maps that all have their domain come from a common set and all have the
//      same category.  Because the GUI would have to group maps into
//      collections anyways, there is no explicit information about a map -- it
//      is all stored in a collection.
//
//  Programmer: Hank Childs
//  Creation:   March 8, 2001
//
// ****************************************************************************

class DBATTS_API avtSILCollection
{
  public:
                            avtSILCollection(std::string, SILCategoryRole,
                                         int supersetIndex, avtSILNamespace *);
    virtual                ~avtSILCollection();

    int                     GetSupersetIndex(void) const
                                      { return supersetIndex; };
    const avtSILNamespace  *GetSubsets(void) const
                                      { return subsets; };
    const std::string      &GetCategory(void) const
                                      { return category; };
    SILCategoryRole         GetRole(void) const
                                      { return role; };

    void                    Print(ostream &) const;

    const std::vector<int> &GetSubsetList(void) const;

  protected:
    std::string             category;
    SILCategoryRole         role;
    int                     supersetIndex;
    avtSILNamespace        *subsets;
};


typedef ref_ptr<avtSILCollection> avtSILCollection_p;


#endif


