// ************************************************************************* //
//                        avtCondenseDatasetFilter.h                         //
// ************************************************************************* //

#ifndef AVT_CONDENSE_DATASET_FILTER_H
#define AVT_CONDENSE_DATASET_FILTER_H

#include <pipeline_exports.h>

#include <avtStreamer.h>


class vtkPolyDataRelevantPointsFilter;
class vtkUnstructuredGridRelevantPointsFilter;
class vtkDataSet;


// ****************************************************************************
//  Class: avtCondenseDatasetFilter
//
//  Purpose:
//      Condenses the size of the dataset by removing irrelevant points and
//      data arrays.
//  
//  Programmer: Kathleen Bonnell 
//  Creation:   November 07, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Apr 10 10:49:10 PDT 2001 
//    Changed ExecuteDomain to ExecuteData.
//
//    Hank Childs, Wed Jun  6 09:09:33 PDT 2001
//    Removed CalcDomainList and Equivalent.
//
//    Hank Childs, Fri Jul 25 21:21:08 PDT 2003
//    Renamed from avtRelevantPointsFilter.
//
// ****************************************************************************

class PIPELINE_API avtCondenseDatasetFilter : public avtStreamer
{
  public:
                         avtCondenseDatasetFilter();
    virtual             ~avtCondenseDatasetFilter();

    virtual const char  *GetType(void) { return "avtCondenseDatasetFilter"; };
    virtual const char  *GetDescription(void) 
                             { return "Removing unneeded points"; };

    virtual void         ReleaseData(void);

  protected:
    vtkPolyDataRelevantPointsFilter         *rpfPD;
    vtkUnstructuredGridRelevantPointsFilter *rpfUG;

    virtual vtkDataSet  *ExecuteData(vtkDataSet *, int, std::string);
};


#endif


