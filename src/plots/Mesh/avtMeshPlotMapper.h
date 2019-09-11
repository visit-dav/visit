// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtMeshPlotMapper.h                            //
// ************************************************************************* //

#ifndef AVT_MESHPLOTMAPPER_H
#define AVT_MESHPLOTMAPPER_H

#include <avtMapper.h>


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
// ****************************************************************************

class avtMeshPlotMapper : public avtMapper
{
  public:
                               avtMeshPlotMapper();
    virtual                   ~avtMeshPlotMapper();

    //virtual void               SetSurfaceRepresentation(int rep);
    virtual bool               GetLighting(void) { return false; }
    virtual bool               ActorIsShiftable(int);

    void                       SetMeshColor(double rgb[3]);
    void                       SetSurfaceColor(double rgb[3]);

    void                       SetOpacity(double val);
    void                       SetLineWidth(int lw);
    void                       SetSurfaceVisibility(bool);
    void                       SetPointSize(int ps);

    void                       InvalidateTransparencyCache(void);

  protected:
    virtual void               CustomizeMappers(void);
    virtual void               SetLabels(std::vector<std::string> &, bool);

  private:

    bool surfaceVis;
    int lineWidth;
    int pointSize;
    double opacity;
    double linesColor[3];
    double polysColor[3];
    std::vector<std::string>   labels;

    void                       NotifyTransparencyActor(void);

};


#endif


