// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtFeatureEdgesFilter.h                         //
// ************************************************************************* //

#ifndef AVT_FEATURE_EDGES_FILTER_H
#define AVT_FEATURE_EDGES_FILTER_H

#include <filters_exports.h>

#include <avtDataTreeIterator.h>


// ****************************************************************************
//  Class: avtFeatureEdgesFilter
//
//  Purpose:
//      Extracts feature edges of a data representation.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 12, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Sep 10 15:31:39 PDT 2002
//    Improve the way memory is handled.  Also removed stuff for making this
//    be a plugin filter and inherited from avtDataTreeIterator.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Eric Brugger, Mon Jul 21 11:41:16 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class AVTFILTERS_API avtFeatureEdgesFilter : public avtDataTreeIterator
{
  public:
                             avtFeatureEdgesFilter();
    virtual                 ~avtFeatureEdgesFilter();

    virtual const char      *GetType(void) { return "avtFeatureEdgesFilter"; };
    virtual const char      *GetDescription(void)
                                 { return "Extracting feature edges"; };

  protected:
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void             UpdateDataObjectInfo(void);
};


#endif


