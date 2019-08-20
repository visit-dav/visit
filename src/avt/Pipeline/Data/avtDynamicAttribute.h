// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtDynamicAttribute.h                           //
// ************************************************************************* //

#ifndef AVT_DYNAMIC_ATTRIBUTE_H
#define AVT_DYNAMIC_ATTRIBUTE_H
#include <pipeline_exports.h>


#include <avtDataObject.h>
#include <avtContract.h>


// ****************************************************************************
//  Class: avtDynamicAttribute
//
//  Purpose:
//      Contains attributes that may be dynamically resolved.  This is a base
//      type that provides an interface.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtDynamicAttribute
{
  public:
                               avtDynamicAttribute();
    virtual                   ~avtDynamicAttribute();

    virtual avtContract_p
                               ModifyContract(avtContract_p);
    virtual void               ResolveAttributes(avtDataObject_p) = 0;
};


#endif


