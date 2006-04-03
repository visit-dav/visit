// ************************************************************************* //
//                      avtVariablePointGlyphMapper.h                        //
// ************************************************************************* //

#ifndef AVT_VARIABLE_POINT_GLYPH_MAPPER_H
#define AVT_VARIABLE_POINT_GLYPH_MAPPER_H

#include <plotter_exports.h>

#include <avtVariableMapper.h>
#include <avtPointGlypher.h>


// ****************************************************************************
//  Class: avtVariablePointGlyphMapper
//
//  Purpose:
//      A mapper for glyph.  This extends the functionality of a mapper by
//      mapping a glyph onto a dataset with a scalar variable.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 12, 2004 
//
//  Modifications:
//    Brad Whitlock, Fri Jul 22 11:21:47 PDT 2005
//    Added an override for the SetGlyphType method that lets us switch
//    mapper inputs when we enter of leave point glyphing mode.
//
// ****************************************************************************

class PLOTTER_API  avtVariablePointGlyphMapper : virtual public avtVariableMapper,
                                                 virtual public avtPointGlypher
{
  public:
                               avtVariablePointGlyphMapper();
    virtual                   ~avtVariablePointGlyphMapper();

    void                       ColorBySingleColor(const unsigned char [3]);
    void                       ColorBySingleColor(const double [3]);
    virtual void               ScaleByVar(const std::string &);
    void                       SetGlyphType(const int type);

  protected:
    double                     singleColor[3];

    virtual void               CustomizeMappers(void);

    virtual vtkDataSet        *InsertFilters(vtkDataSet *, int);
    virtual void               SetUpFilters(int);

  private:

};


#endif


