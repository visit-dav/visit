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


