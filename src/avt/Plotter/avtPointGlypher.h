/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

    void                       SetScale(double);
    void                       SetGlyphType(const int);

    void                       ColorByScalarOn(const std::string &);
    void                       ColorByScalarOff(void);

  protected:
    vtkPolyData               *glyph2D;
    vtkPolyData               *glyph3D;
    double                     scale;
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
    bool                       ProtectedSetFullFrameScaling(bool useScale, const double *s);

  private:
    int                        spatialDim;
    vtkPolyData               *GetGlyphSource(void);
    void                       ClearGlyphs(void);
    void                       SetUpGlyph(void);
};


#endif


