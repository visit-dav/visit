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
//  File: avtProgrammableOperation.C
// ************************************************************************* //

#include <avtProgrammableOperation.h>
#include <InstallationFunctions.h>
#include <vtkDataArray.h>

#ifdef HAVE_LIB_R
#include <vtkRInterface.h>
#endif

#include <avtPythonFilterEnvironment.h>
#include <Python.h>

#include <avtDatasetToDatasetFilter.h>
#include <avtTimeLoopFilter.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkRectilinearGrid.h>

#include <avtParallel.h>
#include <vector>
#include <map>

#ifdef PARALLEL
  #include <mpi.h>
#endif

#include <avtOriginatingSource.h>
#include <avtMetaData.h>
#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtCallback.h>
#include <avtSourceFromDatabase.h>

#include <vtkBitArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkIntArray.h>
#include <vtkLongArray.h>
#include <vtkDoubleArray.h>
#include <vtkStringArray.h>
#include <vtkCharArray.h>

#include "VisItWriteData.h"

using namespace std;

class avtTimeWindowLoopFilter : virtual public avtDatasetToDatasetFilter,
                virtual public avtTimeLoopFilter
{
    /// record which rank has what timesteps and parts of the datasets..
  public:
    avtTimeWindowLoopFilter() { index=0;}
    virtual ~avtTimeWindowLoopFilter() {}
    virtual const char* GetType() {return "avtTimeWindowLoopFilter";}

    //vector< vector<float> > values;
    vector<float> values;
    size_t index;

  protected:
    virtual void            PreLoopInitialize();
    virtual void            Execute();
    virtual void            CreateFinalOutput();
    virtual bool            ExecutionSuccessful() { return true; }

    virtual bool            FilterSupportsTimeParallelization() {
        return true;
    }
    virtual bool            DataCanBeParallelizedOverTime() { return true; }
    //virtual bool            OperationNeedsAllData(void) { return true; }

    bool handleOutputShapeAndData(PyObject* result);

    bool nodeCenteredData;
    int numTuples, idx0, idxN;
  public:
    string script;
    avtPythonFilterEnvironment *environment;
    vtkDataSet* inputDataSet;
    int inputDomain;
    vtkShapedDataArray outputdataArray;
    vtkShapedDataArray globalOutputDataArray;
};

void
avtTimeWindowLoopFilter::PreLoopInitialize()
{
    vtkDataSet *inDS = inputDataSet;

    /// Todo: verify assumption domain ids & leaves traverse the same path?
    std::vector<int> dids;
    GetInputDataTree()->GetAllDomainIds(dids);
    int numLeaves = -1;
    vtkDataSet** datasets = GetInputDataTree()->GetAllLeaves(numLeaves);

    if(numLeaves != dids.size())
        std::cout << "mismatch " << numLeaves << " " << dids.size() << std::endl;
    int domIndex = -1;
    for(int i = 0; i < dids.size(); ++i)
    {
        if(dids[i] == inputDomain)
        {
            domIndex = i;
            break;
        }
    }
    if(domIndex != -1)
        inDS = datasets[domIndex];

    nodeCenteredData = (GetInput()->GetInfo().GetAttributes().GetCentering() == AVT_NODECENT);
    /// TODO: IMPORTANT: make sure request variable is met (not just default)
    if (nodeCenteredData)
        numTuples = inDS->GetPointData()->GetScalars()->GetNumberOfTuples();
    else
        numTuples = inDS->GetCellData()->GetScalars()->GetNumberOfTuples();
    idx0 = 0;
    idxN = numTuples;
    index = 0;
    
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

    
    // values need to be number of tuples * total number of timesteps..
    /// todo: Figure out why GetTotalNumberOfTimeSlicesForRank does not work
    int totalTimes = GetEndTime() - GetStartTime() + 1; //GetTotalNumberOfTimeSlicesForRank();
    //cout<<PAR_Rank()<<": locs: ["<<idx0<<" "<<idxN<<"] times:"<<totalTimes<< " " << numTuples*totalTimes << " "
    //   << GetEndTime() << " " << GetStartTime() << endl;
    //cout<<"Is data replicated: " << GetInput()->GetInfo().GetAttributes().DataIsReplicatedOnAllProcessors()<<endl;

    //format is: T0_val0, T0_val1, ...., T1_val0, T2_val1, ....
    values.resize(numTuples*totalTimes,0);
}

