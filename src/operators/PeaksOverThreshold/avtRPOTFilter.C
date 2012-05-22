#include <avtParallel.h>
#include <avtRPOTFilter.h>
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

using namespace std;

// ****************************************************************************
// Method:  avtRPOTFilter::avtRPOTFilter
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

avtRPOTFilter::avtRPOTFilter()
{
    initialized = false;
    outDS = NULL;
    numTuples = 0;
    nodeCenteredData = false;
    computeMaxes = avtRPOTFilter::YEARLY;
    numYears = 0;
    scalingVal =  850000.0;
    monthDisplay = 0;
    dumpData = false;
}

// ****************************************************************************
// Method:  avtRPOTFilter::~avtRPOTFilter
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

avtRPOTFilter::~avtRPOTFilter()
{
}

// ****************************************************************************
// Method:  avtRPOTFilter::FilterSupportsTimeParallelization
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

bool
avtRPOTFilter::FilterSupportsTimeParallelization()
{
    return true;
}

// ****************************************************************************
// Method:  avtRPOTFilter::DataCanBeParallelizedOverTime
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

bool
avtRPOTFilter::DataCanBeParallelizedOverTime()
{
    return true;
}

// ****************************************************************************
// Method:  avtRPOTFilter::GetIndexFromDay
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

int
avtRPOTFilter::GetIndexFromDay(const int &t)
{
    if (computeMaxes == avtRPOTFilter::MONTHLY)
    {
        int year = t/365;
        int dayMonth[12] = {31,59,90,120,151,181,213,244,274,304,334,365}; //no leap year
        int dayInYear = t % 365;
        for (int i = 0; i < 12; i++)
            if (dayInYear <= dayMonth[i])
                return (year*12) + i;
    }
    else if (computeMaxes == avtRPOTFilter::YEARLY)
        return 0;
}

// ****************************************************************************
// Method:  avtRPOTFilter::ExecutionSuccessful
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

bool
avtRPOTFilter::ExecutionSuccessful()
{
    return true;
}

// ****************************************************************************
// Method:  avtRPOTFilter::Initialize
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

void
avtRPOTFilter::Initialize()
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
    if (computeMaxes == avtRPOTFilter::MONTHLY)
        numArrays = 12;
    else if (computeMaxes == avtRPOTFilter::SEASONAL)
        numArrays = 4;
    else if (computeMaxes == avtRPOTFilter::YEARLY)
        numArrays = 1;
    
    values.resize(numTuples);
    for (int i = 0; i < numTuples; i++)
    {
        values[i].resize(numArrays);
        for (int j = 0; j < numArrays; j++)
        {
            values[i][j].resize(nTimes);
            for (int k = 0; k < nTimes; k++)
                values[i][j][k] = 0.0;
        }
    }

    cout<<"values:"<<values.size()<<" x "<<values[0].size()<<" x "<<values[0][0].size()<<endl;
    //cout<<"values["<<numTuples<<"]["<<numArrays<<"][nTimes]"<<endl;
    
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
// Method:  avtRPOTFilter::PreExecute
//
// Programmer:  Dave Pugmire
// Creation:    March 16, 2012
//
// ****************************************************************************

void
avtRPOTFilter::PreExecute()
{
    avtDatasetToDatasetFilter::PreExecute();
}

// ****************************************************************************
// Method:  avtRPOTFilter::PostExecute
//
// Programmer:  Dave Pugmire
// Creation:    March 16, 2012
//
// ****************************************************************************

void
avtRPOTFilter::PostExecute()
{
    avtDatasetToDatasetFilter::PostExecute();
}

// ****************************************************************************
// Method:  avtRPOTFilter::Execute
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

void
avtRPOTFilter::Execute()
{
    debug1<<"avtRPOTFilter::Execute() time= "<<currentTime<<endl;

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
    int index = GetIndexFromDay(currentTime);

    //cout<<"processing "<<currentTime<<" sv = "<<scalingVal<<endl;
    for (int i = 0; i < nTuples; i++)
        values[i][index][currentTime] = vals[i]*scalingVal;
}

