// ************************************************************************* //
//                           avtPointGlyphMapper.h                           //
// ************************************************************************* //

#ifndef AVT_POINT_GLYPH_MAPPER_H
#define AVT_POINT_GLYPH_MAPPER_H

#include <plotter_exports.h>

#include <avtVariableMapper.h>

class     vtkVisItGlyph3D;
class     vtkPolyData;
class     vtkVisItPolyDataNormals;


// ****************************************************************************
//  Class: avtPointGlyphMapper
//
//  Purpose:
//      A mapper for glyph.  This extends the functionality of a mapper by
//      mapping a glyph onto a dataset with a scalar variable.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 19, 2004 
//
//  Modifications:
//
// ****************************************************************************

class PLOTTER_API  avtPointGlyphMapper : public avtVariableMapper
{
  public:
                               avtPointGlyphMapper();
    virtual                   ~avtPointGlyphMapper();

    void                       SetScale(float);
    void                       SetGlyphType(const int);
    void                       DataScalingOff(void);
    void                       DataScalingOn(const std::string &);

    void                       ColorByScalarOn(const std::string &);
    void                       ColorByScalarOff(const unsigned char [3]);

  protected:
    vtkPolyData               *glyph2D;
    vtkPolyData               *glyph3D;
    float                      scale;
    std::string                scalingVarName;
    std::string                coloringVarName;
    int                        glyphType;
    bool                       dataScaling;
    bool                       colorByScalar;
    unsigned char              glyphColor[3];

    vtkVisItGlyph3D          **glyphFilter;
    vtkVisItPolyDataNormals  **normalsFilter;
    int                        nGlyphFilters;

    virtual void               CustomizeMappers(void);

    virtual vtkDataSet        *InsertFilters(vtkDataSet *, int);
    virtual void               SetUpFilters(int);

  private:
    vtkPolyData               *GetGlyphSource(void);
    void                       ClearGlyphs(void);
    void                       SetUpGlyph(void);

};


#endif


