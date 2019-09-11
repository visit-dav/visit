// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//  Modifications:
//    Kathleen Bonnell, Tue Jun  1 17:00:17 PDT 2004
//    Added method 'UsesAllMaterials'.
//
//    Jeremy Meredith, Mon Aug 28 16:19:48 EDT 2006
//    Added support for scalar enumerations.
//
//    Mark C. Miller, Wed Mar 26 16:23:27 PDT 2008
//    Added support for scalar enumerations w/graphs
//
//    Hank Childs, Sat Nov 15 18:02:46 CST 2008
//    Add a data member that allows for material selection queries to go much
//    faster in simple cases.
//
// ****************************************************************************

class DBATTS_API avtSILRestrictionTraverser
{
  public:
                               avtSILRestrictionTraverser(avtSILRestriction_p);

    bool                       Equal(avtSILRestriction_p);

    void                       GetDomainList(std::vector<int> &);
    void                       GetDomainListAllProcs(std::vector<int> &);
    bool                       UsesAllData(void);
    bool                       UsesAllDomains(void);
 
    bool                       UsesData(int setId) const;
    SetState                   UsesSetData(int setId) const;

    const std::vector<std::string> &
                               GetMaterials(int, bool &);
    bool                       GetSpecies(std::vector<bool> &);
    bool                       UsesAllMaterials(void);

    int                        GetEnumerationCount();
    bool                       GetEnumerationFromGraph(int, std::vector<bool> &,
                                              std::string&);
    bool                       GetEnumeration(int, std::vector<bool> &,
                                              std::string&);
    void                       GetEnumerationMinMaxSetIds(int parentId,
                                   int *minId, int *maxId);


  protected:
    avtSILRestriction_p        silr;

    bool                       preparedForMaterialSearches;
    bool                       noMaterials;
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
    void                       GetDomainList(std::vector<int> &, bool allProcs);
};


#endif


