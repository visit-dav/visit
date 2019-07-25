// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtSILNamespace.h                             //
// ************************************************************************* //

#ifndef AVT_SIL_NAMESPACE_H
#define AVT_SIL_NAMESPACE_H

#include <dbatts_exports.h>

#include <vector>

#include <visitstream.h>

class     NamespaceAttributes;


// ****************************************************************************
//  Class: avtSILNamespace
//
//  Purpose:
//      A namespace of the range of a collection.  Its purpose is to try and
//      provide a abstract base type that allows for SILs and other objects to
//      not worry about what the range of a collection is.  Derived types
//      would handle enumerated namespaces (where every subset out of a
//      collection can be enumerated in a non-problem size manner) and
//      range namespaces (where the number of subsets is proportional to the
//      problem size), for example.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Jan  9 15:37:41 PST 2002
//    Add virtual destructor to clean up memory leaks.
//
//    Dave Bremer, Wed Dec 19 12:38:52 PST 2007
//    Added code to query for an element.
//
//    Hank Childs, Thu Dec 10 14:01:43 PST 2009
//    Change interface so it doesn't require making a big array.
//    Also add a Print method.
//
// ****************************************************************************

class DBATTS_API avtSILNamespace
{
  public:
    virtual                         ~avtSILNamespace() {;};

    virtual void                     Print(ostream &) const = 0;
    virtual int                      GetNumberOfElements(void) const = 0;
    virtual int                      GetElement(int) const = 0;
    virtual NamespaceAttributes     *GetAttributes(void) const = 0;
    virtual bool                     ContainsElement(int e) const = 0;
    static avtSILNamespace          *GetNamespace(const NamespaceAttributes *);
};


#endif


