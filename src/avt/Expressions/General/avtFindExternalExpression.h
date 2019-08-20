// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtFindExternalExpression.h                       //
// ************************************************************************* //

#ifndef AVT_FIND_EXTERNAL_EXPRESSION_H
#define AVT_FIND_EXTERNAL_EXPRESSION_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtFindExternalExpression
//
//  Purpose:
//      An expression that determines which nodes or cells are external.
//          
//  Programmer: Hank Childs
//  Creation:   September 22, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Feb  4 13:46:18 PST 2011
//    Expand this expression to also deal with external cells.  Rename.
//
//    Hank Childs, Wed Feb 16 13:01:36 PST 2011
//    Make sure the output is always a scalar.
//
// ****************************************************************************

class EXPRESSION_API avtFindExternalExpression 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtFindExternalExpression();
    virtual                  ~avtFindExternalExpression();

    virtual const char       *GetType(void) 
                                      { return "avtFindExternalExpression"; };
    virtual const char       *GetDescription(void)
                                      {return "Determining exterior"; };

    void                      SetDoCells(bool dc) { doCells = dc; };

  protected:
    bool                      doCells;
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool              IsPointVariable(void)  { return !doCells; };
    virtual int               GetVariableDimension() { return 1; }
};


#endif


