// ************************************************************************* //
//                             avtSILNamespace.h                             //
// ************************************************************************* //

#ifndef AVT_SIL_NAMESPACE_H
#define AVT_SIL_NAMESPACE_H
#include <dbatts_exports.h>


#include <vector>

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
// ****************************************************************************

class DBATTS_API avtSILNamespace
{
  public:
    virtual                         ~avtSILNamespace() {;};

    virtual const std::vector<int>  &GetAllElements(void) const = 0;
    virtual NamespaceAttributes     *GetAttributes(void) const = 0;
    static avtSILNamespace          *GetNamespace(const NamespaceAttributes *);
};


#endif


