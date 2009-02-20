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
//                             avtPseudocolorPlot.h                          //
// ************************************************************************* //

#ifndef AVT_PSEUDOCOLOR_PLOT_H
#define AVT_PSEUDOCOLOR_PLOT_H


#include <avtLegend.h>
#include <avtPlot.h>
#include <PseudocolorAttributes.h>

class     avtLookupTable;
class     avtPseudocolorFilter;
class     avtShiftCenteringFilter;
class     avtVariablePointGlyphMapper;
class     avtVariableLegend;


// ****************************************************************************
//  Class:  avtPseudocolorPlot
//
//  Purpose:
//      A concrete type of avtPlot, this is the standard pseudocolor plot.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Fri Mar  2 13:10:02 PST 2001
//    Made SetAtts virtual and take a generic AttributeGroup.
//
//    Jeremy Meredith, Sun Mar  4 16:59:57 PST 2001
//    Added a static Create method.
//
//    Hank Childs, Tue Mar 27 14:47:03 PST 2001
//    Inherited from avtSurfaceDataPlot instead of avtPlot and added GetName.
//
//    Kathleen Bonnell, Tue Apr  3 08:56:47 PDT 2001 
//    Made PseudocolorAttributes a data member so other methods can have access
//    to the atts.  Added SetScaling, DefineLogLUT, and DefineSkewLUT methods. 
//
//    Kathleen Bonnell, Tue Apr 24 12:22:01 PDT 2001
//    Added avtShiftCenterFilter. 
//    
//    Jeremy Meredith, Tue Jun  5 20:45:02 PDT 2001
//    Allow storage of attributes as a class member.
//
//    Brad Whitlock, Thu Jun 14 16:49:22 PST 2001
//    Added SetColorTable method.
//
//    Kathleen Bonnell, Wed Aug 29 16:44:31 PDT 2001 
//    Added avtLookupTable and previousMode. 
//
//    Kathleen Bonnell, Thu Oct  4 16:28:16 PDT 2001 
//    Added SetLimitsMode.  Removed SetMin, SetMax, SetMinOff, SetMaxOff.
//
//    Kathleen Bonnell, Wed Mar 13 12:04:53 PST 2002 
//    Added private method SetLegendRanges.  
//
//    Kathleen Bonnell, Thu Mar 28 14:03:19 PST 2002 
//    Removed previousMode, no longer needed. 
//
//    Hank Childs, Sun Jun 23 12:19:23 PDT 2002
//    Add support for point meshes.
//
//    Kathleen Bonnell, Tue Oct 22 08:33:26 PDT 2002
//    Added ApplyRenderingTransformation. 
//    
//    Jeremy Meredith, Tue Dec 10 09:06:18 PST 2002
//    Added GetSmoothingLevel to allow smoothing inside avtPlot.
//
//    Eric Brugger, Thu Mar 25 16:59:55 PST 2004
//    I added the GetDataExtents method.
//
//    Kathleen Bonnell, Thu Aug 19 15:29:46 PDT 2004
//    Added EnhanceSpecification, and topoDim ivar.  Replaced glyphPoints
//    and varMapper with glyphMapper.
//
//    Kathleen Bonnell, Tue Aug 24 15:31:56 PDT 2004 
//    Added SetCellCountMultiplierForSRThreshold.
//
//    Kathleen Bonnell, Tue Nov  2 11:01:28 PST 2004 
//    Added avtPseudocolorFilter.
//
//    Kathleen Bonnell, Fri Nov 12 11:25:23 PST 2004
//    Replaced avtPointGlyphMapper with avtVariablePointGlyphMapper. 
//
//    Brad Whitlock, Thu Jul 21 15:25:44 PST 2005
//    Added SetPointGlyphSize.
//
//    Hank Childs, Wed Aug 13 13:46:31 PDT 2008
//    Add NeedZBufferToCompositeEvenIn2D because thick lines can bleed into
//    other processor's portion of image space.
//
//    Jeremy Meredith, Fri Feb 20 15:09:22 EST 2009
//    Added support for using per-color alpha values from a color table
//    (instead of just a single global opacity for the whole plot).
//
// ****************************************************************************

class avtPseudocolorPlot : public avtSurfaceDataPlot
{
  public:
                                avtPseudocolorPlot();
    virtual                    ~avtPseudocolorPlot();

    static avtPlot             *Create();

    virtual const char         *GetName(void) { return "PseudocolorPlot"; };

    virtual void                SetAtts(const AttributeGroup*);
    virtual void                GetDataExtents(std::vector<double> &);
    virtual bool                SetColorTable(const char *ctName);
    virtual void                ReleaseData(void);
    virtual bool                NeedZBufferToCompositeEvenIn2D(void);

    void                        SetLegend(bool);
    void                        SetLighting(bool);

    void                        SetLimitsMode(int);

    void                        SetOpacityFromAtts();
    void                        SetScaling(int, double);

  protected:
    avtVariablePointGlyphMapper  *glyphMapper;
    avtVariableLegend          *varLegend;
    avtLegend_p                 varLegendRefPtr;
    PseudocolorAttributes       atts;
    avtPseudocolorFilter       *pcfilter;
    avtShiftCenteringFilter    *filter;
    bool                        colorsInitialized;
    int                         topoDim;
    avtLookupTable             *avtLUT;
    bool                        colorTableIsFullyOpaque;

    virtual avtMapper          *GetMapper(void);
    virtual avtDataObject_p     ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p     ApplyRenderingTransformation(avtDataObject_p);
    virtual avtContract_p     
                                EnhanceSpecification(avtContract_p);
    virtual void                CustomizeBehavior(void);
    virtual int                 GetSmoothingLevel();

    virtual avtLegend_p         GetLegend(void) { return varLegendRefPtr; };

    virtual void                SetCellCountMultiplierForSRThreshold(
                                   const avtDataObject_p dob);
private:
    void                        SetLegendRanges(void);
    void                        SetPointGlyphSize();
};


#endif