void
avtTimeWindowLoopFilter::Execute()
{
    vtkDataSet *ds = inputDataSet;

    /// Todo: verify assumption domain ids & leaves traverse the same path?
    std::vector<int> dids;
    GetInputDataTree()->GetAllDomainIds(dids);
    int numLeaves = -1;
    vtkDataSet** datasets = GetInputDataTree()->GetAllLeaves(numLeaves);

    if(numLeaves != dids.size())
        std::cout << "mismatch " << numLeaves << " " << dids.size() << std::endl;
    int domIndex = -1;
    for(int i = 0; i < dids.size(); ++i)
    {
        if(dids[i] == inputDomain)
        {
            domIndex = i;
            break;
        }
    }
    if(domIndex != -1)
        ds = datasets[domIndex];

    //std::cout << inputDataSet << "  " << ds << std::endl;
    //ds = inputDataSet;
    vtkFloatArray *scalars = NULL;

    /// TODO: Important: make sure variable is appropriately casted.
    if (nodeCenteredData)
        scalars = (vtkFloatArray *)ds->GetPointData()->GetScalars();
    else
        scalars = (vtkFloatArray *)ds->GetCellData()->GetScalars();

    float *vals = (float *) scalars->GetVoidPointer(0);

//    int rank = PAR_Rank();

//    int numTimes = GetEndTime() - GetStartTime() + 1;
//    float dsTime = (float)GetInput()->GetInfo().GetAttributes().GetTime();
//    int dsCycle = (float)GetInput()->GetInfo().GetAttributes().GetCycle();

    for (size_t i = 0; i < numTuples; i++)
        values[index++] = vals[i];

    //int rank = PAR_Rank();
    //cout<<"TimeStep: " << rank << " " << currentTime<< " " << GetStartTime() << " " << GetEndTime() << " "
    //<< GetInput()->GetInfo().GetAttributes().DataIsReplicatedOnAllProcessors() << endl;
}

void
avtTimeWindowLoopFilter::CreateFinalOutput()
{
    Barrier();

    avtCallback::ResetTimeout(0);
    //cout<<PAR_Rank() << ": CreateFinalOutput : values= "<<values.size()<<endl;
    
    size_t totalTupleSize = idxN-idx0;
    int numTimes = GetEndTime() - GetStartTime() + 1;
    
    vector<float> finalVals;
#ifdef PARALLEL
    finalVals.resize((idxN-idx0)*numTimes,0);
    float *res = new float[numTimes];
    float *tmp = new float[numTimes];

    map<int,int> cycleMap;
    vector<int> myCycles = GetCyclesForRank();
    for (int i = 0; i < myCycles.size(); i++)
    cycleMap[myCycles[i]] = i;
    size_t finalIdx = 0;
    for (int i = 0; i < numTuples; i++)
    {
    for (int j = 0; j < numTimes; j++)
        res[j] = tmp[j] = 0.0f;

    for (int j = 0; j < numTimes; j++)
    {
        //If I have this timestep...
        map<int, int>:: const_iterator mi = cycleMap.find(j);
        if (mi != cycleMap.end())
        {
        int ti = mi->second;
        int idx = ti*numTuples + i;
        tmp[j] = values[idx];
        }
    }
    
    MPI_Allreduce(tmp, res, numTimes, MPI_FLOAT, MPI_SUM, VISIT_MPI_COMM);
    if (i >= idx0 && i < idxN)
        for (int j = 0; j < numTimes; j++) {
        finalVals[finalIdx++] = res[j];
        }
    }
    delete [] tmp;
    delete [] res;
#else
    //Redo the indexing.....
    finalVals = values;
    size_t idx = 0;
    for (int i = 0; i < numTuples; i++)
    for (int j = 0; j < numTimes; j++)
        finalVals[idx++] = values[j*numTuples +i];
#endif

    //std::cout << GetEndTime() << " " << GetStartTime() << std::endl;
    //std::vector<float> inputArray;

    //inputArray.resize(numTimes);
    //cout<<__FILE__<<" "<<__LINE__<<endl;

    outputdataArray.vtkarray = 0;

    PyObject *retval = PyTuple_New(numTimes);

    for (int i = 0; i < totalTupleSize; i++)
    {
        for(int j = 0; j < numTimes; ++j)
        {
            //std::cout << finalVals[j*totalTupleSize + i] << std::endl;
            //std::cout << finalVals[i*numTimes + j] << " ";
            PyObject* value = PyFloat_FromDouble((double)finalVals[i*numTimes + j]); //[j*totalTupleSize + i]);
            PyTuple_SET_ITEM(retval, j, value);
        }
        //std::cout << std::endl;

        environment->Interpreter()->SetGlobalObject(retval,"__internal_array");
        environment->Interpreter()->RunScript(script);
        PyObject* result = environment->Interpreter()->GetGlobalObject("res");
        handleOutputShapeAndData(result);
    }

    Barrier();

    /// if the same data is replicated on all processors
    /// then make sure to send your results to them..
    /// this way all ranks have the correct final output..
#ifdef PARALLEL

    if(GetInput()->GetInfo().GetAttributes().DataIsReplicatedOnAllProcessors() )
    {
        /// assumption the local data array size is the same on all processors,
        /// the correct was to ensure all of mpi rank conform..
        int multi_dim_size = 1;
        for(int i = 1; i < outputdataArray.shape.size(); ++i)
            multi_dim_size *= outputdataArray.shape[i];

        /// update globalOutputDataArray..
        globalOutputDataArray.shape = outputdataArray.shape;
        globalOutputDataArray.shape[0] = numTuples;

        /// create a one dimensional array with the multi dimensional part..
        globalOutputDataArray.vtkarray = outputdataArray.vtkarray->NewInstance();
        globalOutputDataArray.vtkarray->Allocate(numTuples*multi_dim_size);
        globalOutputDataArray.vtkarray->SetNumberOfTuples(numTuples*multi_dim_size);

        int type = outputdataArray.vtkarray->GetDataType();
        MPI_Datatype mpi_type = MPI_CHAR;

        if(type == VTK_INT) mpi_type = MPI_INTEGER;
        if(type == VTK_LONG) mpi_type = MPI_LONG;
        if(type == VTK_FLOAT) mpi_type = MPI_FLOAT;
        if(type == VTK_DOUBLE) mpi_type = MPI_DOUBLE;
        /// todo: handle strings?
//        if(type == VTK_STRING) mpi_type = MPI_STRING;

//        std::cout << "starting index: " << idx0 << " " << idxN << " " << multi_dim_size <<  " "
//                     << outputdataArray.vtkarray->GetDataSize() << " " << std::endl;

        //cout<<PAR_Rank()<<": ["<<idx0<<" "<<idxN<<"] sendCnt= "<<outputdataArray.vtkarray->GetDataSize()
        //<<" recvCnt= "<<outputdataArray.vtkarray->GetDataSize()
        //<<" globalsz= "<<globalOutputDataArray.vtkarray->GetDataSize()<<endl;
        //get output size & dimension for all processors
        std::vector<int> all_sizes;

        all_sizes.resize(PAR_Size()*outputdataArray.shape.size());
        MPI_Allgather(&outputdataArray.shape.front(),
                      outputdataArray.shape.size(),
                      MPI_INT,
                      &all_sizes.front(),
                      outputdataArray.shape.size(),
                      MPI_INT,
                      VISIT_MPI_COMM);

        std::vector<int> sizes, displs;
        int totalsize = 0;
        for(int x = 0; x < all_sizes.size(); x += outputdataArray.shape.size())
        {
            int lsize = 1;
            for(int y = 0; y < outputdataArray.shape.size(); ++y)
                lsize *= all_sizes[x+y];

            sizes.push_back(lsize);
            displs.push_back(totalsize);
            totalsize += lsize;
        }

//        if(PAR_Rank() == 0)
//        {
//            if(PAR_Size() != sizes.size())
//                std::cout << "PANIC" << std::endl;
//            for(int x = 0; x < PAR_Size(); ++x)
//            {
//                std::cout << sizes[x] << " " << displs[x] << std::endl;
//            }
//        }

        MPI_Allgatherv(outputdataArray.vtkarray->GetVoidPointer(0),
                       outputdataArray.vtkarray->GetDataSize(),
                       mpi_type,
                       globalOutputDataArray.vtkarray->GetVoidPointer(0),
                       &sizes.front(),
                       &displs.front(),
                       mpi_type,
                       VISIT_MPI_COMM);

        /// global array should have all appropriate values now..
        /// clear up local space..
        outputdataArray.shape = intVector();
        outputdataArray.vtkarray->Delete();
        outputdataArray.vtkarray = 0;
    }
#else
    /// just copy..
    globalOutputDataArray = outputdataArray;
#endif
    avtCallback::ResetTimeout(0);
    /// cleanup local
    SetOutputDataTree(new avtDataTree(inputDataSet,inputDomain));
    //cout<<PAR_Rank()<<": DONE."<<endl;
}

