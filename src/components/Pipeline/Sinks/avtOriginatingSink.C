// ************************************************************************* //
//                            avtOriginatingSink.C                           //
// ************************************************************************* //

#include <avtOriginatingSink.h>

#include <avtPipelineSpecification.h>

#include <AbortException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <NoInputException.h>


//
// Define static members.
//

GuideFunction    avtOriginatingSink::guideFunction     = NULL;
void            *avtOriginatingSink::guideFunctionArgs = NULL;



// ****************************************************************************
//  Method: avtOriginatingSink constructor
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
// ****************************************************************************

avtOriginatingSink::avtOriginatingSink()
{
    pipelineIndex = -1;
}


// ****************************************************************************
//  Method: avtOriginatingSink::SetPipelineIndex
//
//  Purpose:
//      Sets the pipeline index for this originating sink.
//
//  Arguments:
//      pI      The new pipeline index.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
// ****************************************************************************

void
avtOriginatingSink::SetPipelineIndex(int pI)
{
    pipelineIndex = pI;
}


// ****************************************************************************
//  Method: avtOriginatingSink::Execute
//
//  Purpose:
//      Executes the pipeline.  This means possibly executing the pipeline
//      multiple times when dynamic load balancing is necessary.
//
//  Arguments:
//      spec    The data specification this pipeline should restrict itself to.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Jun 20 18:09:20 PDT 2001
//    Added support for aborting execution.
//
//    Jeremy Meredith, Thu Jul 26 12:34:47 PDT 2001
//    Added merging of partial updates as per Hank's code.
//
//    Hank Childs, Mon Jul 30 15:08:33 PDT 2001
//    Only clone and repeatedly merge for the second Update and after.
//
//    Hank Childs, Sun Sep 16 14:49:53 PDT 2001
//    Add flow of control debugging information.
//
//    Jeremy Meredith, Wed Sep 19 13:59:14 PDT 2001
//    Removed check for AbortException since the proper action is to simply
//    rethrow it from this function anyway -- so just don't catch it.
//
//    Hank Childs, Fri Sep 28 13:18:47 PDT 2001
//    Added hook for cleaning up after dynamic load balancing.
//
// ****************************************************************************

void
avtOriginatingSink::Execute(avtDataSpecification_p spec)
{
    if (pipelineIndex < 0)
    {
        //
        // We have to know what pipeline we are talking about.
        //
        EXCEPTION0(ImproperUseException);
    }

    avtPipelineSpecification_p pipelineSpec;
    pipelineSpec = new avtPipelineSpecification(spec, pipelineIndex);

    avtDataObject_p input = GetInput();
    if (*input == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    if (!guideFunction)
    {
        debug4 << "No guide function registered with the originating sink,"
               << " doing normal Update." << endl;
        input->Update(pipelineSpec);
    }
    else
    {
        //
        // Handle the first execution differently since we don't want to do
        // extra work if we don't have to.
        //
        if (guideFunction(guideFunctionArgs, pipelineIndex))
        {
            debug4 << "Guide function indicated that we should do the "
                   << "first Update on pipeline " << pipelineIndex << "." 
                   << endl;
            input->Update(pipelineSpec);
        }

        //
        // Now make a copy and continue if there are going to be multiple
        // updates (ie dynamic load balancing mode).
        //
        int iter = 1;
        if (guideFunction(guideFunctionArgs, pipelineIndex))
        {
            avtDataObject_p dob = input->Clone();
            while (guideFunction(guideFunctionArgs, pipelineIndex))
            {
                debug4 << "Doing " << iter++ << " iteration Updating on "
                       << "pipeline " << pipelineIndex << "." << endl;
                input->Update(pipelineSpec);
                dob->Merge(*input);
            }
            input->Copy(*dob);
            DynamicLoadBalanceCleanUp();
        }
        debug4 << "Done with iterating Updates on pipeline "
               << pipelineIndex << endl;
    }

    InputIsReady();
}


// ****************************************************************************
//  Method: avtOriginatingSink::InputIsReady
//
//  Purpose:
//      Called when Execute is done, so that derived types (like Mappers) that
//      need the input to up-to-date, can set up.
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
// ****************************************************************************

void
avtOriginatingSink::InputIsReady(void)
{
    ;
}


// ****************************************************************************
//  Method: avtOriginatingSink::DynamicLoadBalanceCleanUp
//
//  Purpose:
//      A hook to allow derived types to perform some clean up after dynamic
//      load balancing.
//
//  Programmer: Hank Childs
//  Creation:   September 28, 2001
//
// ****************************************************************************

void
avtOriginatingSink::DynamicLoadBalanceCleanUp(void)
{
    ;
}


// ****************************************************************************
//  Method: avtOriginatingSink::SetGuideFunction
//
//  Purpose:
//      Sets the load balancer to be consulted when this originating sink
//      determines if it should do another update.
//
//  Arguments:
//      foo     The function that will call a load balancer.
//      args    The arguments for the function.
//
//  Programmer: Hank Childs
//  Creation:   June 17, 2001
//
// ****************************************************************************

void
avtOriginatingSink::SetGuideFunction(GuideFunction foo, void *args)
{
    guideFunction     = foo;
    guideFunctionArgs = args;
}


