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
//                                 avtLabelPlot.h                            //
// ************************************************************************* //

#ifndef AVT_Label_PLOT_H
#define AVT_Label_PLOT_H
#include <LabelAttributes.h>

#include <avtLegend.h>
#include <avtPlot.h>
#include <avtLabelRenderer.h>

class avtGhostZoneAndFacelistFilter;
class avtCondenseDatasetFilter;
class avtLabelFilter;
class avtLabelsMapper;
class avtLabelSubsetsFilter;
class avtUserDefinedMapper;
class avtVariableLegend;
class avtVertexNormalsFilter;

// ****************************************************************************
//  Class:  avtLabelPlot
//
//  Purpose:
//      A concrete type of avtPlot, this is the Label plot.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jan 7 14:58:26 PST 2004
//
//  Modifications:
//    Brad Whitlock, Wed Dec 15 10:48:35 PDT 2004
//    Removed SetVarName.
//
//    Brad Whitlock, Wed Aug 3 18:04:15 PST 2005
//    I made it use a avtLabelsMapper so we can label subsets.
//
// ****************************************************************************

class avtLabelPlot : public avtSurfaceDataPlot
{
  public:
                                avtLabelPlot();
    virtual                    ~avtLabelPlot();

    static avtPlot             *Create();

    virtual const char         *GetName(void) { return "LabelPlot"; };
    virtual void                SetAtts(const AttributeGroup*);
    virtual void                ReleaseData(void);

    virtual bool                SetForegroundColor(const double *);
    void                        SetLegend(bool);

    virtual avtContract_p EnhanceSpecification(avtContract_p spec);

  protected:
    LabelAttributes                atts;
    avtGhostZoneAndFacelistFilter *ghostAndFaceFilter;
    avtCondenseDatasetFilter      *condenseFilter;
    avtVertexNormalsFilter        *normalFilter;
    avtLabelSubsetsFilter         *labelSubsetsFilter;
    avtLabelFilter                *labelFilter;
    avtLabelRenderer_p             renderer;
    avtLabelsMapper               *labelMapper; 
    avtVariableLegend             *varLegend;
    avtLegend_p                    varLegendRefPtr;

    virtual avtMapper          *GetMapper(void);
    virtual avtDataObject_p     ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p     ApplyRenderingTransformation(avtDataObject_p);
    virtual void                CustomizeBehavior(void);
    virtual void                CustomizeMapper(avtDataObjectInformation &);

    virtual avtLegend_p         GetLegend(void) { return varLegendRefPtr; };
};


#endif
