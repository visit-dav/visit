// ************************************************************************* //
//                            avtMoleculeRenderer.h                          //
// ************************************************************************* //

#ifndef AVT_MOLECULE_RENDERER_H
#define AVT_MOLECULE_RENDERER_H

#include <avtCustomRenderer.h>
#include <MoleculeAttributes.h>

class vtkDataArray;
class avtLookupTable;
class avtMoleculeRendererImplementation;

// ****************************************************************************
//  Class: avtMoleculeRenderer
//
//  Purpose:
//      An implementation of an avtCustomRenderer for a molecule plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  3, 2006
//
//  Modifications:
//
// ****************************************************************************

class avtMoleculeRenderer : public avtCustomRenderer
{
  public:
                            avtMoleculeRenderer();
    virtual                ~avtMoleculeRenderer();
    static avtMoleculeRenderer *New(void);

    void                    SetAtts(const AttributeGroup*);

    void                    InvalidateColors();

    virtual void            ReleaseGraphicsResources();
    virtual void            Render(vtkDataSet *);

    std::string GetPrimaryVariable() { return primaryVariable; }
    std::string primaryVariable;

    void SetLevelsLUT(avtLookupTable *);

  protected:
    avtMoleculeRendererImplementation *rendererImplementation;
    bool                               currentRendererIsValid;

    MoleculeAttributes        atts;

    void                    Initialize(vtkDataSet*);
    bool                    initialized;


    // lighting props
    virtual void GlobalLightingOn();
    virtual void GlobalLightingOff();
    virtual void GlobalSetAmbientCoefficient(const float);
    virtual void SetSpecularProperties(bool,float,float,
                                       const ColorAttribute&);

    float ambient_coeff;
    float spec_coeff;
    float spec_power;
    float spec_r, spec_g, spec_b;
    avtLookupTable *levelsLUT;
};


typedef ref_ptr<avtMoleculeRenderer> avtMoleculeRenderer_p;


#endif


