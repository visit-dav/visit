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
//                             avtBoundaryPlot.h                             //
// ************************************************************************* //

#ifndef AVT_BOUNDARY_PLOT_H
#define AVT_BOUNDARY_PLOT_H


#include <avtPlot.h>
#include <BoundaryAttributes.h>

class     avtLevelsLegend;
class     avtLevelsPointGlyphMapper;
class     avtLookupTable;

class     avtFeatureEdgesFilter;
class     avtGhostZoneAndFacelistFilter;
class     avtGhostZoneFilter;
class     avtFacelistFilter;
class     avtBoundaryFilter;
class     avtSmoothPolyDataFilter;


// ****************************************************************************
//  Method: avtBoundaryPlot
//
//  Purpose:
//      A concrete type of avtPlot for boundaries.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
//  Modifications:
//
//    Mark C. Miller, Wed Mar 24 19:23:21 PST 2004
//    Added AttributesDependOnDatabaseMetaData
//
//    Kathleen Bonnell, Fri Nov 12 10:23:09 PST 2004 
//    Changed mapper type to avtLevelsPointGlyphMapper.
//
//    Brad Whitlock, Thu Jul 21 15:32:26 PST 2005
//    Added SetPointGlyphSize.
//
//    Hank Childs, Wed Aug 13 13:46:31 PDT 2008
//    Add NeedZBufferToCompositeEvenIn2D because thick lines can bleed into
//    other processor's portion of image space.
//
// ****************************************************************************

class
avtBoundaryPlot : public avtVolumeDataPlot
{
  public:
                              avtBoundaryPlot();
    virtual                  ~avtBoundaryPlot();

    static avtPlot           *Create();

    virtual const char       *GetName(void) { return "BoundaryPlot"; };

    virtual void              SetAtts(const AttributeGroup*);
    virtual void              ReleaseData(void);
    virtual bool              SetColorTable(const char *ctName);

    virtual bool              AttributesDependOnDatabaseMetaData(void)
                                  { return true; };
    virtual bool              NeedZBufferToCompositeEvenIn2D(void);


    void                      SetLegend(bool);
    void                      SetLineWidth(int);
    void                      SetLineStyle(int);

  protected:
    avtFeatureEdgesFilter           *wf;
    avtGhostZoneFilter              *gz;
    avtBoundaryFilter               *sub;
    avtSmoothPolyDataFilter         *smooth;

    BoundaryAttributes         atts;
    avtLevelsPointGlyphMapper *levelsMapper;
    avtLevelsLegend           *levelsLegend;
    avtLegend_p                levLegendRefPtr;
    avtLookupTable            *avtLUT;

    void                       SetColors(void); 
    void                       SortLabels(void);
    void                       SetPointGlyphSize();
    virtual avtMapper         *GetMapper(void);
    virtual avtDataObject_p    ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p    ApplyRenderingTransformation(avtDataObject_p);
    virtual void               CustomizeBehavior(void);

    virtual avtLegend_p        GetLegend(void) { return levLegendRefPtr; };
};


#endif


