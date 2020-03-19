// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtSILEnumeratedNamespace.h                       //
// ************************************************************************* //

#ifndef AVT_SIL_ENUMERATED_NAMESPACE_H
#define AVT_SIL_ENUMERATED_NAMESPACE_H

#include <dbatts_exports.h>

#include <avtSILNamespace.h>


// ****************************************************************************
//  Class: avtSILEnumeratedNamespace
//
//  Purpose:
//      This is a concrete type of namespace.  This is used for when a
//      collections maps onto a number of subsets that can be reasonably
//      enumerated.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
//  Modifications:
//
//    Dave Bremer, Wed Dec 19 12:38:52 PST 2007
//    Added code to query for an element.
// 
//    Hank Childs, Thu Dec 10 14:01:43 PST 2009
//    Change interface for getting elements.  Also add Print method.
//
// ****************************************************************************

class DBATTS_API avtSILEnumeratedNamespace : public avtSILNamespace
{
  public:
                        avtSILEnumeratedNamespace(const std::vector<int> &);
    virtual            ~avtSILEnumeratedNamespace() {;};

    virtual int                      GetNumberOfElements(void) const
                                                 { return static_cast<int>(elements.size()); };
    virtual int                      GetElement(int idx) const
                                                 { return elements[idx]; };
    virtual NamespaceAttributes     *GetAttributes(void) const;
    virtual bool                     ContainsElement(int e) const;
    virtual void                     Print(ostream &) const;

  protected:
    std::vector<int>    elements;
    bool                sequentialElems;
    bool                sortedElems;
};


#endif


