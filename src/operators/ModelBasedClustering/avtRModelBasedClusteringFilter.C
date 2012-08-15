#include <avtParallel.h>
#include <avtRModelBasedClusteringFilter.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkIntArray.h>
#include <limits>
#include <vtkRInterface.h>
#include <vtkRectilinearGrid.h>
#include <DebugStream.h>

#include <avtCallback.h>
#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <InvalidFilesException.h>

#ifdef PARALLEL
  #include <mpi.h>
#endif

using namespace std;

// ****************************************************************************
// Method:  avtRModelBasedClusteringFilter::avtRModelBasedClusteringFilter
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

avtRModelBasedClusteringFilter::avtRModelBasedClusteringFilter()
{
    initialized = false;
    outDS = NULL;
    numTuples = 0;
    nodeCenteredData = false;
}

// ****************************************************************************
// Method:  avtRModelBasedClusteringFilter::~avtRModelBasedClusteringFilter
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

avtRModelBasedClusteringFilter::~avtRModelBasedClusteringFilter()
{
}

// ****************************************************************************
// Method:  avtRModelBasedClusteringFilter::FilterSupportsTimeParallelization
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

bool
avtRModelBasedClusteringFilter::FilterSupportsTimeParallelization()
{
    return true;
}

// ****************************************************************************
// Method:  avtRModelBasedClusteringFilter::DataCanBeParallelizedOverTime
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

bool
avtRModelBasedClusteringFilter::DataCanBeParallelizedOverTime()
{
    return true;
}

// ****************************************************************************
// Method:  avtRModelBasedClusteringFilter::CalculateThreshold
//
// Programmer:  Dave Pugmire
// Creation:    May 22, 2012
//
// ****************************************************************************

float
avtRModelBasedClusteringFilter::CalculateThreshold(int loc)
{
    int nVals = values[loc].size();
    if (nVals == 0)
        return 0.0f;
    
    vtkDoubleArray *inData = vtkDoubleArray::New();
    inData->SetNumberOfComponents(1);
    inData->SetNumberOfTuples(nVals);
    for (int i = 0; i < nVals; i++)
        inData->SetValue(i, values[loc][i].val);

    vtkRInterface *RI = vtkRInterface::New();
    RI->AssignVTKDataArrayToRVariable(inData, "inData");
    RI->EvalRscript(CreateQuantileCommand("q", "inData").c_str());
    vtkDoubleArray *Q = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("q"));

    float threshold = 0.0f;
    if (Q)
        threshold = (float)Q->GetComponent(0,0);

    inData->Delete();
    RI->Delete();
    return threshold;
}


// ****************************************************************************
// Method:  avtRModelBasedClusteringFilter::ExecutionSuccessful
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

bool
avtRModelBasedClusteringFilter::ExecutionSuccessful()
{
    return true;
}

// ****************************************************************************
// Method:  avtRModelBasedClusteringFilter::CreateQuantileCommand
//
// Programmer:  Dave Pugmire
// Creation:    May 22, 2012
//
// ****************************************************************************

string
avtRModelBasedClusteringFilter::CreateQuantileCommand(const char *var, const char *in)
{
    char str[128];
    sprintf(str, "%s = quantile(%s, %lf)\n", var, in, atts.GetPercentile());
    string cmd = str;
    return cmd;
}

// ****************************************************************************
// Method:  avtRModelBasedClusteringFilter::Initialize
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

