// ************************************************************************* //
//                           avtTerminatingSource.C                          //
// ************************************************************************* //

#include <avtTerminatingSource.h>

#include <avtMetaData.h>

#include <DebugStream.h>


//
// Define static members.
//

LoadBalanceFunction  avtTerminatingSource::loadBalanceFunction     = NULL;
void                *avtTerminatingSource::loadBalanceFunctionArgs = NULL;
DynamicCheckFunction  avtTerminatingSource::dynamicCheckFunction     = NULL;
void                 *avtTerminatingSource::dynamicCheckFunctionArgs = NULL;
InitializeProgressCallback
                     avtTerminatingSource::initializeProgressCallback = NULL;
void                *avtTerminatingSource::initializeProgressCallbackArgs=NULL;


// ****************************************************************************
//  Method: avtTerminatingSource constructor
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2001
//
// ****************************************************************************

avtTerminatingSource::avtTerminatingSource()
{
    metadata = new avtMetaData(this);
}


// ****************************************************************************
//  Method: avtTerminatingSource destructor
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2001
//
// ****************************************************************************

avtTerminatingSource::~avtTerminatingSource()
{
    if (metadata != NULL)
    {
        delete metadata;
        metadata = NULL;
    }
}


// ****************************************************************************
//  Method: avtTerminatingSource::GetMeshAuxiliaryData
//
//  Purpose:
//      Retrieves mesh auxiliary data from whatever the source to the pipeline
//      is (most likely a database).
//
//  Arguments:
//      auxType     The type of auxiliary data.
//      args        The arguments in some sort of struct.
//      spec        The pipeline specification.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2001
//
// ****************************************************************************

void
avtTerminatingSource::GetMeshAuxiliaryData(const char *type, void *args,
                          avtPipelineSpecification_p spec, VoidRefList &output)
{
    avtDataSpecification_p data = BalanceLoad(spec);
    FetchMeshAuxiliaryData(type, args, data, output);
}


// ****************************************************************************
//  Method: avtTerminatingSource::GetVariableAuxiliaryData
//
//  Purpose:
//      Retrieves variable auxiliary data from whatever the source to the
//      pipeline is (most likely a database).
//
//  Arguments:
//      auxType     The type of auxiliary data.
//      args        The arguments in some sort of struct.
//      spec        The pipeline specification.
//
//  Programmer: Hank Childs
//  Creation:   May 28, 2001
//
// ****************************************************************************

void 
avtTerminatingSource::GetVariableAuxiliaryData(const char *type, void *args,
                          avtPipelineSpecification_p spec, VoidRefList &output)
{
    avtDataSpecification_p data = BalanceLoad(spec);
    FetchVariableAuxiliaryData(type, args, data, output);
}


// ****************************************************************************
//  Method: avtTerminatingSource::GetMaterialAuxiliaryData
//
//  Purpose:
//      Retrieves material auxiliary data from whatever the source to the
//      pipeline is (most likely a database).
//
//  Arguments:
//      auxType     The type of auxiliary data.
//      args        The arguments in some sort of struct.
//      spec        The pipeline specification.
//
//  Programmer: Hank Childs
//  Creation:   May 28, 2001
//
// ****************************************************************************

void 
avtTerminatingSource::GetMaterialAuxiliaryData(const char *type, void *args,
                          avtPipelineSpecification_p spec, VoidRefList &output)
{
    avtDataSpecification_p data = BalanceLoad(spec);
    FetchMaterialAuxiliaryData(type, args, data, output);
}


// ****************************************************************************
//  Method: avtTerminatingSource::Update
//
//  Purpose:
//      Performs an Update.  This is the termination of a pipeline.  The
//      load balancer is consulted at this point.
//
//  Arguments:
//      spec     The pipeline specification.
//
//  Returns:     A bool indicating if the input was changed.  This only returns
//               false now, but this may need more intelligence in the future.
//
//  Programmer:  Hank Childs
//  Creation:    May 28, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Aug 13 15:23:08 PDT 2001
//    Made call to update progress.
//
// ****************************************************************************

