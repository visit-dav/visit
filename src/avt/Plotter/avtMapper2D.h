// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtMapper2D.h                                   //
// ************************************************************************* //

#ifndef AVT_MAPPER2D_H
#define AVT_MAPPER2D_H

#include <plotter_exports.h>

#include <avtMapperBase.h>

class   vtkActor2D;
class   vtkAlgorithmOutput;
class   vtkMapper2D;

class   avtTransparencyActor;
class   ColorAttribute;


// ****************************************************************************
//  Class:  avtMapper2D
//
//  Purpose:
//    Container for vtkMapper2D/vtkActor2D pairs.
//
//  Programmer: Kathleen Biagas
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

class PLOTTER_API avtMapper2D : public avtMapperBase
{
  public:
                               avtMapper2D();
    virtual                   ~avtMapper2D();

    virtual void               GlobalLightingOn(void) {;}
    virtual void               GlobalLightingOff(void){;}
    virtual void               GlobalSetAmbientCoefficient(const double){;}
    virtual int                SetTransparencyActor(avtTransparencyActor *);
    virtual void               SetSpecularProperties(bool,double,double,
                                                     const ColorAttribute&){;}

  protected:
    vtkMapper2D         **mappers;
    vtkActor2D          **actors;

    virtual void               ClearSelf(void);
    void                       SetDefaultRange(void);
    void                       SetUpDrawable(void);
    virtual void               CreateActorMapperPairs(vtkDataSet **);

    virtual void               SetUpFilters(int nDoms);
    virtual vtkAlgorithmOutput *InsertFilters(vtkDataSet *, int dom);

    virtual vtkMapper2D                *CreateMapper2D(void);
};

#endif

