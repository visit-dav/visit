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
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtOriginatingNullDataSink
    : virtual public avtNullDataSink, virtual public avtOriginatingSink
{
  public:
                      avtOriginatingNullDataSink();
    virtual          ~avtOriginatingNullDataSink();
};


#endif


