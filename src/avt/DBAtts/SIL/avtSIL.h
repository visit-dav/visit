// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                   avtSIL.h                                //
// ************************************************************************* //

#ifndef AVT_SIL_H
#define AVT_SIL_H

#include <dbatts_exports.h>

#include <visitstream.h>
#include <vector>

#include <avtSILCollection.h>
#include <avtSILMatrix.h>
#include <avtSILArray.h>
#include <avtSILSet.h>


class   SILAttributes;


// ****************************************************************************
//  Class: avtSIL
//
//  Purpose:
//      Defines a Subset Inclusion Lattice (SIL).  A subset inclusion lattice
//      consists of sets and collections.  A set is some subset of the whole
//      and collection is all of the maps of a certain category that have 
//      elements from a certain set in their domain.
//
//  Terminology:
//      SIL - The subset inclusion lattice.  This has ideas similar to the SIL
//          of SAF, but is "dummied down" so that it will never contain problem
//          size data.  The SIL is a language for restricting sets (meshes).
//          The SIL is often represented in graphical form.  It is a dag
//          (directed acyclic graph).
//      Sets - These are meshes in actuality.  They are typically represented
//          as the vertices in the dag.
//      Maps - Maps denote subset relations.  They go between two sets, the
//          superset and one of its subsets.  There is a category associated
//          with a map.  The category is a name (string), but it has a meaning
//          (referred to as a role) which indicates the type of subsetting -
//          material, domain, processor, etc.
//      Collections - Maps with the same superset and category role typically
//          partition the superset.  These maps taken as a group are called a
//          collection.  For VisIt's representation, it is convenient to
//          consider collections rather than maps.
//      Namespace - Although many elements in SAF can be thought of as sets,
//          representing them this way would cause a SIL to become problem
//          size.  For this reason, a map's range (range meaning the range of 
//          a function) can be represented as an enumeration or as a range of
//          numbers.
//          
//  Programmer: Hank Childs
//  Creation:   March 8, 2001
//
//  Modifications:
//    Brad Whitlock, Wed Jun 20 17:08:45 PST 2001
//    I added convenience methods for returning sets from the SIL.
//
//    Eric Brugger, Thu Nov 29 12:59:30 PST 2001
//    I added const to the methods GetSILSet and GetSILCollection.
//
//    Kathleen Bonnell, Thu Aug 8 17:48:38 PDT 2002  
//    I added the method GetCollectionIndex.
//
//    Hank Childs, Thu Nov 14 15:43:28 PST 2002
//    Added concept of SIL matrices.
//
//    Kathleen Bonnell, Thu Jan 26 07:44:00 PST 2006 
//    Added int arg to GetCollectionIndex, added overloaded GetSetIndex
//    with an int arg.
//
//    Dave Bremer, Thu Dec 20 16:17:25 PST 2007
//    Added SIL arrays to hold sets that can be created using a name
//    template, and added the ability to add sets, arrays, and matrices in
//    any order.
//
//    Dave Bremer, Fri Jan 25 13:07:02 PST 2008
//    Added the methods GetSILSetID and SILSetHasMapsOut, which can give info
//    about a set without actually creating it, possibly avoiding some work.
//
//    Dave Bremer, Tue Apr  1 15:13:05 PDT 2008
//    Added tables to index sets and collections, allowing binary rather
//    than linear searches for sets and collections, and added FindSet
//    and FindColl to perform the searches.
//
//    Hank Childs, Mon Dec 14 13:33:27 PST 2009
//    Added method IsCompatible.
//
// ****************************************************************************

class DBATTS_API avtSIL
{
  public:
                                      avtSIL();
                                      avtSIL(avtSIL *);
                                      avtSIL(const SILAttributes &);
    virtual                          ~avtSIL() {;};
    const avtSIL                     &operator=(const avtSIL &);

    int                               AddWhole(avtSILSet_p);
    int                               AddSubset(avtSILSet_p);
    void                              AddArray(avtSILArray_p);
    void                              AddMatrix(avtSILMatrix_p);
    void                              AddCollection(avtSILCollection_p);

    int                               GetSetIndex(const std::string &name, int collID = -999) const;
    int                               GetCollectionIndex(std::string, int) const;

    void                              Print(ostream &) const;
    void                              Print(ostream &,
                                          std::vector< std::string > perSetInfo,
                                          std::vector< std::string > perCollInfo,
                                          std::vector< std::string > perMatrixInfo) const;

    SILAttributes                    *MakeSILAttributes(void) const;

    const std::vector<int>           &GetWholes(void) const
                                          { return wholesList; }

    avtSILSet_p                       GetSILSet(int index) const;
    avtSILSet_p                       GetSILSet(int index, bool &isTemporary) const;
    avtSILCollection_p                GetSILCollection(int index) const;

    int                               GetNumSets() const;
    int                               GetNumCollections() const;

    int                               GetSILSetID(int index) const;
    bool                              SILSetHasMapsOut(int index) const;

    bool                              IsCompatible(const avtSIL *) const;

    typedef enum { 
        WHOLE_SET, 
        SUBSET, 
        ARRAY, 
        MATRIX, 
        COLLECTION } EntryType;

  protected:
    std::vector<int>                  wholesList;

    int                               AddSet(avtSILSet_p);
    int                               GetNumRealSets() const
                                           { return static_cast<int>(sets.size()); };
    EntryType                         GetCollectionSource(int index, 
                                                          avtSILArray_p  &outArray, 
                                                          avtSILMatrix_p &outMatrix, int &outIndex);
    void                              AddMapsToTemporarySet(avtSILSet_p pSet, int setIndex) const;
    avtSILSet_p                       GetSILSetInternal(int index, bool &isTemporary, 
                                                        bool returnNullIfTemporary) const;

    bool                              FindSet(int iSet, EntryType &outType, 
                                              int &outLocalIndex,
                                              int &outLocalSubIndex) const;

    bool                              FindColl(int iColl, EntryType &outType, 
                                               int &outLocalIndex,
                                               int &outLocalSubIndex) const;


  private:
                                      avtSIL(const avtSIL &) {;};

    std::vector<avtSILSet_p>          sets;
    std::vector<avtSILMatrix_p>       matrices;
    std::vector<avtSILArray_p>        arrays;
    std::vector<avtSILCollection_p>   collections;

    std::vector<EntryType>            order; //order in which things were added.
                                             //the set and collection indices 
                                             //are inferred from this.

    std::vector<int>  setTable;  // setTable and collTable have 3 ints per table entry: global_index, type, local_index
    std::vector<int>  collTable; // and one final value giving the total count/index of the next value to add.
                                 // e.g. setTable  0, 0, 0,   set 0 is a WHOLE_SET in sets[0]
                                 //                1, 2, 0,   sets starting at 1  are in an ARRAY in arrays[0]
                                 //               41, 2, 1,   sets starting at 41 are in an ARRAY in arrays[1]
                                 //              141          the total number of sets is 141

};

#endif


