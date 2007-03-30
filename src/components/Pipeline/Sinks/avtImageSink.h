// ************************************************************************* //
//                             avtImageSink.h                                //
// ************************************************************************* //

#ifndef AVT_IMAGE_SINK_H
#define AVT_IMAGE_SINK_H
#include <pipeline_exports.h>


#include <avtDataObjectSink.h>
#include <avtImage.h>
#include <avtImageRepresentation.h>


// ****************************************************************************
//  Class: avtImageSink
//
//  Purpose:
//      The sink to an avt pipeline.  This could be a transient sink (filter)
//      or a terminating sink (a true end of the pipeline).
//
//  Programmer: Hank Childs
//  Creation:   December 4, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Dec 19 14:30:53 PST 2000
//    Added convenience method GetImageRep.
//
//    Hank Childs, Mon Jun  4 09:01:19 PDT 2001
//    Inherited from avtDataObjectSink.
//
// ****************************************************************************

class PIPELINE_API avtImageSink : virtual public avtDataObjectSink
{
  public:
                                    avtImageSink();
    virtual                        ~avtImageSink() {;};

  protected:
    virtual avtDataObject_p         GetInput(void);
    virtual void                    SetTypedInput(avtDataObject_p);
    avtImageRepresentation         &GetImageRep(void);
    avtImage_p                      GetTypedInput(void) { return input; };

  private:
    avtImage_p                      input;
};


#endif


