// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtFileFormatInterface.C                         //
// ************************************************************************* //

#include <avtFileFormatInterface.h>

#include <avtFileFormat.h>
#include <DebugStream.h>

#include <vtkFloatArray.h>


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
//  Method:  avtMiliFileFormat::GetTimeSpanCurves
//
//  Purpose:
//      Retrieve time span curves for the requested variable/element pairs
//      across the specified time range. 
//
//  Arguments:
//      domain        The domain of interest. 
//      vars          The variables to retrieve curves for.
//      elementIds    The elements (zones/nodes) to retrieve curves for. 
//      tsRange       The timestep range to retrieve curves for. 
//      stride        The timestep stride.
//
//  Returns:
//      A double pointer vtkDataArray arranged as follows:
//      [ curve_0, curve_1, curve_2, ..., curve_N ],
//      where each curve_i is a variable/element pair's value
//      through the given time range with the given stride. 
//
//  Programmer:  Alister Maguire
//  Creation:    Tue Sep  3 13:46:43 MST 2019 
//
//  Modifications
//
// ****************************************************************************

vtkDataArray **
avtFileFormatInterface::GetTimeSpanCurves(int domain,
                                          stringVector vars,
                                          intVector elements,
                                          int *tsRange,
                                          int stride)
{
    avtFileFormat *ff = GetFormat(0);
    bool canRetrieveFromPlugin = false;

    //
    // First, determine whether we can retreive these curves directly
    // from the plugin or we need to use the default method.  
    //
    if (ff != NULL)
    {
        canRetrieveFromPlugin = ff->FormatCanRetrieveSpan(); 
    }

    if (canRetrieveFromPlugin)
    {
        return GetTimeSpanCurvesFromPlugin(domain,
                                           vars,
                                           elements,
                                           tsRange,
                                           stride);
    }
    else
    { 
        return GetTimeSpanCurvesDefault(domain,
                                        vars,
                                        elements,
                                        tsRange,
                                        stride);
    }
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::GetTimeSpanCurves
//
//  Purpose:
//      Retrieve time span curves for the requested variable/element pairs
//      across the specified time range. Use the database plugin's 
//      defined method.  
//
//  Arguments:
//      domain        The domain of interest. 
//      vars          The variables to retrieve curves for.
//      elementIds    The elements (zones/nodes) to retrieve curves for. 
//      tsRange       The timestep range to retrieve curves for. 
//      stride        The timestep stride.
//
//  Returns:
//      A double pointer vtkDataArray arranged as follows:
//      [ curve_0, curve_1, curve_2, ..., curve_N ],
//      where each curve_i is a variable/element pair's value
//      through the given time range with the given stride. 
//
//  Programmer:  Alister Maguire
//  Creation:    Tue Sep  3 13:46:43 MST 2019 
//
//  Modifications
//
// ****************************************************************************

vtkDataArray **
avtFileFormatInterface::GetTimeSpanCurvesFromPlugin(int domain,
                                                    stringVector vars,
                                                    intVector elements,
                                                    int *tsRange,
                                                    int stride)
{
    debug1 << "Calling GetTimeSpanCurvesFromPlugin from " << GetType()
           << "but it's not implemented..." << endl;
    return NULL; 
}


// ****************************************************************************
//  Method:  avtMiliFileFormat::GetTimeSpanCurves
//
//  Purpose:
//      Retrieve time span curves for the requested variable/element pairs
//      across the specified time range. Use the default implementation
//      defined in this class. 
//
//  Arguments:
//      domain        The domain of interest. 
//      vars          The variables to retrieve curves for.
//      elementIds    The elements (zones/nodes) to retrieve curves for. 
//      tsRange       The timestep range to retrieve curves for. 
//      stride        The timestep stride.
//
//  Returns:
//      A double pointer vtkDataArray arranged as follows:
//      [ curve_0, curve_1, curve_2, ..., curve_N ],
//      where each curve_i is a variable/element pair's value
//      through the given time range with the given stride. 
//
//  Programmer:  Alister Maguire
//  Creation:    Tue Sep  3 13:46:43 MST 2019 
//
//  Modifications
//
// ****************************************************************************

vtkDataArray **
avtFileFormatInterface::GetTimeSpanCurvesDefault(int domain,
                                                 stringVector vars,
                                                 intVector elements,
                                                 int *tsRange,
                                                 int stride)
{
    int startT     = tsRange[0];
    int stopT      = tsRange[1] + 1;
    int spanSize   = (stopT - startT) / stride;
    int numElems   = elements.size();
    int numVars    = vars.size();
    int numArrays  = numElems * numVars;

    std::vector< std::vector<float> > results;
    results.reserve(spanSize);

    //
    // Iterate over the requested time states and retrieve the requested vars
    // and elements. 
    //
    for (int ts = startT; ts < stopT; ts += stride)
    {
        if (ts >= stopT)
        {
            break;
        }
    
        floatVector varElRange;
        varElRange.reserve(numArrays);
        
        for (stringVector::iterator varItr = vars.begin();
             varItr < vars.end(); ++varItr)
        {
            std::string curVar = *varItr; 

            //
            // Activate the current timestep and retrieve our variable. 
            //
            ActivateTimestep(ts);
            vtkFloatArray *allValues = (vtkFloatArray *) 
                GetVar(ts, domain, curVar.c_str());

            for (intVector::iterator elItr = elements.begin();
                 elItr < elements.end(); ++elItr)
            {
                //
                // VisIt ids begin at 0, but the interface ids start at 1. 
                // Account for this before retrieving. 
                //
                long int visitId = (*elItr) - 1;
                float val = allValues->GetTuple1(visitId);
                varElRange.push_back(val);
            }
        }

        results.push_back(varElRange);
    }

    vtkFloatArray **spanArrays = new vtkFloatArray *[numArrays];

    for (int i = 0; i < numArrays; ++i)
    {
        spanArrays[i] = NULL;
    }

    //
    // We now need to re-organize the data into the form needed 
    // to produce curves. This is basically an array of variable
    // element arrays that span over cycles/time:
    //
    // [ [c0_v0_e0, c1_v0_e0,...], [c0_v0_e1, c1_v0_e1, ...], ...]
    //
    // where ci_vj_ek is the k'th element of the j'th variable at
    // the i'th cycle. 
    //
    int spanArrIdx = 0;
    for (int v = 0; v < numVars; ++v) 
    {
        int varIdx = (v * numElems);
      
        for (int e = 0; e < numElems; ++e)
        {
            int valueIdx = (varIdx + e);

            vtkFloatArray *singleSpan = vtkFloatArray::New();
            singleSpan->SetNumberOfComponents(1);
            singleSpan->SetNumberOfTuples(spanSize);
            float *spanPtr = (float *) singleSpan->GetVoidPointer(0);

            if (spanPtr == NULL)
            {
                debug2 << "Encountered a null span in GetTimeSpanCurvesDefault"
                    << endl;
                continue;
            }

            for (int tIdx = 0; tIdx < spanSize; ++tIdx)
            {
                spanPtr[tIdx] = results[tIdx][valueIdx];  
            }

            spanArrays[spanArrIdx++] = singleSpan;
        }
    }

    return (vtkDataArray **) spanArrays;
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
