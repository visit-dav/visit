/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//  File: avtRExtremesFilter.C
// ************************************************************************* //

#include <avtParallel.h>
#include <avtRExtremesFilter.h>
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
#include <vtkRectilinearGrid.h>
#include <InvalidFilesException.h>
#include <FileWriter.h>

#ifdef PARALLEL
  #include <mpi.h>
#endif

using namespace std;

string avtRExtremesFilter::dataStr = "data";

// ****************************************************************************
// Method:  avtRExtremesFilter::avtRExtremesFilter
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

avtRExtremesFilter::avtRExtremesFilter()
{
    outDS = NULL;
    numTuples = 0;
    nodeCenteredData = false;
    numYears = 0;
    numDataYears = 0;
    numBins = 0;
    outputValsPerLoc = 0;
    initialized = false;
    idx0 = idxN = 0;
    cycle0 = 0;
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
// Method:  avtRExtremesFilter::GetMonthFromDay
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

int
avtRExtremesFilter::GetMonthFromDay(int t)
{
    int dayInYear = t % daysPerYear;
    int dayCountAtMonthEnd[12] = {31,59,90,120,151,181,213,244,274,304,334,365};
    int month = -1;
    for (int i = 0; i < 12; i++)
        if (dayInYear <= dayCountAtMonthEnd[i])
            return i;
    
    EXCEPTION1(ImproperUseException, "Date out of range.");
}

// ****************************************************************************
// Method:  avtRExtremesFilter::GetYearFromDay
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

int
avtRExtremesFilter::GetYearFromDay(int t)
{
    int x = t/daysPerYear;
    return t/daysPerYear;
}

// ****************************************************************************
// Method:  avtRExtremesFilter::GetSeasonFromDay
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

int
avtRExtremesFilter::GetSeasonFromDay(int t)
{
    int month = GetMonthFromDay(t);
    if (month == 11 || month == 0 || month == 1) // WINTER
        return 0;
    else if (month == 2 || month == 3 || month == 4) //SPRING
        return 1;
    else if (month == 5 || month == 6 || month == 7) //SUMMER
        return 2;
    else if (month == 8 || month == 9 || month == 10) //FALL
        return 3;
    
    EXCEPTION1(ImproperUseException, "Date out of range.");
}

// ****************************************************************************
// Method:  avtRExtremesFilter::GetIndexFromDay
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

int
avtRExtremesFilter::GetIndexFromDay(int t)
{
    if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::ANNUAL)
        return 0;
    else if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::MONTHLY)
        return GetMonthFromDay(t);
    else if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::SEASONAL)
        return GetMonthFromDay(t); //DRP GetSeasonFromDay(t);
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

    daysPerYear = 365;
    /*
    if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::ANNUAL)
    {
        daysPerYear = atts.GetDaysPerYear();
    }
    else if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::SEASONAL||
             atts.GetAggregation() == ExtremeValueAnalysisAttributes::MONTHLY)
    {
        daysPerYear = 0;
        for (int i = 0; i < 12; i++)
        {
            daysPerYear += atts.GetDaysPerMonth()[i];
            dayCountAtMonthEnd[i] = daysPerYear;
        }
    }
    */

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
    numYears = numTimes/daysPerYear;
    numDataYears = numYears;

    int dsCycle = (float)GetInput()->GetInfo().GetAttributes().GetCycle();
#ifdef PARALLEL
    cycle0 = UnifyMinimumValue(dsCycle);
#else
    cycle0 = dsCycle;
