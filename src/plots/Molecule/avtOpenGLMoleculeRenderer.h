// ************************************************************************* //
//                         avtOpenGLMoleculeRenderer.h                       //
// ************************************************************************* //

#ifndef AVT_OPEN_GL_MOLECULE_RENDERER_H
#define AVT_OPEN_GL_MOLECULE_RENDERER_H

#include <avtMoleculeRendererImplementation.h>
#include <MoleculeAttributes.h>
#include <string>

#define MAX_DETAIL_LEVELS 4

// ****************************************************************************
//  Class: avtOpenGLMoleculeRenderer
//
//  Purpose:
//      An implementation of a molecule renderer that uses OpenGL calls.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  3, 2006
//
//  Modifications:
//    Brad Whitlock, Mon Mar 27 16:27:18 PST 2006
//    Added support for imposter rendering.
//
// ****************************************************************************

class avtOpenGLMoleculeRenderer : public avtMoleculeRendererImplementation
{
  public:
                            avtOpenGLMoleculeRenderer();
    virtual                ~avtOpenGLMoleculeRenderer();

  protected:
    virtual void            Render(vtkPolyData *data,
                                   const MoleculeAttributes&,
                                   bool immediateModeRendering,
                                   float varmin, float varmax,
                                   float ambient_coeff,
                                   float spec_coeff, float spec_power,
                                   float spec_r, float spec_g, float spec_b);

    virtual void   InvalidateColors();

    virtual void   SetLevelsLUT(avtLookupTable *);

    bool  immediatemode;
    unsigned int displaylistid;
    MoleculeAttributes atts_for_displaylist;

    float varmin;
    float varmax;

    float ambient_coeff;
    float spec_coeff;
    float spec_power;
    float spec_r;
    float spec_g;
    float spec_b;
    avtLookupTable *levelsLUT;

    float *sphere_pts[MAX_DETAIL_LEVELS];
    float *cyl_pts[MAX_DETAIL_LEVELS];

    bool spheres_calculated;
    void CalculateSpherePts();
    void DrawSphereAsQuads(float, float, float, float r, int);

    bool cylinders_calculated;
    void CalculateCylPts();
    void DrawCylinderBetweenTwoPoints(float*, float *, float r, int);

    int            numcolors;
    unsigned char *colors;
    std::string    colortablename;
    bool           ct_is_discrete;

    void SetColors(vtkPolyData *data,
                   const MoleculeAttributes&);

    void DrawAtomsAsSpheres(vtkPolyData *data,
                            const MoleculeAttributes&);

    void DrawBonds(vtkPolyData *data,
                   const MoleculeAttributes&);

    void *tex;
};


#endif


