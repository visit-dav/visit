// ************************************************************************* //
//                                avtImage.h                                 //
// ************************************************************************* //

#ifndef AVT_IMAGE_H
#define AVT_IMAGE_H
#include <pipeline_exports.h>


#include <ref_ptr.h>

#include <avtDataObject.h>
#include <avtImageRepresentation.h>


// ****************************************************************************
//  Class: avtImage
//
//  Purpose:
//      This is another data object that avt pipelines can handle.  It is an
//      image (vtkImageData).  Further modifications may need to be made to
//      allow for multiple images as in image compositing or tiling.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Nov 21 12:08:42 PST 2001
//    Re-define DerivedCopy.
//
//    Mark C. Miller, Wed Feb  4 19:47:30 PST 2004
//    Added Instance, needed by avtDataObject->Clone
//
// ****************************************************************************

class PIPELINE_API avtImage : public avtDataObject
{
    friend class              avtImageSink;
    friend class              avtImageSource;

    // avtRayCompositer is made a friend because it is an image sink, but is
    // not declared that way to avoid multiple inheritance from sample points
    // sink and image sink.
    friend class              avtRayCompositer;

  public:
                              avtImage(avtDataObjectSource *);
    virtual                  ~avtImage() {;};

    virtual const char       *GetType(void)  { return "avtImage"; };
    virtual int               GetNumberOfCells(bool polysOnly = false) const;
    virtual avtDataObject    *Instance(void);
    virtual avtDataObjectWriter
                             *InstantiateWriter(void);
    virtual void              ReleaseData(void);
    avtImageRepresentation   &GetImage(void);

  protected:
    avtImageRepresentation    image;

    void                      SetImage(const avtImageRepresentation &);

    virtual void              DerivedCopy(avtDataObject *);
};


typedef ref_ptr<avtImage>  avtImage_p;


#endif