#endif

    if (atts.GetDataAnalysisYearRangeEnabled())
    {
        numYears = atts.GetDataAnalysisYear2()-atts.GetDataAnalysisYear1()+1;
        numTimes = numYears*daysPerYear;
        numDataYears = numYears;
    }

    if (atts.GetEnsemble())
    {
        numTimes *= atts.GetNumEnsembles();
        numDataYears *= atts.GetNumEnsembles();
    }

    //cout<<"numTimes = "<<numTimes<<" : numYears = "<<numYears<<" daysPerYear= "<<daysPerYear<<endl;


    //How to compute maxes.
    //Monthly maxes.
    if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::MONTHLY)
        numBins = 12;
    else if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::SEASONAL)
        numBins = 12; //DRP 4;
    else if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::ANNUAL)
        numBins = 1;

    //cout<<"values:"<<values.size()<<" x "<<values[0].size()<<" x "<<values[0][0].size()<<endl;
    //cout<<"values["<<numTuples<<"]["<<numBins<<"][numTimes]"<<endl;
    
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

    float extremeVal = (atts.GetExtremeMethod() == ExtremeValueAnalysisAttributes::MAXIMA ?
                        -numeric_limits<double>::max() : numeric_limits<double>::max());
    
    values.resize(numTuples);
    for (int i = 0; i < numTuples; i++)
    {
        values[i].resize(numDataYears);
        for (int j = 0; j < numDataYears; j++)
        {
            values[i][j].resize(numBins);
            for (int k = 0; k < numBins; k++)
                values[i][j][k] = extremeVal;
        }
    }
    //cout<<"VALUES["<<values.size()<<"]["<<values[0].size()<<"]["<<values[0][0].size()<<"]"<<endl;

    
    if (atts.GetDumpData())
    {
        vtkRectilinearGrid *rg = vtkRectilinearGrid::SafeDownCast(inDS);
        if (inDS->GetDataObjectType() != VTK_RECTILINEAR_GRID || rg == NULL)
        {
            EXCEPTION1(ImproperUseException, "Can't output non-Rectilinear grid data.");
        }

        coordinates[0].resize(rg->GetDimensions()[0]);
        coordinates[1].resize(rg->GetDimensions()[1]);
        coordinates[2].resize(rg->GetDimensions()[2]);

        vtkDataArray *x = rg->GetXCoordinates();
        vtkDataArray *y = rg->GetYCoordinates();
        vtkDataArray *z = rg->GetZCoordinates();

        for (int i = 0; i < coordinates[0].size(); i++)
            coordinates[0][i] = x->GetTuple1(i);
        for (int i = 0; i < coordinates[1].size(); i++)
            coordinates[1][i] = y->GetTuple1(i);
        for (int i = 0; i < coordinates[2].size(); i++)
            coordinates[2][i] = z->GetTuple1(i);
    }

    GenerateOutputInfo();
    initialized = true;
    delete [] leaves;
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

    if (!atts.GetEnsemble() && atts.GetDataAnalysisYearRangeEnabled())
    {
        int currYear = atts.GetDataYearBegin()+(currentTime/daysPerYear);
        if (currYear < atts.GetDataAnalysisYear1() ||
            currYear > atts.GetDataAnalysisYear2())
        {
            //cout<<"Skipping "<<currentTime<<" : year= "<<currYear<<endl;
            return;
        }
    }
    
    int nleaves;
    vtkDataSet **leaves = GetInputDataTree()->GetAllLeaves(nleaves);
    vtkDataSet *ds = leaves[0];
    vtkFloatArray *scalars = NULL;

    if (nodeCenteredData)
        scalars = (vtkFloatArray *)ds->GetPointData()->GetScalars();
    else
        scalars = (vtkFloatArray *)ds->GetCellData()->GetScalars();
    float *vals = (float *) scalars->GetVoidPointer(0);

    float dsTime = (float)GetInput()->GetInfo().GetAttributes().GetTime();
    int dsCycle = (float)GetInput()->GetInfo().GetAttributes().GetCycle();

    int timeNow = dsCycle - cycle0;

    int nTuples = scalars->GetNumberOfTuples();
    int index = GetIndexFromDay(timeNow); //currentTime);
    //int year = GetYearFromDay(timeNow); //currentTime);
    int year = GetYearFromDay(currentTime);
    
    float scaling = atts.GetDataScaling();
    //cout<<"processing "<<currentTime<<" sv = "<<scaling<<" index= "<<index<<" nTuples= "<<nTuples<<endl;
    //cout<<"Processing: ct= "<<currentTime<<" dsTime= "<<dsTime<<" dsCycle= "<<dsCycle<<endl;
    //cout<<" currentTime= "<<currentTime<<" cycle0= "<<cycle0<<" timeNow= "<<timeNow<<" year= "<<year<<endl;

    if (atts.GetExtremeMethod() == ExtremeValueAnalysisAttributes::MAXIMA)
    {
        for (int i = 0; i < nTuples; i++)
        {
            float v = vals[i]*scaling;
            if (v > values[i][year][index])
                values[i][year][index] = v;
        }
    }
    else
    {
        for (int i = 0; i < nTuples; i++)
        {
            float v = vals[i]*scaling;
            if (v < values[i][year][index])
                values[i][year][index] = v;
        }
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

    //Unify maxima/minima
#ifdef PARALLEL

    float *outVals = new float[numBins];
    for (int i = 0; i < numTuples; i++)
    {
        for (int y = 0; y < numDataYears; y++)
        {
            if (atts.GetExtremeMethod() == ExtremeValueAnalysisAttributes::MAXIMA)
                UnifyMaximumFloatArrayAcrossAllProcessors(&values[i][y][0], outVals, numBins);
            else
                UnifyMinimumFloatArrayAcrossAllProcessors(&values[i][y][0], outVals, numBins);
            for (int b = 0; b < numBins; b++)
                values[i][y][b] = outVals[b];
        }
    }
    delete [] outVals;

#endif
    
    vtkRInterface *RI = vtkRInterface::New();

    double *outputData = new double[(idxN-idx0)*outputValsPerLoc];
    for (int i = 0; i < (idxN-idx0)*outputValsPerLoc; i++)
        outputData[i] = -1.0;

    for (int i = 0; i < (idxN-idx0); i++)
    {
        vtkDoubleArray *data = vtkDoubleArray::New();

        SetExtremeData(i+idx0, data);
        
        RI->AssignVTKDataArrayToRVariable(data, dataStr.c_str());

        string command = GenerateRCommand(i+idx0);
        if (atts.GetDumpDebug())
        {
            cout<<"*************************************************************"<<endl;
            cout<<"location= "<<(idx0+i)<<endl;
            cout<<command;
            cout<<"*************************************************************"<<endl;
        }

        RI->EvalRscript(command.c_str());

        data->Delete();

        int nVars = outputInfo.size();
        //PrintOutputInfo();

        for (int j = 0; j < outputInfo.size(); j++)
        {
            vtkDoubleArray *outArr = NULL;
            outArr = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray(outputInfo[j].name.c_str()));
            //cout<<outputInfo[j].name<<" : "<<outArr->GetNumberOfComponents()<<" x "<<outArr->GetNumberOfTuples()<<endl;
            for (int k = 0; k < outArr->GetNumberOfTuples(); k++)
            {
                outputData[i*outputValsPerLoc + outputInfo[j].index + k] = outArr->GetTuple1(k);
                //cout<<"    outputData["<<i*outputValsPerLoc + outputInfo[j].index + k<<"] = "<<outArr->GetTuple1(k)<<endl;
            }
        }
    }
    
#if PARALLEL
    int sz = numTuples*outputValsPerLoc;
    
    double *in = new double[sz], *sum = new double[sz];
    for (int i = 0; i < sz; i++)
    {
        in[i] = 0.0;
        sum[i] = 0.0;
    }
    
    for (int i=0; i<(idxN-idx0); i++)
    {
        for (int j = 0; j < outputValsPerLoc; j++)
        {
            in[(i+idx0)*outputValsPerLoc +j] = outputData[i*outputValsPerLoc+j];
        }
    }
    SumDoubleArray(in, sum, sz);

    delete outputData;
    outputData = new double[sz];
    for (int i = 0; i < sz; i++)
        outputData[i] = sum[i];
    delete [] in;
    delete [] sum;
#endif

    if (PAR_Rank() == 0 && atts.GetDumpData())
    {
        //PrintOutputInfo();
        vector<string> meshDimNms;
        vector<vector<double> > meshDims;
        vector<POTFilterWriteData::varInfo> vars;
        vector<int> arrShape;

        //TODO: These should be swapped in the writer...
        meshDimNms.push_back("Lat");        
        meshDimNms.push_back("Lon");
        meshDimNms.push_back("Aggregation");
        meshDims.resize(3);
        meshDims[0] = coordinates[1];
        meshDims[1] = coordinates[0];
        for (int i = 0; i < numBins; i++)
            meshDims[2].push_back(i);
        int nVars = outputInfo.size();
        int nLocs = meshDims[0].size()*meshDims[1].size();
        int nVals = nLocs * outputValsPerLoc;
        
        int idx = 0;
        vector<string> varnames;
        for (int i = 0; i < nVars; i++)
            varnames.push_back(outputInfo[i].name);
        
        //cout<<"DUMP: nVars= "<<nVars<<" nVals= "<<numTuples*outputValsPerLoc<<endl;
        POTFilterWriteData::writeNETCDFData("GEV.output.nc",
                                            meshDimNms, meshDims,
                                            varnames, outputData);
    }
    
    //Set the return dataset.
    if (PAR_Rank() == 0)
    {
        int displayIdx = 0;
        if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::ANNUAL)
            displayIdx = 0;
        else if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::MONTHLY)
            displayIdx = (int)atts.GetDisplayMonth() - (int)(ExtremeValueAnalysisAttributes::JANUARY);
        else if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::MONTHLY)
            displayIdx = (int)atts.GetDisplaySeason() - (int)(ExtremeValueAnalysisAttributes::WINTER);
        
        //cout<<"Create output.....("<<numOutputComponents<<" x "<<numTuples<<")"<<endl;
        vtkFloatArray *outVar = vtkFloatArray::New();
        outVar->SetNumberOfComponents(1);
        outVar->SetNumberOfTuples(numTuples);
        int idx = 0;
        for (int i = 0; i < numTuples; i++, idx++)
            outVar->SetValue(i, outputData[i*outputValsPerLoc + displayIdx]);
    
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

    RI->Delete();

    avtCallback::ResetTimeout(5*60);



