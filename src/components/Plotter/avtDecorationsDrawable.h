// ************************************************************************* //
//                        avtDecorationsDrawable.h                           //
// ************************************************************************* //

#ifndef AVT_DECORATIONS_DRAWABLE_H
#define AVT_DECORATIONS_DRAWABLE_H

#include <plotter_exports.h>

#include <avtDrawable.h>
#include <avtLabelActor.h>


class     avtDecorationsMapper;


// ****************************************************************************
//  Class: avtDecorationsDrawable
//
//  Purpose:
//      A concrete type of avtDrawable, this allows for adding and removing
//      to/from a renderer the actors needed for decorations.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 12, 2002 
//
//  Modifications:
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

class PLOTTER_API avtDecorationsDrawable : public avtDrawable
{
  public:
                                avtDecorationsDrawable(std::vector<avtLabelActor_p> &);
    virtual                    ~avtDecorationsDrawable();

    void                        SetMapper(avtDecorationsMapper *);

    virtual bool                Interactive(void)  { return true; };

    virtual void                Add(vtkRenderer *);
    virtual void                Remove(vtkRenderer *);

    virtual void                VisibilityOn(void);
    virtual void                VisibilityOff(void);
    virtual int                 SetTransparencyActor(avtTransparencyActor *)
                                    { return -1; };

    virtual void                ShiftByVector(const double [3]);
    virtual void                ScaleByVector(const double [3]);
    virtual void                UpdateScaleFactor();

    virtual void                TurnLightingOn(void);
    virtual void                TurnLightingOff(void);
    virtual void                SetAmbientCoefficient(const double);

    virtual avtDataObject_p     GetDataObject(void);

  protected:
    std::vector<avtLabelActor_p> actors;
    vtkRenderer                 *renderer;
    avtDecorationsMapper        *mapper;
};


#endif


