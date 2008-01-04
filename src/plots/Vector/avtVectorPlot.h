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
//                                avtVectorPlot.h                            //
// ************************************************************************* //

#ifndef AVT_VECTOR_PLOT_H
#define AVT_VECTOR_PLOT_H

#include <avtLegend.h>
#include <avtPlot.h>

#include <VectorAttributes.h>

class     vtkVectorGlyph;

class     avtGhostZoneFilter;
class     avtVectorGlyphMapper;
class     avtVariableLegend;
class     avtVectorFilter;
class     avtLookupTable;

// ****************************************************************************
//  Class:  avtVectorPlot
//
//  Purpose:
//      A concrete type of avtPlot, this is the standard vector plot.
//
//  Programmer: Hank Childs
//  Creation:   March 21, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Mar 27 14:47:03 PST 2001
//    Inherited from avtPointDataPlot instead of avtPlot and added GetName.
//
//    Brad Whitlock, Fri Jun 15 15:11:25 PST 2001
//    Added override of SetColorTable method.
//
//    Kathleen Bonnell, Thu Aug 30 10:47:07 PDT 2001 
//    Added avtLookupTable as member.
//
//    Kathleen Bonnell, Tue Oct 22 08:33:26 PDT 2002
//    Added ApplyRenderingTransformation. 
//
//    Brad Whitlock, Mon Dec 2 11:57:57 PDT 2002
//    I added a legend.
//
//    Mark C. Miller, Wed Aug 11 23:42:18 PDT 2004
//    Added GetCellCountMultiplierForSRThreshold()
//
//    Kathleen Bonnell, Mon Aug  9 14:33:26 PDT 2004 
//    Added magVarName, ComputeMagVarName, and SetMapperColors.
//
//    Mark C. Miller, Mon Aug 23 20:24:31 PDT 2004
//    Changed GetCellCountMultiplierForSRThreshold to Set...
//
//    Kathleen Bonnell, Wed Dec 22 17:01:09 PST 2004 
//    Added SetLimitsMode. 
//
// ****************************************************************************

class avtVectorPlot : public avtPointDataPlot
{
  public:
                                avtVectorPlot();
    virtual                    ~avtVectorPlot();

    virtual const char         *GetName(void) { return "VectorPlot"; };
    virtual void                ReleaseData(void);

    static avtPlot             *Create();

    virtual void                SetAtts(const AttributeGroup*);
    virtual bool                SetColorTable(const char *ctName);

    void                        SetLimitsMode(int);

  protected:
    VectorAttributes            atts;
    bool                        colorsInitialized;

    avtVectorGlyphMapper       *glyphMapper;
    avtVariableLegend          *varLegend;
    avtLegend_p                 varLegendRefPtr;
    avtVectorFilter            *vectorFilter;
    avtGhostZoneFilter         *ghostFilter;
    avtLookupTable             *avtLUT;

    vtkVectorGlyph             *glyph;

    virtual avtMapper          *GetMapper(void);
    virtual avtDataObject_p     ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p     ApplyRenderingTransformation(avtDataObject_p);
    virtual void                CustomizeBehavior(void);
    virtual void                CustomizeMapper(avtDataObjectInformation &);

    virtual avtLegend_p         GetLegend(void) { return varLegendRefPtr; };
    void                        SetLegend(bool);
    void                        SetLegendRanges();
    void                        ComputeMagVarName(const std::string &);
    void                        SetMapperColors(void);
    std::string                 magVarName;

    virtual void                SetCellCountMultiplierForSRThreshold(const avtDataObject_p);
};


#endif