#if 0
    avtCallback::ResetTimeout(0);
    
    //Exchange data....
#ifdef PARALLEL
    float *tmp = new float[numTimes];
    float *res = new float[numTimes];
    int *flags = new int[numTimes];
    int *flagsRes = new int[numTimes];
    float *times = new float[numTimes];
    float *timesRes = new float[numTimes];
    
    for (int i=0; i<numTuples; i++)
    {
        for (int t=0; t<numTimes; t++)
        {
            tmp[t] = 0.0;//-numeric_limits<float>::min();
            flags[t] = 0;
            times[t] = 0.0;
        }
        for (int b = 0; b < numBins; b++)
        {
            int nt = values[i][b].size();
            for (int t=0; t<nt; t++)
            {
                tmp[values[i][b][t].Cycle] = values[i][b][t].val;
                flags[values[i][b][t].Cycle] = 1;
                times[values[i][b][t].Cycle] = values[i][b][t].Cycle;
            }
        }
        MPI_Allreduce(tmp, res, numTimes, MPI_FLOAT, MPI_SUM, VISIT_MPI_COMM);
        MPI_Allreduce(times, timesRes, numTimes, MPI_FLOAT, MPI_SUM, VISIT_MPI_COMM);
        MPI_Allreduce(flags, flagsRes, numTimes, MPI_INT, MPI_SUM, VISIT_MPI_COMM);

        for (int b = 0; b < numBins; b++)
            values[i][b].resize(0);

        //See if it's one of my locations.
        if (i >= idx0 && i < idxN)
        {
            for (int t=0; t<numTimes; t++)
            {
                if (flagsRes[t])
                {
                    int b = GetIndexFromDay(t);
                    values[i][b].push_back(sample(res[t], t, timesRes[t]));
                }
            }
        }
    }
    
    delete [] tmp;
    delete [] res;
    delete [] flags;
    delete [] flagsRes;
#endif

    vtkRInterface *RI = vtkRInterface::New();

    double *outputData = new double[(idxN-idx0)*outputValsPerLoc];
    for (int i = 0; i < (idxN-idx0)*outputValsPerLoc; i++)
        outputData[i] = -1.0;

    for (int i = 0; i < (idxN-idx0); i++)
    {
        vector<float> tresholds = CalculateThresholds(i+idx0);
        
        vtkDoubleArray *exceedences = vtkDoubleArray::New();
        vtkIntArray *dayIndices = vtkIntArray::New();
        vtkIntArray *yearIndices = vtkIntArray::New();
        vtkIntArray *monthIndices = vtkIntArray::New();

        SetExceedenceData(i+idx0, tresholds, exceedences, dayIndices, yearIndices, monthIndices);
        
        RI->AssignVTKDataArrayToRVariable(exceedences, exceedencesStr.c_str());
        RI->AssignVTKDataArrayToRVariable(yearIndices, yearIndicesStr.c_str());
        RI->AssignVTKDataArrayToRVariable(monthIndices, monthIndicesStr.c_str());
        RI->AssignVTKDataArrayToRVariable(dayIndices, dayIndicesStr.c_str());

        string command = GenerateRCommand(tresholds, i+idx0);
        if (atts.GetDumpDebug())
        {
            cout<<"*************************************************************"<<endl;
            cout<<"location= "<<(idx0+i)<<endl;
            cout<<command;
            cout<<"*************************************************************"<<endl;
        }

        RI->EvalRscript(command.c_str());

        exceedences->Delete();
        dayIndices->Delete();
        yearIndices->Delete();
        monthIndices->Delete();

        int nVars = outputInfo.size();
        //PrintOutputInfo();
        for (int j = 0; j < outputInfo.size(); j++)
        {
            vtkDoubleArray *outArr = NULL;
            outArr = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray(outputInfo[j].name.c_str()));
            //cout<<outputInfo[j].name<<" : "<<outArr->GetNumberOfComponents()<<" x "<<outArr->GetNumberOfTuples()<<endl;
            for (int k = 0; k < outArr->GetNumberOfTuples(); k++)
            {
                outputData[i*outputValsPerLoc + outputInfo[j].index + k] = outArr->GetTuple1(k);
                //cout<<"    outputData["<<i*outputValsPerLoc + outputInfo[j].index + k<<"] = "<<outArr->GetTuple1(k)<<endl;
            }
        }
    }

#if PARALLEL
    int sz = numTuples*outputValsPerLoc;
    
    double *in = new double[sz], *sum = new double[sz];
    for (int i = 0; i < sz; i++)
    {
        in[i] = 0.0;
        sum[i] = 0.0;
    }
    
    for (int i=0; i<(idxN-idx0); i++)
    {
        for (int j = 0; j < outputValsPerLoc; j++)
        {
            in[(i+idx0)*outputValsPerLoc +j] = outputData[i*outputValsPerLoc+j];
        }
    }
    SumDoubleArray(in, sum, sz);

    delete outputData;
    outputData = new double[sz];
    for (int i = 0; i < sz; i++)
        outputData[i] = sum[i];
    delete [] in;
    delete [] sum;
