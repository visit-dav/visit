// ************************************************************************* //
//                            avtRecenterFilter.h                            //
// ************************************************************************* //

#ifndef AVT_RECENTER_FILTER_H
#define AVT_RECENTER_FILTER_H

#include <avtSingleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtRecenterFilter
//
//  Purpose:
//      Recenters a variable.
//          
//  Programmer: Hank Childs
//  Creation:   December 10, 2003
//
// ****************************************************************************

class EXPRESSION_API avtRecenterFilter : public avtSingleInputExpressionFilter
{
  public:
                              avtRecenterFilter() {;};
    virtual                  ~avtRecenterFilter() {;};

    virtual const char       *GetType(void) { return "avtRecenterFilter"; };
    virtual const char       *GetDescription(void) {return "Recentering";};

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual bool              IsPointVariable(void);
};


#endif