bool
avtTimeWindowLoopFilter::handleOutputShapeAndData(PyObject* obj)
{
    if(PyTuple_Check(obj))
    {
        size_t size = PyTuple_Size(obj);
        /// make sure the new size and first size matches..
        /// TODO: better error checking..
        /// Convert this logic to construct multi dimensional shape array..
        if(!outputdataArray.vtkarray)
        {
            /// figure out output type from object..
            outputdataArray.shape.push_back(0);
            outputdataArray.shape.push_back(size);
        }
        else
        {
            if(size != outputdataArray.shape[1])
            {
                std::cerr << "shape sizes do not match, this operation does "
                          << "not support varying results from kernels"
                          << size << " " << outputdataArray.shape[1]
                          << std::endl;
                /// error
                return false;
            }
        }

        outputdataArray.shape[0] += 1;

        for(size_t i = 0; i < size; ++i)
        {
            PyObject* retobj = PyTuple_GetItem(obj,i);

            if (PyBool_Check(retobj))
            {
                if(!outputdataArray.vtkarray)
                    outputdataArray.vtkarray = vtkUnsignedCharArray::New();
                vtkUnsignedCharArray::SafeDownCast(outputdataArray.vtkarray)
                        ->InsertNextValue((retobj == Py_False ? 0: 1 ));
            }
            else if(PyInt_Check(retobj))
            {
                if(!outputdataArray.vtkarray)
                    outputdataArray.vtkarray = vtkIntArray::New();
                vtkIntArray::SafeDownCast(outputdataArray.vtkarray)
                        ->InsertNextValue((int)PyInt_AsLong(retobj));
            }
            else if(PyLong_Check(retobj))
            {
                if(!outputdataArray.vtkarray)
                    outputdataArray.vtkarray = vtkLongArray::New();
                vtkLongArray::SafeDownCast(outputdataArray.vtkarray)
                        ->InsertNextValue((long)PyLong_AsLong(retobj));
            }
            else if(PyFloat_Check(retobj))
            {
                if(!outputdataArray.vtkarray)
                    outputdataArray.vtkarray = vtkDoubleArray::New();
                vtkDoubleArray::SafeDownCast(outputdataArray.vtkarray)
                        ->InsertNextValue((double)PyFloat_AsDouble(retobj));
            }
            else if(PyString_Check(retobj))
            {
                if(!outputdataArray.vtkarray)
                    outputdataArray.vtkarray = vtkStringArray::New();
                vtkStringArray::SafeDownCast(outputdataArray.vtkarray)
                        ->InsertNextValue((const char*)PyString_AsString(retobj));
            }
            else
            {
                std::cerr << "Something else.." << std::endl;
            }
        }

    }
    else if(PyList_Check(obj))
    {
        size_t size = PyList_Size(obj);
        /// make sure the new size and first size matches..
        /// TODO: better error checking..
        /// Convert this logic to construct multi dimensional shape array..
        if(!outputdataArray.vtkarray)
        {
            /// figure out output type from object..
            outputdataArray.shape.push_back(0);
            outputdataArray.shape.push_back(size);
        }
        else
        {
            if(size != outputdataArray.shape[1])
            {
                std::cerr << "shape sizes do not match, this operation does "
                          << "not support varying results from kernels: "
                          << size << " " << outputdataArray.shape[1]
                          << std::endl;
                /// error
                return false;
            }
        }
        outputdataArray.shape[0] += 1;
        for(size_t i = 0; i < size; ++i)
        {
            PyObject* retobj = PyList_GetItem(obj,i);

            if (PyBool_Check(retobj))
            {
                if(!outputdataArray.vtkarray)
                    outputdataArray.vtkarray = vtkUnsignedCharArray::New();
                vtkUnsignedCharArray::SafeDownCast(outputdataArray.vtkarray)
                        ->InsertNextValue((retobj == Py_False ? 0: 1 ));
            }
            else if(PyInt_Check(retobj))
            {
                if(!outputdataArray.vtkarray)
                    outputdataArray.vtkarray = vtkIntArray::New();
                vtkIntArray::SafeDownCast(outputdataArray.vtkarray)
                        ->InsertNextValue((int)PyInt_AsLong(retobj));
            }
            else if(PyLong_Check(retobj))
            {
                if(!outputdataArray.vtkarray)
                    outputdataArray.vtkarray = vtkLongArray::New();
                vtkLongArray::SafeDownCast(outputdataArray.vtkarray)
                        ->InsertNextValue((long)PyLong_AsLong(retobj));
            }
            else if(PyFloat_Check(retobj))
            {
                if(!outputdataArray.vtkarray)
                    outputdataArray.vtkarray = vtkDoubleArray::New();
                vtkDoubleArray::SafeDownCast(outputdataArray.vtkarray)
                        ->InsertNextValue((double)PyFloat_AsDouble(retobj));
            }
            else if(PyString_Check(retobj))
            {
                if(!outputdataArray.vtkarray)
                    outputdataArray.vtkarray = vtkStringArray::New();
                vtkStringArray::SafeDownCast(outputdataArray.vtkarray)
                        ->InsertNextValue((const char*)PyString_AsString(retobj));
            }
            else
            {
                std::cerr << "Something else.." << std::endl;
            }
        }
    }
    else
    {
        if(!outputdataArray.vtkarray)
            outputdataArray.shape.push_back(0);

        outputdataArray.shape[0] += 1;

        if (PyBool_Check(obj))
        {
            if(!outputdataArray.vtkarray)
                outputdataArray.vtkarray = vtkUnsignedCharArray::New();
            vtkUnsignedCharArray::SafeDownCast(outputdataArray.vtkarray)
                    ->InsertNextValue((obj == Py_False ? 0: 1 ));
        }
        else if(PyInt_Check(obj))
        {
            if(!outputdataArray.vtkarray)
                outputdataArray.vtkarray = vtkIntArray::New();
            vtkIntArray::SafeDownCast(outputdataArray.vtkarray)
                    ->InsertNextValue((int)PyInt_AsLong(obj));
        }
        else if(PyLong_Check(obj))
        {
            if(!outputdataArray.vtkarray)
                outputdataArray.vtkarray = vtkLongArray::New();
            vtkLongArray::SafeDownCast(outputdataArray.vtkarray)
                    ->InsertNextValue((long)PyLong_AsLong(obj));
        }
        else if(PyFloat_Check(obj))
        {
            if(!outputdataArray.vtkarray)
                outputdataArray.vtkarray = vtkDoubleArray::New();
            vtkDoubleArray::SafeDownCast(outputdataArray.vtkarray)
                    ->InsertNextValue((double)PyFloat_AsDouble(obj));
        }
        else if(PyString_Check(obj))
        {
            if(!outputdataArray.vtkarray)
                outputdataArray.vtkarray = vtkStringArray::New();
            vtkStringArray::SafeDownCast(outputdataArray.vtkarray)
                    ->InsertNextValue((const char*)PyString_AsString(obj));
        }
        else
        {
            std::cerr << "Something else.." << std::endl;
        }
    }


    return true;
}