#endif

    if (PAR_Rank() == 0 && atts.GetDumpData())
    {
        //PrintOutputInfo();
        vector<string> meshDimNms;
        vector<vector<double> > meshDims;
        vector<POTFilterWriteData::varInfo> vars;
        vector<int> arrShape;

        //TODO: These should be swapped in the writer...
        meshDimNms.push_back("Lat");        
        meshDimNms.push_back("Lon");
        meshDimNms.push_back("Aggregation");
        meshDims.resize(3);
        meshDims[0] = coordinates[1];
        meshDims[1] = coordinates[0];
        for (int i = 0; i < numBins; i++)
            meshDims[2].push_back(i);
        int nVars = outputInfo.size();
        int nLocs = meshDims[0].size()*meshDims[1].size();
        int nVals = nLocs * outputValsPerLoc;
        
        int idx = 0;
        vector<string> varnames;
        for (int i = 0; i < nVars; i++)
            varnames.push_back(outputInfo[i].name);
        
        //cout<<"DUMP: nVars= "<<nVars<<" nVals= "<<numTuples*outputValsPerLoc<<endl;
        POTFilterWriteData::writeNETCDFData("GEV.output.nc",
                                            meshDimNms, meshDims,
                                            varnames, outputData);
    }
    
    //Set the return dataset.
    if (PAR_Rank() == 0)
    {
        int displayIdx = 0;
        if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::ANNUAL)
            displayIdx = 0;
        else if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::MONTHLY)
            displayIdx = (int)atts.GetDisplayMonth() - (int)(ExtremeValueAnalysisAttributes::JAN);
        else if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::MONTHLY)
            displayIdx = (int)atts.GetDisplaySeason() - (int)(ExtremeValueAnalysisAttributes::WINTER);
        
        //cout<<"Create output.....("<<numOutputComponents<<" x "<<numTuples<<")"<<endl;
        vtkFloatArray *outVar = vtkFloatArray::New();
        outVar->SetNumberOfComponents(1);
        outVar->SetNumberOfTuples(numTuples);
        int idx = 0;
        for (int i = 0; i < numTuples; i++, idx++)
            outVar->SetValue(i, outputData[i*outputValsPerLoc + displayIdx]);
    
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

    RI->Delete();

    avtCallback::ResetTimeout(5*60);
#endif
}

#if 0

    if (0)
    {
        vector<string> meshDimNms;
        vector<vector<double> > meshDims;
        vector<POTFilterWriteData::varInfo> vars;
        vector<int> arrShape;
        vtkDoubleArray *array = vtkDoubleArray::New();
        
        meshDimNms.push_back("lat");
        meshDimNms.push_back("lon");
        meshDimNms.push_back("index");
        int nx = 4, ny = 5, nz = 1;
        meshDims.push_back(makeArray(nx));
        meshDims.push_back(makeArray(ny));
        meshDims.push_back(makeArray(nz));
        
        int nVals = nx*ny*nz;
        int nVars = 2;
        arrShape.push_back(nVals);
        arrShape.push_back(nVars);
        
        POTFilterWriteData::varInfo v0, v1;
        v0.name = "var1";
        v0.dims.push_back("lat");
        v0.dims.push_back("lon");
        v0.indices.push_back(0);

        v1.name = "var2";
        v1.dims.push_back("lat");
        v1.dims.push_back("lon");
        v1.indices.push_back(1);
        vars.push_back(v0);
        vars.push_back(v1);
        
        vector<double> d = makeArray(nVals*nVars);
        array->SetNumberOfTuples(d.size());
        for (int i = 0; i < d.size(); i++)
            array->SetTuple1(i, d[i]);
        
        POTFilterWriteData::writeNETCDFData("output.nc",
                                            meshDimNms, meshDims,
                                            vars, arrShape, array);

        SetOutputDataTree(new avtDataTree());
        return;
    }
    


    //Exchange data....
    if (atts.GetEnsemble())
    {
        numTimes *= atts.GetNumEnsembles();
    }
