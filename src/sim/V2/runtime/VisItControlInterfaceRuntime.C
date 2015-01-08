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

#include "VisItControlInterfaceRuntime.h"
#include <VisItInterfaceTypes_V2.h>
#include <VisItInterfaceTypes_V2P.h>

#include <DebugStream.h>
#include <SimEngine.h>
#include <NetworkManager.h>
#include <LostConnectionException.h>
#include <LoadBalancer.h>
#ifdef PARALLEL
#include <MPIXfer.h>
#include <avtParallel.h>
#endif
#include <AttributeGroup.h>
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

#include <vtkVisItUtility.h>
#include <avtDatabaseFactory.h>
#include <avtFileDescriptorManager.h>

#include <simv2_NameList.h>


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
            engine->InitializeViewer();
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
// Method: simv2_set_slave_process_callback
//
// Purpose:
//   SimV2 runtime function called when we want to install a slave process callback.
//
// Arguments:
//   spic : The new callback function.
//
// Returns:    
//
// Note:       The slave process callback helps broadcast commands from the
//             viewer to other ranks.
//
// Programmer: Brad Whitlock
// Creation:   way back
//
// Modifications:
//
// ****************************************************************************

void simv2_set_slave_process_callback(void(*spic)())
{
#ifdef PARALLEL
    MPIXfer::SetSlaveProcessInstructionCallback(spic);
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
    SimEngine *engine = (SimEngine*)(e); 
    return engine->AddOperator(operatorType, applyToAll) ? VISIT_OKAY : VISIT_ERROR;
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
    return engine->DeleteActivePlots() ? VISIT_OKAY : VISIT_ERROR;
}

#if 0
// We don't need this stuff just yet...

template <class T>
static std::vector<T> makevector(const T *val, int nval)
{
    std::vector<T> vec;
    vec.reserve(nval);
    for(int i = 0; i < nval; ++i)
        vec.push_back(val[i]);
    return vec;
}

static int 
SetAttributeSubjectValues(AttributeSubject *atts, 
    const std::string &name, int fieldType, void *fieldVal, int fieldLen)
{
    int status = VISIT_OKAY;
    int fIndex = atts->FieldNameToIndex(name);
    if(fIndex < 0)
        return VISIT_ERROR;
    AttributeGroup::FieldType ft = atts->GetFieldType(fIndex);

    if(fieldType == VISIT_FIELDTYPE_CHAR)
    {
        const char *val = (const char *)fieldVal;
        if(ft == AttributeGroup::FieldType_bool)
            atts->SetValue(name, *val > 0);
        else
            atts->SetValue(name, *val);
    }
    else if(fieldType == VISIT_FIELDTYPE_UNSIGNED_CHAR)
    {
        const unsigned char *val = (const unsigned char *)fieldVal;
        if(ft == AttributeGroup::FieldType_uchar)
            atts->SetValue(name, *val);
        else if(ft == AttributeGroup::FieldType_bool)
            atts->SetValue(name, *val > 0);
        else
            status = VISIT_ERROR;
    }
    else if(fieldType == VISIT_FIELDTYPE_INT)
    {
        const int *val = (const int *)fieldVal;
        if(ft == AttributeGroup::FieldType_bool)
            atts->SetValue(name, *val > 0);
//        else if(ft == AttributeGroup::FieldType_long)
//            atts->SetValue(name, long(*val));
        else if(ft == AttributeGroup::FieldType_float)
            atts->SetValue(name, float(*val));
        else if(ft == AttributeGroup::FieldType_double)
            atts->SetValue(name, double(*val));
        else
            atts->SetValue(name, *val);
    }
    else if(fieldType == VISIT_FIELDTYPE_LONG)
    {
        const long *val = (const long *)fieldVal;
        if(ft == AttributeGroup::FieldType_bool)
            atts->SetValue(name, *val > 0);
        else if(ft == AttributeGroup::FieldType_int)
            atts->SetValue(name, int(*val));
        else if(ft == AttributeGroup::FieldType_float)
            atts->SetValue(name, float(*val));
        else if(ft == AttributeGroup::FieldType_double)
            atts->SetValue(name, double(*val));
        else
            atts->SetValue(name, *val);
    }
    else if(fieldType == VISIT_FIELDTYPE_FLOAT)
    {
        const float *val = (const float *)fieldVal;
        if(ft == AttributeGroup::FieldType_bool)
            atts->SetValue(name, *val > 0);
        else if(ft == AttributeGroup::FieldType_int)
            atts->SetValue(name, int(*val));
//        else if(ft == AttributeGroup::FieldType_long)
//            atts->SetValue(name, long(*val));
        else if(ft == AttributeGroup::FieldType_double)
            atts->SetValue(name, double(*val));
        else
            atts->SetValue(name, *val);
    }
    else if(fieldType == VISIT_FIELDTYPE_LONG)
    {
        const double *val = (const double *)fieldVal;
        if(ft == AttributeGroup::FieldType_bool)
            atts->SetValue(name, *val > 0);
        else if(ft == AttributeGroup::FieldType_int)
            atts->SetValue(name, int(*val));
        else if(ft == AttributeGroup::FieldType_float)
            atts->SetValue(name, float(*val));
        else if(ft == AttributeGroup::FieldType_double)
            atts->SetValue(name, *val);
        else
            atts->SetValue(name, long(*val));
    }
    else if(fieldType == VISIT_FIELDTYPE_STRING)
    {
        std::string val((const char *)fieldVal);
        atts->SetValue(name, val);
    }
    // Array and vector
    if(fieldType == VISIT_FIELDTYPE_CHAR_ARRAY)
    {
        const char *val = (const char *)fieldVal;
        atts->SetValue(name, val, fieldLen);
    }
    else if(fieldType == VISIT_FIELDTYPE_UNSIGNED_CHAR_ARRAY)
    {
        const unsigned char *val = (const unsigned char *)fieldVal;
        if(ft == AttributeGroup::FieldType_ucharVector)
            atts->SetValue(name, makevector(val, fieldLen));
        else
            atts->SetValue(name, val, fieldLen);
    }
    else if(fieldType == VISIT_FIELDTYPE_INT_ARRAY)
    {
        const int *val = (const int *)fieldVal;
        if(ft == AttributeGroup::FieldType_intVector)
            atts->SetValue(name, makevector(val, fieldLen));
        else
            atts->SetValue(name, val, fieldLen);
    }
    else if(fieldType == VISIT_FIELDTYPE_LONG_ARRAY)
    {
        const long *val = (const long *)fieldVal;
//        if(ft == AttributeGroup::FieldType_longVector)
//            atts->SetValue(name, makevector(val, fieldLen));
//        else
            atts->SetValue(name, val, fieldLen);
    }
    else if(fieldType == VISIT_FIELDTYPE_FLOAT_ARRAY)
    {
        const float *val = (const float *)fieldVal;
//        if(ft == AttributeGroup::FieldType_floatVector)
//            atts->SetValue(name, makevector(val, fieldLen));
//        else
            atts->SetValue(name, val, fieldLen);
    }
    else if(fieldType == VISIT_FIELDTYPE_DOUBLE_ARRAY)
    {
        const double *val = (const double *)fieldVal;
        if(ft == AttributeGroup::FieldType_doubleVector)
            atts->SetValue(name, makevector(val, fieldLen));
        else
            atts->SetValue(name, val, fieldLen);
    }
    else if(fieldType == VISIT_FIELDTYPE_STRING_ARRAY)
    {
        const char **val = (const char **)fieldVal;
        stringVector s;
        s.resize(fieldLen);
        for(int i = 0; i < fieldLen; ++i)
            s[i] = std::string(val[i]);        
        atts->SetValue(name, &s[0], fieldLen);
    }
    else
        status = VISIT_ERROR;

    return status;
}
#endif

