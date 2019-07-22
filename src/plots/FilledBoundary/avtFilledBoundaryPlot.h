// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtFilledBoundaryPlot.h                          //
// ************************************************************************* //

#ifndef AVT_FILLED_BOUNDARY_PLOT_H
#define AVT_FILLED_BOUNDARY_PLOT_H


#include <avtPlot.h>
#include <FilledBoundaryAttributes.h>

class     avtLevelsLegend;
class     avtLevelsMapper;
class     avtLevelsPointGlyphMapper;
class     avtLookupTable;

class     avtFeatureEdgesFilter;
class     avtGhostZoneAndFacelistFilter;
class     avtGhostZoneFilter;
class     avtFacelistFilter;
class     avtFilledBoundaryFilter;
class     avtSmoothPolyDataFilter;


// ****************************************************************************
//  Method: avtFilledBoundaryPlot
//
//  Purpose:
//      A concrete type of avtPlot for boundarys.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
//  Modifications:
//
//    Mark C. Miller, Wed Mar 24 19:23:21 PST 2004
//    Added AttributesDependOnDatabaseMetaData
//
//    Kathleen Bonnell, Fri Nov 12 10:42:08 PST 2004 
//    Changed mapper to type avtLevelsPointGlyphMapper. 
//
//    Brad Whitlock, Thu Jul 21 15:35:39 PST 2005
//    Added SetPointGlyphSize.
//
//    Kathleen Biagas, Tue Aug 23 11:19:44 PDT 2016
//    Added LevelsMapper as points and surfaces no longer handled by
//    same mapper.
//
// ****************************************************************************

class
avtFilledBoundaryPlot : public avtVolumeDataPlot
{
  public:
                              avtFilledBoundaryPlot();
    virtual                  ~avtFilledBoundaryPlot();

    static avtPlot           *Create();

    virtual const char       *GetName(void) { return "FilledBoundaryPlot"; };

    virtual void              SetAtts(const AttributeGroup*);
    virtual void              ReleaseData(void);
    virtual bool              SetColorTable(const char *ctName);

    virtual bool              AttributesDependOnDatabaseMetaData(void)
                                  { return true; };

    void                      SetLegend(bool);
    void                      SetLineWidth(int);

  protected:
    avtFeatureEdgesFilter           *wf;
    avtGhostZoneAndFacelistFilter   *gzfl;
    avtGhostZoneFilter              *gz;
    avtFacelistFilter               *fl;
    avtFilledBoundaryFilter         *sub;
    avtSmoothPolyDataFilter         *smooth;

    FilledBoundaryAttributes   atts;
    avtLevelsPointGlyphMapper *glyphMapper;
    avtLevelsMapper           *levelsMapper;
    avtLevelsLegend           *levelsLegend;
    avtLegend_p                levLegendRefPtr;
    avtLookupTable             *avtLUT;

    void                      SetColors(void); 
    void                      SortLabels(void);
    void                      SetPointGlyphSize();
    virtual avtMapperBase    *GetMapper(void);
    virtual avtDataObject_p   ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p   ApplyRenderingTransformation(avtDataObject_p);
    virtual void              CustomizeBehavior(void);

    virtual avtLegend_p       GetLegend(void) { return levLegendRefPtr; };
};


#endif


