#include <avtParallel.h>
#include <avtRExtremesFilter.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <limits>
#include <vtkRInterface.h>
#include <vtkRectilinearGrid.h>
#include <DebugStream.h>

#include <avtCallback.h>
#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <InvalidFilesException.h>

using namespace std;

// ****************************************************************************
// Method:  avtRExtremesFilter::avtRExtremesFilter
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

avtRExtremesFilter::avtRExtremesFilter()
{
    initialized = false;
    outDS = NULL;
    numTuples = 0;
    nodeCenteredData = false;
    computeMaxes = avtRExtremesFilter::YEARLY;
    numYears = 0;
    scalingVal =  86400.0 / 10.0;
    monthDisplay = 0;
}

// ****************************************************************************
// Method:  avtRExtremesFilter::~avtRExtremesFilter
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

avtRExtremesFilter::~avtRExtremesFilter()
{
}

// ****************************************************************************
// Method:  avtRExtremesFilter::FilterSupportsTimeParallelization
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

bool
avtRExtremesFilter::FilterSupportsTimeParallelization()
{
    return true;
}

// ****************************************************************************
// Method:  avtRExtremesFilter::DataCanBeParallelizedOverTime
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

bool
avtRExtremesFilter::DataCanBeParallelizedOverTime()
{
    return true;
}

// ****************************************************************************
// Method:  avtRExtremesFilter::GetIndexFromDay
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

int
avtRExtremesFilter::GetIndexFromDay(const int &t)
{
    if (computeMaxes == avtRExtremesFilter::MONTHLY)
    {
        int year = t/365;
        int dayMonth[12] = {31,59,90,120,151,181,213,244,274,304,334,365}; //no leap year
        int dayInYear = t % 365;
        for (int i = 0; i < 12; i++)
            if (dayInYear <= dayMonth[i])
                return (year*12) + i;
    }
    else if (computeMaxes == avtRExtremesFilter::YEARLY)
        return t / 365;
}

// ****************************************************************************
// Method:  avtRExtremesFilter::ExecutionSuccessful
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

bool
avtRExtremesFilter::ExecutionSuccessful()
{
    return true;
}

// ****************************************************************************
// Method:  avtRExtremesFilter::Initialize
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

void
avtRExtremesFilter::Initialize()
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
    int nTimes = t1-t0 + 1;
    numYears = nTimes/365;

    int numArrays = 0;
    //How to compute maxes.
    //Monthly maxes.
    if (computeMaxes == avtRExtremesFilter::MONTHLY)
    {
        numArrays = numYears*12;
    }
    else if (computeMaxes == avtRExtremesFilter::YEARLY)
        numArrays = numYears;
    
    if (numArrays == 0)
        numArrays = 1;
    cout<<"Array: "<<numArrays<<" of "<<numTuples<<endl;
    values.resize(numArrays);
    for (int i = 0; i < numArrays; i++)
    {
        values[i].resize(numTuples);
        for (int j = 0; j < numTuples; j++)
            values[i][j] = -numeric_limits<double>::max();
    }

    //To control how many tuples get processed, set here.
    //numTuples = 512;
    
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

#endif
    
    initialized = true;
    delete [] leaves;
}


// ****************************************************************************
// Method:  avtRExtremesFilter::PreExecute
//
// Programmer:  Dave Pugmire
// Creation:    March 16, 2012
//
// ****************************************************************************

void
avtRExtremesFilter::PreExecute()
{
    avtDatasetToDatasetFilter::PreExecute();
}

// ****************************************************************************
// Method:  avtRExtremesFilter::PostExecute
//
// Programmer:  Dave Pugmire
// Creation:    March 16, 2012
//
// ****************************************************************************

void
avtRExtremesFilter::PostExecute()
{
    avtDatasetToDatasetFilter::PostExecute();
}

// ****************************************************************************
// Method:  avtRExtremesFilter::Execute
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

void
avtRExtremesFilter::Execute()
{
    debug1<<"avtRExtremesFilter::Execute() time= "<<currentTime<<endl;

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
    
    int index = GetIndexFromDay(currentTime);
    vector<double>::iterator it = values[index].begin();
    vector<double>::iterator end = values[index].end();
    //cout<<"index= "<<index<<" "<<values.size()<<" "<<values[index].size()<<" nt= "<<scalars->GetNumberOfTuples()<<endl;
    float sum = 0.0;
    int cnt = 0;

    int nTuples = scalars->GetNumberOfTuples();
    for (int i = 0; i < nTuples; i++)
    {
        float v = vals[i];
        if (v > values[index][i])
            values[index][i] = v;
    }
}

