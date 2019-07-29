// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtMatvfExpression.h                             //
// ************************************************************************* //

#ifndef AVT_MATVF_FILTER_H
#define AVT_MATVF_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;
class     ConstExpr;

// ****************************************************************************
//  Class: avtMatvfExpression
//
//  Purpose:
//      Creates the material fraction at each point.
//          
//  Programmer: Sean Ahern
//  Creation:   March 18, 2003
//
//  Modifications:
//    Jeremy Meredith, Mon Sep 29 12:13:04 PDT 2003
//    Added support for integer material indices.
//
//    Hank Childs, Fri Oct 24 14:49:23 PDT 2003
//    Added ModifyContract.  This is because matvf does not work with
//    ghost zone communication.  It cannot get the avtMaterial object with
//    ghost information and it causes an exception.  This will tell the
//    database that it cannot communicate ghost zones until a better solution
//    comes along.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Wed Feb 18 09:15:23 PST 2004
//    Issue a warning if we encounter a bad material name.
//
//    Cyrus Harrison, Mon Jun 18 10:52:45 PDT 2007
//    Ensure that the output is always scalar (added GetVariableDimension)
//
//    Cyrus Harrison, Tue Feb 19 13:20:50 PST 2008
//    Added doPostGhost member. 
//
// ****************************************************************************

class EXPRESSION_API avtMatvfExpression : public avtSingleInputExpressionFilter
{
  public:
                              avtMatvfExpression();
    virtual                  ~avtMatvfExpression();

    virtual const char       *GetType(void) { return "avtMatvfExpression"; };
    virtual const char       *GetDescription(void)
                                           {return "Calculating Material VF";};
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);


  protected:
    virtual int               GetVariableDimension(void) { return 1; };

    virtual avtContract_p
                              ModifyContract(avtContract_p);

    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool              IsPointVariable(void)  { return false; };
    virtual void              PreExecute(void);

    void                      AddMaterial(ConstExpr *);

    bool                      doPostGhost;
    bool                      issuedWarning;
    std::vector<std::string>  matNames;
    std::vector<int>          matIndices;
};


#endif


