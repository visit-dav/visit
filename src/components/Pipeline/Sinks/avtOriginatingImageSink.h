// ************************************************************************* //
//                           avtOriginatingImageSink.h                       //
// ************************************************************************* //

#ifndef AVT_ORIGINATING_IMAGE_SINK_H
#define AVT_ORIGINATING_IMAGE_SINK_H
#include <pipeline_exports.h>


#include <avtImageSink.h>
#include <avtOriginatingSink.h>


// ****************************************************************************
//  Class: avtOriginatingImageSink
//
//  Purpose:
//      A image sink that originates pipeline execution.
//
//  Programmer: Hank Childs
//  Creation:   May 30, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtOriginatingImageSink
    : virtual public avtImageSink, virtual public avtOriginatingSink
{
  public:
                      avtOriginatingImageSink();
    virtual          ~avtOriginatingImageSink();
};


#endif