avtProgrammableOperation::avtProgrammableOperation()
{}

bool
avtProgrammableOperation::avtVisItForEachLocation::func(ProgrammableOpArguments& args, vtkShapedDataArray& result)
{
    Variant windowArray = args.getArg(0);
    vtkShapedDataArray var = args.getArgAsShapedDataArray(1);
    Variant kernelLanguage = args.getArg(2); //R or Python
    Variant kernel = args.getArg(3); //kernel itself
    Variant kernelName = args.getArg(4); //name of the function to call..
    Variant primaryVariable = args.getArg(5); // primary variable name to modify..

    std::vector<Variant> kernelArgs = args.getArgAsVariantVector(6);

    avtPythonFilterEnvironment* environ = args.GetPythonEnvironment();

    /// register the kernels..
    if(kernelLanguage == "Python")
    {
        environ->Interpreter()->RunScript(kernel.AsString());
    }
    else
    {
#ifdef HAVE_LIB_R
        std::ostringstream rsetup;

        rsetup << "import rpy2, rpy2.robjects\n"
               << kernelName.AsString() <<  " = rpy2.robjects.r(\"\"\"\n"
               << kernel.AsString() << "\n"
               << "\"\"\")\n";
        environ->Interpreter()->RunScript(rsetup.str());
#else
        std::cerr << "R is not supported at this time.." << std::endl;
        return false;
#endif
    }

    std::string arglist = "";

    for(int i = 0; i < kernelArgs.size(); ++i)
        arglist += kernelArgs[i].ConvertToString() + (i == kernelArgs.size()-1 ? "" : ",");

    std::ostringstream resultKernel;
    resultKernel << "res = " << kernelName.AsString() <<  "(numpy.asarray(__internal_array)";

    if(arglist.size() > 0)
        resultKernel << "," << arglist << ")\n";
    else
        resultKernel << ")\n";

    resultKernel << "res = numpy.asarray(res).tolist()\n";


    /// std::cout << resultKernel.str() << std::endl;
    /// run the time loop filter..

    //std::cout << "disabling timer" << std::endl;
    avtCallback::ResetTimeout(0);

    avtTimeWindowLoopFilter *filt = new avtTimeWindowLoopFilter;
    filt->environment = environ;
    filt->inputDataSet = args.GetInputDataSet();
    filt->inputDomain = args.GetInputDomain();
    filt->script = resultKernel.str();

    filt->SetInput(args.GetInput());
    avtDataObject_p dob = filt->GetOutput();

    dob->Update(args.GetContract());

    avtCallback::ResetTimeout(5*60);
    //std::cout << "enabling timer" << std::endl;

    std::cout << PAR_Rank() << " finishing... " << std::endl;
    std::cout << var.vtkarray->GetDataSize() << " "
              << filt->globalOutputDataArray.vtkarray->GetDataSize() << " "
              << filt->globalOutputDataArray.shape[0] << " "
              << filt->globalOutputDataArray.shape[1] << std::endl;

    result = filt->globalOutputDataArray;

    //result.shape.push_back(var->GetDataSize());
    //result.vtkarray = var->NewInstance();
    //result.vtkarray->DeepCopy(var);

    return true;
}

