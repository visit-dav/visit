#ifndef AVT_R_EXTREMES_FILTER_H
#define AVT_R_EXTREMES_FILTER_H

#include <filters_exports.h>
#include <avtDatasetToDatasetFilter.h>
#include <avtTimeLoopFilter.h>
#include <string>
#include <vector>

class vtkDataSet;

// ****************************************************************************
// Class:  avtRExtremesFilter
//
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

class AVTFILTERS_API avtRExtremesFilter : virtual public avtDatasetToDatasetFilter,
                                          virtual public avtTimeLoopFilter
{
  public:
    avtRExtremesFilter();
    virtual ~avtRExtremesFilter();
    virtual const char* GetType() {return "avtRExtremesFilter";}

    std::string newVarName;

    enum ComputeMaxMethod { MONTHLY=0, YEARLY};

    ComputeMaxMethod computeMaxes;
    int monthDisplay;
    std::string codeDir;
    bool dumpData;
    
  protected:
    void                    Initialize();
    virtual void            Execute();
    virtual void            PreExecute();
    virtual void            PostExecute();
    virtual void            CreateFinalOutput();
    virtual bool            ExecutionSuccessful();

    virtual bool            FilterSupportsTimeParallelization();
    virtual bool            DataCanBeParallelizedOverTime(void);
    int                     GetIndexFromDay(const int &t);

    vtkDataSet *outDS;
    int numTuples;
    bool nodeCenteredData, initialized;
    int idx0, idxN;
    std::vector<std::vector<double> > values;
    int numYears;
    float scalingVal;
};

#endif
