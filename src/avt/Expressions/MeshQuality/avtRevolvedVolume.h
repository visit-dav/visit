// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtRevolvedVolume.h                              //
// ************************************************************************* //

#ifndef AVT_REVOLVED_VOLUME_H
#define AVT_REVOLVED_VOLUME_H


#include <avtSingleInputExpressionFilter.h>

class     vtkCell;


// ****************************************************************************
//  Class: avtRevolvedVolume
//
//  Purpose:
//      Calculates the volume a 2D polygon would occupy if it were revolved
//      around an axis.
//
//  Programmer: Hank Childs
//  Creation:   September 8, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  3 11:24:40 PST 2006
//    Added revolveAboutX, GetTriangleVolume2. 
//
//    Hank Childs, Wed May 21 15:55:53 PDT 2008
//    Re-define GetVariableDimension.
//
// ****************************************************************************

class EXPRESSION_API avtRevolvedVolume : public avtSingleInputExpressionFilter
{
  public:
                                avtRevolvedVolume();

    virtual const char         *GetType(void) { return "avtRevolvedVolume"; };
    virtual const char         *GetDescription(void)
                                    { return "Calculating revolved volume"; };
    
  protected:
    bool                        haveIssuedWarning;
    bool                        revolveAboutX;

    virtual vtkDataArray       *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual void                PreExecute(void);

    virtual bool                IsPointVariable(void)  { return false; };
    virtual int                 GetVariableDimension()   { return 1; };

    double                      GetZoneVolume(vtkCell *);
    double                      GetTriangleVolume(double [3], double [3]);
    double                      GetTriangleVolume2(double [3], double [3]);
    double                      RevolveLineSegment(double [2], double [2],
                                                   double *);
};


#endif


