// ************************************************************************* //
//                       avtDatasetToDataObjectFilter.h                      //
// ************************************************************************* //

#ifndef AVT_DATASET_TO_DATA_OBJECT_FILTER_H
#define AVT_DATASET_TO_DATA_OBJECT_FILTER_H

#include <pipeline_exports.h>

#include <avtFilter.h>
#include <avtDatasetSink.h>


// ****************************************************************************
//  Class: avtDatasetToDataObjectFilter
//
//  Purpose:
//      A filter that takes in a dataset as input and has a data object as
//      output.
//
//  Programmer: Hank Childs
//  Creation:   May 31, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtDatasetToDataObjectFilter
    : virtual public avtFilter, virtual public avtDatasetSink
{
  public:
                       avtDatasetToDataObjectFilter();
    virtual           ~avtDatasetToDataObjectFilter();

  protected:
    //                 Note that these variables are only used when an active
    //                 variable is set.
    bool               activeVariableIsPointData;
    bool               hasPointVars;
    bool               hasCellVars;

    void               InputSetActiveVariable(const char *);
    virtual void       SearchDataForDataExtents(double *);
};


#endif


