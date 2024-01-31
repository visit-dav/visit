// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "VisItControlInterfaceRuntime.h"
#include <VisItInterfaceTypes_V2.h>
#include <VisItInterfaceTypes_V2P.h>

#include <DebugStream.h>
#include <DBOptionsAttributes.h>
#include <SimEngine.h>
#include <NetworkManager.h>
#include <LostConnectionException.h>
#include <LoadBalancer.h>
#ifdef PARALLEL
#include <MPIXfer.h>
#include <avtParallel.h>
#endif
#include <AttributeGroup.h>
#include <View2DAttributes.h>
#include <View3DAttributes.h>
#include <StackTimer.h>
#include <TimingsManager.h>
#include <VisItException.h>

#include <avtDataObjectString.h>

#include <PlotPluginManager.h>
#include <PlotPluginInfo.h>
#include <OperatorPluginManager.h>
#include <OperatorPluginInfo.h>

#include <visitstream.h>
#include <map>
#include <vector>
#include <cstring>

#include <vtkVisItUtility.h>
#include <avtDatabaseFactory.h>
#include <avtFileDescriptorManager.h>
#include <StringHelpers.h>

#include <simv2_NameList.h>
#include <simv2_OptionList.h>
#include <simv2_View2D.h>
#include <simv2_View3D.h>

extern void DataCallbacksCleanup(void);

// Engine creation callback.
static Engine *
simv2_create_engine(void *)
{
    return new SimEngine;
}

// ****************************************************************************
// Method: simv2_get_engine
//
// Purpose:
//   SimV2 runtime function to get the engine pointer, creating the object if 
//   necessary.
//
// Returns:    A pointer to the engine.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 18:38:01 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void *simv2_get_engine()
{
    // Set the engine creation callback so it will create our SimEngine subclass.
    EngineBase::SetEngineCreationCallback(simv2_create_engine, NULL);

    // Make sure the timer is initialized. In visit this is normally
    // done in the main function but for the simulation it's done here.
    if (visitTimer == NULL)
    {
        TimingsManager::Initialize("Simulation");
        // really disable the timer since we are very likely
        // running in a resource constrained environment over
        // a long time period
        visitTimer->Disable();
        visitTimer->NoForcedTiming();
    }

    Engine::GetEngine()->EnableSimulationPlugins();
    return (void*)Engine::GetEngine();
}

// ****************************************************************************
// Method: simv2_initialize
//
// Purpose:
//   SimV2 runtime function to initialize the engine.
//
// Arguments:
//   e    : The engine pointer.
//   argc : The number of command line args.
//   argv : The command line args.
//   batch : True if we're initializing the engine for batch.
//
// Returns:    1 on success, 0 on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 18:39:01 PDT 2014
//
// Modifications:
//
//    Brad Whitlock, Mon Aug 17 17:15:56 PDT 2015
//    Parse the command line options to allow plot and operator plugins to be
//    restricted.
//
//    Burlen Loring, Fri Oct  2 15:18:03 PDT 2015
//    Don't need to check the permissions on the .sim2 file.
//
// ****************************************************************************

static int simv2_initialize_helper(void *e, int argc, char *argv[], bool batch)
{
    int retval = 1;
    SimEngine *engine = (SimEngine*)(e);
    TRY
    {
        engine->Initialize(&argc, &argv, false);
        engine->InitializeCompute();
        if(batch)
        {
            // See if we're restricting the plugins.
            bool noconfig = false;
            std::vector<std::string> plotPlugins, operatorPlugins;
            for(int i = 0; i < argc; ++i)
            {
                if(strcmp(argv[i], "-plotplugins") == 0 && (i+1) < argc)
                {
                    plotPlugins = StringHelpers::split(std::string(argv[i+1]), ',');
                    ++i;
                }
                else if(strcmp(argv[i], "-operatorplugins") == 0 && (i+1) < argc)
                {
                    operatorPlugins = StringHelpers::split(std::string(argv[i+1]), ',');
                    ++i;
                }
                else if(strcmp(argv[i], "-noconfig") == 0)
                {
                    noconfig = true;
                    ++i;
                }
            }

            engine->InitializeViewer(plotPlugins, operatorPlugins, noconfig);
        }
        LoadBalancer::SetScheme(LOAD_BALANCE_RESTRICTED);
    }
    CATCHALL
    {
        retval = 0;
    }
    ENDTRY
    return retval;
}

