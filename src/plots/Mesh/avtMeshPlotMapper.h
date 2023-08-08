// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  avtMeshPlotMapper.h
// ****************************************************************************

#ifndef AVT_MESHPLOTMAPPER_H
#define AVT_MESHPLOTMAPPER_H

#include <avtMapper.h>
#include <GlyphTypes.h>

#include <string>

// ****************************************************************************
//  Class:  avtMeshPlotMapper
//
//  Purpose:
//      Mesh plot specific mapper.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 29, 2016
//
//  Modifications:
//    Kathleen Biagas, Wed Apr  3 16:11:16 PDT 2019
//    Added PointSize.
//
//    Kathleen Biagas, Wed Jun 10 13:09:57 PDT 2020
//    Added other settings for points and point scaling.
//    Added CreateActorMapperPairs, CustomizeMappersInternal.
//
// ****************************************************************************

class avtMeshPlotMapper : public avtMapper
{
  public:
                               avtMeshPlotMapper();
    virtual                   ~avtMeshPlotMapper();

    //virtual void               SetSurfaceRepresentation(int rep);
    bool        GetLighting(void) override  { return false; }
    bool        ActorIsShiftable(int) override;

    void        SetMeshColor(double rgb[3]);
    void        SetSurfaceColor(double rgb[3]);

    void        SetOpacity(double val);
    void        SetLineWidth(int lw);
    void        SetSurfaceVisibility(bool);
    void        ScaleByVar(const std::string &);
    void        DataScalingOn(const std::string &, int = 1);
    void        DataScalingOff(void);

    void        SetScale(double);
    void        SetGlyphType(GlyphType);
    void        SetAutoSize(bool val);
    void        SetPointSize(double s);

    bool        SetFullFrameScaling(bool, const double *) override;

    void        InvalidateTransparencyCache(void);

  protected:
    // these are called from avtMapper
    void        CreateActorMapperPairs(vtkDataSet **children) override;
    void        CustomizeMappers(void) override;
    void        SetLabels(std::vector<std::string> &, bool) override;

  private:

    bool        surfaceVis;
    int         lineWidth;
    int         spatialDim;
    double      scale;
    std::string scalingVarName;
    GlyphType   glyphType;
    bool        autoSize;
    double      pointSize;
    bool        dataScaling;

    double      opacity;
    double      linesColor[3];
    double      polysColor[3];
    std::vector<std::string>   labels;

    void        CustomizeMappersInternal(bool invalidateTransparency=false);

    void        NotifyTransparencyActor(void);

    double      ComputePointSize(bool);
};


#endif