void
avtRModelBasedClusteringFilter::Initialize()
{
    if (initialized)
        return;

    int nleaves;
    vtkDataSet **leaves = GetInputDataTree()->GetAllLeaves(nleaves);

    if (nleaves != 1)
    {
        EXCEPTION1(ImproperUseException, "Multi-domain not supported yet.");
    }

    vtkDataSet *inDS = leaves[0];
    outDS = (vtkDataSet *)(inDS->NewInstance());
    outDS->CopyStructure(inDS);

    nodeCenteredData = (GetInput()->GetInfo().GetAttributes().GetCentering() == AVT_NODECENT);
    if (nodeCenteredData)
        numTuples = inDS->GetPointData()->GetScalars()->GetNumberOfTuples();
    else
        numTuples = inDS->GetCellData()->GetScalars()->GetNumberOfTuples();
    
    //cout<<"Initialize: numTuples= "<<numTuples<<endl;
    int t0 = GetStartTime();
    int t1 = GetEndTime();
    numTimes = t1-t0 + 1;
    
    values.resize(numTuples);
    idx0 = 0;
    idxN = numTuples;
    
#ifdef PARALLEL
    int rank = PAR_Rank();
    int nProcs = PAR_Size();

    int nSamplesPerProc = (numTuples / nProcs);
    int oneExtraUntil = (numTuples % nProcs);
    if (rank < oneExtraUntil)
    {
        idx0 = (rank)*(nSamplesPerProc+1);
        idxN = (rank+1)*(nSamplesPerProc+1);
    }
    else
    {
        idx0 = (rank)*(nSamplesPerProc) + oneExtraUntil;
        idxN = (rank+1)*(nSamplesPerProc) + oneExtraUntil;
    }
    debug1<<"I have: ["<<idx0<<" "<<idxN<<"]"<<endl;
#endif
    
    initialized = true;
    delete [] leaves;
}


// ****************************************************************************
// Method:  avtRModelBasedClusteringFilter::PreExecute
//
// Programmer:  Dave Pugmire
// Creation:    March 16, 2012
//
// ****************************************************************************

void
avtRModelBasedClusteringFilter::PreExecute()
{
    avtDatasetToDatasetFilter::PreExecute();
}

// ****************************************************************************
// Method:  avtRModelBasedClusteringFilter::PostExecute
//
// Programmer:  Dave Pugmire
// Creation:    March 16, 2012
//
// ****************************************************************************

void
avtRModelBasedClusteringFilter::PostExecute()
{
    avtDatasetToDatasetFilter::PostExecute();
}

// ****************************************************************************
// Method:  avtRModelBasedClusteringFilter::Execute
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

void
avtRModelBasedClusteringFilter::Execute()
{
    debug1<<"avtRModelBasedClusteringFilter::Execute() time= "<<currentTime<<endl;

    Initialize();
    int nleaves;
    vtkDataSet **leaves = GetInputDataTree()->GetAllLeaves(nleaves);
    vtkDataSet *ds = leaves[0];
    vtkFloatArray *scalars = NULL;

    if (nodeCenteredData)
        scalars = (vtkFloatArray *)ds->GetPointData()->GetScalars();
    else
        scalars = (vtkFloatArray *)ds->GetCellData()->GetScalars();
    float *vals = (float *) scalars->GetVoidPointer(0);

    int nTuples = scalars->GetNumberOfTuples();
    float threshold = atts.GetThreshold();
    
    for (int i = 0; i < nTuples; i++)
    {
        if (vals[i] > threshold)
            values[i].push_back(sample(vals[i], currentTime));
    }
}

// ****************************************************************************
// Method:  avtRModelBasedClusteringFilter::CreateFinalOutput
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// Modifications:
//
//   Dave Pugmire, Wed Apr 11 17:07:26 EDT 2012
//   Remove the processing of locations in rounds. No longer needed.
//
// ****************************************************************************

