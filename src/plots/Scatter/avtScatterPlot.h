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
//                               avtScatterPlot.h                            //
// ************************************************************************* //

#ifndef AVT_SCATTER_PLOT_H
#define AVT_SCATTER_PLOT_H

#include <ScatterAttributes.h>

#include <avtPlot.h>
#include <avtSurfaceAndWireframeRenderer.h>

class avtLookupTable;
class avtVariableLegend;
class avtVariablePointGlyphMapper;
class avtScatterFilter;

// ****************************************************************************
//  Method: avtScatterPlot
//
//  Purpose:
//      A concrete type of avtPlot for scatter plots of scalar variables.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 2 21:52:32 PST 2004 
//
//  Modifications:
//   Brad Whitlock, Thu Jul 21 15:29:40 PST 2005
//   Added SetPointGlyphMode.
//
// ****************************************************************************

class avtScatterPlot : public avtPlot
{
public:
                    avtScatterPlot();
    virtual        ~avtScatterPlot();

    static avtPlot *Create();

    virtual const char *GetName(void)  { return "ScatterPlot"; };
    virtual void    ReleaseData(void);

    virtual void    SetAtts(const AttributeGroup*);
    virtual bool    SetColorTable(const char *ctName);

    virtual bool    Equivalent(const AttributeGroup*);

    void            SetLegend(bool);
    void            SetVarName(const char *);
    virtual bool    SetForegroundColor(const double *);

    virtual int     TargetTopologicalDimension(void);

protected:
    avtVariablePointGlyphMapper     *glyphMapper;
    avtVariableLegend               *varLegend;
    avtLegend_p                      varLegendRefPtr;
    bool                             colorsInitialized;
    avtLookupTable                  *avtLUT;
    avtScatterFilter                *filter;
    double                           fgColor[3];

    ScatterAttributes                atts;

    virtual avtMapper       *GetMapper(void);
    virtual avtDataObject_p  ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p  ApplyRenderingTransformation(avtDataObject_p);
    virtual void             CustomizeBehavior(void);
    virtual avtLegend_p      GetLegend(void) { return varLegendRefPtr; };
    virtual avtContract_p     
                             EnhanceSpecification(avtContract_p);


    void SetScaling(int mode, float skew);
    void SetLimitsMode();
    void GetColorInformation(std::string &colorString,
                             int &mode, float &skew, bool &minFlag,
                             float &minVal, bool &maxFlag, float &maxVal);
    void SetPointGlyphSize();
};


#endif