int simv2_initialize(void *e, int argc, char *argv[])
{
    avtDatabaseFactory::SetCheckFilePermissions(false);
    return simv2_initialize_helper(e, argc, argv, false);
}

int simv2_initialize_batch(void *e, int argc, char *argv[])
{
    avtDatabaseFactory::SetCheckFilePermissions(false);
    return simv2_initialize_helper(e, argc, argv, true);
}

// ****************************************************************************
// Method: simv2_connect_viewer
//
// Purpose:
//   SimV2 runtime function to connect to the viewer.
//
// Arguments:
//   e    : The engine pointer.
//   argc : The number of command line args.
//   argv : The command line args.
//
// Returns:    1 on success, 0 on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 18:40:01 PDT 2014
//
// Modifications:
//
// ****************************************************************************

int simv2_connect_viewer(void *e, int argc, char *argv[])
{
    TRY
    {
        SimEngine *engine = (SimEngine*)(e);
        bool success = engine->ConnectViewer(&argc, &argv);
        if (!success)
        {
            return 0;
        }
        else
        {
            engine->SetUpViewerInterface(&argc, &argv);
            return 1;
        }
    }
    CATCHALL
    {
    }
    ENDTRY

    return 0;
}

// ****************************************************************************
// Method: simv2_get_descriptor
//
// Purpose:
//   SimV2 runtime function to get the engine input socket descriptor.
//
// Arguments:
//   e : The engine pointer.
//
// Returns:    The engine's input socket.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   way back
//
// Modifications:
//
// ****************************************************************************

int simv2_get_descriptor(void *e)
{
    SimEngine *engine = (SimEngine*)(e);
    return engine->GetInputSocket();
}

// ****************************************************************************
// Method: simv2_process_input
//
// Purpose:
//   SimV2 runtime function to process input for the engine (commands from the
//   viewer socket).
//
// Arguments:
//   e : The engine pointer.
//
// Returns:    1 on success, 0 on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   way back
//
// Modifications:
//
// ****************************************************************************

int simv2_process_input(void *e)
{
    SimEngine *engine = (SimEngine*)(e);

    TRY {
#ifdef PARALLEL
        engine->PAR_ProcessInput();
#else
        engine->ProcessInput();
#endif
    }
    CATCH (LostConnectionException)
    {
        // Lost connection to the viewer!
        CATCH_RETURN2(1, 0);
    }
    CATCH2(VisItException, e)
    {
        debug1 << "Caught a VisIt exception: " << e.Message() << endl;
    }
    CATCHALL
    {
        // Unknown processing error!  Ignoring....
        CATCH_RETURN2(1, 1);
    }
    ENDTRY

    return 1;
}

// ****************************************************************************
// Method: simv2_time_step_changed
//
// Purpose:
//   SimV2 runtime function called when the time step changes.
//
// Arguments:
//   e : The engine pointer.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   way back
//
// Modifications:
//
// ****************************************************************************

void simv2_time_step_changed(void *e)
{
    SimEngine *engine = (SimEngine*)(e);
    TRY
    {
        StackTimer t0("VisItTimestepChanged");
        engine->SimulationTimeStepChanged();
    }
    CATCHALL
    {
    }
    ENDTRY
}

// ****************************************************************************
// Method: simv2_execute_command
//
// Purpose:
//   SimV2 runtime function called when we want to execute a command.
//
// Arguments:
//   e : The engine pointer.
//   command : A command string.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   way back
//
// Modifications:
//
// ****************************************************************************

void simv2_execute_command(void *e, const char *command)
{
    TRY
    {
        if(command != NULL)
        {
            StackTimer t0("VisItExecuteCommand");
            SimEngine *engine = (SimEngine*)(e);       
            engine->SimulationInitiateCommand(command);
        }
    }
    CATCHALL
    {
    }
    ENDTRY
}

// ****************************************************************************
// Method: simv2_disconnect
//
// Purpose:
//   SimV2 runtime function called when we want to disconnect from the simulation.
//
// Programmer: Brad Whitlock
// Creation:   way back
//
// Modifications:
//
// ****************************************************************************

