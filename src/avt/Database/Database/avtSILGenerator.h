// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtSILGenerator.h                            //
// ************************************************************************* //

#ifndef AVT_SIL_GENERATOR_H
#define AVT_SIL_GENERATOR_H

#include <database_exports.h>

#include <string>
#include <vector>

#include <avtDatabaseMetaData.h>
#include <avtSILCollection.h>


class     avtSIL;
class     NameschemeAttributes;


// ****************************************************************************
//  Class: avtSILGenerator
//
//  Purpose:
//      Takes avtDatabaseMetaData and constructs a reasonable SIL from it.
//
//  Notes:      Created from pre-existing routines in the avtGenericDatabase.
//
//  Programmer: Hank Childs
//  Creation:   September 6, 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Aug 24 10:34:13 PDT 2005
//    Added origin for the groups.
//
//    Jeremy Meredith, Tue Aug 29 16:52:36 EDT 2006
//    Added enumerated scalar subset support.
//
//    Brad Whitlock, Thu Mar 8 09:46:17 PDT 2007
//    Changed AddSpecies method.
//
//    Dave Bremer, Tue Apr  1 16:43:16 PDT 2008
//    Added a flag to AddSubsets to control the use of SIL Arrays
//
//    Mark C. Miller, Wed Aug 26 11:08:47 PDT 2009
//    Removed custom SIL stuff.
//
//    Hank Childs, Mon Dec  7 14:05:00 PST 2009
//    Added various optimizations that enable the SIL object to get smaller.
//
//    Kathleen Biagas, Thu Aug 22 09:58:50 PDT 2013
//    Add groupNames argment to AddGroups.
//
// ****************************************************************************

class DATABASE_API avtSILGenerator
{
  public:
    void                CreateSIL(avtDatabaseMetaData *, avtSIL *);

  protected:
    void                AddSubsets(avtSIL *, int, int, int, std::vector<int> &,
                                   const std::string &, const std::string &,
                                   const std::vector<std::string> &,
                                   const NameschemeAttributes &,
                                   bool useSILArrays,
                                   SILCategoryRole cat = SIL_DOMAIN,
                                   bool onlyCreateSets = false);
    std::vector<int>    AddGroups(avtSIL *, int, int, int, const std::string &,
                                  const std::string &,
                                  const std::vector< std::string > &);
    void                AddGroupCollections(avtSIL *, int, int,
                                  const std::vector<int> &,
                                  const std::vector<int>&, 
                                  const std::vector<int>&, 
                                  const std::string &, const std::vector<int> &);
    void                AddMaterials(avtSIL *, int, const std::string &,
                                     const std::vector<std::string> &,
                                     std::vector<int> &, int);
    void                AddSpecies(avtSIL *, int,
                                   const std::vector<std::string>&,
                                   const std::string &,
                                   const avtSpeciesMetaData *,
                                   int);
    void                AddMaterialSubsets(avtSIL *, const std::vector<int> &,
                                           int, int , const std::vector<int> &,
                                           const std::string &,
                                           const std::vector<std::string> &,
                                           const std::vector<std::string> &);
    void                AddEnumScalars(avtSIL *, int,
                                       const avtScalarMetaData *);
};


#endif
 

