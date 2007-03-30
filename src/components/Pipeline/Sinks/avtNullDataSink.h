// ************************************************************************* //
//                          avtNullDataSink.h                                //
// ************************************************************************* //

#ifndef AVT_NULL_DATA_SINK_H
#define AVT_NULL_DATA_SINK_H
#include <pipeline_exports.h>


#include <avtDataObjectSink.h>
#include <avtNullData.h>

// ****************************************************************************
//  Class: avtNullDataSink
//
//  Purpose:
//      The sink to an avt pipeline. 
//
//  Programmer: Mark C. Miller
//  Creation:   January 7, 2003 
//
// ****************************************************************************

class PIPELINE_API avtNullDataSink : virtual public avtDataObjectSink
{
  public:
                                    avtNullDataSink();
    virtual                        ~avtNullDataSink() {;};

  protected:
    virtual avtDataObject_p         GetInput();
    virtual void                    SetTypedInput(avtDataObject_p in);
    avtNullData_p                   GetTypedInput() { return input; };

  private:
    avtNullData_p                      input;

};


#endif


