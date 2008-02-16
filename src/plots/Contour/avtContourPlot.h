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
//                            avtContourPlot.h                               //
// ************************************************************************* //

#ifndef AVT_CONTOUR_PLOT_H
#define AVT_CONTOUR_PLOT_H

#include <avtPlot.h>
#include <ContourAttributes.h>

class     avtContourFilter;
class     avtFeatureEdgesFilter;
class     avtLevelsLegend;
class     avtLevelsMapper;
class     avtLookupTable;

// ****************************************************************************
//  Method: avtContourPlot
//
//  Purpose:
//      A concrete type of avtPlot for isocontours.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   Februray 15, 2001 
//
//  Modifications:
//    Jeremy Meredith, Fri Mar  2 13:10:02 PST 2001
//    Made SetAtts virtual and take a generic AttributeGroup.
//
//  Modifications:
//    Kathleen Bonnell, Fri Mar  2 11:34:46 PST 2001
//    Added legend and methods for setting legend parameters.
//    Removed data members varMin, varMax.
//
//    Jeremy Meredith, Sun Mar  4 16:59:57 PST 2001
//    Added a static Create method.
//
//    Kathleen Bonnell, Tue Mar 13 11:35:45 PST 2001 
//    Added SetLegend method.
//
//    Hank Childs, Tue Mar 27 14:47:03 PST 2001
//    Inherited from avtSurfaceDataPlot instead of avtPlot and added GetName.
//
//    Kathleen Bonnell, Tue Mar 27 14:38:23 PST 2001 
//    Removed SetVarExtents method, no longer necessary.  Added arguments
//    to SetIsoValues, CreatePercentValues, CreateNIsoValues methods. 
//
//    Kathleen Bonnell, Tue Apr  3 08:56:47 PDT 2001 
//    Made ContourAttributes a data member, so that all methods have 
//    access to the atts. 
//    
//    Hank Childs, Sun Jun 17 18:14:48 PDT 2001
//    Put code for determining isolevel values into avtContourFilter.
//
//    Kathleen Bonnell, Wed Aug 29 16:44:31 PDT 2001 
//    Added avtLookupTable and numLevels.  
//    Changed the way single colors are handled (no longer with a lut). 
//    Removed SetLookupTableWithMultiColor and SetLookupTableWithSingleColor. 
//
//    Kathleen Bonnell, Sat Sep 22 12:13:57 PDT 2001 
//    Added CustomizeMapper. 
//    
//    Kathleen Bonnell, Tue Oct 22 08:33:26 PDT 2002
//    Added ApplyRenderingTransformation. 
// 
//    Brad Whitlock, Tue Dec 3 09:30:39 PDT 2002
//    I added the SetColorTable method.
//
//    Eric Brugger, Thu Mar 25 16:38:17 PST 2004
//    I added the GetDataExtents method.
//
//    Hank Childs, Fri Feb 15 15:41:15 PST 2008
//    Fix memory leak by adding new data member.
//
// ****************************************************************************

class
avtContourPlot : public avtSurfaceDataPlot
{
  public:
                    avtContourPlot();
    virtual        ~avtContourPlot();

    static avtPlot *Create();

    virtual const char *GetName(void) { return "ContourPlot"; };

    virtual void    SetAtts(const AttributeGroup*);
    virtual void    GetDataExtents(std::vector<double> &);
    virtual void    ReleaseData(void);
    virtual bool    SetColorTable(const char *ctName);

    void            SetLegend(bool);
    void            SetLineWidth(int);
    void            SetLineStyle(int);

  protected:
     avtContourFilter        *contourFilter;
     avtFeatureEdgesFilter   *edgeFilter;
     avtLevelsMapper         *levelsMapper;
     avtLevelsLegend         *levelsLegend;
     avtLegend_p              levLegendRefPtr;
     avtLookupTable          *avtLUT;
     ContourAttributes        atts;
     int                      numLevels;

     virtual avtMapper       *GetMapper(void);
     virtual avtDataObject_p  ApplyOperators(avtDataObject_p);
     virtual avtDataObject_p  ApplyRenderingTransformation(avtDataObject_p);
     virtual void             CustomizeBehavior(void);
     virtual void             CustomizeMapper(avtDataObjectInformation &);
     virtual avtLegend_p      GetLegend(void) { return levLegendRefPtr; };
     void                     SetColors();
};


#endif


