// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtPseudocolorMapper.h                              //
// ************************************************************************* //

#ifndef AVT_PSEUDOCOLORMAPPER_H
#define AVT_PSEUDOCOLORMAPPER_H

#include <avtVariableMapper.h>
#include <GlyphTypes.h>

#include <string>
#include <vector>


// ****************************************************************************
//  Class:  avtPseudocolorMapper
//
//  Purpose:
//      Pseudocolor plot specific mapper, that utilizes a specialized
//      vtkDataSetMapper allowing Multiple representations of the same dataset
//      to be rendered at the same time( eg Surface, Wireframe, and Points).
//
//  Programmer: Kathleen Biagas
//  Creation:   August 24, 2016
//
//  Modifications:
//    Kathleen Biagas, Wed Apr 10 09:06:05 PDT 2019
//    Added pointSize.
//
//    Kathleen Biagas, Tue Nov  5 11:53:53 PST 2019
//    Added methods and ivars for handling points.
//    Added override methods for SetFullFrameScaling, CreateActorMapperPairs,
//    and SetLabels.  Added CustomizeMappersInternal.
//
// ****************************************************************************

class avtPseudocolorMapper : public avtVariableMapper
{
  public:
                avtPseudocolorMapper();
    virtual    ~avtPseudocolorMapper();

    void        SetDrawSurface(bool);
    void        SetDrawWireframe(bool);
    void        SetWireframeColor(double rgb[3]);

    // For points glpyhing
    void        SetDrawPoints(bool);
    void        SetPointsColor(double rgb[3]);

    void        ColorByScalarOn(const std::string &);
    void        ColorByScalarOff(void);

    void        ScaleByVar(const std::string &);
    void        DataScalingOn(const std::string &, int = 1);
    void        DataScalingOff(void);

    void        SetScale(double);
    void        SetGlyphType(GlyphType);
    void        SetPointSize(double s);

    bool        SetFullFrameScaling(bool, const double *) override;

  protected:
    // these are called from avtMapper
    void        CreateActorMapperPairs(vtkDataSet **children) override;
    void        CustomizeMappers(void) override;
    void        SetLabels(std::vector<std::string> &, bool) override;

  private:

    bool        drawSurface;
    bool        drawWireframe;
    double      wireframeColor[3];

    bool        drawPoints;
    double      pointsColor[3];
    bool        colorByScalar;
    int         spatialDim;
    double      scale;
    std::string scalingVarName;
    std::string coloringVarName;
    GlyphType   glyphType;
    double      pointSize;
    bool        dataScaling;

    std::vector<std::string> labels;

    void        CustomizeMappersInternal(bool invalidateTransparency=false);
};


#endif


