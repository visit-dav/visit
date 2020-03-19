// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtBoundaryPlot.h                             //
// ************************************************************************* //

#ifndef AVT_BOUNDARY_PLOT_H
#define AVT_BOUNDARY_PLOT_H


#include <avtPlot.h>
#include <BoundaryAttributes.h>

class     avtLevelsLegend;
class     avtLevelsMapper;
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
//    Kathleen Biagas, Wed Feb 29 13:10:11 MST 2012
//    Add GetExtraInfoForPick.
//
//    Kathleen Biagas, Tue Aug 23 11:16:20 PDT 2016
//    Added LevelsMapper, as points and surfaces are no longer mapped
//    by the same mapper.
//
//    Kathleen Biagas, Tue Dec 20 13:53:40 PST 2016
//    Changed inheritance from avtVolumeDataPlot to avtSurfaceDataPlot.
//    Removed glyph mapper.
//
// ****************************************************************************

class
avtBoundaryPlot : public avtSurfaceDataPlot
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

    virtual const MapNode    &GetExtraInfoForPick(void);

  protected:
    avtFeatureEdgesFilter           *wf;
    avtGhostZoneFilter              *gz;
    avtBoundaryFilter               *sub;
    avtSmoothPolyDataFilter         *smooth;

    BoundaryAttributes         atts;
    avtLevelsMapper           *levelsMapper;
    avtLevelsLegend           *levelsLegend;
    avtLegend_p                levLegendRefPtr;
    avtLookupTable            *avtLUT;

    void                       SetColors(void); 
    void                       SortLabels(void);
    void                       SetPointGlyphSize();
    virtual avtMapperBase     *GetMapper(void);
    virtual avtDataObject_p    ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p    ApplyRenderingTransformation(avtDataObject_p);
    virtual void               CustomizeBehavior(void);

    virtual avtLegend_p        GetLegend(void) { return levLegendRefPtr; };
};


#endif


