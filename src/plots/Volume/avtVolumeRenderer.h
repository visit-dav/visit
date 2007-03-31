// ************************************************************************* //
//                             avtVolumeRenderer.h                           //
// ************************************************************************* //

#ifndef AVT_VOLUME_RENDERER_H
#define AVT_VOLUME_RENDERER_H

#include <avtCustomRenderer.h>
#include <VolumeAttributes.h>

class vtkDataArray;
class avtVolumeRendererImplementation;

// ****************************************************************************
//  Class: avtVolumeRenderer
//
//  Purpose:
//      An implementation of an avtCustomRenderer for a volume plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 27, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Nov 19 08:07:40 PST 2001
//    Added a convenience routine to retrieve the variables from a dataset.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Wed Apr 24 09:16:51 PDT 2002 
//    Added derived types for graphics packages.
//
//    Jeremy Meredith, Tue Sep 30 11:49:42 PDT 2003
//    Added method "ReleaseGraphicsResources".  Moved alphatex to subclass.
//
//    Jeremy Meredith, Thu Oct  2 13:13:23 PDT 2003
//    Made this class be a concrete implementation of a custom renderer.
//    It will chose between actual rendering methods by instantiating an
//    avtVolumeRendererImplementation at render time.
//
// ****************************************************************************

class avtVolumeRenderer : public avtCustomRenderer
{
  public:
                            avtVolumeRenderer();
    virtual                ~avtVolumeRenderer();
    static avtVolumeRenderer *New(void);

    void                    SetAtts(const AttributeGroup*);

    virtual bool            OperatesOnScalars(void) { return true; };
    virtual void            ReleaseGraphicsResources();
    virtual void            Render(vtkDataSet *);

  protected:
    avtVolumeRendererImplementation  *rendererImplementation;
    bool                              currentRendererIsValid;

    VolumeAttributes        atts;

    void                    Initialize(vtkDataSet*);
    bool                    initialized;

    float                   vmin,vmax,vsize;
    float                   omin,omax,osize;
    float                  *gx, *gy, *gz, *gm, *gmn;

    bool                    GetScalars(vtkDataSet*,vtkDataArray*&,vtkDataArray *&);
    void                    GetRange(vtkDataArray *, float &, float &);
};


typedef ref_ptr<avtVolumeRenderer> avtVolumeRenderer_p;


#endif


