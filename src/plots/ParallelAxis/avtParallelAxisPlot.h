// ************************************************************************* //
//                            avtParallelAxisPlot.h                          //
// ************************************************************************* //

#ifndef AVT_PARALLEL_AXIS_PLOT_H
#define AVT_PARALLEL_AXIS_PLOT_H

#include <avtPlot.h>
#include <ParallelAxisAttributes.h>

class avtParallelAxisFilter;
class avtLevelsMapper;
class avtLookupTable;


// *****************************************************************************
//  Method: avtParallelAxisPlot
//
//  Purpose: This class draws a parallel coordinate plot.  In this type of plot,
//           an n-dimensional space is represented as n parallel coordinate axes
//           and a point in that space is represented as a polyline of n vertices
//           and n-1 segments whose i-th vertex lies on the i-th axis at a point
//           corresponding to the point's i-th coordinate.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//   
// ****************************************************************************

class avtParallelAxisPlot : public avtSurfaceDataPlot
{
public:
                             avtParallelAxisPlot();
    virtual                 ~avtParallelAxisPlot();

    static avtPlot          *Create();

    virtual const char      *GetName(void) { return "ParallelAxisPlot"; };

    virtual void             SetAtts(const AttributeGroup*);
    virtual void             ReleaseData(void);

protected:
    avtParallelAxisFilter   *parAxisFilter;
    avtLevelsMapper         *levelsMapper;

    avtLookupTable          *avtLUT;
    ParallelAxisAttributes  atts;

    virtual avtMapper       *GetMapper(void);
    virtual avtDataObject_p  ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p  ApplyRenderingTransformation(avtDataObject_p);
    virtual void             CustomizeBehavior(void);
    virtual void             CustomizeMapper(avtDataObjectInformation &);
    avtPipelineSpecification_p EnhanceSpecification(avtPipelineSpecification_p in_spec);
    virtual avtLegend_p      GetLegend(void) { return NULL; };
    void                     SetColors();
};

#endif


