// ************************************************************************* //
//                             avtNodeDegreeFilter.h                         //
// ************************************************************************* //

#ifndef AVT_NODE_DEGREE_FILTER_H
#define AVT_NODE_DEGREE_FILTER_H

#include <expression_exports.h>

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtNodeDegreeFilter
//
//  Purpose:
//      A filter that creates a new vertex centered variable that counts the
//      number of unique edges that go through each vertex.
//
//  Programmer: Akira Haddox
//  Creation:   June 27, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class EXPRESSION_API avtNodeDegreeFilter : public avtSingleInputExpressionFilter
{
  public:
                              avtNodeDegreeFilter();
    virtual                  ~avtNodeDegreeFilter();

    virtual const char       *GetType(void)  { return "avtNodeDegreeFilter"; };
    virtual const char       *GetDescription(void)
                             { return "Calculating NodeDegree of Each Node"; };

  protected:
    virtual bool              IsPointVariable(void) { return true; };
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
};


#endif


