// ************************************************************************* //
//                               avtImageWriter.h                            //
// ************************************************************************* //

#ifndef AVT_IMAGE_WRITER_H
#define AVT_IMAGE_WRITER_H
#include <pipeline_exports.h>


#include <avtOriginatingImageSink.h>
#include <avtDataObjectWriter.h>


class     avtDataObjectString;


// ****************************************************************************
//  Class: avtImageWriter
//
//  Purpose:
//      A class which takes as input an avtImage and can serialize it.
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 18, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Dec 28 16:33:13 PST 2000
//    Pulled out some functionality in favor of avtDataObjectWriter.
//
//    Hank Childs, Mon Oct  1 09:28:56 PDT 2001
//    Re-worked inheritance hierarchy.
//
// ****************************************************************************

class PIPELINE_API avtImageWriter : public avtOriginatingImageSink,
                       public avtDataObjectWriter
{
  public:
                       avtImageWriter() {;};
    virtual           ~avtImageWriter() {;};

  protected:
    void               DataObjectWrite(avtDataObjectString &);
};


#endif


