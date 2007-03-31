// ************************************************************************* //
//                          avtVectorComposeFilter.h                         //
// ************************************************************************* //

#ifndef AVT_VECTOR_COMPOSE_FILTER_H
#define AVT_VECTOR_COMPOSE_FILTER_H

#include <avtMultipleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtVectorComposeFilter
//
//  Purpose:
//      Creates a vector variable out of three components.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Mar  6 19:15:24 America/Los_Angeles 2003
//
//  Modifications:
//      Sean Ahern, Fri Jun 13 11:39:45 PDT 2003
//      Added NumVariableArguments, specifying that it processes three
//      variable arguments.
//
// ****************************************************************************

class EXPRESSION_API avtVectorComposeFilter 
    : public avtMultipleInputExpressionFilter
{
  public:
                              avtVectorComposeFilter() {;}
    virtual                  ~avtVectorComposeFilter() {;}

    virtual const char       *GetType(void)  
                                    { return "avtVectorComposeFilter"; };
    virtual const char       *GetDescription(void)
                                 {return "Creating a vector from components";};
    virtual int               NumVariableArguments() { return 3; }

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual int               GetVariableDimension(void);
};


#endif


