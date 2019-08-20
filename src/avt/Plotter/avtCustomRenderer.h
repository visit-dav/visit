// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtCustomRenderer.h                           //
// ************************************************************************* //

#ifndef AVT_CUSTOM_RENDERER_H
#define AVT_CUSTOM_RENDERER_H

#include <plotter_exports.h>

#include <ref_ptr.h>

#include <avtDataObject.h>
#include <avtViewInfo.h>

class     ColorAttribute;
class     vtkDataSet;
class     vtkRenderer;
class     vtkActor;

typedef void (*OverrideRenderCallback)(void *, avtDataObject_p &);


// ****************************************************************************
//  Class: avtCustomRenderer
//
//  Purpose:
//      An interface that any custom renderer should derive from.
//
//  Programmer: Hank Childs
//  Creation:   March 26, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Nov 19 15:31:00 PST 2001
//    Added hooks for setting extents.
//
//    Hank Childs, Tue Apr 23 18:25:55 PDT 2002
//    Renamed from OpenGL renderer, since it now has derived types that are
//    not OpenGL.
//
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002  
//    Added methods in support of lighting.
//
//    Kathleen Bonnell, Mon Aug  4 11:14:22 PDT 2003 
//    Added member and methods in support of immediate mode rendering.
//
//    Kathleen Bonnell,  Thu Sep  2 11:44:09 PDT 2004
//    Added SetSurfaceRepresentation and SetSpecularProperties. 
//
//    Brad Whitlock, Wed Aug 22 11:38:46 PDT 2007
//    Added reduced detail mode.
//
//    Hank Childs, Thu Sep 30 00:39:47 PDT 2010
//    Add methods for setting the bbox.
//
//    Carson Brownlee, Fri Jul 27 13:54:29 PDT 2012
//    Add method for setting the actor.
//
// ****************************************************************************

class PLOTTER_API avtCustomRenderer
{
  public:
                            avtCustomRenderer();
    virtual                ~avtCustomRenderer();

    void                    Execute(vtkDataSet *);
    void                    SetView(avtViewInfo &);

    void                    SetRange(double, double);
    void                    SetBoundingBox(const double *);

    virtual bool            OperatesOnScalars(void) { return false; };
    virtual bool            NeedsBoundingBox(void) { return false; };

    virtual void            GlobalLightingOn(void);
    virtual void            GlobalLightingOff(void);
    virtual void            GlobalSetAmbientCoefficient(const double); 

    virtual void            SetSurfaceRepresentation(int rep);
    virtual void            SetSpecularProperties(bool,double,double,
                                                  const ColorAttribute&);


    void                    RegisterOverrideRenderCallback(
                                               OverrideRenderCallback, void *);
    void                    SetVTKRenderer(vtkRenderer *r);
    void                    SetVTKActor(vtkActor *a);
    virtual void            SetAlternateDisplay(void *dpy);

    virtual void            ReducedDetailModeOn() {; }
    virtual bool            ReducedDetailModeOff() { return false; }

  protected:
    avtViewInfo             view;
    double                  varmin, varmax;
    double                  bbox[6];
    vtkRenderer            *VTKRen;
    vtkActor               *VTKActor;

    virtual void            Render(vtkDataSet *) = 0;

    OverrideRenderCallback  overrideRenderCallback;
    void                   *overrideRenderCallbackArgs;
};


typedef ref_ptr<avtCustomRenderer> avtCustomRenderer_p;


#endif


