// ************************************************************************* //
//                      avtCylindricalCoordinatesFilter.h                    //
// ************************************************************************* //

#ifndef AVT_CYLINDRICAL_COORDINATES_FILTER_H
#define AVT_CYLINDRICAL_COORDINATES_FILTER_H


#include <avtSingleInputExpressionFilter.h>


class     vtkCell;
class     vtkCellDataToPointData;
class     vtkDataArray;
class     vtkDataSet;
class     vtkIdList;
class     vtkScalarData;


// ****************************************************************************
//  Class: avtCylindricalCoordinatesFilter
//
//  Purpose:
//      A filter that calculates the cylindrical coordinates for each point.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

class EXPRESSION_API avtCylindricalCoordinatesFilter 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtCylindricalCoordinatesFilter();
    virtual                  ~avtCylindricalCoordinatesFilter();

    virtual const char       *GetType(void)   
                                { return "avtCylindricalCoordinatesFilter"; };
    virtual const char       *GetDescription(void)
                           { return "Calculating cylindrical coordinates."; };

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual bool              IsPointVariable(void)  { return true; };  
    virtual int               GetVariableDimension() { return 3; }
};


#endif


