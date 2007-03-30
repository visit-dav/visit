// ************************************************************************* //
//                        avtRelevantPointsFilter.h                          //
// ************************************************************************* //

#ifndef AVT_RELEVANT_POINTS_FILTER_H
#define AVT_RELEVANT_POINTS_FILTER_H

#include <pipeline_exports.h>

#include <avtStreamer.h>


class vtkPolyDataRelevantPointsFilter;
class vtkUnstructuredGridRelevantPointsFilter;
class vtkDataSet;


// ****************************************************************************
//  Class: avtRelevantPointsFilter
//
//  Purpose:
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
// ****************************************************************************

class PIPELINE_API avtRelevantPointsFilter : public avtStreamer
{
  public:
                         avtRelevantPointsFilter();
    virtual             ~avtRelevantPointsFilter();

    virtual const char  *GetType(void) { return "avtRelevantPointsFilter"; };
    virtual const char  *GetDescription(void) 
                             { return "Removing unneeded points"; };

    virtual void         ReleaseData(void);

  protected:
    vtkPolyDataRelevantPointsFilter         *rpfPD;
    vtkUnstructuredGridRelevantPointsFilter *rpfUG;

    virtual vtkDataSet  *ExecuteData(vtkDataSet *, int, std::string);
};


#endif


