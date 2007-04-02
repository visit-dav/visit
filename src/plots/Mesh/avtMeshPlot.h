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
//                               avtMeshPlot.h                               //
// ************************************************************************* //

#ifndef AVT_MESH_PLOT_H
#define AVT_MESH_PLOT_H

#include <LineAttributes.h>
#include <MeshAttributes.h>

#include <avtPlot.h>
#include <avtSurfaceAndWireframeRenderer.h>
#include <avtGhostZoneAndFacelistFilter.h>

class     vtkProperty;

class     avtMeshFilter;
class     avtSmoothPolyDataFilter;
class     avtUserDefinedMapper;
class     avtVariableLegend;
class     avtVariablePointGlyphMapper;


// ****************************************************************************
//  Method: avtMeshPlot
//
//  Purpose:
//      A concrete type of avtPlot for meshes.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 21, 2001 
//
//  Modifications:
//
//    Jeremy Meredith, Tue Jun  5 20:45:02 PDT 2001
//    Allow storage of attributes as a class member.
//
//    Kathleen Bonnell, Fri Jun  15 14:15:43 PDT 2001
//    Added mesh filter. 
//
//    Kathleen Bonnell, Fri Jun 29 10:27:44 PDT 2001 
//    Added method to set opaque render mode. 
//
//    Hank Childs, Thu Jul 26 11:59:16 PDT 2001
//    Went back to inheriting from avtPlot, since this can be a line plot or
//    a surface plot.
//
//    Kathleen Bonnell, Wed Aug 22 15:22:55 PDT 2001 
//    Use avtUserDefinedMapper and avtSurfaceAndWireframeRenderer instead
//    of avtWireframeMapper to better draw opaque mode.  Modified arguments
//    to SetLineWidth and SetLineStyle.  Added member property.
//    
//    Kathleen Bonnell, Sep  5 12:13:54 PDT 2001
//    Add method to set the color for opaque mode.
//
//    Kathleen Bonnell, Wed Sep 26 15:14:07 PDT 2001 
//    Add methods to set the background/foreground colors.
//
//    Jeremy Meredith, Fri Nov  9 10:15:17 PST 2001
//    Added an Equivalent method.
//
//    Hank Childs, Sun Jun 23 21:07:55 PDT 2002
//    Added support for point meshes.
//
//    Jeremy Meredith, Mon Jul  8 18:38:04 PDT 2002
//    Added facelist filter before the mesh lines were calculated.
//
//    Hank Childs, Fri Jul 19 08:59:21 PDT 2002
//    Use the ghost zone and facelist filter instead of the normal facelist
//    filter.
//
//    Kathleen Bonnell, Tue Oct 22 08:33:26 PDT 2002
//    Added ApplyRenderingTransformation. 
//    
//    Jeremy Meredith, Tue Dec 10 10:04:18 PST 2002
//    Added smooth poly data filter.
//
//    Kathleen Bonnell, Mon Mar 24 17:48:27 PST 2003
//    Added SetOpaqueColor with double* arg.  Added flag
//    wireframeRenderingIsInappropriate. 
//
//    Kathleen Bonnell, Wed Aug 27 15:45:45 PDT 2003 
//    Added SetOpaqueMeshIsAppropriate.
//    
//    Kathleen Bonnell, Thu Sep  4 11:15:30 PDT 2003 
//    Removed argument from 'SetRenderOpaque', added 'ShouldRenderOpaque'.
//
//    Mark C. Miller, Thu Aug 12 21:17:35 PDT 2004
//    Added GetCellCountMultiplierForSRThreshold
//
//    Mark C. Miller, Mon Aug 23 20:24:31 PDT 2004
//    Changed GetCellCountMultiplierForSRThreshold to Set...
//    
//    Kathleen Bonnell, Tue Aug 24 16:12:03 PDT 2004 
//    Added avtMeshType arg to SetOpaqueMeshIsAppropriate.
//    
//    Kathleen Bonnell, Tue Nov  2 10:41:33 PST 2004 
//    Replaced avtPointToGlyphFilter with avtPointGlyphMapper.  Removed
//    avtMeshType arg from SetOpaqueMeshIsAppropriate.
//    
//    Kathleen Bonnell, Tue Nov  2 10:41:33 PST 2004 
//    Replaced avtPointGlyphMapper with avtVariablePointGlyphMapper.
//
//    Brad Whitlock, Wed Dec 15 10:48:35 PDT 2004
//    Removed SetVarName.
//
//    Brad Whitlock, Thu Jul 21 15:25:44 PST 2005
//    Added SetPointGlyphSize.
//
// ****************************************************************************

class
avtMeshPlot : public avtPlot
{
  public:
                    avtMeshPlot();
    virtual        ~avtMeshPlot();

    static avtPlot *Create();

    virtual const char *GetName(void)  { return "MeshPlot"; };
    virtual void    ReleaseData(void);

    virtual void    SetAtts(const AttributeGroup*);
    virtual bool    Equivalent(const AttributeGroup*);

    void            SetLegend(bool);
    void            SetLineWidth(_LineWidth);
    void            SetLineStyle(_LineStyle);
    void            SetPointSize(float);
    void            SetRenderOpaque(void);
    void            SetMeshColor(const unsigned char *);
    void            SetMeshColor(const double *);
    void            SetOpaqueColor(const unsigned char *, bool force = false);
    void            SetOpaqueColor(const double *, bool force = false);
    virtual bool    SetBackgroundColor(const double *);
    virtual bool    SetForegroundColor(const double *);

    virtual int     TargetTopologicalDimension(void);
    virtual const AttributeSubject 
                   *SetOpaqueMeshIsAppropriate(bool);


  protected:
    avtSurfaceAndWireframeRenderer_p renderer;
    avtUserDefinedMapper            *mapper;
    avtVariablePointGlyphMapper     *glyphMapper;
    avtVariableLegend               *varLegend;
    avtGhostZoneAndFacelistFilter   *ghostAndFaceFilter;
    avtMeshFilter                   *filter;
    vtkProperty                     *property;
    avtLegend_p                      varLegendRefPtr;
    avtSmoothPolyDataFilter         *smooth;
    double                           bgColor[3];
    double                           fgColor[3];
    bool                             wireframeRenderingIsInappropriate;
    MeshAttributes           atts;

    virtual avtMapper       *GetMapper(void);
    virtual avtDataObject_p  ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p  ApplyRenderingTransformation(avtDataObject_p);
    virtual void             CustomizeBehavior(void);
    virtual void             CustomizeMapper(avtDataObjectInformation &);
    virtual avtLegend_p      GetLegend(void) { return varLegendRefPtr; };
    void                     SetCellCountMultiplierForSRThreshold(
                                 const avtDataObject_p);

  private:
    bool                     ShouldRenderOpaque(void);
    void                     SetPointGlyphSize();
};


#endif


