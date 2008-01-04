/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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


