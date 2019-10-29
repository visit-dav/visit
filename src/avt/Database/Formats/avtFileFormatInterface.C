// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtFileFormatInterface.C                         //
// ************************************************************************* //

#include <avtFileFormatInterface.h>

#include <avtFileFormat.h>
#include <DebugStream.h>
#include <ImproperUseException.h>

#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkDataArray.h>

#include <QueryOverTimeAttributes.h>


using     std::vector;


avtFileFormatInterface::avtFileFormatInterface()
{
    isEnsemble = false;
}


// ****************************************************************************
//  Method: avtFileFormatInterface destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtFileFormatInterface::~avtFileFormatInterface()
{
    ;
}


// ****************************************************************************
//  Method: avtFileFormatInterface::SetCache
//
//  Purpose:
//      Sets the cache for each file format.
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************

void
avtFileFormatInterface::SetCache(avtVariableCache *cache)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        avtFileFormat *ff = GetFormat(i);
        ff->SetCache(cache);
    }
}


// ****************************************************************************
//  Method: avtFileFormatInterface::RegisterVariableList
//
//  Purpose:
//      Registers a variable list with each file format.
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************

void
avtFileFormatInterface::RegisterVariableList(const char *var,
                                             const vector<CharStrRef> &vars2nd)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        avtFileFormat *ff = GetFormat(i);
        ff->RegisterVariableList(var, vars2nd);
    }
}


// ****************************************************************************
//  Method: avtFileFormatInterface::RegisterDataSelections
//
//  Purpose:
//      Registers data selections with each file format and a vector of bools
//      in which format can indicate whether a given selection was indeed
//      applied. This is typically used
//      just prior to avtGenericDatabase' attempt to get data from a format
//      to give the format an opportunity to handle the selection on read.
//
//  Programmer: Mark C. Miller 
//  Creation:   September 22, 2004 
//
// ****************************************************************************

void
avtFileFormatInterface::RegisterDataSelections(
    const vector<avtDataSelection_p> &selList,
    vector<bool> *wasApplied) 
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        avtFileFormat *ff = GetFormat(i);
        ff->RegisterDataSelections(selList, wasApplied);
    }
}

// ****************************************************************************
//  Method: avtFileFormatInterface::GetType
//
//  Purpose:
//      Gets the type of file format.
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************

const char *
avtFileFormatInterface::GetType(void)
{
    int nFormats = GetNumberOfFileFormats();
    if (nFormats > 0)
    {
        avtFileFormat *ff = GetFormat(0);
        return ff->GetType();
    }

    return "Unknown Format";
}


// ****************************************************************************
//  Method: avtFileFormatInterface::HasVarsDefinedOnSubMeshes
//
//  Purpose:
//      Declares whether or not some variables are defined on sub-meshes
//      only.  This can affect caching strategies.
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************

bool
avtFileFormatInterface::HasVarsDefinedOnSubMeshes(void)
{
    int nFormats = GetNumberOfFileFormats();
    if (nFormats > 0)
    {
        avtFileFormat *ff = GetFormat(0);
        return ff->HasVarsDefinedOnSubMeshes();
    }

    return false;
}


// ****************************************************************************
//  Method: avtFileFormatInterface::PerformsMaterialSelection
//
//  Purpose:
//      Determines whether or not the underlying file format wants to do its
//      own material selection.
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************
 
bool
avtFileFormatInterface::PerformsMaterialSelection(void)
{
    int nFormats = GetNumberOfFileFormats();
    if (nFormats > 0)
    {
        avtFileFormat *ff = GetFormat(0);
        return ff->PerformsMaterialSelection();
    }

    return false;
}


// ****************************************************************************
//  Method: avtFileFormatInterface::TurnMaterialSelectionOff
//
//  Purpose:
//      Tells the file format that it should not do material selection.   
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************

void
avtFileFormatInterface::TurnMaterialSelectionOff(void)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        avtFileFormat *ff = GetFormat(i);
        ff->TurnMaterialSelectionOff();
    }
}


// ****************************************************************************
//  Method: avtFileFormatInterface::TurnMaterialSelectionOn
//
//  Purpose:
//      Tells the file format that it should do material selection.   
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
// ****************************************************************************

void
avtFileFormatInterface::TurnMaterialSelectionOn(const char *matname)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        avtFileFormat *ff = GetFormat(i);
        ff->TurnMaterialSelectionOn(matname);
    }
}


// ****************************************************************************
//  Method: avtFileFormatInterface::CanDoStreaming
//
//  Purpose:
//      Indicates if the file format supports streaming.   
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2003
//
//  Modifications:
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
// ****************************************************************************

