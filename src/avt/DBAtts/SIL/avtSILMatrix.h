// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtSILMatrix.h                              //
// ************************************************************************* //

#ifndef AVT_SIL_MATRIX_H
#define AVT_SIL_MATRIX_H

#include <dbatts_exports.h>

#include <visitstream.h>
#include <string>
#include <vector>

#include <ref_ptr.h>

#include <avtSILCollection.h>
#include <avtTypes.h>
#include <avtSILSet.h>

class     avtSIL;
class     SILMatrixAttributes;


// ****************************************************************************
//  Class: avtSILMatrix
//
//  Purpose:
//      Defines a SIL matrix.  This is appropriate when there are two 
//      collections that produce a ton of subsets (for example domains and
//      materials).  The functionality of the avtSILMatrix could be represented
//      entirely with the avtSIL, avtSILSet, and avtSILCollection classes.  It
//      is necessary entirely for efficiency reasons.
//
//  Programmer: Hank Childs
//  Creation:   November 14, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Nov 13 16:47:18 PST 2003
//    Added argument forLoadBalance to TurnSet.  This was causing a serious
//    bug with AMR.
//
//    Dave Bremer, Thu Dec 20 10:31:43 PST 2007
//    Added method to see if a set is contained in one of this matrix's 
//    collections.
//
//    Dave Bremer, Fri Jan 25 13:07:02 PST 2008
//    Added GetSILSetID method.
// ****************************************************************************

class DBATTS_API avtSILMatrix
{
  public:
                          avtSILMatrix(const std::vector<int> &,
                                       SILCategoryRole, const std::string &, 
                                       const std::vector<int> &,
                                       SILCategoryRole, const std::string &);
                          avtSILMatrix(const SILMatrixAttributes &);
                          avtSILMatrix(const avtSILMatrix *);
    virtual              ~avtSILMatrix() {;};

    void                  Print(ostream &) const;
    void                  SetSIL(avtSIL *);
    void                  SetStartSet(int ssa) { setsStartAt = ssa; };
    void                  SetStartCollection(int csa) 
                                                 { collectionsStartAt = csa; };
    int                   GetStartCollection() const { return collectionsStartAt; }

    avtSILSet_p           GetSILSet(int index) const;
    avtSILCollection_p    GetSILCollection(int index) const;

    int                   GetSILSetID(int index) const;
    int                   GetNumSets(void) const;
    int                   GetNumCollections(void) const;

    SetState              GetSetState(const std::vector<unsigned char> &,
                                      int) const;
    bool                  GetMaterialList(int, MaterialList &, 
                                     const std::vector<unsigned char> &) const;
    void                  TurnSet(std::vector<unsigned char> &, int,
                                  SetState, bool forLoadBalance) const;
    int                   SetIsInCollection(int set) const;

    SILMatrixAttributes  *MakeAttributes(void) const;

    const std::vector<int> &GetSet1(void) const  { return set1; };
    const std::vector<int> &GetSet2(void) const  { return set2; };
    SILCategoryRole         GetRole1(void) const { return role1; };
    SILCategoryRole         GetRole2(void) const { return role2; };
    SILCategoryRole         GetRoleForCollection(int ind) const
                         { return (ind < int(set1.size()) ? role2 : role1); };

  protected:
    avtSIL               *sil;
    int                   setsStartAt;
    int                   collectionsStartAt;

    std::vector<int>      set1;
    std::string           category1;
    SILCategoryRole       role1;
    bool                  set1IsSequential;

    std::vector<int>      set2;
    std::string           category2;
    SILCategoryRole       role2;
    bool                  set2IsSequential;

    void                  Initialize(const std::vector<int> &,
                                     SILCategoryRole, const std::string &, 
                                     const std::vector<int> &,
                                     SILCategoryRole, const std::string &);
};


typedef ref_ptr<avtSILMatrix> avtSILMatrix_p;


#endif



