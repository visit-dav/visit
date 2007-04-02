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
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Thu Oct  6 15:41:21 PDT 2005
//    Added support for recentering vectors ['6637].
//
// ****************************************************************************

class EXPRESSION_API avtRecenterFilter : public avtSingleInputExpressionFilter
{
  public:
                              avtRecenterFilter();
    virtual                  ~avtRecenterFilter();

    virtual const char       *GetType(void) { return "avtRecenterFilter"; };
    virtual const char       *GetDescription(void) {return "Recentering";};

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual bool              IsPointVariable(void);
    virtual int               GetVariableDimension(void);
};


#endif