bool
avtFileFormatInterface::CanDoStreaming(void)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        avtFileFormat *ff = GetFormat(i);
        if (! ff->CanDoStreaming())
        {
            return false;
        }
    }

    return true;
}


// ****************************************************************************
//  Method: avtFileFormatInterface::DoingStreaming
//
//  Purpose:
//      Tells the file format whether or not we are streaming.
//
//  Programmer: Hank Childs
//  Creation:   December 22, 2010
//
// ****************************************************************************

void
avtFileFormatInterface::DoingStreaming(bool v)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
        GetFormat(i)->DoingStreaming(v);
}


// ****************************************************************************
//  Method: avtFileFormatInterface::HasInvariantMetaData
//
//  Purpose:
//      Indicates if the file format can have metadata that varies with time 
//
//  Programmer: Mark C. Miller
//  Creation:   September 30, 2003
//
// ****************************************************************************

bool
avtFileFormatInterface::HasInvariantMetaData(void)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        if (! GetFormat(i)->HasInvariantMetaData())
            return false;
    }

    return true;
}

// ****************************************************************************
//  Method: avtFileFormatInterface::HasInvariantSIL
//
//  Purpose:
//      Indicates if the file format can have a SIL that varies with time 
//
//  Programmer: Mark C. Miller
//  Creation:   September 30, 2003
//
// ****************************************************************************

bool
avtFileFormatInterface::HasInvariantSIL(void)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        if (! GetFormat(i)->HasInvariantSIL())
            return false;
    }

    return true;
}

// ****************************************************************************
//  Method: avtFileFormatInterface::CanCacheVariable
//
//  Purpose:
//      Indicates if the file format can cache the given named variable 
//
//  Programmer: Mark C. Miller
//  Creation:   December 10, 2003
//
// ****************************************************************************

bool
avtFileFormatInterface::CanCacheVariable(const char *var_name)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        if (! GetFormat(i)->CanCacheVariable(var_name))
            return false;
    }

    return true;
}


// ****************************************************************************
//  Method: avtFileFormatInterface::SetResultMustBeProducedOnlyOnThisProcessor
//
//  Purpose:
//      This method is for telling formats that do their own domain
//      decomposition whether or not they can count on other processors to 
//      help them produce results.  In the case of integral curves that produce
//      point selections, the file format reader will *not* be able to count
//      on the other processors, although it can in all other cases (at the
//      time of the writing of this comment).
//
//  Programmer: Hank Childs
//  Creation:   April 3, 2009
//
// ****************************************************************************

void
avtFileFormatInterface::SetResultMustBeProducedOnlyOnThisProcessor(bool b)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        GetFormat(i)->SetResultMustBeProducedOnlyOnThisProcessor(b);
    }
}


// ****************************************************************************
//  Method: avtFileFormatInterface::SetStrictMode
//
//  Purpose:
//      Tells the file format whether it should be strict about returning
//      errors on files that may actually be of this type of format.
//
//  Programmer: Jeremy Meredith
//  Creation:   January  8, 2010
//
// ****************************************************************************

void
avtFileFormatInterface::SetStrictMode(bool strictMode)
{
    int nFormats = GetNumberOfFileFormats();
    for (int i = 0 ; i < nFormats ; i++)
    {
        avtFileFormat *ff = GetFormat(i);
        ff->SetStrictMode(strictMode);
    }
}


// ****************************************************************************
//  Method: avtFileFormatInterface::GetCycles
//
//  Purpose:
//      Retrieve all available cycles (to be implemented by children).
//
//  Arguments:
//      unnamed int      Domain. 
//      unnamed vector   Vector to store cycles. 
//
//  Programmer: Alister Maguire
//  Creation:   Tue Sep  3 13:46:43 MST 2019 
//
// ****************************************************************************

void
avtFileFormatInterface::GetCycles(int, intVector &)
{
    debug2 << "Trying to retrieve cycles, but it's not implemented..." << endl;
}


// ****************************************************************************
//  Method: avtFileFormatInterface::GetTimes
//
//  Purpose:
//      Retrieve all available times (to be implemented by children).
//
//  Arguments:
//      unnamed int      Domain. 
//      unnamed vector   Vector to store times. 
//
//  Programmer: Alister Maguire
//  Creation:   Tue Sep  3 13:46:43 MST 2019 
//
// ****************************************************************************

