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
//  Method: avtFileFormatInterface::GetTimeAndElementSpanVars
//
//  Purpose:
//
//  Programmer: 
//  Creation:   
//
// ****************************************************************************

vtkDataArray **
avtFileFormatInterface::GetTimeAndElementSpanVars(int domain,
                                                  intVector elements,
                                                  stringVector vars,
                                                  int *tsRange,
                                                  int stride)
{
    cerr << "INSIDE DEFAULT SPAN GETTER" << endl;//FIXME

    //TODO: incorporate stride. 
    int startT     = tsRange[0];
    int stopT      = tsRange[1] + stride;
    int spanSize   = (startT - stopT) / stride;
    int numElems   = elements.size();
    int numVars    = vars.size();
    int numArrays  = numElems * numVars;

    std::vector< std::vector<float> > results;
    //results.reserve(spanSize);

    for (int ts = startT; ts < stopT; ts += stride)
    {
        cerr << "TS: " << ts << endl;
        floatVector varElRange;
        //varElRange.reserve(numArrays);
        
        for (stringVector::iterator varItr = vars.begin();
             varItr < vars.end(); ++varItr)
        {
            std::string curVar = *varItr; 
            cerr << "VAR: " << curVar << endl;//FIXME

            vtkFloatArray *allValues = (vtkFloatArray *) GetVar(ts, domain, curVar.c_str());

            for (intVector::iterator elItr = elements.begin();
                 elItr < elements.end(); ++elItr)
            {
                long int visitId = (*elItr) - 1;
                //cerr << "\nGETTING ELEMENT: " << visitId << endl;
                float val = allValues->GetTuple1(visitId);
                //cerr << "DONE GETTING ELEMENT: " << visitId << endl;
                //cerr << "VAL IS: " << val << endl;
                varElRange.push_back(val);
                //cerr << "DONE ADDING TO VAR EL RANGE" << endl;//FIXME
            }
        }

        //cerr << "ADDING ARRAY TO RESULTS" << endl;//FIXME
        results.push_back(varElRange);
        //cerr << "DONE ADDING ARRAY TO RESULTS" << endl;//FIXME
    }

    vtkFloatArray **spanArrays = new vtkFloatArray *[numArrays];

    for (int i = 0; i < numArrays; ++i)
    {
        spanArrays[i] = NULL;
    }

    cerr << "MAX INDEX: " << numArrays << endl;//FIXME
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
                cerr << "POINTER IS NULL" << endl;//FIXME

            for (int tIdx = 0; tIdx < spanSize; ++tIdx)
            {
                spanPtr[tIdx] = results[tIdx][valueIdx];  
                cerr << results[tIdx][valueIdx] << endl;//FIXME
            }

            spanArrays[spanArrIdx++] = singleSpan;
        }
    }

    return (vtkDataArray **) spanArrays;
}
