// ************************************************************************* //
//                            avtTensorGlyphMapper.h                         //
// ************************************************************************* //

#ifndef AVT_TENSOR_GLYPH_MAPPER_H
#define AVT_TENSOR_GLYPH_MAPPER_H

#include <plotter_exports.h>

#include <avtMapper.h>

class     vtkTensorGlyph;
class     vtkLookupTable;
class     vtkPolyData;


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
// ****************************************************************************

class PLOTTER_API  avtTensorGlyphMapper : public avtMapper
{
  public:
                               avtTensorGlyphMapper(vtkPolyData *);
    virtual                   ~avtTensorGlyphMapper();

    void                       ColorByMagOn(void);
    void                       ColorByMagOff(const unsigned char [3]);
    void                       SetScale(float);
    void                       SetLookupTable(vtkLookupTable *lut);

  protected:
    vtkPolyData               *glyph;
    vtkLookupTable            *lut;
    bool                       colorByMag;
    unsigned char              glyphColor[3];
    float                      scale;

    vtkTensorGlyph           **tensorFilter;
    int                        nTensorFilters;

    virtual void               CustomizeMappers(void);

    virtual vtkDataSet        *InsertFilters(vtkDataSet *, int);
    virtual void               SetUpFilters(int);
};


#endif


