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

    void                      SetLegend(bool);
    void                      SetLineWidth(int);
    void                      SetLineStyle(int);

  protected:
    avtFeatureEdgesFilter           *wf;
    avtGhostZoneAndFacelistFilter   *gzfl;
    avtGhostZoneFilter              *gz;
    avtFacelistFilter               *fl;
    avtBoundaryFilter               *sub;
    avtSmoothPolyDataFilter         *smooth;

    BoundaryAttributes          atts;
    avtLevelsMapper          *levelsMapper;
    avtLevelsLegend          *levelsLegend;
    avtLegend_p               levLegendRefPtr;
    avtLookupTable           *avtLUT;

    void                      SetColors(void); 
    void                      SortLabels(void); 
    virtual avtMapper        *GetMapper(void);
    virtual avtDataObject_p   ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p   ApplyRenderingTransformation(avtDataObject_p);
    virtual void              CustomizeBehavior(void);

    virtual avtLegend_p       GetLegend(void) { return levLegendRefPtr; };
};


#endif


