// ************************************************************************* //
//                              avtSILGenerator.h                            //
// ************************************************************************* //

#ifndef AVT_SIL_GENERATOR_H
#define AVT_SIL_GENERATOR_H

#include <database_exports.h>

#include <string>
#include <vector>

#include <avtDatabaseMetaData.h>


class     avtSIL;


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
// ****************************************************************************

class DATABASE_API avtSILGenerator
{
  public:
    void                CreateSIL(avtDatabaseMetaData *, avtSIL *);

  protected:
    void                AddSubsets(avtSIL *, int, int, int, std::vector<int> &,
                                   const std::string &, const std::string &,
                                   const std::vector<std::string> &);
    void                AddGroups(avtSIL *, int, int, const std::vector<int> &,
                                  const std::vector<int>&, const std::string &,
                                  const std::string &, const std::string &);
    void                AddMaterials(avtSIL *, int, const std::string &,
                                     const std::vector<std::string> &,
                                     std::vector<int> &, int);
    void                AddSpecies(avtSIL *, int,
                                   const std::vector<std::string>&,
                                   const std::string &,
                                   const std::vector<avtMatSpeciesMetaData*> &,
                                   int);
    void                AddMaterialSubsets(avtSIL *, const std::vector<int> &,
                                           int, int , const std::vector<int> &,
                                           const std::string &,
                                           const std::vector<std::string> &,
                                           const std::vector<std::string> &);
};


#endif
 

