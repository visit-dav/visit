// ************************************************************************* //
//                            avtVectorGlyphMapper.h                         //
// ************************************************************************* //

#ifndef AVT_VECTOR_GLYPH_MAPPER_H
#define AVT_VECTOR_GLYPH_MAPPER_H

#include <plotter_exports.h>

#include <avtMapper.h>
#include <LineAttributes.h>

class     vtkGlyph3D;
class     vtkLookupTable;
class     vtkPolyData;


// ****************************************************************************
//  Class: avtVectorGlyphMapper
//
//  Purpose:
//      A mapper for glyph.  This extends the functionality of a mapper by
//      mapping a glyph onto a dataset with a vector variable.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2001
//
//  Modifications:
//    Brad Whitlock, Fri Jun 15 16:00:24 PST 2001
//    Added SetLUTColors method.
//
//    Kathleen Bonnell, Mon Jun 25 12:45:06 PDT 2001 
//    Added SetLineStyle method, stipplePattern data member.
//
//    Kathleen Bonnell, Sat Aug 18 18:09:04 PDT 2001
//    Use enum types from LineAttributes.h to ensure proper
//    line width and style are sent down to vtk. 
//
//    Kathleen Bonnell, Mon Aug 20 18:19:25 PDT 2001 
//    Removed int parameter from InsertFilters, SetUpFilters as it
//    was associated with a mode no longer supported by avtMapper.
//    Also removed member glyphFilterStride, as it is no longer necessary.  
//
//    Kathleen Bonnell, Wed Aug 29 16:44:31 PDT 2001 
//    Added vtkLookupTable member and Set method.  Removed SetLUTColors. 
//    
//    Hank Childs, Wed Sep 24 09:42:29 PDT 2003
//    Renamed to vector glyph mapper.
//
// ****************************************************************************

class PLOTTER_API  avtVectorGlyphMapper : public avtMapper
{
  public:
                               avtVectorGlyphMapper(vtkPolyData *);
    virtual                   ~avtVectorGlyphMapper();

    void                       SetLineWidth(_LineWidth lw);
    void                       SetLineStyle(_LineStyle ls);
    void                       ColorByMagOn(void);
    void                       ColorByMagOff(const unsigned char [3]);
    void                       SetScale(float);
    void                       SetLookupTable(vtkLookupTable *lut);

  protected:
    vtkPolyData               *glyph;
    vtkLookupTable            *lut;
    _LineWidth                 lineWidth;
    _LineStyle                 lineStyle;
    bool                       colorByMag;
    unsigned char              glyphColor[3];
    float                      scale;

    vtkGlyph3D               **glyphFilter;
    int                        nGlyphFilters;

    virtual void               CustomizeMappers(void);

    virtual vtkDataSet        *InsertFilters(vtkDataSet *, int);
    virtual void               SetUpFilters(int);
};


#endif


