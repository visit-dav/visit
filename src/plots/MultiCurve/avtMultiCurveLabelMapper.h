// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtMultiCurveLabelMapper.h                          //
// ************************************************************************* //

#ifndef AVT_MULTI_CURVE_LABEL_MAPPER_H
#define AVT_MULTI_CURVE_LABEL_MAPPER_H

#include <avtDecorationsMapper.h>
#include <ColorAttributeList.h>
#include <LineAttributes.h>

#include <vector>

class     vtkDataSet;


// ****************************************************************************
//  Class: avtMultiCurveLabelMapper
//
//  Purpose:
//    A mapper for labels for the multi curve plot.  This extends the
//    functionality of a decorations mapper by mapping markers and ids
//    onto a dataset.
//
//  Programmer: Eric Brugger
//  Creation:   December 12, 2008
//
//  Modifications:
//    Eric Brugger, Wed Feb 18 12:02:11 PST 2009
//    I added the ability to display identifiers at each of the points.
//    I replaced setLabelVisibility with setMarkerVisibility and added
//    setIdVisibility.
//
//    Eric Brugger, Mon Mar  9 17:57:34 PDT 2009
//    I enhanced the plot so that the markers and identifiers displayed for
//    the points are in the same color as the curve, instead of always in
//    black.
//
//    Eric Brugger, Tue Feb 19 16:15:46 PST 2013
//    I added the ability to set a scale factor and the line width for the
//    markers.
//
// ****************************************************************************

class avtMultiCurveLabelMapper : public avtDecorationsMapper
{
  public:
                               avtMultiCurveLabelMapper();
    virtual                   ~avtMultiCurveLabelMapper();

    void                       SetScale(double);
    void                       SetMarkerLineWidth(_LineWidth);
    void                       SetMarkerVisibility(bool);
    void                       SetIdVisibility(bool);

    void                       SetColors(const ColorAttributeList &c);
    void                       GetLevelColor(const int, double[4]);

  protected:
    bool                       markerVisibility;
    bool                       idVisibility;
    double                     scale;
    _LineWidth                 markerLineWidth;

    ColorAttributeList         cal;
    std::vector<int>           colors;

    virtual void               CustomizeMappers(void);
    virtual void               SetDatasetInput(vtkDataSet *, int);
};


#endif