void simv2_disconnect()
{
    TRY
    {
        Engine::DisconnectSimulation();

        if (visitTimer)
            TimingsManager::Finalize();

        // KSB: This may be needed for memory leaks, but I've commented
        // it out for now, as it's use causes a segfault should VisIt
        // try to reconnect to the same simulation.
        //vtkVisItUtility::CleanupStaticVTKObjects();
        avtFileDescriptorManager::DeleteInstance();

        DataCallbacksCleanup();
    }
    CATCHALL
    {
    }
    ENDTRY
}

// ****************************************************************************
// Method: simv2_set_worker_process_callback
//
// Purpose:
//   SimV2 runtime function called when we want to install a worker process callback.
//
// Arguments:
//   spic : The new callback function.
//
// Returns:    
//
// Note:       The worker process callback helps broadcast commands from the
//             viewer to other ranks.
//
// Programmer: Brad Whitlock
// Creation:   way back
//
// Modifications:
//
// ****************************************************************************

void simv2_set_worker_process_callback(void(*spic)())
{
#ifdef PARALLEL
    MPIXfer::SetWorkerProcessInstructionCallback(spic);
#endif
}

// ****************************************************************************
// Method: simv2_set_command_callback
//
// Purpose:
//   SimV2 runtime function called when we want to install a callback to process
//   commands.
//
// Arguments:
//   e : The engine pointer.
//   sc : The command callback
//   scdata : The command callback data.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   way back
//
// Modifications:
//
// ****************************************************************************

void simv2_set_command_callback(void *e,void(*sc)(const char*,const char*,void*),
    void *scdata)
{
    SimEngine *engine = (SimEngine*)(e);
    engine->SetSimulationCommandCallback(sc, scdata);
}

// ****************************************************************************
// Method: simv2_debug_logs
//
// Purpose:
//   SimV2 runtime function that adds a message to the debug logs.
//
// Arguments:
//   level : The debug level.
//   msg   : The message to write.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 18:54:26 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
simv2_debug_logs(int level, const char *msg)
{
    if(level == 1)
    {
        debug1 << msg;
    }
    else if(level == 2)
    {
        debug2 << msg;
    }
    else if(level == 3)
    {
        debug3 << msg;
    }
    else if(level == 4)
    {
        debug4 << msg;
    }
    else if(level == 5)
    {
        debug5 << msg;
    }
}

// ****************************************************************************
// Method: simv2_set_mpicomm
//
// Purpose:
//   SimV2 runtime function that sets the MPI communicator.
//
// Arguments:
//   comm : The MPI communicator.
//
// Returns:    VISIT_OKAY, VISIT_ERROR
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 18:55:08 PDT 2014
//
// Modifications:
//
// ****************************************************************************

int
simv2_set_mpicomm(void *comm)
{
#ifdef PARALLEL
    return PAR_SetComm(comm) ? VISIT_OKAY : VISIT_ERROR;
#else
    return VISIT_ERROR;
#endif
}

// ****************************************************************************
// Method: simv2_set_mpicomm_f
//
// Purpose:
//   SimV2 runtime function that sets the MPI communicator from Fortran which
//   uses an integer communicator handle.
//
// Arguments:
//   comm : The Fortran MPI communicator handle (integer).
//
// Returns:    VISIT_OKAY, VISIT_ERROR
//
// Note:       
//
// Programmer: William T. Jones
// Creation:   Wed Sep 4 10:27:03 PDT 2013
//
// Modifications:
//
// ****************************************************************************

int
simv2_set_mpicomm_f(int *comm)
{
#ifdef PARALLEL
    MPI_Fint *commF = (MPI_Fint*)comm;
    static MPI_Comm commC = MPI_Comm_f2c(*commF);
    return PAR_SetComm((void*)&commC) ? VISIT_OKAY : VISIT_ERROR;
#else
    return VISIT_ERROR;
#endif
}

///////////////////////////////////////////////////////////////////////////////
/// THESE FUNCTIONS ARE MORE EXPERIMENTAL
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: simv2_save_window
//
// Purpose:
//   SimV2 runtime function called when we want to save a window.
//
// Arguments:
//   e : The engine pointer.
//   filename : The filename to save to.
//   w : The image width
//   h : The image height
//   format : The image format.
//
// Returns:    VISIT_OKAY or VISIT_ERROR.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 18:52:03 PDT 2014
//
// Modifications:
//
// ****************************************************************************

