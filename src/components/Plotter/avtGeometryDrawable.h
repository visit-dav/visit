// ************************************************************************* //
//                           avtGeometryDrawable.h                           //
// ************************************************************************* //

#ifndef AVT_GEOMETRY_DRAWABLE_H
#define AVT_GEOMETRY_DRAWABLE_H

#include <plotter_exports.h>

#include <avtDrawable.h>


class     vtkActor;
class     vtkDataObjectCollection;

class     avtMapper;


// ****************************************************************************
//  Class: avtGeometryDrawable
//
//  Purpose:
//      A concrete type of avtDrawable, this allows for adding and removing
//      actors that come from geometry to/from a renderer.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Mar 12 16:54:39 PST 2001
//    Allowed for geometry to be shifted by a vector.
//
//    Kathleen Bonnell, Thu Mar 15 10:51:59 PST 2001
//    Added member actorsVisibility to store visible state of individual actors.
//
//    Hank Childs, Sun Jul  7 14:16:38 PDT 2002
//    Add support for transparency.
//
//    Kathleen Bonnell, Fri Jul 12 16:21:37 PDT 2002
//    Added method ScaleByVector.
//
//    Kathleen Bonnell, Fri Jul 19 08:39:04 PDT 2002 
//    Added method UpdateScaleFactor.
//
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002   
//    Added methods in support of lighting. 
//
//    Brad Whitlock, Mon Sep 23 15:57:46 PST 2002
//    Added a method to set the surface representation. I added another method
//    for setting the drawable's immediate mode rendering flag.
//
// ****************************************************************************

class PLOTTER_API avtGeometryDrawable : public avtDrawable
{
  public:
                                avtGeometryDrawable(int, vtkActor **);
    virtual                    ~avtGeometryDrawable();

    void                        SetMapper(avtMapper *);

    virtual bool                Interactive(void)  { return true; };

    virtual void                Add(vtkRenderer *);
    virtual void                Remove(vtkRenderer *);

    virtual void                VisibilityOn(void);
    virtual void                VisibilityOff(void);
    virtual int                 SetTransparencyActor(avtTransparencyActor *);
    virtual int                 SetExternallyRenderedImagesActor(
                                    avtExternallyRenderedImagesActor*);

    virtual void                ShiftByVector(const float [3]);
    virtual void                ScaleByVector(const float [3]);
    virtual void                UpdateScaleFactor(void);

    virtual void                TurnLightingOn(void);
    virtual void                TurnLightingOff(void);
    virtual void                SetAmbientCoefficient(const float);

    virtual void                SetSurfaceRepresentation(int rep);
    virtual void                SetImmediateModeRendering(bool val);

    virtual avtDataObject_p     GetDataObject(void);

  protected:
    int                         nActors;
    vtkActor                  **actors;
    vtkRenderer                *renderer;
    int                        *actorsVisibility;
    avtMapper                  *mapper;
};


#endif