#ifdef PARALLEL
    float *tmp = new float[numTimes];
    float *res = new float[numTimes];
    int *flags = new int[numTimes];
    int *flagsRes = new int[numTimes];
    float *times = new float[numTimes];
    float *timesRes = new float[numTimes];
    
    for (int i=0; i<numTuples; i++)
    {
        for (int t=0; t<numTimes; t++)
        {
            tmp[t] = 0.0;//-numeric_limits<float>::min();
            flags[t] = 0;
            times[t] = 0.0;
        }
        for (int b = 0; b < numBins; b++)
        {
            int nt = values[i][b].size();
            for (int t=0; t<nt; t++)
            {
                //if(PAR_Rank()== 0 ) cout<<values[i][b][t].time<<endl;
                tmp[values[i][b][t].Cycle] = values[i][b][t].val;
                flags[values[i][b][t].Cycle] = 1;
                times[values[i][b][t].Cycle] = values[i][b][t].Time;
            }
        }
        MPI_Allreduce(tmp, res, numTimes, MPI_FLOAT, MPI_SUM, VISIT_MPI_COMM);
        MPI_Allreduce(times, timesRes, numTimes, MPI_FLOAT, MPI_SUM, VISIT_MPI_COMM);
        MPI_Allreduce(flags, flagsRes, numTimes, MPI_INT, MPI_SUM, VISIT_MPI_COMM);

        for (int b = 0; b < numBins; b++)
            values[i][b].resize(0);

        //See if it's one of my locations.
        if (i >= idx0 && i < idxN)
        {
            for (int t=0; t<numTimes; t++)
            {
                if (flagsRes[t])
                {
                    int b = GetIndexFromDay(t);
                    values[i][b].push_back(sample(res[t], t, timesRes[t]));
                }
            }
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
            "source('%s/auxil.r')\n"   \
            "source('%s/pp.fit2.r')\n" \
            "source('%s/potVisit.r')\n",
            codeDir.c_str(),
            codeDir.c_str(),
            codeDir.c_str());
    
    char potCmd[1024];
            
    vector<float> thresholds(numBins);
    
    int nValsPerOut = 1;
    if (atts.GetComputeCovariates())
    {
        nValsPerOut *= atts.GetCovariateReturnYears().size();
        if (atts.GetComputeRVDifferences())
            nValsPerOut++;
    }
    
    vector<vector<vector<float> > > outputs_rv, outputs_serv;
    outputs_rv.resize(nValsPerOut);
    outputs_serv.resize(nValsPerOut);
    for (int n = 0; n < nValsPerOut; n++)
    {
        outputs_rv[n].resize(numBins);
        outputs_serv[n].resize(numBins);
        for (int b = 0; b < numBins; b++)
        {
            outputs_rv[n][b].resize(numTuples);
            outputs_serv[n][b].resize(numTuples);
        }
    }

    vector<vector<vector<float> > > outputs_mle, outputs_semle;
    if (atts.GetComputeParamValues())
    {
        outputs_mle.resize(4);
        outputs_semle.resize(4);
        for (int n = 0; n < 4; n++)
        {
            outputs_mle[n].resize(numBins);
            outputs_semle[n].resize(numBins);
            for (int b = 0; b < numBins; b++)
            {
                outputs_mle[n][b].resize(numTuples);
                outputs_semle[n][b].resize(numTuples);
            }
        }
    }

    for (int i = idx0; i < idxN; i++)
    {
        //DebugData(i, "./dumps/x0");
        for (int j = 0; j < numBins; j++)
            thresholds[j] = CalculateThreshold(i, j);

        int numExceedences = 0;
        for (int b = 0; b < numBins; b++)
        {
            int nt = values[i][b].size();
            if (atts.GetCutoffMode() == ExtremeValueAnalysisAttributes::UPPER_TAIL)
            {
                for (int t = 0; t < nt; t++)
                    if (values[i][b][t].val > thresholds[b])
                        numExceedences++;
            }
            else
            {
                for (int t = 0; t < nt; t++)
                    if (values[i][b][t].val < thresholds[b])
                        numExceedences++;
            }
        }
        
        //cout<<"numExceedences: "<<numExceedences<<endl;
        vtkDoubleArray *exceedences = vtkDoubleArray::New();
        vtkIntArray *dayIndices = vtkIntArray::New();
        vtkIntArray *monthIndices = vtkIntArray::New();
        vtkIntArray *yearIndices = vtkIntArray::New();
        exceedences->SetNumberOfComponents(1);
        exceedences->SetNumberOfTuples(numExceedences);
        dayIndices->SetNumberOfComponents(1);
        dayIndices->SetNumberOfTuples(numExceedences);
        monthIndices->SetNumberOfComponents(1);
        monthIndices->SetNumberOfTuples(numExceedences);
        yearIndices->SetNumberOfComponents(1);
        yearIndices->SetNumberOfTuples(numExceedences);
        
        int idx = 0;
        for (int b = 0; b < numBins; b++)
        {
            int nt = values[i][b].size();
            for (int t = 0; t < nt; t++)
            {
                if (values[i][b][t].val > thresholds[b])
                {
                    exceedences->SetValue(idx, values[i][b][t].val);
                    dayIndices->SetValue(idx, values[i][b][t].Time+1);
                    if (atts.GetAggregation() != ExtremeValueAnalysisAttributes::ANNUAL)
                        monthIndices->SetValue(idx, GetMonthFromDay(values[i][b][t].Time)+1);
                    yearIndices->SetValue(idx, GetYearFromDay(values[i][b][t].Time));//+1);
                 
                    idx++;
                    if (idx == numExceedences)
                        break;
                }
            }
        }
        RI->AssignVTKDataArrayToRVariable(exceedences, "exceedences");
        RI->AssignVTKDataArrayToRVariable(dayIndices, "dayIndices");
        RI->AssignVTKDataArrayToRVariable(monthIndices, "monthIndices");
        string threshStr, aggrStr;
        if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::ANNUAL)
        {
            aggrStr = "'annual'";
            char str[32];
            sprintf(str, "%f", thresholds[0]);
            threshStr = str;
        }
        else if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::MONTHLY)
        {
            aggrStr = "'monthly'";
            threshStr = "thresholds";
            vtkDoubleArray *thresh = vtkDoubleArray::New();
            thresh->SetNumberOfComponents(1);
            thresh->SetNumberOfTuples(12);
            for (int t = 0; t < 12; t++)
                thresh->SetValue(t, thresholds[t]);
            RI->AssignVTKDataArrayToRVariable(thresh, "thresholds");
        }
        else if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::SEASONAL)
        {
            aggrStr = "'seasonal'";
            threshStr = "thresholds";
            vtkDoubleArray *thresh = vtkDoubleArray::New();
            thresh->SetNumberOfComponents(1);
            thresh->SetNumberOfTuples(4);
            for (int t = 0; t < 4; t++)
                thresh->SetValue(t, thresholds[t]);
            RI->AssignVTKDataArrayToRVariable(thresh, "thresholds");
        }
        
        int nCovariates = 0;
        string yearIndicesNm = "NULL", monthIndicesNm = "NULL";
        string covByYear = "NULL", newDataStr = "NULL", rvDiffStr = "NULL";
        string useLocModelStr = "NULL", useShapeModelStr = "NULL", useScaleModelStr = "NULL";
        string returnParamsStr = "FALSE";

        if (atts.GetComputeParamValues())
            returnParamsStr = "TRUE";
        if (atts.GetAggregation() != ExtremeValueAnalysisAttributes::ANNUAL)
            monthIndicesNm = "monthIndices";
        
        if (atts.GetComputeCovariates())
        {
            nCovariates = 1;
            yearIndicesNm = "yearIndices";
            covByYear = "covByYear";
            
            vtkIntArray *covByYearArr = vtkIntArray::New();
            covByYearArr->SetNumberOfComponents(1);
            
            if (0)//atts.GetEnsemble())
            {
                int N = numYears * atts.GetNumEnsembles();
                covByYearArr->SetNumberOfTuples(N);
                int idx = 0;
                for (int e = 0; e < atts.GetNumEnsembles(); e++)
                {
                    for (int y = 0; y < numYears; y++, idx++)
                    {
                        covByYearArr->SetValue(idx, y + atts.GetDataYearBegin());
                    }
                }
            }
            else
            {
                covByYearArr->SetNumberOfTuples(numYears);
                for (int y = 0; y < numYears; y++)
                    covByYearArr->SetValue(y, y + atts.GetDataYearBegin());
            }
            RI->AssignVTKDataArrayToRVariable(yearIndices, yearIndicesNm.c_str());
            RI->AssignVTKDataArrayToRVariable(covByYearArr, covByYear.c_str());
            covByYearArr->Delete();
            
            newDataStr = "newData";
            vtkIntArray *newData = vtkIntArray::New();
            newData->SetNumberOfComponents(1);
            newData->SetNumberOfTuples(atts.GetCovariateReturnYears().size());
            for (int y = 0; y < atts.GetCovariateReturnYears().size(); y++)
            {
                newData->SetValue(y, atts.GetCovariateReturnYears()[y]);//-atts.GetDataYearBegin() +1);
            }
            RI->AssignVTKDataArrayToRVariable(newData, newDataStr.c_str());
            newData->Delete();

            if (atts.GetComputeRVDifferences())
            {
                rvDiffStr = "rvDiff";
                vtkIntArray *rvDiff = vtkIntArray::New();
                rvDiff->SetNumberOfComponents(1);
                rvDiff->SetNumberOfTuples(2);
                rvDiff->SetValue(0, atts.GetRvDifference1());//-atts.GetDataYearBegin() +1);
                rvDiff->SetValue(1, atts.GetRvDifference2());//-atts.GetDataYearBegin() +1);
                RI->AssignVTKDataArrayToRVariable(rvDiff, rvDiffStr.c_str());
                rvDiff->Delete();
            }
            if (atts.GetCovariateModelLocation())
                useLocModelStr = "1";
            if (atts.GetCovariateModelShape())
                useShapeModelStr = "1";
            if (atts.GetCovariateModelScale())
                useScaleModelStr = "1";

        }
        string dumpStr0 = "", dumpStr1 = "";
        if (1)
        {
            char tmp[128];
            sprintf(tmp, "save.image(file='input_%d.RData')\n", PAR_Rank());
            dumpStr0 = tmp;
            sprintf(tmp, "save.image(file='output_%d.RData')\n", PAR_Rank());
            dumpStr1 = tmp;
        }
        string outputStr = "rv = output$returnValue\n";
        outputStr += "se_rv = output$se.returnValue\n";
        if (atts.GetComputeCovariates() && atts.GetComputeRVDifferences())
        {
            outputStr += "rvDiff = output$returnValueDiff;\n";
            outputStr += "se_rvDiff = output$se.returnValueDiff;\n";
        }
        if (atts.GetComputeParamValues())
        {
            outputStr += "mle = output$mle;\n";
            outputStr += "se_mle = output$se.mle;\n";
        }
        string optimMethod = (atts.GetOptimizationMethod() == ExtremeValueAnalysisAttributes::NELDER_MEAD ?
                              "Nelder-Mead" : "BFGS");
        string upper_tail = (atts.GetCutoffMode() == ExtremeValueAnalysisAttributes::UPPER_TAIL ?
                             "TRUE" : "FALSE");
        string multiDayEventHandling = (atts.GetNoConsecutiveDay() ? "'norun'" : "NULL");
        char tmp[64];
        sprintf(tmp, "%d", atts.GetDataYearBegin());
        string initialYear = tmp;

        string numPerYearStr;
        if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::ANNUAL)
        {
            sprintf(tmp, "%d", atts.GetDaysPerYear());
            numPerYearStr = tmp;
        }
        else
        {
            numPerYearStr = "c(";
            for (int m = 0; m < 12; m++)
            {
                sprintf(tmp, "%d", atts.GetDaysPerMonth()[m]);
                numPerYearStr = numPerYearStr + tmp;
                if (m != 11)
                    numPerYearStr = numPerYearStr + ",";
            }
            numPerYearStr = numPerYearStr + ")";
        }

        int nReplicates = 1;
        if (atts.GetEnsemble())
            nReplicates = atts.GetNumEnsembles();
        sprintf(potCmd,
                "%s" \
                "require(ismev)\n" \
                "output = potFit(data = exceedences, "\
                "day = dayIndices, month = %s, year=%s, initialYear=%s, "\
                "nYears=%d, threshold=%s, aggregation=%s, " \
                "numPerYear=%s, nReplicates=%d, " \
                "nCovariates=%d, covariatesByYear=%s, "\
                "locationModel=%s, scaleModel=%s, shapeModel=%s, "\
                "newData=%s, rvDifference=%s, " \
                "returnParams=%s, "\
                "optimMethod=%s, multiDayEventHandling=%s, upper.tail=%s" \
                ")\n"\
                "%s" \
                "%s",
                dumpStr0.c_str(),
                monthIndicesNm.c_str(),
                yearIndicesNm.c_str(), 
                initialYear.c_str(),
                numYears, threshStr.c_str(), aggrStr.c_str(),
                numPerYearStr.c_str(), nReplicates,
                nCovariates, covByYear.c_str(),
                useLocModelStr.c_str(), useShapeModelStr.c_str(), useScaleModelStr.c_str(),
                newDataStr.c_str(), rvDiffStr.c_str(),
                returnParamsStr.c_str(),
                optimMethod.c_str(), multiDayEventHandling.c_str(), upper_tail.c_str(),
                dumpStr1.c_str(),
                outputStr.c_str());
        
        string command = fileLoad;
        command += potCmd;
        cout<<command<<endl;
        RI->EvalRscript(command.c_str());

        cout<<"*************************************************************"<<endl;
        cout<<GenerateRCommand(i);
        cout<<"*************************************************************"<<endl;

        vtkDoubleArray *out_rv = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("rv"));
        vtkDoubleArray *out_serv = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("se_rv"));
        //cout<<"valsPerLoc: "<<nValsPerOut<<endl;
        //cout<<"out_rv  : "<<out_rv->GetNumberOfComponents()<<" x "<<out_rv->GetNumberOfTuples()<<endl;
        //cout<<"out_serv: "<<out_serv->GetNumberOfComponents()<<" x "<<out_serv->GetNumberOfTuples()<<endl;

        int N = nValsPerOut;
        if (atts.GetComputeRVDifferences())
            N--;
        
        for (int b = 0; b < numBins; b++)
        {
            for (int n = 0; n < N; n++)
            {
                outputs_rv[n][b][i] = (float)out_rv->GetComponent(b,n);
                outputs_serv[n][b][i] = (float)out_serv->GetComponent(b,n);
            }
        }
        
        vtkDoubleArray *out_rvDiff = NULL, *out_servDiff = NULL;
        if (atts.GetComputeCovariates() && atts.GetComputeRVDifferences())
        {
            out_rvDiff = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("rvDiff"));
            out_servDiff = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("se_rvDiff"));
            //cout<<"out_rvDiff  : "<<out_rvDiff->GetNumberOfComponents()<<" x "<<out_rvDiff->GetNumberOfTuples()<<endl;
            //cout<<"out_servDiff: "<<out_servDiff->GetNumberOfComponents()<<" x "<<out_servDiff->GetNumberOfTuples()<<endl;
            int N = nValsPerOut-1;
            for (int n = nValsPerOut-1; n < nValsPerOut; n++)
            {
                for (int b = 0; b < numBins; b++)
                {
                    outputs_rv[n][b][i] = (float)out_rvDiff->GetComponent(b,0);
                    outputs_serv[n][b][i] = (float)out_servDiff->GetComponent(b,0);
                }
            }
        }
        if (atts.GetComputeParamValues())
        {
            vtkDoubleArray *out_mle, *out_semle;
            out_mle = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("mle"));
            out_semle = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("se_mle"));
            for (int n = 0; n < 4; n++)
            {
                for (int b = 0; b < numBins; b++)
                {
                    outputs_mle[n][b][i] = (float)out_mle->GetComponent(b,n);
                    outputs_semle[n][b][i] = (float)out_semle->GetComponent(b,n);
                }
            }
        }
        
        exceedences->Delete();
        dayIndices->Delete();
        monthIndices->Delete();
        yearIndices->Delete();
    }