void
avtFileFormatInterface::GetTimes(int, doubleVector &)
{
    debug2 << "Trying to retrieve times, but it's not implemented..." << endl;
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::GetQOTMesh
//
//  Purpose:
//      Retrieve a query over time mesh. Currently, this is a reduced 
//      point mesh such that each point is located at position (x, 0, 0),
//      where x is the timestep, simulation time, or cycle (whichever was
//      requested).
//
//      All of the mesh's future arrays should be the same size (number of
//      requested timesteps), and each will be a variable/element pair
//      through time. The value located at position 'i' of every array
//      should correspond to the value of the array's variable/element 
//      pair associated with the timestep/time/cycle from the x value
//      of the 'ith' point. 
//
//  Arguments:
//      QOTAtts    The query over time attributes. 
//      domain     The domain to query. 
//      tsRange    The timestep range. 
//      tsStride   The timestep stride. 
//
//  Returns:
//      A query over time mesh. 
//
//  Programmer:  Alister Maguire
//  Creation:    Tue Sep 24 11:15:10 MST 2019 
//
//  Modifications
//
// ****************************************************************************

vtkDataSet *
avtFileFormatInterface::GetQOTMesh(const QueryOverTimeAttributes *QOTAtts,
                                   int domain,
                                   int *tsRange,
                                   int tsStride)
{
    QueryOverTimeAttributes::TimeType tType = QOTAtts->GetTimeType();

    int startT = tsRange[0];
    int stopT  = tsRange[1];

    //
    // We want to always include the last time step. In cases where this
    // doesn't occur naturally, we need to manually add it. 
    //
    bool addLastStep = ((stopT - startT) % tsStride == 0.0) ? false : true;

    //
    // First, let's make sure this timestep range is valid. 
    //
    intVector cycles;
    GetCycles(domain, cycles);
    int numTS = cycles.size();

    if (startT < 0 || startT >= numTS ||
        stopT < 0 || stopT >= numTS ||
        startT > stopT)
    {
        char msg[256]; 
        snprintf(msg, 256, "Invalid timestep range requested.");
        EXCEPTION1(ImproperUseException, msg);
    }

    int numPoints = (int) ceil((float)(stopT - startT) / 
        (float) tsStride) + 1;

    doubleVector xCoords;
    xCoords.reserve(numPoints);

    switch (tType)
    {
        case QueryOverTimeAttributes::Cycle:
        {
            for (int i = startT; i <= stopT; i += tsStride)
            {
                xCoords.push_back((double) cycles[i]);
            }

            if (addLastStep)
            {
                xCoords.push_back((double) cycles[stopT]);
            }

            break;
        }
        case QueryOverTimeAttributes::DTime:
        {
            doubleVector times;
            GetTimes(domain, times);

            for (int i = startT; i <= stopT; i += tsStride)
            {
                xCoords.push_back((double) times[i]);
            }

            if (addLastStep)
            {
                xCoords.push_back((double) times[stopT]);
            }

            break;
        }
        case QueryOverTimeAttributes::Timestep:
        {
            for (int i = startT; i <= stopT; i += tsStride)
            {
                xCoords.push_back((double) i);
            }

            if (addLastStep)
            {
                xCoords.push_back((double) stopT);
            }

            break;
        }
        default:
        {
            char msg[256]; 
            snprintf(msg, 256, "Unknown time type requested.");
            EXCEPTION1(ImproperUseException, msg);
        }
    }

    vtkPolyData *polyData = vtkPolyData::New();
    vtkPoints *points     = vtkPoints::New();
    points->Allocate(numPoints);

    for (int i = 0; i < numPoints; ++i)
    {
        points->InsertNextPoint(xCoords[i], 0.0, 0.0);
    }

    polyData->SetPoints(points);
    points->Delete();

    return (vtkDataSet *) polyData;
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::GetQOTVar
//
//  Purpose:
//      Retrieve a query over time variable. 
//      This will produce an array that contains the values of a single 
//      element/variable pair through time. Each index 'i' of the array will
//      correspond with the 'ith' timestep. 
//
//  Arguments:
//      domain       The domain to read the variable from. 
//      varPath      The variable path to retrieve. 
//      element      The element to retrieve. 
//      tsRange      The timestep range. 
//      tsStride     The timestep stride. 
//
//  Returns:
//      A query over time variable. 
//
//  Programmer:  Alister Maguire
//  Creation:    Tue Sep 24 11:15:10 MST 2019
//
//  Modifications
//
// ****************************************************************************

vtkDataArray *
avtFileFormatInterface::GetQOTVar(int domain,
                                  const char *varPath,
                                  int element,
                                  int *tsRange,
                                  int tsStride)
{
    int startT = tsRange[0];
    int stopT  = tsRange[1];

    //
    // We want to always include the last time step. In cases where this
    // doesn't occur naturally, we need to manually add it. 
    //
    bool addLastStep = ((stopT - startT) % tsStride == 0.0) ? false : true;
    int numTuples    = (int) ceil((float)(stopT - startT) / 
        (float) tsStride) + 1;

    vtkFloatArray *spanArray = vtkFloatArray::New();
    spanArray->SetNumberOfTuples(numTuples);
    spanArray->SetNumberOfComponents(1);

    //
    // Iterate over the requested time states and retrieve the requested
    // var/element pair. 
    //
    int tupIdx = 0;
    for (int ts = startT; ts <= stopT; ts += tsStride, ++tupIdx)
    {
        //
        // Activate the current timestep and retrieve our variable. 
        //
        TRY
        {
            ActivateTimestep(ts);
            vtkFloatArray *allValues = (vtkFloatArray *) 
                GetVar(ts, domain, varPath);

            float targetVal = allValues->GetTuple1(element);
            spanArray->SetTuple1(tupIdx, targetVal);

            allValues->Delete();
        }
        CATCH2(VisItException, e)
        {
            spanArray->SetTuple1(tupIdx, 
                std::numeric_limits<float>::quiet_NaN());
        }
        ENDTRY
    }

    if (addLastStep)
    {
        TRY
        {
            ActivateTimestep(stopT);
            vtkFloatArray *allValues = (vtkFloatArray *) 
                GetVar(stopT, domain, varPath);

            float targetVal = allValues->GetTuple1(element);
            spanArray->SetTuple1(tupIdx, targetVal);

            allValues->Delete();
        }
        CATCH2(VisItException, e)
        {
            spanArray->SetTuple1(tupIdx, 
                std::numeric_limits<float>::quiet_NaN());
        }
        ENDTRY
    }

    return (vtkDataArray *) spanArray;
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::GetQOTVectorVar
//
//  Purpose:
//      Retrieve a query over time vector variable. 
//      This will produce an array that contains the values of a single 
//      element/variable pair through time. Each index 'i' of the array will
//      correspond with the 'ith' timestep. 
//
//  Arguments:
//      domain       The domain to read the variable from. 
//      varPath      The variable path to retrieve. 
//      vDim         The dimension of the vector. 
//      element      The element to retrieve. 
//      tsRange      The timestep range. 
//      tsStride     The timestep stride. 
//
//  Returns:
//      A query over time vector variable. 
//
//  Programmer:  Alister Maguire
//  Creation:    
//
//  Modifications
//
// ****************************************************************************

vtkDataArray *
avtFileFormatInterface::GetQOTVectorVar(int domain,
                                        const char *varPath,
                                        int vDim,
                                        int element,
                                        int *tsRange,
                                        int tsStride)
{
    int startT = tsRange[0];
    int stopT  = tsRange[1];

    //
    // We want to always include the last time step. In cases where this
    // doesn't occur naturally, we need to manually add it. 
    //
    bool addLastStep = ((stopT - startT) % tsStride == 0.0) ? false : true;
    int numTuples    = (int) ceil((float)(stopT - startT) / 
        (float) tsStride) + 1;

    //
    // VisIt ids begin at 0, but the interface ids start at 1. 
    // Account for this before retrieving. 
    //
    long int vecPos  = element * vDim;

    vtkFloatArray *spanArray = vtkFloatArray::New();
    spanArray->SetNumberOfComponents(vDim);
    spanArray->SetNumberOfTuples(numTuples);

    double *targetVec = new double[vDim];
    double *NaNVec    = new double[vDim];

    for (int i = 0; i < vDim; ++i)
    {
        targetVec[i] = 0.0;
        NaNVec[i]    = std::numeric_limits<float>::quiet_NaN();
    }

    //
    // Iterate over the requested time states and retrieve the requested vars
    // and elements. 
    //
    int tupIdx = 0;
    for (int ts = startT; ts <= stopT; ts += tsStride, ++tupIdx)
    {
        //
        // Activate the current timestep and retrieve our variable. 
        //
        TRY
        {
            ActivateTimestep(ts);
            vtkFloatArray *allValues = (vtkFloatArray *) 
                GetVectorVar(ts, domain, varPath);

            allValues->GetTuple(element, targetVec);
            spanArray->SetTuple(tupIdx, targetVec);

            allValues->Delete();
        }
        CATCH2(VisItException, e)
        {
            spanArray->SetTuple(tupIdx, NaNVec);
        }
        ENDTRY
    }

    if (addLastStep) 
    {
        TRY
        {
            ActivateTimestep(stopT);
            vtkFloatArray *allValues = (vtkFloatArray *) 
                GetVectorVar(stopT, domain, varPath);

            allValues->GetTuple(element, targetVec);
            spanArray->SetTuple(tupIdx, targetVec);

            allValues->Delete();
        }
        CATCH2(VisItException, e)
        {
            spanArray->SetTuple(tupIdx, NaNVec);
        }
        ENDTRY
    }

    delete [] targetVec;
    delete [] NaNVec;

    return (vtkDataArray *) spanArray;
}