ProgrammableOperation::ResponseType
avtProgrammableOperation::avtVisItForEachLocation::getSignature(std::string& name,
                          stringVector& argnames,
                          std::vector<ScriptType>& argtypes)
{
    name = "visit_foreach_location";
    argnames.push_back("window");
    argtypes.push_back(ProgrammableOperation::INT_VECTOR_TYPE);

    argnames.push_back("variableName");
    argtypes.push_back(ProgrammableOperation::VTK_DATA_ARRAY_TYPE);

    argnames.push_back("kernelLanguage");
    argtypes.push_back(ProgrammableOperation::STRING_TYPE);

    argnames.push_back("kernel");
    argtypes.push_back(ProgrammableOperation::STRING_TYPE);

    argnames.push_back("kernelName");
    argtypes.push_back(ProgrammableOperation::STRING_TYPE);

    argnames.push_back("primaryVariable");
    argtypes.push_back(ProgrammableOperation::STRING_TYPE);

    argnames.push_back("kernelArgs");
    argtypes.push_back(ProgrammableOperation::VARIANT_VECTOR_TYPE);
    return ProgrammableOperation::VTK_MULTI_DIMENSIONAL_DATA_ARRAY;
}

#ifdef HAVE_LIB_R
bool
avtProgrammableOperation::avtVisItForEachLocationR::func(ProgrammableOpArguments& args, vtkShapedDataArray &result)
{
    Variant windowArray = args.getArg(0);
    vtkShapedDataArray var = args.getArgAsShapedDataArray(1);
    Variant kernelName = args.getArg(2); //name of the function to call..
    Variant primaryVariable = args.getArg(3); // primary variable name to modify..

    std::vector<Variant> kernelArgs = args.getArgAsVariantVector(4);

    avtPythonFilterEnvironment* environ = args.GetPythonEnvironment();

    std::ostringstream rsetup;
    rsetup << "import rpy2, rpy2.robjects\n"
           << kernelName.AsString() <<  " = rpy2.robjects.r('" << kernelName.AsString() << "')\n";
    environ->Interpreter()->RunScript(rsetup.str());

    std::string arglist = "";

    for(int i = 0; i < kernelArgs.size(); ++i)
        arglist += kernelArgs[i].ConvertToString() + (i == kernelArgs.size()-1 ? "" : ",");

    std::ostringstream resultKernel;
    resultKernel << "res = " << kernelName.AsString() <<  "(numpy.asarray(__internal_array)";

    if(arglist.size() > 0)
        resultKernel << "," << arglist << ")\n";
    else
        resultKernel << ")\n";

    resultKernel << "res = numpy.asarray(res).tolist()\n";

    /// run the time loop filter..

    //std::cout << resultKernel << std::endl;
    avtTimeWindowLoopFilter *filt = new avtTimeWindowLoopFilter;
    filt->environment = args.GetPythonEnvironment();
    filt->inputDataSet = args.GetInputDataSet();
    filt->inputDomain = args.GetInputDomain();
    filt->script = resultKernel.str();

    filt->SetInput(args.GetInput());
    avtDataObject_p dob = filt->GetOutput();

    dob->Update(args.GetContract());

    result = filt->globalOutputDataArray;

//    result.shape = var.shape;
//    result.vtkarray = var.vtkarray->NewInstance();
//    result.vtkarray->DeepCopy(var.vtkarray);

    return true;
}

