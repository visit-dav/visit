// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtHistogramMapper.h                           //
// ************************************************************************* //

#ifndef AVT_HISTOGRAMMAPPER_H
#define AVT_HISTOGRAMMAPPER_H

#include <avtMapper.h>


// ****************************************************************************
//  Class:  avtHistogramMapper
//
//  Purpose:
//      Does not map scalars to colors, all actors colored by single color.
//      Allows changing edge colors and edge visibility
//  
//      Specializes avtMapper by allowing edges to be drawn at same time
//      as surface.
//
//      Surface and edges can have different colors.
//
//      Ignores Lighting.
//
//  Programmer: Kathleen Biagas 
//  Creation:   May 11, 2016 
//
//  Modifications:
//
// ****************************************************************************

class avtHistogramMapper : public avtMapper
{
  public:
                               avtHistogramMapper();
    virtual                   ~avtHistogramMapper();


    // From avtMapper
    virtual bool               GetLighting(void) { return false; }
    virtual void               SetSurfaceRepresentation(int rep);

    // this class
    void                       SetEdgeVisibility(bool val);

    void                       SetColor(double rgb[3]);
    void                       SetEdgeColor(double rgb[3]);

    void                       SetOpacity(double val);
    void                       SetLineWidth(int lw);

  protected:
    bool                       edgeVis;
    double                     edgeColor[3];
    double                     surfaceColor[3];
    double                     opacity;
    int                        lineWidth;

    virtual void               CustomizeMappers(void);
    void                       NotifyTransparencyActor();
};


#endif


