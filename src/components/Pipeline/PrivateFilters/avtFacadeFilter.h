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
// ****************************************************************************

class PIPELINE_API avtFacadeFilter : virtual public avtFilter
{
  public:
                                   avtFacadeFilter() {;};
    virtual                       ~avtFacadeFilter() {;};

    virtual avtDataObject_p        GetInput(void);
    virtual avtDataObject_p        GetOutput(void);

    virtual avtTerminatingSource  *GetTerminatingSource(void);
    virtual avtQueryableSource    *GetQueryableSource(void);
    virtual bool                   Update(avtPipelineSpecification_p);

  protected:
    virtual void                   SetTypedInput(avtDataObject_p);

    virtual avtFilter             *GetFirstFilter(void) = 0;
    virtual avtFilter             *GetLastFilter(void) = 0;

    virtual void                   Execute(void);
};


#endif