int
simv2_save_window(void *e, const char *filename, int w, int h, int format)
{
    StackTimer t0("VisItSaveWindow");
    SimEngine *engine = (SimEngine*)(e);
    return engine->SaveWindow(filename, w, h, format) ?
           VISIT_OKAY : VISIT_ERROR;
}

// ****************************************************************************
// Method: simv2_add_plot
//
// Purpose:
//   SimV2 runtime function called when we want to add a plot
//
// Arguments:
//   e        : The engine pointer.
//   plotType : The plot type.
//   var      : The plot variable.
//
// Returns:    VISIT_OKAY or VISIT_ERROR.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 18:52:03 PDT 2014
//
// Modifications:
//
// ****************************************************************************

int
simv2_add_plot(void *e, const char *plotType, const char *var)
{
    StackTimer t0("VisItAddPlot");
    SimEngine *engine = (SimEngine*)(e); 
    return engine->AddPlot(plotType, var) ? VISIT_OKAY : VISIT_ERROR;
}

// ****************************************************************************
// Method: simv2_add_operator
//
// Purpose:
//   SimV2 runtime function called when we want to add a operator
//
// Arguments:
//   e            : The engine pointer.
//   operatorType : The operator type.
//   applyToAll   : Whether to apply the operator to all plots.
//
// Returns:    VISIT_OKAY or VISIT_ERROR.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 18:52:03 PDT 2014
//
// Modifications:
//
// ****************************************************************************

int
simv2_add_operator(void *e, const char *operatorType, int applyToAll)
{
    StackTimer t0("VisItAddOperator");
    SimEngine *engine = (SimEngine*)(e); 
    return engine->AddOperator(operatorType, applyToAll!=0) ? VISIT_OKAY : VISIT_ERROR;
}

// ****************************************************************************
// Method: simv2_draw_plots
//
// Purpose:
//   SimV2 runtime function called when we want to draw a plot.
//
// Arguments:
//   e : The engine pointer.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 18:03:28 PDT 2014
//
// Modifications:
//
// ****************************************************************************

int
simv2_draw_plots(void *e)
{
    SimEngine *engine = (SimEngine*)(e);
    StackTimer t0("VisItDrawPlots");
    return engine->DrawPlots() ? VISIT_OKAY : VISIT_ERROR;
}

// ****************************************************************************
// Method: simv2_delete_active_plots
//
// Purpose:
//   SimV2 runtime function called when we want to delete the active plots.
//
// Arguments:
//   e : The engine pointer.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 18:03:28 PDT 2014
//
// Modifications:
//
// ****************************************************************************

int
simv2_delete_active_plots(void *e)
{
    SimEngine *engine = (SimEngine*)(e);
    StackTimer t0("VisItDeleteActivePlots");
    return engine->DeleteActivePlots() ? VISIT_OKAY : VISIT_ERROR;
}

// ****************************************************************************
// Method: simv2_set_active_plots
//
// Purpose:
//   SimV2 function to set the active plots.
//
// Arguments:
//   e    : The engine pointer.
//   ids  : The list of plot ids.
//   nids : The number of plot ids.
//
// Returns:    OKAY on success, FALSE on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb  2 13:57:29 PST 2015
//
// Modifications:
//
// ****************************************************************************

int
simv2_set_active_plots(void *e, const int *ids, int nids)
{
    SimEngine *engine = (SimEngine*)(e);
    StackTimer t0("VisItSetActivePlots");
    return engine->SetActivePlots(ids, nids) ? VISIT_OKAY : VISIT_ERROR;
}

// ****************************************************************************
// Method: simv2_change_plot_var
//
// Purpose:
//   SimV2 function to change the variable on plots.
//
// Arguments:
//   e    : The engine pointer.
//   var  : The new variable.
//   all  : Whether to do it on all plots or just the selected plots.
//
// Returns:    OKAY on success, FALSE on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb  2 13:57:29 PST 2015
//
// Modifications:
//
// ****************************************************************************

