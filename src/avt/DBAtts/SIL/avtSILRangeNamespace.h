// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtSILRangeNamespace.h                            //
// ************************************************************************* //

#ifndef AVT_SIL_RANGE_NAMESPACE_H
#define AVT_SIL_RANGE_NAMESPACE_H

#include <dbatts_exports.h>

#include <avtSILNamespace.h>


// ****************************************************************************
//  Class: avtSILRangeNamespace
//
//  Purpose:
//      This is a concrete type of namespace.  This is used for when a
//      collection's maps onto a number of subsets that is problem size and
//      they can only be represented through a range.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
//  Modifications:
//
//    Dave Bremer, Wed Dec 19 12:38:52 PST 2007
//    Added code to query for an element.
//
//    Hank Childs, Thu Dec 10 14:09:42 PST 2009
//    Support new interface for getting elements.  Also add Print method.
//
// ****************************************************************************

class DBATTS_API avtSILRangeNamespace : public avtSILNamespace
{
  public:
                                     avtSILRangeNamespace(int set, int min,
                                                          int max); // valid vals >= min & <= max
    virtual                         ~avtSILRangeNamespace() {;};

    virtual int                      GetNumberOfElements(void) const
                                                { return maxRange-minRange+1; };
    virtual int                      GetElement(int idx) const
                                                { return minRange+idx; };

    virtual NamespaceAttributes     *GetAttributes(void) const;

    virtual bool                     ContainsElement(int e) const;
    virtual void                     Print(ostream &) const;

  protected:
    int                              set;
    int                              minRange;
    int                              maxRange;
};


#endif


