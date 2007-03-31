// ************************************************************************* //
//                        avtCurveConstructorFilter.h                        //
// ************************************************************************* //

#ifndef AVT_CURVE_CONSTRUCTOR_FILTER_H
#define AVT_CURVE_CONSTRUCTOR_FILTER_H


#include <avtDatasetToDatasetFilter.h>


// ****************************************************************************
//  Class: avtCurveConstructorFilter
//
//  Purpose:
//      A filter that will construct one uniform curve from fragments of
//      curves.
//
//  Programmer: Kathleen Bonnell
//  Creation:   Sat Apr 20 13:01:58 PST 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 12 16:53:11 PDT 2002  
//    Removed vtk filters associated with label-creation.  Now handled by
//    the plot.
//
//    Kathleen Bonnell, Mon Dec 23 08:23:26 PST 2002
//    Added RefashionDataObjectInfo. 
//    
//    Hank Childs, Fri Oct  3 11:10:29 PDT 2003
//    Moved from /plots/Curve.  Renamed to CurveConstructorFilter.
//
// ****************************************************************************

class avtCurveConstructorFilter : public avtDatasetToDatasetFilter
{
  public:
                              avtCurveConstructorFilter();
    virtual                  ~avtCurveConstructorFilter();

    virtual const char       *GetType(void)  
                                       { return "avtCurveConstructorFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Constructing Curve"; };

 
  protected:
    virtual void              Execute(void);
    virtual void              VerifyInput(void);
    avtPipelineSpecification_p
                           PerformRestriction(avtPipelineSpecification_p spec);
    virtual void              RefashionDataObjectInfo(void);
};


#endif


