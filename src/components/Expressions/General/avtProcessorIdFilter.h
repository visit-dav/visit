// ************************************************************************* //
//                           avtProcessorIdFilter.h                          //
// ************************************************************************* //

#ifndef AVT_PROCESSOR_ID_FILTER_H
#define AVT_PROCESSOR_ID_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtProcessorIdFilter
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
// ****************************************************************************

class EXPRESSION_API avtProcessorIdFilter : public avtSingleInputExpressionFilter
{
  public:
                              avtProcessorIdFilter();
    virtual                  ~avtProcessorIdFilter();

    virtual const char       *GetType(void) { return "avtProcessorIdFilter"; };
    virtual const char       *GetDescription(void)
                                           {return "Assigning processor ID.";};
  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual bool              IsPointVariable(void)  { return true; };
};


#endif


