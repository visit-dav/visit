// ************************************************************************* //
//                                 avtActor.h                                //
// ************************************************************************* //

#ifndef AVT_ACTOR_H
#define AVT_ACTOR_H

#include <plotter_exports.h>

#include <ref_ptr.h>

#include <avtBehavior.h>
#include <avtDataObject.h>
#include <avtDrawable.h>
#include <VisWindowTypes.h>

class     vtkRenderer;

class     avtTransparencyActor;


// ****************************************************************************
//  Class: avtActor
//
//  Purpose:
//      This is a class that is output by an avtPlot that can be inputted to
//      a VisWindow.  An actor is a drawable with a behavior.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Mar  8 10:31:46 PST 2001
//    Allows for different renderers for the decoration and the actual
//    rendering.
//
//    Kathleen Bonnell, Tue Apr  3 15:24:00 PDT 2001 
//    Add method to retrieve renderOrder.
//
//    Kathleen Bonnell, Tue May  7 09:36:15 PDT 2002 
//    Add method GetDataExtents. 
//
//    Hank Childs, Sun Jul  7 12:55:05 PDT 2002
//    Added support for transparency.
//
//    Kathleen Bonnell, Fri Jul 12 16:20:08 PDT 2002 
//    Added support for decorations.
//
//    Kathleen Bonnell, Fri Jul 19 08:39:04 PDT 2002 
//    Added UpdateScaleFactor. 
//
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002 
//    Added methods in support of lighting.
//
//    Brad Whitlock, Mon Sep 23 15:50:38 PST 2002
//    I added a method to set the actor's surface representation and another
//    method to set its immediate mode rendering flag.
//
//    Mark C. Miller, Thu Dec 19 16:19:23 PST 2002
//    Added support for externally rendered images
//
//    Eric Brugger, Wed Aug 20 09:49:33 PDT 2003
//    Added GetWindowMode.
//
//    Kathleen Bonnell, Mon Sep 29 13:44:01 PDT 2003 
//    Added bool arg to GetRenderOrder. 
//
//    Jeremy Meredith, Fri Nov 14 11:10:53 PST 2003
//    Added a method to set the specular properties.
//
//    Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//    Elminated externally rendered images actor dependence
//
// ****************************************************************************

class PLOTTER_API avtActor
{
  public:
                                  avtActor();
    virtual                      ~avtActor();

    void                          SetBehavior(avtBehavior_p);
    avtBehavior_p                 GetBehavior(void) { return behavior; };
    void                          SetDrawable(avtDrawable_p);
    void                          SetDecorations(avtDrawable_p);

    void                          Add(vtkRenderer *, vtkRenderer *);
    void                          Remove(vtkRenderer *, vtkRenderer *);

    void                          GetActualBounds(float [6]);
    void                          GetOriginalBounds(float [6]);
    void                          GetDataExtents(float &dmin, float &dmax);
    int                           GetDimension(void);
    WINDOW_MODE                   GetWindowMode(void);
    int                           GetRenderOrder(bool aa);
    avtLegend_p                   GetLegend(void);

    void                          ShiftByVector(const float [3]);
    void                          ScaleByVector(const float [3]);
    void                          UpdateScaleFactor();

    void                          VisibilityOn(void);
    void                          VisibilityOff(void);
    void                          SetTransparencyActor(avtTransparencyActor *);

    void                          TurnLightingOn(void);
    void                          TurnLightingOff(void);
    void                          SetAmbientCoefficient(const float);

    void                          SetSurfaceRepresentation(int rep);
    void                          SetImmediateModeRendering(bool val);

    void                          SetSpecularProperties(bool,float,float,
                                                        const ColorAttribute&);

    avtDataObject_p               GetDataObject(void);

  protected:
    avtBehavior_p                 behavior;
    avtDrawable_p                 drawable;
    avtDrawable_p                 decorations;
    avtTransparencyActor         *transparencyActor;
    int                           transparencyIndex;

    vtkRenderer                  *renderer;
};


typedef ref_ptr<avtActor> avtActor_p;


#endif


