// ************************************************************************* //
//                        avtOriginatingNullDataSink.h                       //
// ************************************************************************* //

#ifndef AVT_ORIGINATING_NULL_DATA_SINK_H
#define AVT_ORIGINATING_NULL_DATA_SINK_H
#include <pipeline_exports.h>


#include <avtNullDataSink.h>
#include <avtOriginatingSink.h>


// ****************************************************************************
//  Class: avtOriginatingNullDataSink
//
//  Purpose:
//      A null data sink that originates pipeline execution.
//
//  Programmer: Mark C. Miller 
//  Creation:   January 7, 2003 
//
// ****************************************************************************

class PIPELINE_API avtOriginatingNullDataSink
    : virtual public avtNullDataSink, virtual public avtOriginatingSink
{
  public:
                      avtOriginatingNullDataSink()  {;};
    virtual          ~avtOriginatingNullDataSink()  {;};
};


#endif


