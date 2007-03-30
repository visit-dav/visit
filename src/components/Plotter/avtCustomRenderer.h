// ************************************************************************* //
//                             avtCustomRenderer.h                           //
// ************************************************************************* //

#ifndef AVT_CUSTOM_RENDERER_H
#define AVT_CUSTOM_RENDERER_H

#include <plotter_exports.h>

#include <ref_ptr.h>

#include <avtDataObject.h>
#include <avtViewInfo.h>

class     vtkDataSet;
class     vtkRenderer;

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
// ****************************************************************************

class PLOTTER_API avtCustomRenderer
{
  public:
                            avtCustomRenderer();
    virtual                ~avtCustomRenderer();

    void                    Execute(vtkDataSet *);
    void                    SetView(avtViewInfo &);

    void                    SetRange(float, float);

    virtual bool            OperatesOnScalars(void) { return false; };

    virtual void            GlobalLightingOn(void);
    virtual void            GlobalLightingOff(void);
    virtual void            GlobalSetAmbientCoefficient(const float); 

    void                    RegisterOverrideRenderCallback(
                                               OverrideRenderCallback, void *);
    void                    SetVTKRenderer(vtkRenderer *r);

  protected:
    avtViewInfo             view;
    float                   varmin, varmax;
    vtkRenderer            *VTKRen;

    virtual void            Render(vtkDataSet *) = 0;

    OverrideRenderCallback  overrideRenderCallback;
    void                   *overrideRenderCallbackArgs;
};


typedef ref_ptr<avtCustomRenderer> avtCustomRenderer_p;


#endif