int
simv2_change_plot_var(void *e, const char *var, int all)
{
    SimEngine *engine = (SimEngine*)(e);
    StackTimer t0("VisItChangePlotVar");
    return engine->ChangePlotVar(var, all) ? VISIT_OKAY : VISIT_ERROR;
}

// ****************************************************************************
// Method: simv2_set_plot_options
//
// Purpose:
//   SimV2 function to set plot options.
//
// Arguments:
//   e : The engine pointer.
//   fieldName : The name of the field to set.
//   fieldType : The type of the data we're passing in.
//   fieldVal  : A pointer to the field data we're passing in.
//   fieldLen  : The length of the field data (if it is an array).
//
// Returns:    OKAY on success, FALSE on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb  2 14:07:11 PST 2015
//
// Modifications:
//
// ****************************************************************************

int
simv2_set_plot_options(void *e, const char *fieldName, 
    int fieldType, void *fieldVal, int fieldLen)
{
    SimEngine *engine = (SimEngine*)(e);
    StackTimer t0("VisItSetPlotOptions");
    return engine->SetPlotOptions(fieldName, fieldType, fieldVal, fieldLen) ? VISIT_OKAY : VISIT_ERROR;
}

// ****************************************************************************
// Method: simv2_set_operator_options
//
// Purpose:
//   SimV2 function to set operator options.
//
// Arguments:
//   e : The engine pointer.
//   fieldName : The name of the field to set.
//   fieldType : The type of the data we're passing in.
//   fieldVal  : A pointer to the field data we're passing in.
//   fieldLen  : The length of the field data (if it is an array).
//
// Returns:    OKAY on success, FALSE on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb  2 14:07:11 PST 2015
//
// Modifications:
//
// ****************************************************************************

int
simv2_set_operator_options(void *e, 
    const char *fieldName, int fieldType, void *fieldVal, int fieldLen)
{
    SimEngine *engine = (SimEngine*)(e);
    StackTimer t0("VisItSetOperatorOptions");
    return engine->SetOperatorOptions(fieldName, fieldType, fieldVal, fieldLen) ? VISIT_OKAY : VISIT_ERROR;
}

// ****************************************************************************
// Method: simv2_exportdatabase
//
// Purpose:
//   SimV2 runtime function called when we want to execute a command.
//
// Arguments:
//   e        : The engine pointer.
//   filename : The filename to export.
//   format   : The export format.
//   names    : The list of variables to export.
//   options  : The optional options to use when exporting.
//   
// Returns:    VISIT_OKAY on success; VISIT_ERROR on failure.
//
// Note:       EXPERIMENTAL
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 10:53:32 PDT 2014
//
// Modifications:
//    Kathleen Biagas, Fri Sep 10 09:14:56 PDT 2021
//    Add support for VISIT_DATATYPE_ENUM.
//
// ****************************************************************************

int
simv2_exportdatabase_with_options(void *e, const char *filename, const char *format, 
    visit_handle names, visit_handle options)
{
    StackTimer t0("VisItExportDatabase");

    int status = VISIT_ERROR;
    stringVector varNames;
    int n;
    if(simv2_NameList_getNumName(names, &n) == VISIT_OKAY)
    {
        for(int i = 0; i < n; ++i)
        {
            char *var = NULL;
            if(simv2_NameList_getName(names, i, &var) == VISIT_OKAY)
            {
                if(var != NULL)
                {
                    varNames.push_back(var);
                    free(var); // simv2_NameList_getName used malloc.
                }
            }
        }
    }
    else
    {
        varNames.push_back("default");
    }

    // Turn the option list into a DBOptionsAttributes.
    DBOptionsAttributes opt;
    if(options != VISIT_INVALID_HANDLE)
    {
        int nvalues = 0;
        if(simv2_OptionList_getNumValues(options, &nvalues) == VISIT_OKAY &&
           nvalues > 0)
        {
            for(int i = 0; i < nvalues; ++i)
            {
                char *name = NULL;
                int type;
                if(simv2_OptionList_getName(options, i, &name) == VISIT_OKAY &&
                   simv2_OptionList_getType(options, i, &type) == VISIT_OKAY)
                {
                    void *pvalue = NULL;
                    if(simv2_OptionList_getValue(options, i, &pvalue) == VISIT_OKAY)
                    {
                        switch(type)
                        {
                        case VISIT_DATATYPE_CHAR:
                            opt.SetBool(name, *((unsigned char *)pvalue) != 0);
                            break;
                        case VISIT_DATATYPE_INT:
                            opt.SetInt(name, *((int *)pvalue));
                            break;
                        case VISIT_DATATYPE_FLOAT:
                            opt.SetFloat(name, *((float *)pvalue));
                            break;
                        case VISIT_DATATYPE_DOUBLE:
                            opt.SetDouble(name, *((double *)pvalue));
                            break;
                        case VISIT_DATATYPE_STRING:
                            opt.SetString(name, std::string((const char *)pvalue));
                            break;
                        case VISIT_DATATYPE_ENUM:
                            opt.SetEnum(name, *((int *)pvalue));
                            break;
                        }
                    }
                }
                
                if(name != NULL)
                    free(name);
            }
        }
    }

    SimEngine *engine = (SimEngine*)e;
    if(engine->ExportDatabase(filename, format, varNames, opt))
        status = VISIT_OKAY;

    return status;
}

