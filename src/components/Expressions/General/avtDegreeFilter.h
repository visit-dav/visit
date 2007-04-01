// ************************************************************************* //
//                             avtDegreeFilter.h                             //
// ************************************************************************* //

#ifndef AVT_DEGREE_FILTER_H
#define AVT_DEGREE_FILTER_H


#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtDegreeFilter
//
//  Purpose:
//      A filter that creates a new vertex centered variable that counts the
//      number of cells incident to each vertex.
//
//  Programmer: Hank Childs
//  Creation:   June 7, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class EXPRESSION_API avtDegreeFilter : public avtSingleInputExpressionFilter
{
  public:
                              avtDegreeFilter();
    virtual                  ~avtDegreeFilter();

    virtual const char       *GetType(void)   { return "avtDegreeFilter"; };
    virtual const char       *GetDescription(void)
                                 { return "Calculating Degree of Each Node"; };

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
};


#endif


