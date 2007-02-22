/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                              avtSurfacePlot.h                             //
// ************************************************************************* //

#ifndef AVT_SURFACE_PLOT_H
#define AVT_SURFACE_PLOT_H


#include <LineAttributes.h>
#include <SurfaceAttributes.h>

#include <avtLegend.h>
#include <avtPlot.h>
#include <avtSurfaceAndWireframeRenderer.h>

class     vtkProperty;

class     avtLookupTable;
class     avtSurfaceFilter;
class     avtWireframeFilter;
class     avtUserDefinedMapper;
class     avtVariableLegend;


// ****************************************************************************
//  Class:  avtSurfacePlot
//
//  Purpose:
//      A concrete type of avtPlot, this generates a surface plot from 2d data. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 05, 2001 
//
//  Modifications:
//
//    Hank Childs, Tue Mar 27 14:47:03 PST 2001
//    Inherited from avtSurfaceDataPlot instead of avtPlot and added GetName.
//
//    Kathleen Bonnell, Fri Mar 30 08:56:47 PDT 2001 
//    Added methods SetLineWidth, SetScaling, SetMin, SetMinOff, 
//    SetMax, SetMaxOff 
//
//    Jeremy Meredith, Tue Jun  5 20:45:02 PDT 2001
//    Allow storage of attributes as a class member.
//
//    Brad Whitlock, Fri Jun 15 14:22:57 PST 2001
//    Added override of the SetColorTable method.
//
//    Kathleen Bonnell, Thu Jun 21 17:33:08 PDT 2001 
//    Added method SetLineStyle.
//
//    Kathleen Bonnell, Tue Aug 21 10:06:05 PDT 2001 
//    Use avtSurfaceAndWireframeRenderer instead of avtVariableMapper. 
//    Modified parameters to SetLineWidth, SetLineStyle.  Removed
//    methods related to setting min/max. Add methods SetRepresentation,
//    SetSurfaceAttributes, SetWireframeAttributes.
//
//    Kathleen Bonnell, Wed Aug 29 16:44:31 PDT 2001 
//    Add avtLookupTable in place of vtkLookupTable. 
//    
//    Kathleen Bonnell, Thu Oct 11 12:45:30 PDT 2001 
//    Added method SetLimitsMode. 
//    
//    Kathleen Bonnell, Tue Oct 22 08:33:26 PDT 2002
//    Added ApplyRenderingTransformation. 
//    
//    Kathleen Bonnell, Mon May 24 14:13:55 PDT 2004 
//    Added avtWireframeFilter.
//
//    Brad Whitlock, Wed Dec 15 10:48:35 PDT 2004
//    Removed SetVarName.
//
// ****************************************************************************

class avtSurfacePlot : public avtSurfaceDataPlot
{
  public:
                                avtSurfacePlot();
    virtual                    ~avtSurfacePlot();

    static avtPlot             *Create();

    virtual void                SetAtts(const AttributeGroup*);
    virtual void                ReleaseData(void);
    virtual bool                SetColorTable(const char *ctName);

    virtual const char         *GetName(void)  { return "SurfacePlot"; };

    void                        SetLegend(bool);
    void                        SetLighting(bool);
    void                        SetLineWidth(_LineWidth);
    void                        SetLineStyle(_LineStyle);
    void                        SetScaling(const int, const double);
    void                        SetRepresentation(bool);
    void                        SetSurfaceAttributes(bool);
    void                        SetWireframeAttributes(bool);
    void                        SetLimitsMode(int);

  protected:
    avtSurfaceAndWireframeRenderer_p  renderer;
    avtUserDefinedMapper           *mapper;
    avtVariableLegend              *varLegend;
    avtLegend_p                     varLegendRefPtr;
    avtLookupTable                 *avtLUT;
    avtSurfaceFilter               *surfaceFilter;
    avtWireframeFilter             *wireFilter;
    vtkProperty                    *property;
    SurfaceAttributes               atts;
    bool                            colorsInitialized;

    virtual avtMapper          *GetMapper(void);
    virtual avtDataObject_p     ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p     ApplyRenderingTransformation(avtDataObject_p);
    virtual void                CustomizeBehavior();
    virtual avtLegend_p         GetLegend(void){ return varLegendRefPtr; };
};


#endif


