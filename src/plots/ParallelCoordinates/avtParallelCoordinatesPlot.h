// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtParallelCoordinatesPlot.h                      //
// ************************************************************************* //

#ifndef AVT_PARALLEL_COORDINATES_PLOT_H
#define AVT_PARALLEL_COORDINATES_PLOT_H

#include <avtPlot.h>
#include <ParallelCoordinatesAttributes.h>
#include <avtParallelCoordinatesFilter.h>

class avtLevelsMapper;
class avtLookupTable;


// *****************************************************************************
//  Method: avtParallelCoordinatesPlot
//
//  Purpose: This class draws a parallel coordinate plot.  In this type of plot,
//           an n-dimensional space is represented as n parallel coordinate axes
//           and a point in that space is represented as a polyline of n vertices
//           and n-1 segments whose i-th vertex lies on the i-th axis at a point
//           corresponding to the point's i-th coordinate.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Notes: initial implementation taken from Mark Blair's ParallelAxis plot.
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Mar  3 13:37:13 PST 2009
//    Removed CanDo2DViewScaling (moved into Viewer PluginInfo)
//
//    Hank Childs, Mon Apr  6 14:09:41 PDT 2009
//    Reimplement RegisterNamedSelection.
//
// ****************************************************************************

class avtParallelCoordinatesPlot : public avtSurfaceDataPlot
{
public:
                             avtParallelCoordinatesPlot();
    virtual                 ~avtParallelCoordinatesPlot();

    static avtPlot          *Create();

    virtual const char      *GetName(void) { return "ParallelCoordinatesPlot"; };

    virtual void             SetAtts(const AttributeGroup*);
    virtual void             ReleaseData(void);

    virtual void             RegisterNamedSelection(const std::string &s)
                                { namedSelections.push_back(s); };

    virtual bool             CompatibleWithCumulativeQuery() const { return false; }

protected:
    avtParallelCoordinatesFilter   *parAxisFilter;
    avtLevelsMapper         *levelsMapper;

    avtLookupTable          *avtLUT;
    ParallelCoordinatesAttributes  atts;
    double                   bgColor[3];
    std::vector<std::string> namedSelections;

    virtual avtMapperBase   *GetMapper(void);
    virtual avtDataObject_p  ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p  ApplyRenderingTransformation(avtDataObject_p);
    virtual void             CustomizeBehavior(void);
    virtual void             CustomizeMapper(avtDataObjectInformation &);
    avtContract_p EnhanceSpecification(avtContract_p in_spec);
    virtual avtLegend_p      GetLegend(void) { return NULL; };
    void                     SetColors();
    virtual bool             SetBackgroundColor(const double *);
};

#endif


