// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtVariablePointGlyphMapper.h                        //
// ************************************************************************* //

#ifndef AVT_VARIABLE_POINT_GLYPH_MAPPER_H
#define AVT_VARIABLE_POINT_GLYPH_MAPPER_H

#include <plotter_exports.h>

#include <avtVariableMapper.h>
#include <avtPointMapper.h>


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
//    Brad Whitlock, Wed Jul 26 13:53:29 PST 2006
//    Added SetFullFrameScaling.
//
//    Kathleen Biagas, Wed Feb 6 19:38:27 PDT 2013
//    Changed signature of InsertFilters.
//
//    Kathleen Biagas, Tue Aug 23 11:38:11 PDT 2016
//    Changed inheritance from avtPointGlypher to avtPointMapper.
//    Removed glyph-related methods.
//
// ****************************************************************************

class PLOTTER_API  avtVariablePointGlyphMapper :
    virtual public avtVariableMapper,
    virtual public avtPointMapper
{
  public:
                               avtVariablePointGlyphMapper();
    virtual                   ~avtVariablePointGlyphMapper();

    void                       ColorBySingleColor(const unsigned char [3]);
    void                       ColorBySingleColor(const double [3]);

  protected:
    double                     singleColor[3];

    virtual void               CustomizeMappers(void);

  private:

};


#endif


