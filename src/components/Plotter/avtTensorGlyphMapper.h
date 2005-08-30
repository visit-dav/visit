// ************************************************************************* //
//                            avtTensorGlyphMapper.h                         //
// ************************************************************************* //

#ifndef AVT_TENSOR_GLYPH_MAPPER_H
#define AVT_TENSOR_GLYPH_MAPPER_H

#include <plotter_exports.h>

#include <avtMapper.h>

class     vtkVisItTensorGlyph;
class     vtkLookupTable;
class     vtkPolyData;
class     vtkVisItPolyDataNormals;


// ****************************************************************************
//  Class: avtTensorGlyphMapper
//
//  Purpose:
//      A mapper for tensor.  This extends the functionality of a mapper by
//      glyphing tensors onto ellipsoids.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
//  Modifications:
//
//    Hank Childs, Wed May  5 16:23:29 PDT 2004
//    Add normals calculation.
//
//    Eric Brugger, Wed Nov 24 12:58:22 PST 2004
//    Added scaleByMagnitude and autoScale.
//
//    Kathleen Bonnell, Tue Aug 30 15:11:01 PDT 2005 
//    Use VisIt's version of TensorGlyph so that orignal zone and node
//    arrays can be copied through.
//
// ****************************************************************************

class PLOTTER_API  avtTensorGlyphMapper : public avtMapper
{
  public:
                               avtTensorGlyphMapper(vtkPolyData *);
    virtual                   ~avtTensorGlyphMapper();

    void                       ColorByMagOn(void);
    void                       ColorByMagOff(const unsigned char [3]);
    void                       SetScale(float);
    void                       SetScaleByMagnitude(bool);
    void                       SetAutoScale(bool);
    void                       SetLookupTable(vtkLookupTable *lut);

  protected:
    vtkPolyData               *glyph;
    vtkLookupTable            *lut;
    bool                       colorByMag;
    unsigned char              glyphColor[3];
    float                      scale;
    bool                       scaleByMagnitude;
    bool                       autoScale;

    vtkVisItTensorGlyph      **tensorFilter;
    vtkVisItPolyDataNormals  **normalsFilter;
    int                        nTensorFilters;

    virtual void               CustomizeMappers(void);

    virtual vtkDataSet        *InsertFilters(vtkDataSet *, int);
    virtual void               SetUpFilters(int);
};


#endif


