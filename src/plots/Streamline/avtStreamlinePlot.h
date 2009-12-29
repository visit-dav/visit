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
//                                 avtStreamlinePlot.h                       //
// ************************************************************************* //

#ifndef AVT_Streamline_PLOT_H
#define AVT_Streamline_PLOT_H

#include <avtLegend.h>
#include <avtPlot.h>

#include <StreamlineAttributes.h>
#include <avtStreamlineRenderer.h>

class     avtLookupTable;
class     avtShiftCenteringFilter;
class     avtStreamlinePolyDataFilter;
class     avtVariableLegend;
class     avtVariableMapper;
class     avtUserDefinedMapper;

// ****************************************************************************
//  Class:  avtStreamlinePlot
//
//  Purpose:
//      A concrete type of avtPlot, this is the Streamline plot.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Oct 21 12:48:28 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Mon Jul 21 14:11:33 PDT 2008
//    Add method EnhanceSpecification.
//
//    Hank Childs, Wed Aug 13 10:47:32 PDT 2008
//    Indicate that we need z-buffer compositing, even in 2D, as the
//    streamline plot uses a different decomposition of the data and the
//    compositor assumes that z-buffer issues are resolved before it starts
//    compositing in 2D ... unless this flag is on.
//
//    Hank Childs, Tue Dec  2 13:54:46 PST 2008
//    Use an avtStreamlinePolyDataFilter to fit the new class refactoring.
//
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Add custom renderer and lots of appearance options to the streamlines plots.
//
// ****************************************************************************

class avtStreamlinePlot : public avtLineDataPlot
{
  public:
                                avtStreamlinePlot();
    virtual                    ~avtStreamlinePlot();

    virtual const char         *GetName(void) { return "StreamlinePlot"; };

    static avtPlot             *Create();

    virtual void                SetAtts(const AttributeGroup*);
    virtual bool                SetColorTable(const char *ctName);

    virtual void                ReleaseData(void);
    virtual bool                NeedZBufferToCompositeEvenIn2D(void)
                                              { return true; };

  protected:

    StreamlineAttributes        atts;
    avtUserDefinedMapper       *mapper;
    avtVariableLegend          *varLegend;
    avtLegend_p                 varLegendRefPtr;
    avtStreamlinePolyDataFilter *streamlineFilter;
    avtShiftCenteringFilter    *shiftCenteringFilter;
    avtLookupTable             *avtLUT;
    avtStreamlineRenderer_p     renderer;

    virtual avtMapper          *GetMapper(void);
    virtual avtDataObject_p     ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p     ApplyRenderingTransformation(avtDataObject_p);
    virtual void                CustomizeBehavior(void);
    virtual avtContract_p       EnhanceSpecification(avtContract_p);

    virtual avtLegend_p         GetLegend(void) { return varLegendRefPtr; };

    void                        UpdateMapperAndLegend();
    void                        SetLighting(bool);
};


#endif
