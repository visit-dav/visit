// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtOriginatingImageSource.h                        //
// ************************************************************************* //

#ifndef AVT_ORIGINATING_IMAGE_SOURCE_H
#define AVT_ORIGINATING_IMAGE_SOURCE_H

#include <pipeline_exports.h>

#include <avtImageSource.h>
#include <avtOriginatingSource.h>


// ****************************************************************************
//  Class: avtOriginatingImageSource
//
//  Purpose:
//      A source that originates a pipeline.  It does an update differently
//      than what a non-originating source (filter) would.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 18, 2000 
//
//  Modifications:
//
//    Hank Childs, Mon Jun  4 08:02:35 PDT 2001
//    Changed inheritance hierarchy.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtOriginatingImageSource
    : public virtual avtImageSource, public virtual avtOriginatingSource
{
  public:
                                 avtOriginatingImageSource();
    virtual                     ~avtOriginatingImageSource();

  protected:
    virtual bool                 FetchData(avtDataRequest_p);
    virtual bool                 FetchImage(avtDataRequest_p,
                                                 avtImageRepresentation &) = 0;
};


#endif


