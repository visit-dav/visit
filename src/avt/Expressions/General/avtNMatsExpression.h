// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtNMatsExpression.h                             //
// ************************************************************************* //

#ifndef AVT_NMATS_FILTER_H
#define AVT_NMATS_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;

// ****************************************************************************
//  Class: avtNMatsExpression
//
//  Purpose:
//      Determines the number of materials in a zone.
//          
//  Programmer: Hank Childs
//  Creation:   August 21, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class EXPRESSION_API avtNMatsExpression : public avtSingleInputExpressionFilter
{
  public:
                              avtNMatsExpression();
    virtual                  ~avtNMatsExpression();

    virtual const char       *GetType(void) { return "avtNMatsExpression"; };
    virtual const char       *GetDescription(void)
                                           {return "Finding NMats";};
  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool              IsPointVariable(void)  { return false; };

    virtual avtContract_p
                             ModifyContract(avtContract_p);
};


#endif


