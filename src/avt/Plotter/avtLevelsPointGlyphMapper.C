// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtLevelsPointGlyphMapper.C                       //
// ************************************************************************* //

#include <avtLevelsPointGlyphMapper.h>



// ****************************************************************************
//  Method: avtLevelsPointGlyphMapper constructor
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 12, 2004 
//
//  Modifications:
//    Jeremy Meredith, Thu Aug  7 14:37:30 EDT 2008
//    Made constructor initializers match true initializer order.
//
//    Kathleen Biagas, Tue Aug 23 11:34:11 PDT 2016
//    Changed inheritance from avtPointGlypher to avtPointMapper.
//
// ****************************************************************************

avtLevelsPointGlyphMapper::avtLevelsPointGlyphMapper():
    avtLevelsMapper(), avtPointMapper()
{
    ColorByScalarOff();
}


// ****************************************************************************
//  Method: avtLevelsPointGlyphMapper destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 12, 2004
//
//  Modifications:
//
// ****************************************************************************

avtLevelsPointGlyphMapper::~avtLevelsPointGlyphMapper()
{
}


// ****************************************************************************
//  Method: avtLevelsPointGlyphMapper::CustomizeMappers
//
//  Purpose:
//    A hook from the base class that allows the variable mapper to force
//    the vtk mappers to be the same as its state.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 12, 2004
//
//  Modifications:
//    Brad Whitlock, Thu Aug 25 15:21:23 PST 2005
//    Added support for point texturing.
// 
//    Kathleen Biagas, Tue Jul 12 13:37:48 MST 2016
//    Comment out point sprites until this can be figured out with vtk-7.
//
//    Kathleen Biagas, Tue Aug 23 11:37:19 PDT 2016
//    All glyph related customization now handled by avtPointMapper.
//
// ****************************************************************************

void
avtLevelsPointGlyphMapper::CustomizeMappers(void)
{
    avtLevelsMapper::CustomizeMappers();
    avtPointMapper::CustomizeMappers();
}