// ****************************************************************************
// Method:  avtRPOTFilter::CreateFinalOutput
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
avtRPOTFilter::CreateFinalOutput()
{
    avtCallback::ResetTimeout(0);

    //Exchange data....
    int nTimes = values[0][0].size();
    int numArrays = values[0].size();
    debug1<<__LINE__<<endl;
#ifdef PARALLEL
    double *tmp = new double[nTimes];

    for (int i = 0; i < numTuples; i++)
    {
        for (int j = 0; j < numArrays; j++)
        {
            UnifyMaximumDoubleArrayAcrossAllProcessors(&(values[i][j][0]), tmp, nTimes);
            for (int k = 0; k < nTimes; k++)
                values[i][j][k] = tmp[k];
        }
    }
    delete [] tmp;
#endif
    debug1<<__LINE__<<endl;
    
    vtkRInterface *RI = vtkRInterface::New();
    vtkDoubleArray *inData = vtkDoubleArray::New();

    inData->SetNumberOfComponents(1);
    inData->SetNumberOfTuples(nTimes);
    char quantileCmd[128];
    sprintf(quantileCmd, "q = quantile(inData, %f)\n", percentile);

    char fileLoad[1024];
    sprintf(fileLoad,
            "source('%s/auxil.r')\n"    \
            "source('%s/pp.fit2.r')\n" \
            "source('%s/potVisit.r')\n",
            codeDir.c_str(),
            codeDir.c_str(),
            codeDir.c_str());
    
    char potCmd[1024];
            
    debug1<<__LINE__<<endl;
    for (int j = 0; j < numArrays; j++)
    {
        for (int i = idx0; i < idxN; i++)
        {
            debug1<<i<<": ["<<idx0<<" "<<idxN<<"]"<<endl;
            int idx = 0;
            for (int k = 0; k < nTimes; k++)
            {
                inData->SetValue(idx, values[i][j][k]);
                idx++;
            }
            RI->AssignVTKDataArrayToRVariable(inData, "inData");
            RI->EvalRscript(quantileCmd);
            vtkDoubleArray *Q = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("q"));
            float threshold = (float)Q->GetComponent(0,0);

            int numExceedences = 0;
            for (int k = 0; k < nTimes; k++)
                if (values[i][j][k] > threshold)
                    numExceedences++;

            vtkDoubleArray *exceedences = vtkDoubleArray::New();
            vtkIntArray *dayIndices = vtkIntArray::New();
            exceedences->SetNumberOfComponents(1);
            exceedences->SetNumberOfTuples(numExceedences);
            dayIndices->SetNumberOfComponents(1);
            dayIndices->SetNumberOfTuples(numExceedences);
            
            idx = 0;
            for (int k = 0; k < nTimes; k++)
            {
                if (values[i][j][k] > threshold)
                {
                    exceedences->SetValue(idx, values[i][j][k]);
                    dayIndices->SetValue(idx, k);
                    idx++;
                    if (idx == numExceedences)
                        break;
                }
            }
            debug1<<__LINE__<<endl;
            RI->AssignVTKDataArrayToRVariable(exceedences, "exceedences");
            RI->AssignVTKDataArrayToRVariable(dayIndices, "dayIndices");
            sprintf(potCmd,
                    "require(ismev)\n" \
                    /*
                    "save.image(file='tmp.RData')\n"                  \
                    */
                    "output = potFit(data = exceedences, day = dayIndices, aggregation = 'annual', nYears = %d, threshold = %f)\n"
                    "rv = output$returnValue\n"\
                    "se_rv = output$se.returnValue\n",
                    numYears, threshold);
            string command = fileLoad;
            command += potCmd;
            //cout<<"command="<<command<<endl;
            RI->EvalRscript(command.c_str());

            debug1<<__LINE__<<endl;
            vtkDoubleArray *output = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("rv"));

            values[i][j].resize(1);
            values[i][j][0] = (float)output->GetComponent(0,0);
            //cout<<i<<" : "<<values[i][j][0]<<endl;
            debug1<<__LINE__<<endl;
        }
    }
    
    inData->Delete();

    debug1<<__LINE__<<endl;
#if PARALLEL
    float *in = new float[numTuples];
    float *sum = new float[numTuples];
    
    for (int i = 0; i < numTuples; i++)
        in[i] = 0.0f;
    
    for (int j = 0; j < numArrays; j++)
    {
        for (int i = idx0; i < idxN; i++)
            in[i] = values[i][j][0];
        
        SumFloatArray(in, sum, numTuples);
        if (PAR_Rank() == 0)
        {
            for (int i = 0; i < numTuples; i++)
                values[i][j][0] = sum[i];
        }
    }

    delete [] in;
    delete [] sum;
