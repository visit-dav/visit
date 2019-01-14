/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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


