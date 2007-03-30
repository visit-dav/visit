// ************************************************************************* //
//                          avtFilledBoundaryPlot.h                          //
// ************************************************************************* //

#ifndef AVT_FILLED_BOUNDARY_PLOT_H
#define AVT_FILLED_BOUNDARY_PLOT_H


#include <avtPlot.h>
#include <FilledBoundaryAttributes.h>

class     avtLevelsLegend;
class     avtLevelsMapper;
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

    void                      SetLegend(bool);
    void                      SetLineWidth(int);
    void                      SetLineStyle(int);

  protected:
    avtFeatureEdgesFilter           *wf;
    avtGhostZoneAndFacelistFilter   *gzfl;
    avtGhostZoneFilter              *gz;
    avtFacelistFilter               *fl;
    avtFilledBoundaryFilter         *sub;
    avtSmoothPolyDataFilter         *smooth;

    FilledBoundaryAttributes atts;
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


