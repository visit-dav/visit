// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtProcessorIdExpression.h                          //
// ************************************************************************* //

#ifndef AVT_PROCESSOR_ID_FILTER_H
#define AVT_PROCESSOR_ID_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtProcessorIdExpression
//
//  Purpose:
//      Identifies the processor id for each dataset.  This is mostly good for
//      debugging.
//          
//  Programmer: Hank Childs
//  Creation:   November 19, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Wed Aug 22 09:50:06 PDT 2007
//    Declare variable dimension to always be 1.
//
// ****************************************************************************

class EXPRESSION_API avtProcessorIdExpression : public avtSingleInputExpressionFilter
{
  public:
                              avtProcessorIdExpression();
    virtual                  ~avtProcessorIdExpression();

    virtual const char       *GetType(void) { return "avtProcessorIdExpression"; };
    virtual const char       *GetDescription(void)
                                           {return "Assigning processor ID.";};
  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool              IsPointVariable(void)  { return true; };
    virtual int               GetVariableDimension() { return 1; }
};


#endif


