// ************************************************************************* //
//                      avtDatasetToVTKDatasetFilter.h                       //
// ************************************************************************* //

#ifndef AVT_DATASET_TO_VTK_DATASET_FILTER_H
#define AVT_DATASET_TO_VTK_DATASET_FILTER_H
#include <pipeline_exports.h>


#include <avtDatasetToDatasetFilter.h>


class   vtkAVTPipelineBridge;
class   vtkAppendFilter;
class   vtkDataObject;


// ****************************************************************************
//  Class: avtDatasetToVTKDatasetFilter
//
//  Purpose:
//      This is a filter that converts avtDatasets to a VTK dataset.  It runs
//      all of its domains through an append data set filter to make sure there
//      are no issues with multiple domains.  To make the VTK pipeline magic
//      work, it needs a VTK class that it has hooks into. 
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
//    Jeremy Meredith, Thu Sep 28 12:50:55 PDT 2000
//    Removed CreateOutputDatasets.
//
// ****************************************************************************

class PIPELINE_API avtDatasetToVTKDatasetFilter : public avtDatasetToDatasetFilter
{
  public:
                                 avtDatasetToVTKDatasetFilter();
    virtual                     ~avtDatasetToVTKDatasetFilter();

    virtual const char          *GetType()
                                    { return "avtDatasetToVTKDatasetFilter"; };

    virtual vtkDataSet          *GetVTKOutput(void);

  protected:
    vtkAVTPipelineBridge        *bridge;
    vtkAppendFilter             *appendFilter;

    virtual void                 CalcDomainList(void);
    virtual void                 Execute(class avtDomainList *);
};


#include <vtkUnstructuredGridSource.h>

// ****************************************************************************
//  Class: vtkAVTPipelineBridge
//
//  Purpose:
//      An object of this type acts as a bridge between a VTK pipeline and an
//      AVT pipeline.  When a VTK pipeline does an update on the dataset of
//      this source's output, it goes to this bridge, which then starts the
//      AVT pipeline.  This allows for there to be no central executive in
//      the transition between AVT and VTK.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Sep 28 08:40:06 PDT 2000
//    Change UpdateInformation to UpdateData so we can hook into the VTK
//    pipeline a little better.
//
// ****************************************************************************

class PIPELINE_API vtkAVTPipelineBridge : public vtkUnstructuredGridSource
{
  public:
    static vtkAVTPipelineBridge   *New();
    void                           SetFilter(avtDatasetToVTKDatasetFilter *);

    virtual void                   UpdateData(vtkDataObject *);

  protected:
                                   vtkAVTPipelineBridge();

    avtDatasetToVTKDatasetFilter  *filter;
};


#endif


