// ************************************************************************* //
//                             avtBinaryMathFilter.h                         //
// ************************************************************************* //

#ifndef AVT_BINARY_MATH_FILTER_H
#define AVT_BINARY_MATH_FILTER_H

#include <expression_exports.h>
#include <avtMultipleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtBinaryMathFilter
//
//  Purpose:
//      A filter that performs a calculation on two variables.
//
//  Programmer: Sean Ahern
//  Creation:   Tue Jun 25 19:40:54 PDT 2002
//
//  Modifications:
//      Sean Ahern, Fri Jun 13 11:16:09 PDT 2003
//      Added the NumVariableArguments function, specifying that all
//      subclasses of BinaryMathFilter process two variable arguments.
//
//      Hank Childs, Thu Feb  5 17:11:06 PST 2004
//      Moved inlined constructor and destructor definitions to .C files
//      because certain compilers have problems with them.
//
//      Hank Childs, Thu Jan 20 17:30:09 PST 2005
//      Add data member cur_mesh so derived types can access mesh when 
//      deriving variables.
//
//      Hank Childs, Fri Aug 19 14:04:25 PDT 2005
//      Do a better job of determining the variable dimension.
//
// ****************************************************************************

class EXPRESSION_API avtBinaryMathFilter 
    : public avtMultipleInputExpressionFilter
{
  public:
                              avtBinaryMathFilter();
    virtual                  ~avtBinaryMathFilter();

    virtual const char       *GetType(void)   { return "avtBinaryMathFilter";};
    virtual const char       *GetDescription(void) = 0;
    virtual int               NumVariableArguments() { return 2; }

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual vtkDataArray     *CreateArray(vtkDataArray *);
    virtual void              DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                          vtkDataArray *out, int, int) = 0;
    virtual int               GetNumberOfComponentsInOutput(int ncompsIn1,
                                                            int ncompsIn2)
                                         { return (ncompsIn1 > ncompsIn2
                                                    ? ncompsIn1 
                                                    : ncompsIn2);
                                         };
    virtual int               GetVariableDimension(void);

    avtCentering              centering;
    vtkDataSet               *cur_mesh;
};


#endif


