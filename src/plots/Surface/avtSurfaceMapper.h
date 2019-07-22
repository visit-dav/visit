// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtSurfaceMapper.h                              //
// ************************************************************************* //

#ifndef AVT_SURFACEMAPPER_H
#define AVT_SURFACEMAPPER_H

#include <avtMapper.h>
#include <string>
#include <vector>

class vtkLookupTable;

// ****************************************************************************
//  Class:  avtSurfaceMapper
//
//  Purpose:
//      Surface plot specific mapper.
//
//  Programmer: Kathleen Biagas
//  Creation:   July 18, 2016
//
//  Modifications:
//
// ****************************************************************************

class avtSurfaceMapper : public avtMapper
{
  public:
                               avtSurfaceMapper();
    virtual                   ~avtSurfaceMapper();

    // these are called from avtMapper
    virtual void               SetSurfaceRepresentation(int rep);
    virtual bool               GetLighting(void) { return !ignoreLighting; }


    // these are called from the plot

    void                       SetEdgeVisibility(bool);
    void                       SetEdgeColor(double rgb[3]);
    void                       SetLineWidth(int lw);
    void                       SetSurfaceColor(double rgb[3]);


    void                       CanApplyGlobalRepresentation(bool);
    void                       SetRepresentation(bool);
    void                       SetIgnoreLighting(bool);
    void                       SetLookupTable(vtkLookupTable *);

    void                       SetScalarVisibility(bool);
    void                       SetScalarRange(double, double);
    bool                       GetDataRange(double &rmin, double &rmax);
    bool                       GetCurrentDataRange(double &rmin, double &rmax);

  protected:
    // these are called from avtMapper
    virtual void               CustomizeMappers(void);

  private:

    bool            edgeVis;
    bool            scalarVis;
    int             lineWidth;
    double          edgeColor[3];
    double          surfaceColor[3];
    bool            canApplyGlobalRep;
    bool            ignoreLighting;
    bool            wireMode;
    vtkLookupTable *lut;
    double          scalarRange[2];

    void                       NotifyTransparencyActor(void);


};


#endif


