// ************************************************************************* //
//                         avtSourceFromNullData.h                           //
// ************************************************************************* //

#ifndef AVT_SOURCE_FROM_NULL_DATA
#define AVT_SOURCE_FROM_NULL_DATA
#include <pipeline_exports.h>

#include <avtTerminatingNullDataSource.h>


// ****************************************************************************
//  Class: avtSourceFromNullData
//
//  Purpose:
//      A source object (pipeline terminator) that is created from null data 
//
//  Programmer: Mark C. Miller
//  Creation:   January 8, 2003 
//
// ****************************************************************************

class PIPELINE_API avtSourceFromNullData : virtual public avtTerminatingNullDataSource 
{
  public:
                           avtSourceFromNullData() {;};
    virtual               ~avtSourceFromNullData() {;};
};

#endif
