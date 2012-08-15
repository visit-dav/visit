#ifndef AVT_R_MODEL_BASED_CLUSTERING_FILTER_H
#define AVT_R_MODEL_BASED_CLUSTERING_FILTER_H

#include <filters_exports.h>
#include <avtDatasetToDatasetFilter.h>
#include <avtTimeLoopFilter.h>
#include <ModelBasedClusteringAttributes.h>
#include <string>
#include <vector>

class vtkDataSet;

// ****************************************************************************
// Class:  avtRModelBasedClusteringFilter
//
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

class AVTFILTERS_API avtRModelBasedClusteringFilter : virtual public avtDatasetToDatasetFilter,
                                     virtual public avtTimeLoopFilter
{
  public:
    avtRModelBasedClusteringFilter();
    virtual ~avtRModelBasedClusteringFilter();
    virtual const char* GetType() {return "avtRModelBasedClusteringFilter";}

    std::string newVarName;
    std::string codeDir;
    ModelBasedClusteringAttributes   atts;
    
  protected:
    void                    Initialize();
    virtual void            Execute();
    virtual void            PreExecute();
    virtual void            PostExecute();
    virtual void            CreateFinalOutput();
    virtual bool            ExecutionSuccessful();

    virtual bool            FilterSupportsTimeParallelization();
    virtual bool            DataCanBeParallelizedOverTime(void);

    float                   CalculateThreshold(int loc);
    std::string             CreateQuantileCommand(const char *var, const char *in);

    vtkDataSet *outDS;
    int numTuples, numTimes;
    bool nodeCenteredData, initialized;
    int idx0, idxN;

    class sample
    {
    public:
        sample() {val=-1; time=-1;}
        sample(float v, int t) {val=v; time=t;}
        float val;
        int time;
    };
    
    //values[location][time_i]
    std::vector<std::vector<sample> > values;
};

#endif