void
avtRModelBasedClusteringFilter::CreateFinalOutput()
{
    avtCallback::ResetTimeout(0);

    //Exchange data....
#ifdef PARALLEL
    float *tmp = new float[numTimes];
    float *res = new float[numTimes];
    int *flags = new int[numTimes];
    int *flagsRes = new int[numTimes];
    
    for (int i=0; i<numTuples; i++)
    {
        for (int t=0; t<numTimes; t++)
        {
            tmp[t] = 0.0;
            flags[t] = 0;
        }
        int nt = values[i].size();
        for (int t=0; t<nt; t++)
        {
            tmp[values[i][t].time] = values[i][t].val;
            flags[values[i][t].time] = 1;
        }
        MPI_Allreduce(tmp, res, numTimes, MPI_FLOAT, MPI_SUM, VISIT_MPI_COMM);
        MPI_Allreduce(flags, flagsRes, numTimes, MPI_INT, MPI_SUM, VISIT_MPI_COMM);

        values[i].resize(0);

        //See if it's one of my locations.
        if (i >= idx0 && i < idxN)
        {
            for (int t=0; t<numTimes; t++)
                if (flagsRes[t])
                    values[i].push_back(sample(res[t], t));
        }
    }
    
    delete [] tmp;
    delete [] res;
    delete [] flags;
    delete [] flagsRes;
#endif

    vtkRInterface *RI = vtkRInterface::New();
    char fileLoad[1024];
    sprintf(fileLoad,
            "source('%s/auxil.r')\n",
            codeDir.c_str());
    
    char cmd[1024];
    
    vector<float> outputs;
    outputs.resize(numTuples);

    for (int i = idx0; i < idxN; i++)
    {
        float threshold = CalculateThreshold(i);
        int numValsOverThreshold = 0;
        int nt = values[i].size();
        for (int t = 0; t < nt; t++)
            if (values[i][t].val > threshold)
                numValsOverThreshold++;
        
        //cout<<i<<": "<<numValsOverThreshold<<endl;
        
        //cout<<"numExceedences: "<<numExceedences<<endl;
        vtkDoubleArray *vals = vtkDoubleArray::New();
        vals->SetNumberOfComponents(1);
        vals->SetNumberOfTuples(numValsOverThreshold);
        
        int idx = 0;
        nt = values[i].size();
        for (int t = 0; t < nt; t++)
        {
            if (values[i][t].val > threshold)
            {
                vals->SetValue(idx, values[i][t].val);
                idx++;
                if (idx == numValsOverThreshold)
                    break;
            }
        }
        
        RI->AssignVTKDataArrayToRVariable(vals, "vals");
        
        string command = fileLoad;
        command += cmd;
        //cout<<command<<endl;
        RI->EvalRscript(command.c_str());

        //vtkDoubleArray *out_rv = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("rv"));

        outputs[i] = numValsOverThreshold;
        vals->Delete();
    }

#if PARALLEL
    float *in = new float[numTuples];
    float *sum = new float[numTuples];
    
    for (int i=0; i<numTuples; i++)
    {
        if (i >= idx0 && i < idxN)
            in[i] = outputs[i];
        else
            in[i] = 0.0f;
    }
    SumFloatArray(in, sum, numTuples);
    if (PAR_Rank() == 0)
    {
        for (int i=0; i<numTuples; i++)
            outputs[i] = sum[i];
    }
    delete [] in;
    delete [] sum;

#endif
    if (PAR_Rank() == 0)
    {
        //cout<<"Create output.....("<<numOutputComponents<<" x "<<numTuples<<")"<<endl;
        vtkFloatArray *outVar = vtkFloatArray::New();
        outVar->SetNumberOfComponents(1);
        outVar->SetNumberOfTuples(numTuples);
        int idx = 0;
        for (int i = 0; i < numTuples; i++, idx++)
        {
            outVar->SetValue(i, outputs[i]);
        }
    
        if (nodeCenteredData)
            outDS->GetPointData()->SetScalars(outVar);
        else
            outDS->GetPointData()->SetScalars(outVar);
        
        outVar->Delete();

        avtDataTree_p outputTree = new avtDataTree(outDS, 0);
        SetOutputDataTree(outputTree);
        outDS->Delete();
    }
    else
        SetOutputDataTree(new avtDataTree());

    avtCallback::ResetTimeout(5*60);
}