#endif
    debug1<<__LINE__<<endl;

    if (PAR_Rank() == 0)
    {
        //cout<<"Create output.....("<<numOutputComponents<<" x "<<numTuples<<")"<<endl;
        vtkFloatArray *outVar = vtkFloatArray::New();
        outVar->SetNumberOfComponents(1);
        outVar->SetNumberOfTuples(numTuples);
        int idx = 0;
        for (int i = 0; i < numTuples; i++, idx++)
        {
            outVar->SetValue(i, values[i][0][0]);
        }
    
        if (nodeCenteredData)
            outDS->GetPointData()->SetScalars(outVar);
        else
            outDS->GetPointData()->SetScalars(outVar);
        
        outVar->Delete();

        avtDataTree_p outputTree = new avtDataTree(outDS, 0);
        SetOutputDataTree(outputTree);
        outDS->Delete();

        /*
        if (dumpData)
        {
            string nm;
            for (int i = 0; i < results.size(); i++)
            {
                if (i == 0)
                    nm = "gev_returnValue.txt";
                else if (i == 1)
                    nm = "gev_se.returnValue.txt";
                    
                ofstream ofile(nm.c_str());
                for (int j = 0; j < numTuples-1; j++)
                    ofile<<results[i][j]<<", ";
                ofile<<results[i][numTuples-1];
            }
        }
        */
    }
    else
        SetOutputDataTree(new avtDataTree());

    avtCallback::ResetTimeout(5*60);




#if 0

    avtCallback::ResetTimeout(0);
    
    debug1<<"avtRPOTFilter::CreateFinalOutput()"<<endl;
    
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
    int numResults = 1;
    if (dumpData)
        numResults = 2;
    
    vector<vector<float> > results;
    results.resize(numResults);
    for (int i = 0; i < numResults; i++)
    {
        results[i].resize(numTuples);
        for (int j = 0; j < numTuples; j++)
            results[i][j] = 0.0f;
    }

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
    if (computeMaxes == avtRPOTFilter::MONTHLY)
    {
        sprintf(cmd,
                "output=gevFit(inData,'monthly', %d, %d)\n"     \
                "result = output$returnValue[%d,]\n",
                numYears, (idxN-idx0), monthDisplay+1);
    }
    else if (computeMaxes == avtRPOTFilter::YEARLY)
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
    if (dumpData)
        command = command + "result2 = output$se.returnValue\n";
    cout<<command<<endl;
    RI->EvalRscript(command.c_str());
    RI->EvalRscript("save.image(file='tmp.RData')\n");
        
    for (int i = 0; i < results.size(); i++)
    {
        vtkDoubleArray *output;
        if (i == 0)
            output = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("result"));
        else
            output = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("result2"));
        int j = 0;
        for (int idx = idx0; idx < idxN; idx++, j++)
        {
            results[i][idx] = (float)output->GetComponent(0, j);
        }
    }
    
    //output->Delete(); //Looks like RI will release this??
    inData->Delete();
    RI->Delete();

#ifdef PARALLEL
    float *s = new float[numTuples];
    for (int i = 0; i < results.size(); i++)
    {
        SumFloatArray(&(results[i][0]), s, numTuples);
        if (PAR_Rank() == 0)
            memcpy(&(results[i][0]), s, numTuples*sizeof(float));
    }
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
            outVar->SetValue(i, results[0][idx]);
        }
    
        if (nodeCenteredData)
            outDS->GetPointData()->SetScalars(outVar);
        else
            outDS->GetPointData()->SetScalars(outVar);
        
        outVar->Delete();

        avtDataTree_p outputTree = new avtDataTree(outDS, 0);
        SetOutputDataTree(outputTree);
        outDS->Delete();

        if (dumpData)
        {
            string nm;
            for (int i = 0; i < results.size(); i++)
            {
                if (i == 0)
                    nm = "gev_returnValue.txt";
                else if (i == 1)
                    nm = "gev_se.returnValue.txt";
                    
                ofstream ofile(nm.c_str());
                for (int j = 0; j < numTuples-1; j++)
                    ofile<<results[i][j]<<", ";
                ofile<<results[i][numTuples-1];
            }
        }
    }
    else
        SetOutputDataTree(new avtDataTree());

    avtCallback::ResetTimeout(5*60);
#endif
}
