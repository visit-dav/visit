// ************************************************************************* //
//                            avtNullDataWriter.h                            //
// ************************************************************************* //

#ifndef AVT_NULL_DATA_WRITER_H
#define AVT_NULL_DATA_WRITER_H
#include <pipeline_exports.h>


#include <avtOriginatingNullDataSink.h>
#include <avtDataObjectWriter.h>


class     avtDataObjectString;


// ****************************************************************************
//  Class: avtNullDataWriter
//
//  Purpose:
//      A class which takes as input an avtNullData and can serialize it.
//
//  Programmer: Mark C. Miller 
//  Creation:   January 7, 2003 
//
// ****************************************************************************

class PIPELINE_API avtNullDataWriter : public avtOriginatingNullDataSink,
                       public avtDataObjectWriter
{
  public:
                       avtNullDataWriter() {;};
    virtual           ~avtNullDataWriter() {;};

    //virtual bool       MustMergeParallelStreams(void) { return true; };

  protected:
    void               DataObjectWrite(avtDataObjectString &);
};


#endif


