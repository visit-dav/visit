// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtVertexNormalsFilter.h                          //
// ************************************************************************* //

#ifndef AVT_VERTEX_NORMALS_FILTER_H
#define AVT_VERTEX_NORMALS_FILTER_H

#include <filters_exports.h>

#include <avtDataTreeIterator.h>


// ****************************************************************************
//  Class: avtVertexNormalsFilter
//
//  Purpose:
//      Calculates vertex normals for a dataset. 
//
//  Programmer: Hank Childs
//  Creation:   December 31, 2001
//
//  Modifications:
//    Hank Childs, Tue Sep 10 15:17:21 PDT 2002
//    Removed data members to conform with new memory management paradigm.
//
//    Jeremy Meredith, Tue Aug 12 10:55:35 PDT 2003
//    Added ability to have cell normals.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Thu Feb 26 09:48:39 PST 2004
//    Decide whether or not to do point normals internally to this filter
//    (removed interface so that this could be decided externally).
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//    Eric Brugger, Tue Jul 22 12:19:04 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
//    Alister Maguire, Mon Apr 27 11:41:06 PDT 2020
//    Added UpdateDataObjectInfo and zonesHaveBeenSplit.
//
// ****************************************************************************

class AVTFILTERS_API avtVertexNormalsFilter : public avtDataTreeIterator
{
  public:
                         avtVertexNormalsFilter();
    virtual             ~avtVertexNormalsFilter();

    virtual const char  *GetType(void) { return "avtVertexNormalsFilter"; };
    virtual const char  *GetDescription(void) 
                             { return "Calculating normals"; };

  protected:
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual bool                   FilterUnderstandsTransformedRectMesh();
    virtual void                   UpdateDataObjectInfo(void);

    bool zonesHaveBeenSplit;
};


#endif


