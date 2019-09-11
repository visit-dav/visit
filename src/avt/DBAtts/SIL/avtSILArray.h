// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtSILArray.h                              //
// ************************************************************************* //

#ifndef AVT_SIL_ARRAY_H
#define AVT_SIL_ARRAY_H

#include <dbatts_exports.h>
#include <string>
#include <ref_ptr.h>
#include <avtSILSet.h>
#include <avtSILCollection.h>
#include <avtTypes.h>
#include <vectortypes.h>
#include <NameschemeAttributes.h>

class SILArrayAttributes;


// ****************************************************************************
//  Class: avtSILArray
//
//  Purpose:    
//      Conceptually it contains a number of avtSILSets that would be created
//      with names that follow a pattern, and which can be grouped into one
//      collection.
//
//  Programmer: Dave Bremer
//  Creation:   Thu Dec 20 12:12:30 PST 2007
//
//  Modifications:
//
//    Dave Bremer, Fri Jan 25 13:07:02 PST 2008
//    Added GetSILSetID method.
//
//    Dave Bremer, Fri Mar 28 19:38:29 PDT 2008
//    Modified to handle an explicit list of names.
//
//    Hank Childs, Tue Dec  8 08:44:07 PST 2009
//    Add support for name schemes.  Add Print method.
//
// ****************************************************************************

class DBATTS_API avtSILArray
{
  public:
                        avtSILArray(const std::string &pfx, int nSets, 
                                    int firstSetName,  bool uniqueIDs,
                                    const std::string &cat,
                                    SILCategoryRole r, int parent);
                        avtSILArray(const stringVector &names, 
                                    int nSets, int firstSetName, 
                                    bool uniqueIDs, const std::string &cat,
                                    SILCategoryRole r, int parent);
                        avtSILArray(const NameschemeAttributes &namescheme, 
                                    int nSets, int firstSetName, 
                                    bool uniqueIDs, const std::string &cat,
                                    SILCategoryRole r, int parent);
                        avtSILArray(const SILArrayAttributes &atts);
    virtual            ~avtSILArray() {;};

    void                Print(ostream &) const;
    int                 GetNumSets() const   { return iNumSets; }
    avtSILSet_p         GetSILSet(int index) const;
    avtSILCollection_p  GetSILCollection() const;

    int                 GetSILSetID(int index) const;
    int                 GetParent() const    { return iColParent; }
    void                SetFirstSetIndex(int s)    {iFirstSet = s;}
    void                SetCollectionIndex(int c)  {iColIndex = c;}
    SetState            GetSetState(const std::vector<unsigned char> &useSet) const;
    void                TurnSet(std::vector<unsigned char> &useSet,
                                SetState val, bool forLoadBalance) const;
    int                 GetSetIndex(const std::string &name) const;

    bool                IsCompatible(const avtSILArray *) const;

    SILArrayAttributes *MakeAttributes(void) const;

  protected:
    //For making the sets
    std::string      prefix;     //prefix or template for the block name
    stringVector     names;      //explicit list of block names
    NameschemeAttributes namescheme;   //namescheme for labeling block names

    int              iNumSets;
    int              iFirstSetName;  //Often 0 or 1, depending for example on a 
                                     //code's block numbering preference
    bool             bUseUniqueIDs;

    //For making the collection
    int              iFirstSet;      //Index of first set in the collection, 
                                     //in the containing SIL's index space.
    int              iColIndex;      //Index of this collection, in the 
                                     //containing SIL.
    std::string      category;
    SILCategoryRole  role;
    int              iColParent;

};


typedef ref_ptr<avtSILArray> avtSILArray_p;


#endif


