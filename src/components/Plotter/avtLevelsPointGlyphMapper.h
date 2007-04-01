// ************************************************************************* //
//                      avtLevelsPointGlyphMapper.h                          //
// ************************************************************************* //

#ifndef AVT_LEVELS_POINT_GLYPH_MAPPER_H
#define AVT_LEVELS_POINT_GLYPH_MAPPER_H

#include <plotter_exports.h>

#include <avtLevelsMapper.h>
#include <avtPointGlypher.h>


// ****************************************************************************
//  Class: avtLevelsPointGlyphMapper
//
//  Purpose:
//    A mapper for glyph.  This extends the functionality of a mapper by
//    mapping a glyph onto a dataset.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 12, 2004 
//
//  Modifications:
//
// ****************************************************************************

class PLOTTER_API  avtLevelsPointGlyphMapper : virtual public avtLevelsMapper,
                                               virtual public avtPointGlypher
{
  public:
                               avtLevelsPointGlyphMapper();
    virtual                   ~avtLevelsPointGlyphMapper();

    virtual void               ScaleByVar(const std::string &);

  protected:
    virtual void               CustomizeMappers(void);
    virtual vtkDataSet        *InsertFilters(vtkDataSet *, int);
    virtual void               SetUpFilters(int);

  private:

};


#endif


