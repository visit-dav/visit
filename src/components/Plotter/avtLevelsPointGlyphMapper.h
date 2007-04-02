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
//    Brad Whitlock, Fri Jul 22 11:21:47 PDT 2005
//    Added an override for the SetGlyphType method that lets us switch
//    mapper inputs when we enter of leave point glyphing mode.
//
// ****************************************************************************

class PLOTTER_API  avtLevelsPointGlyphMapper : virtual public avtLevelsMapper,
                                               virtual public avtPointGlypher
{
  public:
                               avtLevelsPointGlyphMapper();
    virtual                   ~avtLevelsPointGlyphMapper();

    virtual void               ScaleByVar(const std::string &);
    void                       SetGlyphType(const int type);

  protected:
    virtual void               CustomizeMappers(void);
    virtual vtkDataSet        *InsertFilters(vtkDataSet *, int);
    virtual void               SetUpFilters(int);

  private:

};


#endif