ProgrammableOperation::ResponseType
avtProgrammableOperation::avtVisItForEachLocationR::getSignature(std::string& name,
                          stringVector& argnames,
                          std::vector<ScriptType>& argtypes)
{
    name = "visit_foreach_location_r";
    argnames.push_back("window");
    argtypes.push_back(ProgrammableOperation::INT_VECTOR_TYPE);

    argnames.push_back("variableName");
    argtypes.push_back(ProgrammableOperation::VTK_DATA_ARRAY_TYPE);

    argnames.push_back("kernelName");
    argtypes.push_back(ProgrammableOperation::STRING_TYPE);

    argnames.push_back("primaryVariable");
    argtypes.push_back(ProgrammableOperation::STRING_TYPE);

    argnames.push_back("kernelArgs");
    argtypes.push_back(ProgrammableOperation::VARIANT_VECTOR_TYPE);
    return ProgrammableOperation::VTK_MULTI_DIMENSIONAL_DATA_ARRAY;
}

bool
avtProgrammableOperation::avtVisItGetRSupportDirectory::func(ProgrammableOpArguments& args, Variant& result)
{
    std::string vlibdir = GetVisItLibraryDirectory() + VISIT_SLASH_CHAR + "r_support";
    std::string vlibrdir  = vlibdir  + VISIT_SLASH_CHAR + "Rscripts" + VISIT_SLASH_CHAR;
    result = vlibrdir;
    return true;
}

ProgrammableOperation::ResponseType
avtProgrammableOperation::avtVisItGetRSupportDirectory::getSignature(std::string& name,
                          stringVector& argnames,
                          std::vector<ProgrammableOperation::ScriptType>& argtypes)
{
    (void) argnames;
    (void) argtypes;

    name = "visit_get_r_support_dir";

    return ProgrammableOperation::CONSTANT;
}

#endif

bool
avtProgrammableOperation::avtVisItForEachLocationPython::func(ProgrammableOpArguments& args, vtkShapedDataArray& result)
{
    Variant windowArray = args.getArg(0);
    vtkShapedDataArray var = args.getArgAsShapedDataArray(1);
    Variant kernelName = args.getArg(2); //name of the function to call..
    Variant primaryVariable = args.getArg(3); // primary variable name to modify..

    std::vector<Variant> kernelArgs = args.getArgAsVariantVector(4);

    std::string arglist = "";

    for(int i = 0; i < kernelArgs.size(); ++i)
        arglist += kernelArgs[i].ConvertToString() + (i == kernelArgs.size()-1 ? "" : ",");

    std::ostringstream resultKernel;
    resultKernel << "res = " << kernelName.AsString() <<  "(numpy.asarray(__internal_array)";

    if(arglist.size() > 0)
        resultKernel << "," << arglist << ")\n";
    else
        resultKernel << ")\n";

    resultKernel << "res = numpy.asarray(res).tolist()\n";

    //std::cout << resultKernel.str() << std::endl;
    /// run the time loop filter..

    avtTimeWindowLoopFilter *filt = new avtTimeWindowLoopFilter;
    filt->environment = args.GetPythonEnvironment();
    filt->inputDataSet = args.GetInputDataSet();
    filt->inputDomain = args.GetInputDomain();
    filt->script = resultKernel.str();

    filt->SetInput(args.GetInput());
    avtDataObject_p dob = filt->GetOutput();
    dob->Update(args.GetContract());

    result = filt->globalOutputDataArray;

    //result.shape.push_back(var->GetDataSize());
    //result.vtkarray = var->NewInstance();
    //result.vtkarray->DeepCopy(var);

    return true;
}

