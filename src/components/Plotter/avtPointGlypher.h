// ************************************************************************* //
//                           avtPointGlypher.h                               //
// ************************************************************************* //

#ifndef AVT_POINT_GLYPHER_H
#define AVT_POINT_GLYPHER_H

#include <plotter_exports.h>
#include <string>

class     vtkDataSet;
class     vtkPolyData;
class     vtkVisItGlyph3D;
class     vtkVisItPolyDataNormals;


// ****************************************************************************
//  Class: avtPointGlypher
//
//  Purpose:
//    Turns points into glyphs.  
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 19, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 12 09:14:16 PST 2004
//    Renamed from avtPointGlyphMapper, no longer derived from 
//    avtVariableMapper, modified some method names so that they don't collide
//    with avtMapper method names.
//
// ****************************************************************************

class PLOTTER_API  avtPointGlypher  
{
  public:
                               avtPointGlypher();
    virtual                   ~avtPointGlypher();

    virtual void               ScaleByVar(const std::string &) = 0;
    void                       DataScalingOff(void);

    void                       SetScale(float);
    void                       SetGlyphType(const int);

    void                       ColorByScalarOn(const std::string &);
    void                       ColorByScalarOff(void);

  protected:
    vtkPolyData               *glyph2D;
    vtkPolyData               *glyph3D;
    float                      scale;
    std::string                scalingVarName;
    std::string                coloringVarName;
    int                        scalingVarDim;
    int                        glyphType;
    bool                       dataScaling;
    bool                       colorByScalar;

    vtkVisItGlyph3D          **glyphFilter;
    vtkVisItPolyDataNormals  **normalsFilter;
    int                        nGlyphFilters;

    virtual vtkDataSet        *InsertGlyphs(vtkDataSet *, int, int);
    virtual void               SetUpGlyphs(int);
    virtual void               CustomizeGlyphs(int);

    void                       DataScalingOn(const std::string &, int = 1);

  private:
    int                        spatialDim;
    vtkPolyData               *GetGlyphSource(void);
    void                       ClearGlyphs(void);
    void                       SetUpGlyph(void);
};


#endif


