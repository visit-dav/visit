// ************************************************************************* //
//                            avtMultipleInputSink.C                         //
// ************************************************************************* //

#include <avtMultipleInputSink.h>

#include <BadIndexException.h>


// ****************************************************************************
//  Method: avtMultipleInputSink constructor
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2001
//
// ****************************************************************************

avtMultipleInputSink::avtMultipleInputSink()
{
    sinks  = NULL;
    nSinks = 0;
}


// ****************************************************************************
//  Method: avtMultipleInputSink destructor
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2001
//
// ****************************************************************************

avtMultipleInputSink::~avtMultipleInputSink()
{
    if (sinks != NULL)
    {
        // Delete each sink in the array?
        delete [] sinks;
        sinks = NULL;
    }
}


// ****************************************************************************
//  Method: avtMultipleInputSink::SetNumSinks
//
//  Purpose:
//      Sets the number of sinks for this filter.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2001
//
// ****************************************************************************

void
avtMultipleInputSink::SetNumSinks(int ns)
{
    if (sinks != NULL)
    {
        delete [] sinks;
    }

    nSinks = ns;

    sinks = new avtDataObjectSink*[nSinks];
    for (int i = 0 ; i < nSinks ; i++)
    {
        sinks[i] = NULL;
    }
}


// ****************************************************************************
//  Method: avtMultipleInputSink::SetSink
//
//  Purpose:
//      Sets a sink as one of the sinks for the multiple inputs.
//
//  Arguments:
//      s       The sink.
//      n       The sink index.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2001
//
// ****************************************************************************

void
avtMultipleInputSink::SetSink(avtDataObjectSink *s, int n)
{
    if (n < 0 || n >= nSinks)
    {
        EXCEPTION2(BadIndexException, n, nSinks);
    }

    // Delete old sink?
    sinks[n] = s;
}


// ****************************************************************************
//  Method: avtMultipleInputSink::SetNthInput
//
//  Purpose:
//      Sets the input to one of the data object sinks.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2001
//
// ****************************************************************************

void
avtMultipleInputSink::SetNthInput(avtDataObject_p input, int n)
{
    if (n < 0 || n >= nSinks)
    {
        EXCEPTION2(BadIndexException, n, nSinks);
    }

    sinks[n]->SetInput(input);
}


// ****************************************************************************
//  Method: avtMultipleInputSink::GetInput
//
//  Purpose:
//      The GetInput routine must be defined for sinks.  Meet that interface.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2001
//
// ****************************************************************************

avtDataObject_p
avtMultipleInputSink::GetInput(void)
{
    if (nSinks <= 0)
    {
        EXCEPTION2(BadIndexException, 0, nSinks);
    }

    return sinks[0]->GetInput();
}


// ****************************************************************************
//  Method: avtMultipleInputSink::SetTypedInput
//
//  Purpose:
//      The SetTypedInput routine must be defined for sinks.  Meet that
//      interface.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2001
//
// ****************************************************************************

void
avtMultipleInputSink::SetTypedInput(avtDataObject_p dob)
{
    if (nSinks <= 0)
    {
        EXCEPTION2(BadIndexException, 0, nSinks);
    }

    sinks[0]->SetTypedInput(dob);
}


// ****************************************************************************
//  Method: avtMultipleInputSink::GetNthInput
//
//  Purpose:
//      Gets the input to one of the data object sinks.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2001
//
// ****************************************************************************

avtDataObject_p
avtMultipleInputSink::GetNthInput(int n)
{
    if (n < 0 || n >= nSinks)
    {
        EXCEPTION2(BadIndexException, n, nSinks);
    }

    return sinks[n]->GetInput();
}


// ****************************************************************************
//  Method: avtMultipleInputSink::UpdateInput
//
//  Purpose:
//      Does an Update on all of the inputs.
//
//  Arguments:
//      spec    The pipeline specification.
//
//  Returns:    Whether or not anything was modified up stream.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2001
//
// ****************************************************************************

bool
avtMultipleInputSink::UpdateInput(avtPipelineSpecification_p spec)
{
    bool modifiedUpstream = false;
    for (int i = 0 ; i < nSinks ; i++)
    {
        avtDataObject_p dob = sinks[i]->GetInput();
        bool modifiedHere = dob->Update(spec);
        modifiedUpstream = modifiedUpstream || modifiedHere;
    }

    return modifiedUpstream;
}


