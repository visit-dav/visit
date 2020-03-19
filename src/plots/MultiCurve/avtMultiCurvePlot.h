// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtMultiCurvePlot.h                          //
// ************************************************************************* //

#ifndef AVT_MultiCurve_PLOT_H
#define AVT_MultiCurve_PLOT_H


#include <avtPlot.h>

#include <MultiCurveAttributes.h>

class     avtLevelsLegend;
class     avtLevelsMapper;
class     avtLookupTable;
class     avtMultiCurveFilter;
class     avtMultiCurveLabelMapper;


// ****************************************************************************
//  Class:  avtMultiCurvePlot
//
//  Purpose:
//      A concrete type of avtPlot, this is the MultiCurve plot.
//
//  Programmer: xml2avt
//  Creation:   omitted
//
//  Modifications:
//    Eric Brugger, Fri Feb 20 16:20:09 PST 2009
//    I added a legend to the plot.
//
//    Eric Brugger, Thu Feb 28 13:46:03 PST 2013
//    I added the ability to set a scale factor and the line width for the
//    markers.
//
// ****************************************************************************

class avtMultiCurvePlot : public avtSurfaceDataPlot
{
  public:
                                avtMultiCurvePlot();
    virtual                    ~avtMultiCurvePlot();

    virtual const char         *GetName(void) { return "MultiCurvePlot"; };

    static avtPlot             *Create();

    virtual void                SetAtts(const AttributeGroup*);
    void                        SetLineWidth(int);
    void                        SetMarkerScale(double);
    void                        SetMarkerLineWidth(int);
    void                        SetLegend(bool);

  protected:
    MultiCurveAttributes        atts;

    avtMultiCurveFilter        *MultiCurveFilter;

    avtLevelsMapper            *levelsMapper;
    avtLevelsLegend            *levelsLegend;
    avtLegend_p                 levelsLegendRefPtr;
    avtMultiCurveLabelMapper   *decoMapper;
    avtLookupTable             *avtLUT;

    virtual avtMapperBase      *GetMapper(void);
    virtual avtDataObject_p     ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p     ApplyRenderingTransformation(avtDataObject_p);
    virtual void                CustomizeBehavior(void);
    virtual void                CustomizeMapper(avtDataObjectInformation &);

    virtual avtLegend_p         GetLegend(void) { return NULL; };
    virtual avtDecorationsMapper *GetDecorationsMapper(void);
    virtual avtContract_p       EnhanceSpecification(avtContract_p);
};


#endif