int
simv2_set_plot_options(void * /*e*/, int plotID, const char *fieldName, 
    int fieldType, void *fieldVal, int fieldLen)
{
    int status = VISIT_ERROR;
#if 0
    TRY
    {
        std::map<int, PlotInformation>::iterator it = GetPlotInformation()->find(plotID);
        if(it != GetPlotInformation()->end() && fieldVal != NULL)
        {
            status = SetAttributeSubjectValues(it->second.atts, std::string(fieldName), fieldType, fieldVal, fieldLen);
        }
    }
    CATCHALL
    {
    }
    ENDTRY
#endif
    return status;
}

int
simv2_set_operator_options(void * /*e*/, int plotID, int operatorID, const char *fieldName, 
    int fieldType, void *fieldVal, int fieldLen)
{
    int status = VISIT_ERROR;
#if 0
    TRY
    {
        std::map<int, PlotInformation>::iterator it = GetPlotInformation()->find(plotID);
        if(it != GetPlotInformation()->end() && fieldVal != NULL)
        {
            if(operatorID >= 0 && operatorID < static_cast<int>(it->second.operators.size()))
            {
                status = SetAttributeSubjectValues(it->second.operators[operatorID].atts, 
                    std::string(fieldName), fieldType, fieldVal, fieldLen); 
            }
        }
    }
    CATCHALL
    {
    }
    ENDTRY
#endif
    return status;
}

// ****************************************************************************
// Method: simv2_exportdatabase
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
// Note:       EXPERIMENTAL
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 10:53:32 PDT 2014
//
// Modifications:
//
// ****************************************************************************

int
simv2_exportdatabase(void *e, const char *filename, const char *format, 
    visit_handle names)
{
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
                     varNames.push_back(var);
            }
        }
    }
    else
    {
        varNames.push_back("default");
    }

    SimEngine *engine = (SimEngine*)e;
    if(engine->ExportDatabase(filename, format, varNames))
        status = VISIT_OKAY;

    return status;
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
    SimEngine *engine = (SimEngine*)e;
    return engine->RestoreSession(filename) ? VISIT_OKAY : VISIT_ERROR;
}