// ****************************************************************************
// Method:  avtRExtremesFilter::CreateFinalOutput
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
avtRExtremesFilter::CreateFinalOutput()
{
    avtCallback::ResetTimeout(0);
    
    debug1<<"avtRExtremesFilter::CreateFinalOutput()"<<endl;
    
    //Unify maxima
#ifdef PARALLEL
    double *outvalues = new double[values[0].size()];
    for (int i = 0; i < values.size(); i++)
    {
        UnifyMaximumDoubleArrayAcrossAllProcessors(&values[i][0], outvalues, values[i].size());
        for (int j = 0; j < values[i].size(); j++)
            values[i][j] = outvalues[j];
    }
    delete [] outvalues;
    //cout<<PAR_Rank()<<" ["<<idx0<<" "<<idxN<<"]"<<endl;
#endif

    //Run R code.
    int numVals = idxN-idx0;
    float *result = new float[numTuples];
    for (int i = 0; i < numTuples; i++)
        result[i] = 0.0f;

    vtkRInterface *RI = vtkRInterface::New();
    vtkDoubleArray *inData = vtkDoubleArray::New();
    inData->SetNumberOfComponents(1);
    inData->SetNumberOfTuples((idxN-idx0)*values.size());
    int idx = 0, N = idxN-idx0;

    for (int i = 0; i < N; i++)
        for (int d = 0; d < values.size(); d++)
        {
            double v = scalingVal * values[d][idx0+i];
            inData->SetValue(idx, v);
            idx++;
        }
    RI->AssignVTKDataArrayToRVariable(inData, "inData");

    cout<<"inData : "<<inData->GetNumberOfTuples()<<" : "<<inData->GetNumberOfComponents()<<endl;
    cout<<"I have "<<idx0<<" "<<idxN<<endl;

    char cmd[512];
    if (computeMaxes == avtRExtremesFilter::MONTHLY)
    {
        sprintf(cmd,
                "output=gevFit(inData,'monthly', %d, %d)\n"     \
                "result = output$returnValue[%d,]\n",
                numYears, (idxN-idx0), monthDisplay+1);
    }
    else if (computeMaxes == avtRExtremesFilter::YEARLY)
    {
        sprintf(cmd,
                "output=gevFit(inData,'annual', %d, %d)\n"      \
                "result = output$returnValue\n",
                numYears, (idxN-idx0));
    }
        
    string command;
    char fileLoad[1024];
    sprintf(fileLoad, "source('%s/auxil.r')\n", codeDir.c_str());
    command += fileLoad;
    sprintf(fileLoad, "source('%s/gev.fit2.r')\n", codeDir.c_str());
    command += fileLoad;
    sprintf(fileLoad, "source('%s/gevVisit.r')\n", codeDir.c_str());
    command += fileLoad;
        
    command += cmd;
    cout<<command<<endl;
    RI->EvalRscript(command.c_str());
        
    vtkDoubleArray *output = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("result"));

    int j = 0;
    idx = idx0;
    for (int i = idx0; i < idxN; i++, j++, idx++)
    {
        result[idx] = (float)output->GetComponent(0, j);
    }
    
    //output->Delete(); //Looks like RI will release this??
    inData->Delete();
    RI->Delete();

#ifdef PARALLEL
    float *s = new float[numTuples];
    SumFloatArray(result, s, numTuples);
    if (PAR_Rank() == 0)
        memcpy(result, s, numTuples*sizeof(float));
    delete [] s;
#endif

    //SumFloatArrayAcrossAllProcessors(result, s, 3*numTuples);

    if (PAR_Rank() == 0)
    {
        //cout<<"Create output.....("<<numOutputComponents<<" x "<<numTuples<<")"<<endl;
        vtkFloatArray *outVar = vtkFloatArray::New();
        outVar->SetNumberOfComponents(1);
        outVar->SetNumberOfTuples(numTuples);
        int idx = 0;
        for (int i = 0; i < numTuples; i++, idx++)
        {
            outVar->SetValue(i, result[idx]);
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

    delete [] result;
    avtCallback::ResetTimeout(5*60);
}
