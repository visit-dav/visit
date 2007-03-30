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
// ****************************************************************************

class PIPELINE_API avtOriginatingImageSink
    : virtual public avtImageSink, virtual public avtOriginatingSink
{
  public:
                      avtOriginatingImageSink()  {;};
    virtual          ~avtOriginatingImageSink()  {;};
};


#endif


