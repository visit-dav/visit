// ************************************************************************* //
//                                avtImageMapper.h                           //
// ************************************************************************* //

#ifndef AVT_IMAGE_MAPPER_H
#define AVT_IMAGE_MAPPER_H
#include <plotter_exports.h>


#include <avtDrawable.h>
#include <avtImageSink.h>

class     vtkActor2D;
class     vtkImageMapper;


// ****************************************************************************
//  Class: avtImageMapper
//
//  Purpose:
//      Maps an image into a drawable.
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
// ****************************************************************************

class PLOTTER_API avtImageMapper : public avtImageSink
{
  public:
                              avtImageMapper();
    virtual                  ~avtImageMapper();

    avtDrawable_p             GetDrawable() { return drawable; };
    void                      Draw(vtkRenderer *);

  protected:
    avtDrawable_p             drawable;

    vtkImageMapper           *mapper;
    vtkActor2D               *actor;

    virtual void              ChangedInput(void);
};


#endif