// Left in for compatibility
int
simv2_exportdatabase(void *e, const char *filename, const char *format, 
    visit_handle names, visit_handle options)
{
    return simv2_exportdatabase_with_options(e, filename, format, names, VISIT_INVALID_HANDLE);
}

// ****************************************************************************
// Method: simv2_restoresession
//
// Purpose:
//   SimV2 runtime function called when we want to restore a session.
//
// Arguments:
//   e : The engine pointer.
//   filename : The session filename.
//
// Returns:    
//
// Note:       EXPERIMENTAL
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 10:53:32 PDT 2014
//
// Modifications:
//
// ****************************************************************************

int
simv2_restoresession(void *e, const char *filename)
{
    StackTimer t0("VisItRestoreSession");

    SimEngine *engine = (SimEngine*)e;
    return engine->RestoreSession(filename) ? VISIT_OKAY : VISIT_ERROR;
}

// ****************************************************************************
// Method: simv2_set_view2D
//
// Purpose:
//   Sets the view.
//
// Arguments:
//   e : The engine pointer.
//   v : A handle to a VisIt_View2D object.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  1 17:22:28 PDT 2017
//
// Modifications:
//
// ****************************************************************************

int
simv2_set_view2D(void *e, visit_handle v)
{
    StackTimer t0("VisItSetView2D");

    int retval = VISIT_ERROR;
    View2DAttributes *atts = simv2_View2D_GetAttributes(v);
    if(atts != NULL)
    {
        SimEngine *engine = (SimEngine*)e;
        retval = engine->SetView2D(*atts) ? VISIT_OKAY : VISIT_ERROR;
    }
    return retval;
}

int
simv2_get_view2D(void *e, visit_handle v)
{
    int retval = VISIT_ERROR;
    View2DAttributes *atts = simv2_View2D_GetAttributes(v);
    if(atts != NULL)
    {
        SimEngine *engine = (SimEngine*)e;
        retval = engine->GetView2D(*atts) ? VISIT_OKAY : VISIT_ERROR;
    }
    return retval;
}

// ****************************************************************************
// Method: simv2_set_view3D
//
// Purpose:
//   Sets the view.
//
// Arguments:
//   e : The engine pointer.
//   v : A handle to a VisIt_View3D object.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  1 17:22:28 PDT 2017
//
// Modifications:
//
// ****************************************************************************

int
simv2_set_view3D(void *e, visit_handle v)
{
    StackTimer t0("VisItSetView3D");

    int retval = VISIT_ERROR;
    View3DAttributes *atts = simv2_View3D_GetAttributes(v);
    if(atts != NULL)
    {
        SimEngine *engine = (SimEngine*)e;
        retval = engine->SetView3D(*atts) ? VISIT_OKAY : VISIT_ERROR;
    }
    return retval;
}

int
simv2_get_view3D(void *e, visit_handle v)
{
    int retval = VISIT_ERROR;
    View3DAttributes *atts = simv2_View3D_GetAttributes(v);
    if(atts != NULL)
    {
        SimEngine *engine = (SimEngine*)e;
        retval = engine->GetView3D(*atts) ? VISIT_OKAY : VISIT_ERROR;
    }
    return retval;
}