ProgrammableOperation::ResponseType
avtProgrammableOperation::avtVisItForEachLocationPython::getSignature(std::string& name,
                          stringVector& argnames,
                          std::vector<ScriptType>& argtypes)
{
    name = "visit_foreach_location_python";
    argnames.push_back("window");
    argtypes.push_back(ProgrammableOperation::INT_VECTOR_TYPE);

    argnames.push_back("variableName");
    argtypes.push_back(ProgrammableOperation::VTK_DATA_ARRAY_TYPE);

    argnames.push_back("kernelName");
    argtypes.push_back(ProgrammableOperation::STRING_TYPE);

    argnames.push_back("primaryVariable");
    argtypes.push_back(ProgrammableOperation::STRING_TYPE);

    argnames.push_back("kernelArgs");
    argtypes.push_back(ProgrammableOperation::VARIANT_VECTOR_TYPE);
    return ProgrammableOperation::VTK_MULTI_DIMENSIONAL_DATA_ARRAY;
}

bool
avtProgrammableOperation::avtVisItWriteData::func(ProgrammableOpArguments& args,
                                            Variant &result)
{
    std::string filename = args.getArg(0).AsString();
    std::string format = args.getArg(1).AsString();
    bool local = args.getArg(2).AsString() == "local";
    stringVector dimNames = args.getArg(3).AsStringVector();
    doubleVector dimX = args.getArg(4).AsDoubleVector();
    doubleVector dimY = args.getArg(5).AsDoubleVector();
    doubleVector dimZ = args.getArg(6).AsDoubleVector();
    vtkShapedDataArray var = args.getArgAsShapedDataArray(7);
    stringVector varnames = args.getArg(8).AsStringVector();
    intVector indices = args.getArg(9).AsIntVector();

    std::vector<std::vector<double> > dimensions;
    int nDims = dimNames.size();
    dimensions.resize(nDims);
    if (nDims > 0)
    {
    dimensions[0].resize(dimX.size());
    for (int i = 0; i < dimX.size(); i++)
        dimensions[0][i] = dimX[i];
    }
    if (nDims > 1)
    {
    dimensions[1].resize(dimY.size());
    for (int i = 0; i < dimY.size(); i++)
        dimensions[1][i] = dimY[i];
    }
    if (nDims > 2)
    {
    dimensions[2].resize(dimZ.size());
    for (int i = 0; i < dimZ.size(); i++)
        dimensions[2][i] = dimZ[i];
    }

    std::cout << "write out file as: "<<filename;
    std::cout<<" dims= [";
    for (int i = 0; i < dimNames.size(); i++)
    std::cout<<dimNames[i]<<":"<<dimensions[i].size()<<" ";
    std::cout<<"] ";
    std::cout<<" vars,index = [";
    for (int i = 0; i < varnames.size(); i++)
    std::cout<<varnames[i]<<","<<indices[i]<<" ";
    std::cout<<"] ";
    std::cout<<" format= "<<format<<std::endl;

    /// this data has all of the information already..
    /// write it out..
    if(args.GetInput()->GetInfo().GetAttributes().DataIsReplicatedOnAllProcessors())
    {
        //VisItWriteData::write_data(filename,varname,var.vtkarray);
        VisItWriteData::write_data(filename, dimNames, dimensions, varnames, indices, var.shape, var.vtkarray);
    }

    result = true;
    return true;
}

ProgrammableOperation::ResponseType
avtProgrammableOperation::avtVisItWriteData::getSignature(std::string& name,
                          stringVector& argnames,
                          std::vector<ProgrammableOperation::ScriptType>& argtypes)
{
    name = "visit_write";

    argnames.push_back("filename");
    argtypes.push_back(ProgrammableOperation::STRING_TYPE);

    argnames.push_back("format");
    argtypes.push_back(ProgrammableOperation::STRING_TYPE);

    argnames.push_back("local_or_global");
    argtypes.push_back(ProgrammableOperation::STRING_TYPE);

    argnames.push_back("dimension_names");
    argtypes.push_back(ProgrammableOperation::STRING_VECTOR_TYPE);

    argnames.push_back("dimension_X");
    argtypes.push_back(ProgrammableOperation::DOUBLE_VECTOR_TYPE);
    argnames.push_back("dimension_Y");
    argtypes.push_back(ProgrammableOperation::DOUBLE_VECTOR_TYPE);
    argnames.push_back("dimension_Z");
    argtypes.push_back(ProgrammableOperation::DOUBLE_VECTOR_TYPE);


    argnames.push_back("variable");
    argtypes.push_back(ProgrammableOperation::VTK_DATA_ARRAY_TYPE);

    argnames.push_back("varnames");
    argtypes.push_back(ProgrammableOperation::STRING_VECTOR_TYPE);

    argnames.push_back("indices");
    argtypes.push_back(ProgrammableOperation::INT_VECTOR_TYPE);

    return ProgrammableOperation::CONSTANT;
}

