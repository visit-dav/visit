// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                avtSILSet.h                                //
// ************************************************************************* //

#ifndef AVT_SIL_SET_H
#define AVT_SIL_SET_H
#include <dbatts_exports.h>


#include <visitstream.h>

#include <string>
#include <vector>

#include <ref_ptr.h>


// ****************************************************************************
//  Class: avtSILSet
//
//  Purpose:
//      This is the representation of a set for a SIL.  It contains the name
//      of the set and links to all of the collections involving that set.
//
//  Programmer: Hank Childs
//  Creation:   March 8, 2001
//
//  Modifications:
//
//    Hank Childs, Mon May 21 09:12:17 PDT 2001
//    Add methods to make more usable.
//
//    Hank Childs, Fri Nov 15 10:25:02 PST 2002
//    Add concept of matrices.
//
//    Dave Bremer, Thu Dec 20 10:31:43 PST 2007
//    Removed unused data used in matrix operations, and consolidated
//    AddMatrixRow and AddMatrixColumn into AddMatrixMapOut.
//
//    Dave Bremer, Fri Jan 25 13:07:02 PST 2008
//    Made GetIdentifier() a const method.
// ****************************************************************************

class DBATTS_API avtSILSet
{
  public:
                            avtSILSet(const std::string &name, int id);
    virtual                ~avtSILSet() {;};

    const std::string      &GetName(void) const
                                        { return name; };

    void                    AddMapIn(int);
    void                    AddMapOut(int);
    void                    AddMatrixMapOut(int);

    int                     GetIdentifier(void) const { return id; };

    void                    Print(ostream &) const;

    const std::vector<int> &GetMapsIn(void) const  { return mapsIn; };
    const std::vector<int> &GetMapsOut(void) const { return allMapsOut; };
    const std::vector<int> &GetRealMapsOut(void) const { return mapsOut; };

  protected:
    std::string             name;
    int                     id;
    std::vector<int>        mapsIn;
    std::vector<int>        mapsOut;
    std::vector<int>        allMapsOut;
};


typedef ref_ptr<avtSILSet> avtSILSet_p;


#endif


