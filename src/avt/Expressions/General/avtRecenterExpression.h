// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtRecenterExpression.h                            //
// ************************************************************************* //

#ifndef AVT_RECENTER_FILTER_H
#define AVT_RECENTER_FILTER_H

#include <avtSingleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtRecenterExpression
//
//  Purpose:
//      Recenters a variable.
//          
//  Programmer: Hank Childs
//  Creation:   December 10, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Thu Oct  6 15:41:21 PDT 2005
//    Added support for recentering vectors ['6637].
//    
//    Sean Ahern, Wed Sep 10 12:01:43 EDT 2008
//    Added optional argument to specify explicit centering.
//
//    Jeremy Meredith, Tue Apr 28 13:52:59 EDT 2009
//    Add singleton support, now that the base avtExpressionFilter class'
//    Recenter method also supports singletons.
//
//    Cyrus Harrison, Wed Apr 17 13:46:14 PDT 2013
//    Added modify contract, so we can request ghosts.
//
// ****************************************************************************

class EXPRESSION_API avtRecenterExpression : public avtSingleInputExpressionFilter
{
  public:
                              avtRecenterExpression();
    virtual                  ~avtRecenterExpression();

    virtual const char       *GetType(void) { return "avtRecenterExpression"; };
    virtual const char       *GetDescription(void) {return "Recentering";};
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);

    typedef enum {
        Toggle,
        Nodal,
        Zonal
    } RecenterType;

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool              IsPointVariable(void);
    virtual int               GetVariableDimension(void);
    virtual bool              CanHandleSingletonConstants(void) {return true;}
    virtual avtContract_p     ModifyContract(avtContract_p);

    RecenterType              recenterMode;
};


#endif