bool
avtProgrammableOperation::avtVisItMaxAcrossTime::func(ProgrammableOpArguments& args, vtkShapedDataArray &result)
{
    cout<<"avtProgrammableOperation::avtVisItMaxAcrossTime::func()"<<endl;
    cout<<"args= "<<args.getArgSize()<<endl;
    for (int i = 0; i < args.getArgSize(); i++)
    cout<<i<<": "<<endl;
    
    vtkShapedDataArray var = args.getArgAsShapedDataArray(0);
    //var->Print(cout);
    
    result.shape = var.shape;
    result.vtkarray = var.vtkarray->NewInstance();
    result.vtkarray->DeepCopy(var.vtkarray);
    return true;
}

ProgrammableOperation::ResponseType
avtProgrammableOperation::avtVisItMaxAcrossTime::getSignature(std::string& name,
                    stringVector& argnames,
                        std::vector<ProgrammableOperation::ScriptType>& argtypes)
{
    name = "visit_max_across_time";

    argnames.push_back("variable");
    argtypes.push_back(ProgrammableOperation::VTK_DATA_ARRAY_TYPE);
    
    return ProgrammableOperation::VTK_DATA_ARRAY;
}

/// we are always returning true unless the script
/// itself is failing not the inquiry..
bool
avtProgrammableOperation::avtVisItGetVarInfo::func(ProgrammableOpArguments& args, Variant& result)
{
    std::string varName = args.getArg(0).AsString();
    vtkDataSet* dataset = args.GetInputDataSet();
    bool pointData = true;
    vtkDataArray* array = dataset->GetPointData()->GetScalars(varName.c_str());

    if(!array) {
        array = dataset->GetCellData()->GetScalars(varName.c_str());
        pointData = false;
    }

    /// for now just deal with scalars..
    if(array == NULL)
    {
        result = "";
        return true;
    }

    /// now extract information from the array..
    /// through in mesh dimensions to help inquiring class reshape
    /// information correctly..
    JSONNode resultNode;
    resultNode["type"] = pointData ? "pointdata" : "celldata";
    JSONNode::JSONArray dims(3,-1);
    resultNode["dims"] = dims;

    result = resultNode.ToString();

    return true;
}

ProgrammableOperation::ResponseType
avtProgrammableOperation::avtVisItGetVarInfo::getSignature(std::string& name,
                          stringVector& argnames,
                          std::vector<ScriptType>& argtypes)
{
    name = "visit_get_var_info";

    argnames.push_back("variableName");
    argtypes.push_back(ProgrammableOperation::STRING_TYPE);

    return ProgrammableOperation::CONSTANT;
}


bool
avtProgrammableOperation::avtProgrammableOperation::avtVisItGetMeshInfo::func(ProgrammableOpArguments& args, Variant& result)
{
    vtkDataSet *dataset = args.GetInputDataSet();

    vtkRectilinearGrid *rg = vtkRectilinearGrid::SafeDownCast(dataset);
    if (dataset->GetDataObjectType() != VTK_RECTILINEAR_GRID || rg == NULL)
    {
    result = "";
    return true;
    }
    
    JSONNode resultNode;
    resultNode["type"] = "rectilinear_grid";
    JSONNode::JSONArray dims(3, -1);
    int numX = rg->GetDimensions()[0];
    int numY = rg->GetDimensions()[1];
    int numZ = rg->GetDimensions()[2];

    dims[0] = numX;
    dims[1] = numY;
    dims[2] = numZ;

    vtkDataArray *x = rg->GetXCoordinates();
    vtkDataArray *y = rg->GetYCoordinates();
    vtkDataArray *z = rg->GetZCoordinates();

    JSONNode::JSONArray xc(numX, -1);
    JSONNode::JSONArray yc(numY, -1);
    JSONNode::JSONArray zc(numZ, -1);

    for (int i = 0; i < numX; i++)
    xc[i] = x->GetTuple1(i);
    for (int i = 0; i < numY; i++)
    yc[i] = y->GetTuple1(i);
    for (int i = 0; i < numZ; i++)
    zc[i] = z->GetTuple1(i);
    
    resultNode["dims"] = dims;
    resultNode["x_coords"] = xc;
    resultNode["y_coords"] = yc;
    resultNode["z_coords"] = zc;
    result = resultNode.ToString();
    return true;
}

ProgrammableOperation::ResponseType
avtProgrammableOperation::avtProgrammableOperation::avtVisItGetMeshInfo::getSignature(std::string& name,
                          stringVector& argnames,
                          std::vector<ScriptType>& argtypes)
{
    name = "visit_get_mesh_info";

    return ProgrammableOperation::CONSTANT;
}

void
avtProgrammableOperation::RegisterOperations(ProgrammableOpManager *manager)
{
    manager->Register(&vfel);
    manager->Register(&vfelp);
    manager->Register(&vfef);
    manager->Register(&vgvi);
    manager->Register(&vgmi);
    manager->Register(&vmax);
    manager->Register(&avwd);

#ifdef HAVE_LIB_R
   manager->Register(&vfelr);
   manager->Register(&avag);
#endif
}
