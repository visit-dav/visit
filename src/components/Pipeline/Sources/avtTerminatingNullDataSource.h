// ************************************************************************* //
//                     avtTerminatingNullDataSource.h                        //
// ************************************************************************* //

#ifndef AVT_TERMINATING_NULL_DATA_SOURCE_H
#define AVT_TERMINATING_NULL_DATA_SOURCE_H
#include <pipeline_exports.h>


#include <avtNullDataSource.h>
#include <avtTerminatingSource.h>


// ****************************************************************************
//  Class: avtTerminatingNullDataSource
//
//  Purpose:
//      A source that terminates a pipeline.  It does an update differently
//      than what a non-terminating source (filter) would.
//
//  Programmer: Mark C. Miller
//  Creation:   January 8, 2003 
//
// ****************************************************************************

class PIPELINE_API avtTerminatingNullDataSource
    : public virtual avtNullDataSource, public virtual avtTerminatingSource
{
  public:
                                 avtTerminatingNullDataSource() {;};
    virtual                     ~avtTerminatingNullDataSource() {;};

  protected:
    virtual bool                 FetchData(avtDataSpecification_p) { return false; };
};


#endif