#if PARALLEL
    float *in0 = new float[numTuples], *in1 = new float[numTuples];
    float *sum0 = new float[numTuples], *sum1 = new float[numTuples];
    int N = outputs_rv.size();
    
    for (int n = 0; n < N; n++)
        in0[n] = in1[n] = sum0[n] = sum1[n] = 0.0;
    for (int n = 0; n < N; n++)
    {
        for (int b = 0; b<numBins; b++)
        {
            for (int i=0; i<numTuples; i++)
            {
                if (i >= idx0 && i < idxN)
                {
                    in0[i] = outputs_rv[n][b][i];
                    in1[i] = outputs_serv[n][b][i];
                }
                else
                    in0[i] = in1[i] = 0.0f;
            }
            SumFloatArray(in0, sum0, numTuples);
            SumFloatArray(in1, sum1, numTuples);
            if (PAR_Rank() == 0)
            {
                for (int i=0; i<numTuples; i++)
                {
                    outputs_rv[n][b][i] = sum0[i];
                    outputs_serv[n][b][i] = sum1[i];
                }
            }
        }
    }
    if (atts.GetComputeParamValues())
    {
        for (int n = 0; n < N; n++)
            in0[n] = in1[n] = sum0[n] = sum1[n] = 0.0;
        N = outputs_mle.size();
        for (int n = 0; n < N; n++)
        {
            for (int b = 0; b<numBins; b++)
            {
                for (int i=0; i<numTuples; i++)
                {
                    if (i >= idx0 && i < idxN)
                    {
                        in0[i] = outputs_mle[n][b][i];
                        in1[i] = outputs_semle[n][b][i];
                    }
                    else
                        in0[i] = in1[i] = 0.0f;
                }
                SumFloatArray(in0, sum0, numTuples);
                SumFloatArray(in1, sum1, numTuples);
                if (PAR_Rank() == 0)
                {
                    for (int i=0; i<numTuples; i++)
                    {
                        outputs_mle[n][b][i] = sum0[i];
                        outputs_semle[n][b][i] = sum1[i];
                    }
                }
            }
        }
    }

    delete [] in0;
    delete [] in1;
    delete [] sum0;
    delete [] sum1;

