// ************************************************************************* //
//                         avtSourceFromAVTImage.h                           //
// ************************************************************************* //

#ifndef AVT_SOURCE_FROM_AVT_IMAGE_H
#define AVT_SOURCE_FROM_AVT_IMAGE_H
#include <pipeline_exports.h>


#include <avtInlinePipelineSource.h>
#include <avtTerminatingImageSource.h>

class avtImageRepresentation;

// ****************************************************************************
//  Class: avtSourceFromAVTImage
//
//  Purpose:
//      A source object (pipeline terminator) that is created from an AVT
//      dataset.
//
//  Programmer: Mark C. Miller 
//  Creation:   26Feb03 
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtSourceFromAVTImage : virtual public avtTerminatingImageSource,
                                virtual public avtInlinePipelineSource
{
  public:
                          avtSourceFromAVTImage(avtImage_p ds);
    virtual              ~avtSourceFromAVTImage();

  protected:
    avtImageRepresentation            image;

    virtual bool          FetchImage(avtDataSpecification_p spec,
                             avtImageRepresentation &imageRep);
};


#endif


