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
// ****************************************************************************

class DBATTS_API avtSILSet
{
  public:
                            avtSILSet(std::string, int);
    virtual                ~avtSILSet() {;};

    const std::string      &GetName(void) const
                                        { return name; };

    void                    AddMapIn(int);
    void                    AddMapOut(int);
    void                    AddMatrixRow(int, int, int);
    void                    AddMatrixColumn(int, int, int);

    int                     GetIdentifier(void) { return id; };

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
    std::vector<int>        idOfMatrixRow;
    std::vector<int>        idInMatrixRow;
    std::vector<int>        idOfMatrixColumn;
    std::vector<int>        idInMatrixColumn;
};


typedef ref_ptr<avtSILSet> avtSILSet_p;


#endif