bool
avtTerminatingSource::Update(avtPipelineSpecification_p spec)
{
    if (!ArtificialPipeline())
    {
        UpdateProgress(0, 0, "avtTerminatingSource", "Retrieving data.");
    }
    avtDataSpecification_p data = BalanceLoad(spec);
    bool rv = FetchData(data);
    if (!ArtificialPipeline())
    {
        UpdateProgress(1, 0, "avtTerminatingSource", "Retrieving data.");
    }
    return rv;
}


// ****************************************************************************
//  Method: avtTerminatingSource::SetLoadBalancer
//
//  Purpose:
//      Sets the load balancer to be consulted when this terminating source
//      is about to get the actual data.
//
//  Arguments:
//      foo     The function that will call a load balancer.
//      args    The arguments for the function.
//
//  Programmer: Hank Childs
//  Creation:   May 28, 2001
//
// ****************************************************************************

void
avtTerminatingSource::SetLoadBalancer(LoadBalanceFunction foo, void *args)
{
    loadBalanceFunction     = foo;
    loadBalanceFunctionArgs = args;
}


// ****************************************************************************
//  Method: avtTerminatingSource::SetDynamicChecker
//
//  Purpose:
//      Sets the load balancer to be consulted to determine if load
//      balancing will be dynamic.
//
//  Arguments:
//      foo     The function that will call a load balancer dynamic checker.
//      args    The arguments for the function.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 19, 2001
//
// ****************************************************************************

void
avtTerminatingSource::SetDynamicChecker(DynamicCheckFunction foo, void *args)
{
    dynamicCheckFunction     = foo;
    dynamicCheckFunctionArgs = args;
}


// ****************************************************************************
//  Method: avtTerminatingSource::RegisterInitializeProgressCallback
//
//  Purpose:
//      Registers the InitializeProgressCallback.  This will be called before
//      any pipeline execution occurs to declare how many filters there are in
//      the whole pipeline.
//
//  Arguments:
//      pc      The initialize progress callback.
//      args    The arguments to the initialize progress callback.
//
//  Programmer: Hank Childs
//  Creation:   June 20, 2001
//
// ****************************************************************************

void
avtTerminatingSource::RegisterInitializeProgressCallback(
                                     InitializeProgressCallback pc, void *args)
{
    initializeProgressCallback     = pc;
    initializeProgressCallbackArgs = args;
}


// ****************************************************************************
//  Method: avtTerminatingSource::FetchMeshAuxiliaryData
//
//  Purpose:
//      Defines an implementation for fetching auxiliary data about a mesh.
//      This implementation does nothing and is only defined so that derived
//      types don't need to define a method that potentially doesn't make
//      sense for that type.
//
//  Arguments:
//      dataType  The type of data about the mesh (extents, etc).
//      args      The arguments describing the data.
//      spec      The data we are interested in.
//
//  Returns:      The requested data as a void *.
//
//  Programmer:   Hank Childs
//  Creation:     May 28, 2001
//
// ****************************************************************************  
void
avtTerminatingSource::FetchMeshAuxiliaryData(const char *, void *,
                                   avtDataSpecification_p, VoidRefList &output)
{
    output.nList = 0;
}


// ****************************************************************************
//  Method: avtTerminatingSource::FetchVariableAuxiliaryData
//
//  Purpose:
//      Defines an implementation for fetching auxiliary data about a variable.
//      This implementation does nothing and is only defined so that derived
//      types don't need to define a method that potentially doesn't make
//      sense for that type.
//
//  Arguments:
//      dataType  The type of data about the variable (extents, etc).
//      args      The arguments describing the data.
//      spec      The data we are interested in.
//
//  Returns:      The requested data as a void *.
//
//  Programmer:   Hank Childs
//  Creation:     May 28, 2001
//
// ****************************************************************************  
void 
avtTerminatingSource::FetchVariableAuxiliaryData(const char *,void *,
                                   avtDataSpecification_p, VoidRefList &output)
{
    output.nList = 0;
}


