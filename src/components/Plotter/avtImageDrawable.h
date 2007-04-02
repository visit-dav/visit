// ************************************************************************* //
//                            avtImageDrawable.h                             //
// ************************************************************************* //

#ifndef AVT_IMAGE_DRAWABLE_H
#define AVT_IMAGE_DRAWABLE_H

#include <plotter_exports.h>

#include <avtDrawable.h>

class     vtkActor2D;


// ****************************************************************************
//  Class: avtImageDrawable
//
//  Purpose:
//      A concrete type of drawable that allows for images to be placed in the
//      VisWindow.
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 12 16:21:37 PDT 2002
//    Added method ScaleByVector.
//
//    Kathleen Bonnell, Fri Jul 19 08:39:04 PDT 2002 
//    Added method UpdateScaleFactor.
//
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002  
//    Added methods in support of lighting. 
//
//    Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//    Removed method to set externally rendered images actor
//
// ****************************************************************************

class PLOTTER_API avtImageDrawable : public avtDrawable
{
  public:
                                 avtImageDrawable(vtkActor2D *);
    virtual                     ~avtImageDrawable();

    virtual bool                 Interactive(void)  { return false; };

    virtual void                 Add(vtkRenderer *);
    virtual void                 Remove(vtkRenderer *);

    virtual void                 VisibilityOn(void);
    virtual void                 VisibilityOff(void);
    virtual int                  SetTransparencyActor(avtTransparencyActor*)
                                     { return -1; };

    virtual void                 ShiftByVector(const double [3]);
    virtual void                 ScaleByVector(const double [3]);
    virtual void                 UpdateScaleFactor();

    virtual void                 TurnLightingOn(void);
    virtual void                 TurnLightingOff(void);
    virtual void                 SetAmbientCoefficient(const double);

    virtual avtDataObject_p      GetDataObject(void);

  protected:
    vtkActor2D                  *image;
    vtkRenderer                 *renderer;
};


#endif



