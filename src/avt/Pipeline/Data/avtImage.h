// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Mark C. Miller, Wed Feb  4 19:47:30 PST 2004
//    Added Instance, needed by avtDataObject->Clone
//
//    Mark C. Miller, Wed Mar 31 21:10:56 PST 2004
//    Added GetSize method
//
//    Burlen Loring, Wed Aug 26 09:09:11 PDT 2015
//    Make SetImage public
//
//    Burlen Loring, Sun Sep  6 14:58:03 PDT 2015
//    Changed the return type of GetNumberOfCells to long long
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
    virtual                  ~avtImage();

    virtual const char       *GetType(void)  { return "avtImage"; };
    virtual long long         GetNumberOfCells(bool polysOnly = false) const;
    virtual avtDataObject    *Instance(void);
    virtual avtDataObjectWriter
                             *InstantiateWriter(void);
    virtual void              ReleaseData(void);
    avtImageRepresentation   &GetImage(void);
    void                      SetImage(const avtImageRepresentation &);
    virtual void              GetSize(int *width, int *height) const;
    virtual float             GetCompressionRatio() const
                                  {return image.GetCompressionRatio(); };
  protected:
    avtImageRepresentation    image;


    virtual void              DerivedCopy(avtDataObject *);
};

typedef ref_ptr<avtImage>  avtImage_p;

#endif