// ****************************************************************************
//  Method: avtTerminatingSource::FetchMaterialAuxiliaryData
//
//  Purpose:
//      Defines an implementation for fetching auxiliary data about a material.
//      This implementation does nothing and is only defined so that derived
//      types don't need to define a method that potentially doesn't make
//      sense for that type.
//
//  Arguments:
//      dataType  The type of data about the material.
//      args      The arguments describing the data.
//      spec      The data we are interested in.
//
//  Returns:      The requested data as a void *.
//
//  Programmer:   Hank Childs
//  Creation:     May 28, 2001
//
// ****************************************************************************  
void
avtTerminatingSource::FetchMaterialAuxiliaryData(const char *,void *,
                                   avtDataSpecification_p, VoidRefList &output)
{
    output.nList = 0;
}


// ****************************************************************************
//  Method: avtTerminatingSource::BalanceLoad
//
//  Purpose:
//      This calls a function that will take in the input data specification 
//      and return the chunk of the data that pipeline should operate on for
//      this execution.
//      
//  Arguments:
//      spec     The pipeline specification
// 
//  Returns:     The data specification after load balancing.
//
//  Programmer:  Hank Childs
//  Creation:    May 28, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Jun 20 17:26:46 PDT 2001
//    Added call to InitPipeline.
//
//    Hank Childs, Tue Jul 24 10:14:55 PDT 2001
//    Added condition to call InitPipeline to account for auxiliary data.
//
//    Hank Childs, Wed Oct  9 10:14:37 PDT 2002
//    Check to see if we are using all the domains as a courtesy to the
//    database.  (It may not have enough information if we are running in
//    parallel).
//
// ****************************************************************************

avtDataSpecification_p
avtTerminatingSource::BalanceLoad(avtPipelineSpecification_p spec)
{
    bool usesAllDomains = spec->GetDataSpecification()
                                                   ->GetSIL().UsesAllDomains();

    //
    // If it shouldn't use load balancing, then it has to do with auxiliary
    // data coming through our meta-data mechanism.  Calling InitPipeline
    // would change the data attributes and it also causes an unnecessary
    // callback to our progress mechanism.
    //
    if (spec->ShouldUseLoadBalancing())
    {
        InitPipeline(spec);
    }

    //
    // Allow the load balancer to split the load across processors.
    //
    avtDataSpecification_p rv = NULL;
    if (!UseLoadBalancer())
    {
        debug5 << "This source should not load balance the data." << endl;
        rv = spec->GetDataSpecification();
    }
    else if (! spec->ShouldUseLoadBalancing())
    {
        debug5 << "This pipeline has indicated that no load balancing should "
               << "be used." << endl;
        rv = spec->GetDataSpecification();
    }
    else if (loadBalanceFunction != NULL)
    {
        debug5 << "Using load balancer to reduce data." << endl;
        rv = loadBalanceFunction(loadBalanceFunctionArgs, spec);
    }
    else
    {
        debug1 << "No load balancer exists to reduce data." << endl;
        rv = spec->GetDataSpecification();
    }

    //
    // Return the portion for this processor.
    //
    rv->SetUsesAllDomains(usesAllDomains);
    return rv;
}


// ****************************************************************************
//  Method: avtTerminatingSource::InitPipeline
//
//  Purpose:
//      Initializes the rest of the pipeline.  This determines if the data
//      specification is all of the data and also makes a callback declaring
//      how many total filters there are.
//
//  Arguments:
//      spec    The pipeline specification for all of the nodes.
//
//  Programmer: Hank Childs
//  Creation:   June 20, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Jul 31 11:32:57 PDT 2001
//    Do not re-initialize the pipeline if we have an artificial pipeline.
//
//    Hank Childs, Fri Sep 14 09:58:14 PDT 2001
//    Indicate if we are using all of the domains.
//
//    Jeremy Meredith, Thu Sep 20 01:03:48 PDT 2001
//    Added check for dynamic load balancing, which has only one stage.
//
//    Kathleen Bonnell, Fri Oct 12 12:07:01 PDT 2001 
//    Set IsThisDynamic.  
//
//    Hank Childs, Thu Oct 25 16:44:24 PDT 2001
//    Allow for the source to prevent dynamic load balancing.
//
//    Hank Childs, Fri Oct 26 15:35:37 PDT 2001
//    Allow for terminating sources to have stages.
//
// ****************************************************************************

