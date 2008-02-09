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
//                              avtVolumePlot.h                              //
// ************************************************************************* //

#ifndef AVT_VOLUME_PLOT_H
#define AVT_VOLUME_PLOT_H

#include <VolumeAttributes.h>
#include <WindowAttributes.h>

#include <avtVolumeRenderer.h>
#include <avtPlot.h>
#include <avtVolumeVariableLegend.h>

#include <string>

class WindowAttributes;

class avtLookupTable;
class avtShiftCenteringFilter;
class avtUserDefinedMapper;
class avtVolumeFilter;
class avtResampleFilter;


// ****************************************************************************
//  Method: avtVolumePlot
//
//  Purpose:
//      A concrete type of avtPlot for volume-rendered plots.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 27, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Apr 19 15:07:38 PST 2001
//    I added a legend to the plot.
//
//    Kathleen Bonnell, Fri Aug 31 08:50:30 PDT 2001 
//    Added avtLookupTable. 
//
//    Hank Childs, Tue Nov 20 12:09:13 PST 2001
//    Added support for software volume rendering as well.
//
//    Hank Childs, Fri Feb  8 19:38:57 PST 2002
//    Allow for data to be smoothed with a shift centering filter.
//
//    Kathleen Bonnell, Tue Oct 22 08:33:26 PDT 2002
//    Added ApplyRenderingTransformation. 
//    
//    Hank Childs, Wed Nov 24 16:44:44 PST 2004
//    Integrated this plot with SR mode, meaning that a lot of infrastructure
//    for delivering images could be removed.
//
//    Hank Childs, Sun Dec  4 17:55:06 PST 2005
//    Added GetNumberOfStagesForImageBasedPlots.
//
//    Kathleen Bonnell, Wed May  9 16:58:50 PDT 2007
//    Set CanDo2DViewScaling to false.
//
// ****************************************************************************

class
avtVolumePlot : public avtVolumeDataPlot
{
  public:
                        avtVolumePlot();
    virtual            ~avtVolumePlot();

    static avtPlot     *Create();

    virtual const char *GetName(void) { return "VolumePlot"; };
    virtual void        SetAtts(const AttributeGroup*);
    virtual void        ReleaseData(void);
    void                SetLegend(bool);

    virtual bool        PlotIsImageBased(void);
    virtual avtImage_p  ImageExecute(avtImage_p, const WindowAttributes &);
    virtual int         GetNumberOfStagesForImageBasedPlot(
                                                     const WindowAttributes &);
    virtual bool        Equivalent(const AttributeGroup *);

    virtual bool        CanCacheWriterExternally(void) { return false; }

    virtual bool        CanDo2DViewScaling(void) { return false; }

  protected:
    VolumeAttributes         atts;
    avtVolumeFilter         *volumeFilter;
    avtResampleFilter       *resampleFilter;
    avtShiftCenteringFilter *shiftCentering;
    avtVolumeRenderer_p      renderer;
    avtUserDefinedMapper    *mapper;
    avtLookupTable          *avtLUT;

    avtVolumeVariableLegend *varLegend;
    avtLegend_p              varLegendRefPtr;

    virtual avtMapper       *GetMapper(void);
    virtual avtDataObject_p  ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p  ApplyRenderingTransformation(avtDataObject_p);
    virtual void             CustomizeBehavior(void);
    virtual avtLegend_p      GetLegend(void) { return varLegendRefPtr; };
    void                     SetLegendOpacities();
    virtual avtContract_p
                             EnhanceSpecification(avtContract_p);
};


#endif


