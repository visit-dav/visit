// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtSILCollection.h                             //
// ************************************************************************* //

#ifndef AVT_SIL_COLLECTION_H
#define AVT_SIL_COLLECTION_H

#include <dbatts_exports.h>

#include <visitstream.h>

#include <string>
#include <vector>

#include <ref_ptr.h>

#include <avtSILNamespace.h>


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
    SIL_ENUMERATION,   /* 8 */
    SIL_USERD          /* 9 */
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
//  Modifications:
//
//    Dave Bremer, Thu Dec 20 14:23:29 PST 2007
//    Added the ContainsElement method
//
//    Hank Childs, Thu Dec 10 15:01:59 PST 2009
//    Remove method GetSubsetList, add GetNumberOfSubsets, GetSubset in its
//    place (prevents memory bloat).
//
//    Mark C. Miller, Wed Aug 22 08:28:17 PDT 2012
//    Added private operator= and copy constructor to prevent inadvertent use.
//    The class has a data member that points to heap memory but these
//    methods were never implemented and it was easier to simply prevent their
//    accidental use than to figure out how best to implement them.
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

    int                     GetNumberOfSubsets(void) const
                                  { return subsets->GetNumberOfElements(); };
    int                     GetSubset(int idx) const
                                  { return subsets->GetElement(idx); };

    bool                    ContainsElement(int e) const;

  protected:
    std::string             category;
    SILCategoryRole         role;
    int                     supersetIndex;
    avtSILNamespace        *subsets;

  private:
    const avtSILCollection& operator=(const avtSILCollection& sc);
    avtSILCollection(const avtSILCollection& sc);
  
};


typedef ref_ptr<avtSILCollection> avtSILCollection_p;


#endif


