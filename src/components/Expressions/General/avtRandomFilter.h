// ************************************************************************* //
//                             avtRandomFilter.h                             //
// ************************************************************************* //

#ifndef AVT_RANDOM_FILTER_H
#define AVT_RANDOM_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtRandomFilter
//
//  Purpose:
//      Creates a random number at each point.  Mostly used for odd effects in
//      movie-making.
//          
//  Programmer: Hank Childs
//  Creation:   March 7, 2003
//
// ****************************************************************************

class EXPRESSION_API avtRandomFilter : public avtSingleInputExpressionFilter
{
  public:
                              avtRandomFilter() {;};
    virtual                  ~avtRandomFilter() {;};

    virtual const char       *GetType(void) { return "avtRandomFilter"; };
    virtual const char       *GetDescription(void)
                                           {return "Assigning random #.";};
  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual bool              IsPointVariable(void)  { return true; };
};


#endif


