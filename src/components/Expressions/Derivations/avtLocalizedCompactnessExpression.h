// ************************************************************************* //
//                     avtLocalizedCompactnessExpression.h                    //
// ************************************************************************* //

#ifndef AVT_LOCALIZED_COMPACTNESS_EXPRESSION_H
#define AVT_LOCALIZED_COMPACTNESS_EXPRESSION_H

#include <avtSingleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtLocalizedCompactnessExpression
//
//  Purpose:
//      Calculates localized compactness on a per point basis.
//      This is defined as the percent of the surrounding spherical region that
//      is the same material.
//      
//  Caveats:
//      1) It is assumed that the input is a rectilinear grid.
//      2) User settable knobs are:
//         2a) the radius to consider
//         2b) a weighting factor that, for a given point, allows portions of
//             the spherical region that are closer to count more heavily.
//      3) This calculation works in 2D, but it assumes that the 2D data set
//         is in cylindrical coordinates and it will revolve the data set
//         around the X-axis.
//
//
//  Programmer: Hank Childs
//  Creation:   April 29, 2006
//
// ****************************************************************************

class EXPRESSION_API avtLocalizedCompactnessExpression
    : public avtSingleInputExpressionFilter
{
  public:
                              avtLocalizedCompactnessExpression();
    virtual                  ~avtLocalizedCompactnessExpression();

    virtual const char       *GetType(void)
                                      { return "avtLocalizedCompactnessExpression"; };
    virtual const char       *GetDescription(void)
                                      {return "Calculating localized compactness"; };

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual bool              IsPointVariable(void)  { return true; };
    virtual int               GetVariableDimension(void) { return 1; };
};



#endif


