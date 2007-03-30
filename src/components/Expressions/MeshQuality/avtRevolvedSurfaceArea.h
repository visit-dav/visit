// ************************************************************************* //
//                          avtRevolvedSurfaceArea.h                         //
// ************************************************************************* //

#ifndef AVT_REVOLVED_SURFACE_AREA_H
#define AVT_REVOLVED_SURFACE_AREA_H


#include <avtSingleInputExpressionFilter.h>

class     vtkCell;


// ****************************************************************************
//  Class: avtRevolvedSurfaceArea
//
//  Purpose:
//      Calculates the surface area a 1D line would have it if were revolved
//      around the line Y=0.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2003
//
// ****************************************************************************

class EXPRESSION_API avtRevolvedSurfaceArea 
    : public avtSingleInputExpressionFilter
{
  public:
                                avtRevolvedSurfaceArea();

    virtual const char         *GetType(void) 
                                          { return "avtRevolvedSurfaceArea"; };
    virtual const char         *GetDescription(void)
                               { return "Calculating revolved surface area"; };
    
  protected:
    bool                        haveIssuedWarning;

    virtual vtkDataArray       *DeriveVariable(vtkDataSet *);
    virtual void                PreExecute(void);

    virtual bool                IsPointVariable(void)  { return false; };

    double                      GetLineArea(vtkCell *);
    double                      GetTriangleVolume(double [3], double [3]);
    double                      RevolveLineSegment(double [2], double [2],
                                                   double *);
};


#endif


