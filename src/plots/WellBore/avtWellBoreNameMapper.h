// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtWellBoreNameMapper.h                            //
// ************************************************************************* //

#ifndef AVT_WELLBORENAME_MAPPER_H
#define AVT_WELLBORENAME_MAPPER_H


#include <avtDecorationsMapper.h>

class     vtkDataSet;


// ****************************************************************************
//  Class: avtWellBoreNameMapper
//
//  Purpose:
//      A mapper for well bore names.  This extends the functionality of a 
//      decorations mapper by mapping a label-glyph onto a well dataset.
//
//  Programmer: Eric Brugger
//  Creation:   November 6, 2008 
//
// ****************************************************************************

class avtWellBoreNameMapper : public avtDecorationsMapper
{
  public:
                               avtWellBoreNameMapper();
    virtual                   ~avtWellBoreNameMapper();

    void                       SetLabelColor(double [3]);
    void                       SetLabelColor(double, double, double);
    void                       SetScale(double);
    void                       SetLabelVisibility(bool);

  protected:
    bool                       labelVis;
    double                     labelColor[3];
    double                     scale;

    virtual void               CustomizeMappers(void);

    virtual void               SetDatasetInput(vtkDataSet *, int);
};


#endif


