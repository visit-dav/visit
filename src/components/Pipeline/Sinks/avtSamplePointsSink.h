// ************************************************************************* //
//                          avtSamplePointsSink.h                            //
// ************************************************************************* //

#ifndef AVT_SAMPLE_POINTS_SINK_H
#define AVT_SAMPLE_POINTS_SINK_H
#include <pipeline_exports.h>


#include <avtDataObjectSink.h>
#include <avtSamplePoints.h>


// ****************************************************************************
//  Class: avtSamplePointsSink
//
//  Purpose:
//      This is a data object sink whose input is sample points.
//
//  Programmer: Hank Childs
//  Creation:   December 4, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Jun  4 08:55:55 PDT 2001
//    Inherited from avtDataObjectSink.
//
// ****************************************************************************

class PIPELINE_API avtSamplePointsSink : virtual public avtDataObjectSink
{
  public:
                                    avtSamplePointsSink();
    virtual                        ~avtSamplePointsSink() {;};

  protected:
    virtual avtDataObject_p         GetInput(void);
    virtual void                    SetTypedInput(avtDataObject_p);
    avtSamplePoints_p               GetTypedInput(void) { return input; };

  private:
    avtSamplePoints_p               input;
};


#endif


