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
//                         avtPointToGlyphFilter.h                           //
// ************************************************************************* //

#ifndef AVT_POINT_TO_GLYPH_FILTER_H
#define AVT_POINT_TO_GLYPH_FILTER_H

#include <filters_exports.h>

#include <avtDataTreeIterator.h>


class vtkDataSet;
class vtkPolyData;


// ****************************************************************************
//  Class: avtPointToGlyphFilter
//
//  Purpose:
//      Creates a glyph for each point in a VTK dataset.  Current this only
//      creates hexahedrons/quads.
//
//  Programmer: Hank Childs
//  Creation:   June 22, 2002
//
//  Modifications:
//
//    Jeremy Meredith, Fri Dec 20 11:32:51 PST 2002
//    Added code to scale by a variable.
//
//    Hank Childs, Thu Aug 21 22:03:57 PDT 2003
//    Added more point glyphs.
//
// ****************************************************************************

class AVTFILTERS_API avtPointToGlyphFilter : public avtDataTreeIterator
{
  public:
                         avtPointToGlyphFilter();
    virtual             ~avtPointToGlyphFilter();

    void                 SetPointSize(double);
    void                 SetScaleVariable(const std::string&);
    void                 SetScaleByVariableEnabled(bool);
    void                 SetGlyphType(int);

    virtual const char  *GetType(void) { return "avtPointToGlyphFilter"; };
    virtual const char  *GetDescription(void) 
                             { return "Creating point glyphs"; };

  protected:
    double               pointSize;
    int                  glyphType;
    std::string          scaleVar;
    bool                 scaleByVarEnabled;
    vtkPolyData         *glyph3D;
    vtkPolyData         *glyph2D;

    virtual vtkDataSet  *ExecuteData(vtkDataSet *, int, std::string);
    virtual void         UpdateDataObjectInfo(void);

    void                 SetUpGlyph();
};


#endif


