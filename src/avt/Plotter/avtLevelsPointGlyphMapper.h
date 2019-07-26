// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtLevelsPointGlyphMapper.h                          //
// ************************************************************************* //

#ifndef AVT_LEVELS_POINT_GLYPH_MAPPER_H
#define AVT_LEVELS_POINT_GLYPH_MAPPER_H

#include <plotter_exports.h>

#include <avtLevelsMapper.h>
#include <avtPointMapper.h>


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
//    Kathleen Biagas, Wed Feb 6 19:38:27 PDT 2013
//    Changed signature of InsertFilters.
//
//    Kathleen Biagas, Tue Aug 23 11:34:11 PDT 2016
//    Changed inheritance from avtPointGlypher to avtPointMapper. Removed
//    Glyph related methods.
//
// ****************************************************************************

class PLOTTER_API  avtLevelsPointGlyphMapper : virtual public avtLevelsMapper,
                                               virtual public avtPointMapper
{
  public:
                               avtLevelsPointGlyphMapper();
    virtual                   ~avtLevelsPointGlyphMapper();


  protected:
    virtual void               CustomizeMappers(void);

  private:

};


#endif


