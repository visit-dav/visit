// ************************************************************************* //
//                              avtFacadeFilter.h                            //
// ************************************************************************* //

#ifndef AVT_FACADE_FILTER
#define AVT_FACADE_FILTER

#include <pipeline_exports.h>

#include <avtFilter.h>

// ****************************************************************************
//  Class: avtFacadeFilter
//
//  Purpose:
//      Sometimes a filter is wrapped around another filter or a pipeline of
//      filters.  This can be so that it does a conversion of the attributes or
//      so that it can meet some interface that the original filter does not.
//      This class provides some of the dirty work so that pipelines can be 
//      well-maintained without getting too complicated.
//
//  Programmer: Hank Childs
//  Creation:   April 16, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Tue Jun  7 14:48:49 PDT 2005
//    Add methods to access all facaded filters, as well as populating common
//    avtFilter methods (release data, perform restriction).
//
// ****************************************************************************

class PIPELINE_API avtFacadeFilter : virtual public avtFilter
{
  public:
                                   avtFacadeFilter();
    virtual                       ~avtFacadeFilter();

    virtual avtDataObject_p        GetInput(void);
    virtual avtDataObject_p        GetOutput(void);

    virtual avtTerminatingSource  *GetTerminatingSource(void);
    virtual avtQueryableSource    *GetQueryableSource(void);
    virtual bool                   Update(avtPipelineSpecification_p);
    virtual void                   ReleaseData(void);

  protected:
    virtual void                   SetTypedInput(avtDataObject_p);

    virtual int                    GetNumberOfFacadedFilters(void) = 0;
    virtual avtFilter             *GetIthFacadedFilter(int) = 0;

    virtual void                   Execute(void);

    virtual avtPipelineSpecification_p PerformRestriction(
                                                  avtPipelineSpecification_p);
    virtual void                   RefashionDataObjectInfo(void);
};


#endif



