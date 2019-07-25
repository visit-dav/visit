// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtPseudocolorMapper.h                              //
// ************************************************************************* //

#ifndef AVT_PSEUDOCOLORMAPPER_H
#define AVT_PSEUDOCOLORMAPPER_H

#include <avtVariableMapper.h>

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
// ****************************************************************************

class avtPseudocolorMapper : public avtVariableMapper
{
  public:
                               avtPseudocolorMapper();
    virtual                   ~avtPseudocolorMapper();


    void                       SetDrawSurface(bool);
    void                       SetDrawWireframe(bool);
    void                       SetDrawPoints(bool);
    void                       SetPointSize(int);
    void                       SetWireframeColor(double rgb[3]);
    void                       SetPointsColor(double rgb[3]);




  protected:
    // these are called from avtMapper
    virtual vtkDataSetMapper  *CreateMapper(void);
    virtual void               CustomizeMappers(void);

  private:

    bool   drawSurface;
    bool   drawWireframe;
    bool   drawPoints;
    int    pointSize;
    double wireframeColor[3];
    double pointsColor[3];
};


#endif


