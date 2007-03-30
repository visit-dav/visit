// ************************************************************************* //
//                             avtFacadeFilter.C                             //
// ************************************************************************* //

#include <avtFacadeFilter.h>

#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtFacadeFilter::GetInput
//
//  Purpose:
//      Gets the input for this filter.  Since this is a facade, it goes to the
//      first input in the pipeline and gets its filter.
//
//  Programmer: Hank Childs
//  Creation:   April 16, 2002
//
// ****************************************************************************

avtDataObject_p
avtFacadeFilter::GetInput(void)
{
    return GetFirstFilter()->GetInput();
}


// ****************************************************************************
//  Method: avtFacadeFilter::GetOutput
//
//  Purpose:
//      Gets the output for this filter.  Since this is a facade, it goes to 
//      the last input in the pipeline and gets its filter.
//
//  Programmer: Hank Childs
//  Creation:   April 16, 2002
//
// ****************************************************************************

avtDataObject_p
avtFacadeFilter::GetOutput(void)
{
    return GetLastFilter()->GetOutput();
}


// ****************************************************************************
//  Method: avtFacadeFilter::GetTerminatingSource
//
//  Purpose:
//      Gets the terminating source of the piepline.  Since this is a facade,
//      it goes to the first filter in the pipeline and gets its terminating
//      source.  This can probably be achieved by using any filter in the
//      pipeline.
//
//  Programmer: Hank Childs
//  Creation:   April 16, 2002
//
// ****************************************************************************

avtTerminatingSource *
avtFacadeFilter::GetTerminatingSource(void)
{
    return GetLastFilter()->GetTerminatingSource();
}


// ****************************************************************************
//  Method: avtFacadeFilter::Update
//
//  Purpose:
//      Responds as if it is a normal filter doing an Update.  This can be
//      achieved by making the last filter do an Update.
//
//  Programmer: Hank Childs
//  Creation:   April 16, 2002
//
// ****************************************************************************

bool
avtFacadeFilter::Update(avtPipelineSpecification_p spec)
{
    return GetLastFilter()->Update(spec);
}


// ****************************************************************************
//  Method: avtFacadeFilter::SetTypedInput
//
//  Purpose:
//      Sets the input of the facade.  This passes the input on to the first
//      filter in the pipeline.
//
//  Programmer: Hank Childs
//  Creation:   April 16, 2002
//
// ****************************************************************************

void
avtFacadeFilter::SetTypedInput(avtDataObject_p input)
{
    GetFirstFilter()->SetInput(input);
}


// ****************************************************************************
//  Method: avtFacadeFilter::Execute
//
//  Purpose:
//      This must be defined so that the derived types are concrete.  Execute
//      should never be called, since the Update/Execute cycle should go
//      through the filters that are facaded.  If it is called, an exception
//      should be called.
//
//  Programmer: Hank Childs
//  Creation:   April 16, 2002
//
// ****************************************************************************

void
avtFacadeFilter::Execute(void)
{
    EXCEPTION0(ImproperUseException);
}