#endif
    if (PAR_Rank() == 0)
    {
        int displayIdx = 0;
        if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::ANNUAL)
            displayIdx = 0;
        else if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::MONTHLY)
            displayIdx = (int)atts.GetDisplayMonth() - (int)(ExtremeValueAnalysisAttributes::JAN);
        else if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::MONTHLY)
            displayIdx = (int)atts.GetDisplaySeason() - (int)(ExtremeValueAnalysisAttributes::WINTER);
        
        //cout<<"Create output.....("<<numOutputComponents<<" x "<<numTuples<<")"<<endl;
        vtkFloatArray *outVar = vtkFloatArray::New();
        outVar->SetNumberOfComponents(1);
        outVar->SetNumberOfTuples(numTuples);
        int idx = 0;
        for (int i = 0; i < numTuples; i++, idx++)
        {
            outVar->SetValue(i, outputs_rv[0][displayIdx][i]);
        }
    
        if (nodeCenteredData)
            outDS->GetPointData()->SetScalars(outVar);
        else
            outDS->GetPointData()->SetScalars(outVar);
        
        outVar->Delete();

        avtDataTree_p outputTree = new avtDataTree(outDS, 0);
        SetOutputDataTree(outputTree);
        outDS->Delete();

        //cout<<"outputs: "<<outputs_rv.size()<<" "<<outputs_rv[0].size()<<" "<<outputs_rv[0][0].size()<<endl;
        if (atts.GetDumpData())
        {
            int N = outputs_rv.size();
            for (int b = 0; b < numBins; b++)
            {
                for (int n = 0; n < N; n++)
                {
                    string nm0 = GetDumpFileName(b, n, 0);
                    string nm1 = GetDumpFileName(b, n, 1);
                    ofstream ofile0(nm0.c_str());
                    ofstream ofile1(nm1.c_str());
                    //cout<<nm0<<": "<<N<<" x "<<numTuples<<endl;
                    //cout<<nm1<<": "<<N<<" x "<<numTuples<<endl;
                    for (int i = 0; i < numTuples; i++)
                    {
                        ofile0<<outputs_rv[n][b][i]<<" ";
                        ofile1<<outputs_serv[n][b][i]<<" ";
                    }
                    ofile0<<endl;
                    ofile1<<endl;
                }
            }
            if (atts.GetComputeParamValues())
            {
                int N = outputs_mle.size();
                for (int b = 0; b < numBins; b++)
                {
                    string nm0 = GetDumpFileName(b, -1, 2);
                    string nm1 = GetDumpFileName(b, -1, 3);
                    ofstream ofile0(nm0.c_str());
                    ofstream ofile1(nm1.c_str());
                    //cout<<nm0<<": "<<N<<" x "<<numTuples<<endl;
                    //cout<<nm1<<": "<<N<<" x "<<numTuples<<endl;
                    for (int i = 0; i < numTuples; i++)
                    {
                        for (int n = 0; n < N; n++)
                        {
                            //ofile0<<outputs_mle[n][b][i]<<" ";
                            //ofile1<<outputs_semle[n][b][i]<<" ";
                            ofile0<<outputs_mle[n][b][i]<<endl;
                            ofile1<<outputs_semle[n][b][i]<<endl;
                        }
                    }
                    ofile0<<endl;
                    ofile1<<endl;
                }
            }
        }
    }
    else
        SetOutputDataTree(new avtDataTree());

    avtCallback::ResetTimeout(5*60);
}
#endif


