/*****************************************************************************
*
* Copyright (c) 2000 - 2013, Lawrence Livermore National Security, LLC
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

#include <Engine.h>
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

extern void DataCallbacksCleanup(void);

//
// Data that helps us keep track of plot and operator attributes.
//
struct OperatorInformation
{
    std::string       id;
    AttributeSubject *atts;
};

struct PlotInformation
{
    std::string                      id;
    std::string                      plotName;
    std::string                      plotDB;
    std::string                      plotVar;
    int                              network;
    AttributeSubject                *atts;
    int                              windowID;
    std::vector<OperatorInformation> operators;
};

static std::map<int,PlotInformation> *plotInformation = NULL;

static std::map<int,PlotInformation> *
GetPlotInformation()
{
    if(plotInformation == NULL)
        plotInformation = new std::map<int,PlotInformation>();
    return plotInformation;
}

// ****************************************************************************
//  Library:  VisItEngine, Version 2
//
//  Purpose:
//    Wrapper for simulations to control an engine.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 25, 2004
//
//  Modifications:
//
// ****************************************************************************

void *simv2_get_engine()
{
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

    Engine *engine = Engine::Instance();
    engine->EnableSimulationPlugins();
    return (void*)engine;
}

int simv2_initialize(void *e, int argc, char *argv[])
{
    int retval = 1;
    Engine *engine = (Engine*)(e);
    TRY
    {
        engine->Initialize(&argc, &argv, false);
        engine->InitializeCompute();
        LoadBalancer::SetScheme(LOAD_BALANCE_RESTRICTED);
    }
    CATCHALL
    {
        retval = 0;
    }
    ENDTRY
    return 1;
}

int simv2_connect_viewer(void *e, int argc, char *argv[])
{
    TRY
    {
        Engine *engine = (Engine*)(e);
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

int simv2_get_descriptor(void *e)
{
    Engine *engine = (Engine*)(e);
    return engine->GetInputSocket();
}

#include <DebugStream.h>

int simv2_process_input(void *e)
{
    Engine *engine = (Engine*)(e);

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

void simv2_time_step_changed(void *e)
{
    Engine *engine = (Engine*)(e);
    TRY
    {
        engine->SimulationTimeStepChanged();
    }
    CATCHALL
    {
    }
    ENDTRY
}

void simv2_execute_command(void *e, const char *command)
{
    TRY
    {
        if(command != NULL)
        {
            Engine *engine = (Engine*)(e);       
            engine->SimulationInitiateCommand(command);
        }
    }
    CATCHALL
    {
    }
    ENDTRY
}

void simv2_disconnect()
{
    TRY
    {
        Engine::DisconnectSimulation();
        if (visitTimer)
            TimingsManager::Finalize();
        DataCallbacksCleanup();
    }
    CATCHALL
    {
    }
    ENDTRY
}

void simv2_set_slave_process_callback(void(*spic)())
{
#ifdef PARALLEL
    MPIXfer::SetSlaveProcessInstructionCallback(spic);
#endif
}

void simv2_set_command_callback(void *e,void(*sc)(const char*,const char*,void*),
    void *scdata)
{
    Engine *engine = (Engine*)(e);
    engine->SetSimulationCommandCallback(sc, scdata);
}

int
simv2_save_window(void *e, const char *filename, int w, int h, int format)
{
    Engine *engine = (Engine*)(e);
    int retval = VISIT_OKAY;
    TRY
    {
        SaveWindowAttributes::FileFormat fmt;
        if(format == VISIT_IMAGEFORMAT_BMP)
            fmt = SaveWindowAttributes::BMP;
        else if(format == VISIT_IMAGEFORMAT_JPEG)
            fmt = SaveWindowAttributes::JPEG;
        else if(format == VISIT_IMAGEFORMAT_PNG)
            fmt = SaveWindowAttributes::PNG;
        else if(format == VISIT_IMAGEFORMAT_POVRAY)
            fmt = SaveWindowAttributes::POVRAY;
        else if(format == VISIT_IMAGEFORMAT_PPM)
            fmt = SaveWindowAttributes::PPM;
        else if(format == VISIT_IMAGEFORMAT_RGB)
            fmt = SaveWindowAttributes::RGB;
        else
            fmt = SaveWindowAttributes::TIFF;
#if 1
        intVector networkIds;
        std::map<int, PlotInformation>::iterator it = GetPlotInformation()->begin();
        for(; it != GetPlotInformation()->end(); ++it)
        {
            debug1 << "Plot network: " << it->second.network << endl;
            networkIds.push_back(it->second.network);
        }

        retval =  engine->GetNetMgr()->SaveWindow(networkIds, filename, w, h, fmt) ?
            VISIT_OKAY : VISIT_ERROR;
#else
        retval =  engine->SaveWindow(filename, w, h, fmt) ?
            VISIT_OKAY : VISIT_ERROR;
#endif
    }
    CATCHALL
    {
        retval = VISIT_ERROR;
    }
    ENDTRY

    return retval;
}

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

int
simv2_set_mpicomm(void *comm)
{
#ifdef PARALLEL
    return PAR_SetComm(comm) ? VISIT_OKAY : VISIT_ERROR;
#else
    return VISIT_ERROR;
#endif
}

int
simv2_add_plot(void *e, const char *simfile, const char *plotType, const char *var, int *plotID)
{
    static int plotCounter = 0;
    const char *mName = "simv2_add_plot: ";

    Engine *engine = (Engine*)(e);
    NetworkManager *netmgr = engine->GetNetMgr();
    int status = VISIT_ERROR;

    *plotID = -1;
    TRY
    {
        int windowID = 0;

        // Force us to get new metadata, thus setting Engine::filename so 
        // Engine::SimulationTimeStepChanged works later.
        engine->PopulateSimulationMetaData(simfile, "SimV2_1.0");

        // PreparePlotRPC
        std::string id(plotType);
        if (!netmgr->GetPlotPluginManager()->PluginAvailable(id))
        {
            debug1 << mName << "Requested plot does not exist for the engine" << endl;
            CATCH_RETURN2(1, VISIT_ERROR);
        }
        AttributeSubject *atts = netmgr->GetPlotPluginManager()->
            GetEnginePluginInfo(id)->AllocAttributes();

        // Save the plot information.
        if(atts != NULL)
        {
            char plotName[10];
            SNPRINTF(plotName, 10, "Plot%04d", plotCounter);
            PlotInformation p;
            p.id = id;
            p.plotName = plotName;
            p.plotDB = simfile;           
            p.plotVar = var;
            p.network = -1;
            p.atts = atts;
            p.windowID = windowID;
            GetPlotInformation()->operator[](plotCounter) = p;

            debug3 << mName << "Added new " << plotType << " plot of " << var
                   << ". plotID=" << plotCounter << endl;

            *plotID = plotCounter;
            plotCounter++;
        }

        status = VISIT_OKAY;
    }
    CATCHALL
    {
        debug3 << mName << "Cancelling network" << endl;
        netmgr->CancelNetwork();
    }
    ENDTRY

    return status;
}

int
simv2_add_operator(void *e, int plotID, const char *operatorType, int *operatorID)
{
    Engine *engine = (Engine*)(e);
    NetworkManager *netmgr = engine->GetNetMgr();
    int status = VISIT_ERROR;
    TRY
    {
        std::map<int, PlotInformation>::iterator it = GetPlotInformation()->find(plotID);
        if(it != GetPlotInformation()->end())
        {
            std::string id(operatorType);

            if (!netmgr->GetOperatorPluginManager()->PluginAvailable(id))
            {
                debug1 << "Requested operator does not exist for the engine" << endl;
                CATCH_RETURN2(1, VISIT_ERROR);
            }

            AttributeSubject *atts = netmgr->GetOperatorPluginManager()->
                GetEnginePluginInfo(id)->AllocAttributes();
            if(atts != NULL)
            {
                OperatorInformation op;
                op.id = id;
                op.atts = atts;
                it->second.operators.push_back(op);
                *operatorID = it->second.operators.size()-1;

                debug3 << "Added new " << operatorType << " operator to plot " << it->second.id << endl;
            }
            else
                *operatorID = -1;

            status = VISIT_OKAY;
        }
    }
    CATCHALL
    {
        status = VISIT_ERROR;
    }
    ENDTRY

    return status;
}

int
simv2_draw_plot(void *e, int plotID)
{
    const char *mName = "simv2_draw_plot: ";
    Engine *engine = (Engine*)(e);
    NetworkManager *netmgr = engine->GetNetMgr();
    int status = VISIT_ERROR;
    TRY
    {
        std::map<int, PlotInformation>::iterator it = GetPlotInformation()->find(plotID);
        if(it != GetPlotInformation()->end())
        {
            if(true) //it->second.network == -1)
            {
                // ReadRPC
                debug3 << mName << "Starting new network for file=" 
                       << it->second.plotDB << " var=" << it->second.plotVar << endl;
                netmgr->StartNetwork("SimV2_1.0", 
                    it->second.plotDB, it->second.plotVar, 0);

                // Add the operators.
                for(size_t i = 0; i < it->second.operators.size(); ++i)
                {
                    debug3 << mName << "Adding " << it->second.operators[i].id << " operator to the network" << endl;
                    netmgr->AddFilter(it->second.operators[i].id, it->second.operators[i].atts);
                }

                // MakePlotRPC
                debug3 << mName << "Adding " << it->second.id << " plot to the network." << endl;
                std::vector<double> dataExtents;
                netmgr->MakePlot(it->second.plotName, it->second.id, it->second.atts, dataExtents);
        
                // End network.
                it->second.network = netmgr->EndNetwork(it->second.windowID);
                debug3 << mName << "Ending network. id = " << it->second.network << endl;
                status = (it->second.network != -1) ? VISIT_OKAY : VISIT_ERROR;
            }
            else
            {
                debug3 << mName << "Use network. id=" << it->second.network << endl;
                netmgr->UseNetwork(it->second.network);

                // Update the plot atts.
                debug3 << mName << "Updating plot attributes" << endl;
                netmgr->UpdatePlotAtts(it->second.network, it->second.atts);

                status = VISIT_OKAY;
            }

            // Set up some callback functions
            avtDataObjectSource::RegisterProgressCallback(
                Engine::EngineUpdateProgressCallback, NULL);
            LoadBalancer::RegisterProgressCallback(
                Engine::EngineUpdateProgressCallback, NULL);
            avtOriginatingSource::RegisterInitializeProgressCallback(
                Engine::EngineInitializeProgressCallback, NULL);

            debug3 << mName << "Get network manager output" << endl;

            // Try and force the network to execute so it will be around for when
            // we render.
            float cellCountMultiplier = 0.f;
            netmgr->GetOutput(false, false, &cellCountMultiplier);
        }
    }
    CATCHALL
    {
        debug3 << mName << "Cancelling network" << endl;
        netmgr->CancelNetwork();
        status = VISIT_ERROR;
    }
    ENDTRY

    return status;
}

int
simv2_delete_plot(void *e, int plotID)
{
    Engine *engine = (Engine*)(e);
    NetworkManager *netmgr = engine->GetNetMgr();
    std::map<int, PlotInformation>::iterator it = GetPlotInformation()->find(plotID);
    if(it != GetPlotInformation()->end())
    {
        if(it->second.network != -1)
            netmgr->DoneWithNetwork(it->second.network);

        // Delete the plot information.
        for(size_t i = 0; i < it->second.operators.size(); ++i)
            delete it->second.operators[i].atts;
        delete it->second.atts;
        GetPlotInformation()->erase(it);
    }
    return VISIT_OKAY;
}

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

int
simv2_set_plot_options(void * /*e*/, int plotID, const char *fieldName, 
    int fieldType, void *fieldVal, int fieldLen)
{
    int status = VISIT_ERROR;
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

    return status;
}

int
simv2_set_operator_options(void * /*e*/, int plotID, int operatorID, const char *fieldName, 
    int fieldType, void *fieldVal, int fieldLen)
{
    int status = VISIT_ERROR;
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

    return status;
}
