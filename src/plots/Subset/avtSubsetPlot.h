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
//                             avtSubsetPlot.h                               //
// ************************************************************************* //

#ifndef AVT_SUBSET_PLOT_H
#define AVT_SUBSET_PLOT_H


#include <avtPlot.h>
#include <SubsetAttributes.h>

class     avtLevelsLegend;
class     avtLevelsPointGlyphMapper;
class     avtLookupTable;

class     avtFeatureEdgesFilter;
class     avtGhostZoneAndFacelistFilter;
class     avtGhostZoneFilter;
class     avtFacelistFilter;
class     avtSubsetFilter;
class     avtSmoothPolyDataFilter;


// ****************************************************************************
//  Method: avtSubsetPlot
//
//  Purpose:
//      A concrete type of avtPlot for subsets.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 16, 2001 
//
//  Modifications:
//    Eric Brugger, Fri Dec 14 13:03:02 PST 2001
//    I modified the class so that it now inherits from avtVolumeDataPlot,
//    since the plot will now take care of reducing the topology from 3 to 2.
//
//    Jeremy Meredith, Tue Mar 12 17:23:11 PST 2002
//    Added a line style.
//
//    Kathleen Bonnell, Wed Apr 10 09:45:43 PDT 2002 
//    Added SetColors. 
//
//    Kathleen Bonnell, Tue Oct 22 08:33:26 PDT 2002
//    Added ApplyRenderingTransformation. 
//
//    Jeremy Meredith, Tue Dec 10 10:04:18 PST 2002
//    Added smooth poly data filter.
//
//    Brad Whitlock, Tue Nov 26 11:03:31 PDT 2002
//    Added the SetColorTable method.
//
//    Kathleen Bonnell, Thu Dec 19 12:27:09 PST 2002 
//    Added the SortLabels method.
//
//    Mark C. Miller, Wed Mar 24 19:23:21 PST 2004
//    Added AttributesDependOnDatabaseMetaData
//
//    Kathleen Bonnell, Fri Nov 12 11:47:49 PST 2004 
//    Changed mapper type to avtLevelsPointGlyphMapper. 
//
//    Brad Whitlock, Thu Jul 21 15:38:31 PST 2005
//    Added SetPointGlyphSize.
//
//    Hank Childs, Fri Aug  3 13:46:26 PDT 2007
//    Added another ghost zone filter (gz2) for removing edges on the coarse
//    fine boundary of AMR grids.
//
//    Hank Childs, Wed Aug 13 11:42:07 PDT 2008
//    Add NeedZBufferToCompositeEvenIn2D because thick lines can bleed into
//    other processor's portion of image space.
//
// ****************************************************************************

class
avtSubsetPlot : public avtVolumeDataPlot
{
  public:
                              avtSubsetPlot();
    virtual                  ~avtSubsetPlot();

    static avtPlot           *Create();

    virtual const char       *GetName(void) { return "SubsetPlot"; };


    virtual void              SetAtts(const AttributeGroup*);
    virtual void              ReleaseData(void);
    virtual bool              SetColorTable(const char *ctName);

    virtual bool              AttributesDependOnDatabaseMetaData(void)
                                  { return true; };

    void                      SetLegend(bool);
    void                      SetLineWidth(int);
    void                      SetLineStyle(int);
    virtual bool              NeedZBufferToCompositeEvenIn2D(void);

  protected:
    avtFeatureEdgesFilter           *wf;
    avtGhostZoneAndFacelistFilter   *gzfl;
    avtGhostZoneFilter              *gz;
    avtGhostZoneFilter              *gz2;
    avtFacelistFilter               *fl;
    avtSubsetFilter                 *sub;
    avtSmoothPolyDataFilter         *smooth;

    SubsetAttributes          atts;
    avtLevelsPointGlyphMapper *levelsMapper;
    avtLevelsLegend          *levelsLegend;
    avtLegend_p               levLegendRefPtr;
    avtLookupTable           *avtLUT;

    void                      SetColors(void); 
    void                      SortLabels(void);
    void                      SetPointGlyphSize();
    virtual avtMapper        *GetMapper(void);
    virtual avtDataObject_p   ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p   ApplyRenderingTransformation(avtDataObject_p);
    virtual void              CustomizeBehavior(void);

    virtual avtLegend_p       GetLegend(void) { return levLegendRefPtr; };
};


#endif