void
avtRExtremesFilter::GenerateOutputInfo()
{
    int index = 0;
    char tmp1[64], tmp2[64];
    if (!atts.GetComputeReturnValues())
    {
        outputInfo.push_back(outputType("rv", "output$returnValue", numBins, index));
        index += numBins;
        outputInfo.push_back(outputType("se_rv", "output$se.returnValue", numBins, index));
        index += numBins;
    }

    if (atts.GetComputeReturnValues())
    {
        int nv = atts.GetReturnValues().size();
        for (int i = 0; i < nv; i++)
        {
            sprintf(tmp1, "rv_%d", (atts.GetReturnValues()[i]));
            if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::ANNUAL)
                sprintf(tmp2, "output$returnValue[%d]", (i+1));
            else
            {
                if (nv == 1)
                    sprintf(tmp2, "output$returnValue");
                else
                    sprintf(tmp2, "output$returnValue[%d,]", (i+1));
            }
            outputInfo.push_back(outputType(tmp1, tmp2, numBins, index));
            index += numBins;

            sprintf(tmp1, "se_rv_%d", (atts.GetReturnValues()[i]));
            if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::ANNUAL)
                sprintf(tmp2, "output$se.returnValue[%d]", (i+1));
            else
            {
                if (nv == 1)
                    sprintf(tmp2, "output$se.returnValue");
                else
                    sprintf(tmp2, "output$se.returnValue[%d,]", (i+1));             
            }
            outputInfo.push_back(outputType(tmp1, tmp2, numBins, index));
            index += numBins;
        }
    }
    
    if (atts.GetComputeReturnValues() && atts.GetComputeRVDifferences())
    {
        sprintf(tmp1, "rvDiff_%d_%d", atts.GetRvDifference1(), atts.GetRvDifference2());
        outputInfo.push_back(outputType(tmp1, "output$returnValueDiff", numBins, index));
        index += numBins;

        sprintf(tmp1, "se_rvDiff_%d_%d", atts.GetRvDifference1(), atts.GetRvDifference2());
        outputInfo.push_back(outputType(tmp1, "output$se.returnValueDiff", numBins, index));
        index += numBins;
    }

    outputValsPerLoc = index;
}

void
avtRExtremesFilter::PrintOutputInfo()
{
    cout<<"OUTPUTINFO: "<<endl;
    for (int i = 0; i < outputInfo.size(); i++)
    {
        cout<<i<<": "<<outputInfo[i].name<<" "<<outputInfo[i].rName<<" idx= "<<outputInfo[i].index<<" dim= "<<outputInfo[i].dim<<endl;
    }
}

string
avtRExtremesFilter::GenerateRCommand(int loc)
{
    string command;
    char tmp[512];

    if (atts.GetDumpDebug())
    {
        sprintf(tmp, "save.image(file='input_%d.RData')\n", loc);
        command += tmp;
    }

    //load files and packages.
    sprintf(tmp, "source('%s/auxil.r')\nsource('%s/gev.fit2.r')\nsource('%s/gevVisit.r')\n",
            codeDir.c_str(), codeDir.c_str(), codeDir.c_str());
    command += tmp;
    command += "output = gevFit(";
    command += "data=" + dataStr;

    if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::ANNUAL)
        command += ", aggregation='annual'";
    else if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::SEASONAL)
        command += ", aggregation='seasonal'";
    else if (atts.GetAggregation() == ExtremeValueAnalysisAttributes::MONTHLY)
        command += ", aggregation='monthly'";
    
    sprintf(tmp, ", nYears=%d", numYears);
    command += tmp;
    
    int nReplicates = 1;
    if (atts.GetEnsemble())
        nReplicates = atts.GetNumEnsembles();
    sprintf(tmp, ", nReplicates=%d", nReplicates);
    command += tmp;


     if (atts.GetComputeReturnValues())
     {
         command += ", nCovariates=1";

         int y0 = atts.GetDataYearBegin();
         if (atts.GetDataAnalysisYearRangeEnabled())
             y0 = atts.GetDataAnalysisYear1();
         command += ", covariates=c(";
         for (int y = 0; y < numYears; y++)
         {
             sprintf(tmp, "%d", y + y0);
             command += tmp;
             if (y < (numYears-1))
                 command += ", ";
         }
         command += ")";
        
         command += ", newData=c(";
         int n = atts.GetReturnValues().size();
         for (int i = 0; i < n; i++)
         {
             sprintf(tmp, "%d", atts.GetReturnValues()[i]);
             command += tmp;
             if (i != (n-1))
                 command += ",";
         }
         command += ")";
         if (atts.GetComputeRVDifferences())
         {
             sprintf(tmp, ", rvDifference=c(%d, %d)", atts.GetRvDifference1(), atts.GetRvDifference2());
             command += tmp;
         }
         
         if (atts.GetCovariateModelLocation())
             command += ", locationModel=1";
         if (atts.GetCovariateModelScale())
             command += ", scaleModel=1";
         if (atts.GetCovariateModelShape())
             command += ", shapeModel=1";
         
         if (atts.GetComputeParamValues())
             command += ", returnParams=TRUE";
     }

     if (atts.GetOptimizationMethod() == ExtremeValueAnalysisAttributes::NELDER_MEAD)
         command += ", optimMethod='Nelder-Mead'";
     else if (atts.GetOptimizationMethod() == ExtremeValueAnalysisAttributes::BFGS)
         command += ", optimMethod='BFGS'";

     if (atts.GetExtremeMethod() == ExtremeValueAnalysisAttributes::MAXIMA)
         command += ", maxes=TRUE";
     else if (atts.GetExtremeMethod() == ExtremeValueAnalysisAttributes::MINIMA)
         command += ", maxes=FALSE";
     
    command += ")\n";

    for (int i = 0; i < outputInfo.size(); i++)
        command += outputInfo[i].name + "=" + outputInfo[i].rName + "\n";

    if (atts.GetDumpDebug())
    {
        sprintf(tmp, "save.image(file='output_%d.RData')\n", loc);
        command += tmp;
    }

    return command;
}

void
avtRExtremesFilter::DebugData(int loc, std::string nm)
{
    /*
    char filename[128];
    sprintf(filename, "%s_r%02d_%04d.txt", nm.c_str(), PAR_Rank(), loc);
    
    ofstream ofile(filename);
    for (int b = 0; b < numBins; b++)
    {
        ofile<<"["<<b<<"] ("<<values[loc][b].size()<<") : ";
        for (int t = 0; t < values[loc][b].size(); t++)
            ofile<<values[loc][b][t].val<<" ";    
        ofile<<endl;
    }
    */
}


void
avtRExtremesFilter::SetExtremeData(int loc, vtkDoubleArray *data)
{
    data->SetNumberOfComponents(1);
    data->SetNumberOfTuples(numDataYears*numBins);

    int idx = 0;
    for (int y = 0; y < numDataYears; y++)
    {
        for (int b = 0; b < numBins; b++)
        {
            data->SetValue(idx, values[loc][y][b]);
            idx++;
        }
    }
}
