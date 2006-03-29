#ifndef AVT_MOLECULE_RENDERER_IMPLEMENTATION_H
#define AVT_MOLECULE_RENDERER_IMPLEMENTATION_H

class avtLookupTable;
class vtkPolyData;
class vtkDataArray;
class MoleculeAttributes;
struct avtViewInfo;

// ****************************************************************************
//  Class:  avtMoleculeRendererImplementation
//
//  Purpose:
//    Implements the rendering-only portion of a molecule renderer in a
//    relatively stateless manner.  Meant to be instantiated at render
//    time by avtMoleculeRenderer::Render, though it can be kept around
//    across renderers while the implementation itself has not changed.
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  3, 2006
//
// ****************************************************************************
class avtMoleculeRendererImplementation
{
  public:
                   avtMoleculeRendererImplementation() {}
    virtual       ~avtMoleculeRendererImplementation() {}
    virtual void   Render(vtkPolyData *data, const MoleculeAttributes&,
                          bool immediateModeRendering,
                          float varmin, float varmax,
                          float ambient_coeff,
                          float spec_coeff, float spec_power,
                          float spec_r, float spec_g, float spec_b) = 0;
    virtual void   InvalidateColors() { };
    virtual void   SetLevelsLUT(avtLookupTable *) = 0;
private:
    avtLookupTable *levelsLUT;
};

#endif
