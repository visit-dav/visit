// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtDatasetCollection.h                            //
// ************************************************************************* //

#ifndef AVT_DATASET_COLLECTION_H
#define AVT_DATASET_COLLECTION_H

#include <database_exports.h>

#include <vector>
#include <string>

#include <array_ref_ptr.h>
#include <void_ref_ptr.h>

#include <avtDataTree.h>
#include <avtMaterial.h>
#include <avtSpecies.h>

class     vtkDataSet;

class     avtMixedVariable;


// ****************************************************************************
//  Struct: avtDatasetCollection
//
//  Purpose:
//      This struct is only used by the avtGenericDatabase.  It is used to
//      assemble intermediate results in an organized way.  It is only pulled
//      off into its own file (as opposed to coupling it with the generic
//      database code) in an attempt to clean up portions of the code.
//
//  Programmer: Hank Childs
//  Creation:   July 4, 2002  [Split into its own file]
//
//  Modifications:
//    Brad Whitlock, Mon Jul 15 16:27:25 PST 2002
//    Added API.
//
//    Hank Childs, Sun Mar 13 10:41:17 PST 2005
//    Fixed problem with memory leak.
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added code for setting/getting Var and Vars2nd
// ****************************************************************************

struct DATABASE_API avtDatasetCollection
{
  public:
                                            avtDatasetCollection(int);
    virtual                                ~avtDatasetCollection();
 
    avtDataTree_p                          *avtds;
    std::vector<std::vector<std::string> >  labels;
    std::vector<std::vector<std::string> >  matnames;
    std::vector<bool>                       needsMatSelect;
    std::vector<int>                        numMats;
    void                                    SetVar(const char *cvar)
                                                {  var = cvar; };
    const std::string                       GetVar() const
                                                { return var; };
    void                                    SetVars2nd(const std::vector<CharStrRef> &v2nd)
                                                { vars2nd = v2nd; };
    const std::vector<CharStrRef>          &GetVars2nd() const
                                                { return vars2nd; };
    void                                    SetNumMaterials(int dom,int nmats);
    int                                     GetNDomains(void)
                                                { return nDomains; };
    void                                    SetDataset(int, int, vtkDataSet *);
    vtkDataSet                             *GetDataset(int i, int j)
                                                { return vtkds[i][j]; };
    vtkDataSet                            **GetDatasetList(int i)
                                                { return vtkds[i]; };
    avtDataTree_p                           AssembleDataTree(
                                                           std::vector<int> &);
 
    void                                    SetMaterial(int i,avtMaterial *mat)
                                                { materials[i] = mat; };
    avtMaterial                            *GetMaterial(int i)
                                                { return materials[i]; };
    void                                    MaterialsShouldBeFreed(void)
                                              {materialsShouldBeFreed = true;};
 
    void                                    SetSpecies(int i, avtSpecies *spec)
                                                { species[i] = spec; };
    avtSpecies                             *GetSpecies(int i)
                                                { return species[i]; };
    void                                    SpeciesShouldBeFreed(void)
                                              {speciesShouldBeFreed = true;};
 
    void                                    AddMixVar(int i, void_ref_ptr mix);
    void_ref_ptr                            GetMixVar(int i, std::string);
    std::vector<void_ref_ptr>              &GetAllMixVars(int i);
    void                                    ReplaceMixVar(int i,
                                                          void_ref_ptr mix);

  private:
    std::string                             var;
    std::vector<CharStrRef>                 vars2nd;
    int                                     nDomains;
    vtkDataSet                           ***vtkds;
    avtMaterial                           **materials;
    avtSpecies                            **species;
    std::vector<void_ref_ptr>              *mixvars;
    bool                                    materialsShouldBeFreed;
    bool                                    speciesShouldBeFreed;
};


#endif


