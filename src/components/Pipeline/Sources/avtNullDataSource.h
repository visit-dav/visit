// ************************************************************************* //
//                         avtNullDataSource.h                               //
// ************************************************************************* //

#ifndef AVT_NULL_DATA_SOURCE_H
#define AVT_NULL_DATA_SOURCE_H
#include <pipeline_exports.h>


#include <avtDataObjectSource.h>
#include <avtNullData.h>

// ****************************************************************************
//  Class: avtNullDataSource
//
//  Purpose:
//      A data object source whose data object is null data 
//
//  Programmer: Mark C. Miller 
//  Creation:   January 8, 2003 
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtNullDataSource : virtual public avtDataObjectSource
{
  public:
                                avtNullDataSource();
    virtual                    ~avtNullDataSource();

    avtDataObject_p             GetOutput(void);
    avtNullData_p               GetTypedOutput(void) { return nullData; };

  protected:
    avtNullData_p               nullData;

    void                        SetOutput(avtNullData_p);

};


#endif


