// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     avtOriginatingNullDataSource.h                        //
// ************************************************************************* //

#ifndef AVT_ORIGINATING_NULL_DATA_SOURCE_H
#define AVT_ORIGINATING_NULL_DATA_SOURCE_H
#include <pipeline_exports.h>


#include <avtNullDataSource.h>
#include <avtOriginatingSource.h>


// ****************************************************************************
//  Class: avtOriginatingNullDataSource
//
//  Purpose:
//      A source that originates a pipeline.  It does an update differently
//      than what a non-originating source (filter) would.
//
//  Programmer: Mark C. Miller
//  Creation:   January 8, 2003 
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtOriginatingNullDataSource
    : public virtual avtNullDataSource, public virtual avtOriginatingSource
{
  public:
                                 avtOriginatingNullDataSource();
    virtual                     ~avtOriginatingNullDataSource();

  protected:
    virtual bool                 FetchData(avtDataRequest_p) 
                                    { return false; };
};


#endif