void
avtTerminatingSource::InitPipeline(avtPipelineSpecification_p spec)
{
    if (!ArtificialPipeline())
    {
        if (!CanDoDynamicLoadBalancing())
        {
            spec->NoDynamicLoadBalancing();
        }

        //
        // Determine if the data specification is *all* of the data.  This is
        // important to things like facelists.
        //
        avtDataSpecification_p data = spec->GetDataSpecification();
        bool uad = data->GetSIL().UsesAllData();
        GetOutput()->GetInfo().GetValidity().SetUsingAllData(uad);
    
        bool uadom = data->GetSIL().UsesAllDomains();
        GetOutput()->GetInfo().GetValidity().SetUsingAllDomains(uadom);

        if (initializeProgressCallback != NULL &&
            dynamicCheckFunction       != NULL)
        {
            //
            // Each filter is a stage, plus a stage to get the data.
            // In dynamic load balancing, there are only calculate/send stages.
            //
            int nstages;
            if (dynamicCheckFunction(dynamicCheckFunctionArgs,spec))
            {
                nstages = 1;
                GetOutput()->GetInfo().GetValidity().SetIsThisDynamic(true);
            }
            else
            {
                int sourceStages = NumStagesForFetch(data);
                nstages = spec->GetNFilters() + sourceStages;
                GetOutput()->GetInfo().GetValidity().SetIsThisDynamic(false);
            }
            initializeProgressCallback(initializeProgressCallbackArgs,nstages);
        }
    }
}


// ****************************************************************************
//  Method: avtTerminatingSource::GetFullDataSpecification
//
//  Purpose:
//      Gets a full data description.  Since this doesn't make as much sense
//      for some sources as others (say a database), it is defined here to
//      return a dummied up description.
//
//  Returns:    A totally generic data description.
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
// ****************************************************************************

avtDataSpecification_p
avtTerminatingSource::GetFullDataSpecification(void)
{
    return new avtDataSpecification("dummy_specification", -1, -1);
}


// ****************************************************************************
//  Method: avtTerminatingSource::GetGeneralPipelineSpecification
//
//  Purpose:
//       Gets a pipeline that the load balancer knows not to muck with.
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
// ****************************************************************************

avtPipelineSpecification_p
avtTerminatingSource::GetGeneralPipelineSpecification(void)
{
    avtDataSpecification_p data = GetFullDataSpecification();
    return new avtPipelineSpecification(data, 0);
}


// ****************************************************************************
//  Method: avtTerminatingSource::CanDoDynamicLoadBalancing
//
//  Purpose:
//      Returns whether or not this source can do dynamic load balancing.
//
//  Programmer: Hank Childs
//  Creation:   October 25, 2001
//
// ****************************************************************************

bool
avtTerminatingSource::CanDoDynamicLoadBalancing(void)
{
    //
    // We'd like to do it unless we have a reason not to.
    //
    return true;
}


// ****************************************************************************
//  Method: avtTerminatingSource::NumStagesForFetch
//
//  Purpose:
//      This returns how many stages there are for the fetch.  Some databases
//      have more than one stage (read from file format, perform material
//      selection, etc).
//
//  Returns:    A good default number of stages (1).
//
//  Programmer: Hank Childs
//  Creation:   October 26, 2001
//
// ****************************************************************************

int
avtTerminatingSource::NumStagesForFetch(avtDataSpecification_p)
{
    return 1;
}


