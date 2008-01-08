/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <ViewerProxy.h>
#include <ViewerRPC.h>
#include <PlotPluginManager.h>
#include <PlotPluginInfo.h>
#include <OperatorPluginManager.h>
#include <OperatorPluginInfo.h>
#include <DebugStream.h>
#include <snprintf.h>
#include <visit-config.h>

#include <PyAnnotationAttributes.h>
#include <PyConstructDDFAttributes.h>
#include <PyExportDBAttributes.h>
#include <PyGlobalLineoutAttributes.h>
#include <PyHostProfile.h>
#include <PyInteractorAttributes.h>
#include <PyKeyframeAttributes.h>
#include <PyLightAttributes.h>
#include <PyMaterialAttributes.h>
#include <PyMeshManagementAttributes.h>
#include <PyPickAttributes.h>
#include <PyPrinterAttributes.h>
#include <PyProcessAttributes.h>
#include <PyRenderingAttributes.h>
#include <PySaveWindowAttributes.h>
#include <PySILRestriction.h>
#include <PyQueryOverTimeAttributes.h>
#include <PyViewCurveAttributes.h>
#include <PyView2DAttributes.h>
#include <PyView3DAttributes.h>
#include <PyWindowInformation.h>

#include <avtSILRestriction.h>
#include <avtSILRestrictionTraverser.h>

#include <AnimationAttributes.h>
#include <Expression.h>
#include <ExpressionList.h>
#include <GlobalAttributes.h>
#include <LightAttributes.h>
#include <LightList.h>
#include <WindowInformation.h>

#define SLEN 5000

//
// Extern data
//
extern ViewerProxy *viewer;

//
// Static data
//
static std::string  macroString;
static bool         macroRecord = false;
static FILE        *logFile;
static bool         logging = false;
static int          logLevel = 0;

//
// Macro functions
//

void
Macro_SetString(const std::string &s)
{
    macroString = s;
}

const std::string &
Macro_GetString()
{
    return macroString;
}

void
Macro_SetRecord(bool val)
{
    macroRecord = val;
}

bool
Macro_GetRecord()
{
    return macroRecord;
}

//
// Log file functions
//

bool
LogFile_Open(const char *filename)
{
    logging = true;
    logFile = fopen(filename, "wb");
    if(logFile)
    {
        fprintf(logFile, "# Visit %s log file\n", VERSION);
        fprintf(logFile, "ScriptVersion = \"%s\"\n", VERSION);
        fprintf(logFile, "if ScriptVersion != Version():\n");
        fprintf(logFile, "    print \"This script is for VisIt %%s. "
                "It may not work with version %%s\" %% "
                "(ScriptVersion, Version())\n");
    }

    return logging;
}

void
LogFile_Close()
{
    if(logFile)
        fclose(logFile);
}

void
LogFile_SetEnabled(bool val)
{
    logging = val;
}

bool
LogFile_GetEnabled()
{
    return logging;
}

void
LogFile_IncreaseLevel()
{
    ++logLevel;
}

void
LogFile_DecreaseLevel()
{
    --logLevel;
    if(logLevel < 0)
       logLevel = 0;
}

void
LogFile_Write(const char *str)
{
    if(logging && logLevel == 0)
    {
        // Add the string to the macro string
        if(macroRecord)
            macroString += str;

        // Write to the log
        if(logFile != NULL)
            fprintf(logFile, "%s", str);
    }
}

// ****************************************************************************
//
// State Logging functions
//
// Purpose: The purpose of the log functions is to turn ViewerRPC information
//          into valid Python code for the VisIt module that can be logged or
//          recorded into a macro.
//
// Note:    The logging functions take ViewerRPC data, whose important fields,
//          vary depending on the RPC being executed, and use that data to
//          create the Python code returned in the "str" string. The exact
//          usage of the ViewerRPC fields for a given RPC must match the
//          usage in ViewerProxy.C in order for the logging functions to remain
//          accurate.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 10 11:50:15 PDT 2006
//
// Modifications:
//   Brad Whitlock, Wed Mar 8 17:05:11 PST 2006
//   Added RedoView.
//
// ****************************************************************************

#define MSG_NOT_IMPLEMENTED 0
#define MSG_UNSUPPORTED     1

static void MESSAGE_COMMENT(const char *name, char *str, int reason)
{
    if(reason == MSG_NOT_IMPLEMENTED)
    {
        SNPRINTF(str, SLEN, "# Logging for %s is not implemented yet.\n", name);
    }
    else if(reason == MSG_UNSUPPORTED)
    {
        SNPRINTF(str, SLEN, "# The %s RPC is not supported in the VisIt module "
                 "so it will not be logged.\n", name);
    }
}


static void log_AddWindowRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "AddWindow()\n");
}

static void log_DeleteWindowRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "DeleteWindow()\n");
}

static void log_SetWindowLayoutRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "SetWindowLayout(%d)\n", rpc->GetWindowLayout());
}

static void log_SetActiveWindowRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "SetActiveWindow(%d)\n", rpc->GetWindowId());
}

static void log_ClearWindowRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ClearWindow()\n");
}

static void log_ClearAllWindowsRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ClearAllWindows()\n");
}

static void log_OpenDatabaseRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "OpenDatabase(\"%s\", %d)\n",
             rpc->GetDatabase().c_str(),
             rpc->GetIntArg1());
}

static void log_CloseDatabaseRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "OpenDatabase(\"%s\")\n",
             rpc->GetDatabase().c_str());
}

static void log_ActivateDatabaseRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ActivateDatabase(\"%s\")\n",
             rpc->GetDatabase().c_str());
}

static void log_CheckForNewStatesRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "CheckForNewStates(\"%s\")\n",
             rpc->GetDatabase().c_str());
}

static void log_CreateDatabaseCorrelationRPC(ViewerRPC *rpc, char *str)
{
     std::string s("CreateDatabaseCorrelation(\"");
     s += rpc->GetDatabase();
     s += "\",(";
     const stringVector &dbs = rpc->GetProgramOptions();
     for(int i = 0; i < dbs.size(); ++i)
     {
         s += dbs[i];
         if(i < dbs.size() - 1)
             s += ", ";
     }
     SNPRINTF(str, SLEN, "%s), %d)\n", s.c_str(), rpc->GetIntArg1());
}

static void log_AlterDatabaseCorrelationRPC(ViewerRPC *rpc, char *str)
{
     std::string s("AlterDatabaseCorrelation(\"");
     s += rpc->GetDatabase();
     s += "\",(";
     const stringVector &dbs = rpc->GetProgramOptions();
     for(int i = 0; i < dbs.size(); ++i)
     {
         s += dbs[i];
         if(i < dbs.size() - 1)
             s += ", ";
     }
     SNPRINTF(str, SLEN, "%s), %d)\n", s.c_str(), rpc->GetIntArg1());
}

static void log_DeleteDatabaseCorrelationRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "DeleteDatabaseCorrelation(\"%s\")\n", 
             rpc->GetDatabase().c_str());
}

static void log_ReOpenDatabaseRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "OpenDatabase(\"%s\", %d)\n",
             rpc->GetDatabase().c_str(),
             rpc->GetIntArg1());
}

static void log_ReplaceDatabaseRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ReplaceDatabase(\"%s\", %d)\n",
             rpc->GetDatabase().c_str(),
             rpc->GetIntArg1());
}

static void log_OverlayDatabaseRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "OverlayDatabase(\"%s\")\n", 
             rpc->GetDatabase().c_str());
}

static void log_OpenComponentHelper(ViewerRPC *rpc, char *str, const char *mName)
{
    const stringVector &argv = rpc->GetProgramOptions();

    if(argv.size() == 0)
    {
        SNPRINTF(str, SLEN, "%s(\"%s\")\n",
                 mName, rpc->GetProgramHost().c_str());
    }
    else if(argv.size() == 1)
    {
        SNPRINTF(str, SLEN, "%s(\"%s\", \"%s\")\n",
                mName, rpc->GetProgramHost().c_str(), argv[0].c_str());
    }
    else
    {
        std::string tmp("launchArguments = (");
        for(int i = 0; i < argv.size(); ++i)
        {
            tmp += "\"";
            tmp += argv[i];
            tmp += "\"";
            if(i < argv.size() - 1)
                tmp += ", ";
        }
        tmp += ")\n";
        SNPRINTF(str, SLEN, "%s%s(\"%s\", launchArguments)\n",
                 tmp.c_str(), mName, rpc->GetProgramHost().c_str());
    }
}

static void log_OpenComputeEngineRPC(ViewerRPC *rpc, char *str)
{
    log_OpenComponentHelper(rpc, str, "OpenComputeEngine");
}

static void log_OpenMDServerRPC(ViewerRPC *rpc, char *str)
{
    log_OpenComponentHelper(rpc, str, "OpenMDServer");
}

static void log_CloseComputeEngineRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "CloseComputeEngine(\"%s\", \"%s\")\n",
             rpc->GetProgramHost().c_str(),
             rpc->GetProgramSim().c_str());
}

static void log_AnimationSetNFramesRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "AnimationSetNFrames(%d)\n", rpc->GetNFrames());
}

static void log_AnimationPlayRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("AnimationPlay", str, MSG_UNSUPPORTED);
}

static void log_AnimationReversePlayRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("AnimationReversePlay", str, MSG_UNSUPPORTED);
}

static void log_AnimationStopRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("AnimationStop", str, MSG_UNSUPPORTED);
}

static void log_TimeSliderNextStateRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "TimeSliderNextState()\n");
}

static void log_TimeSliderPreviousStateRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "TimeSliderPreviousState\n");
}

static void log_SetTimeSliderStateRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "SetTimeSliderState(%d)\n", rpc->GetStateNumber());
}

static void log_SetActiveTimeSliderRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "SetActiveTimeSlider(\"%s\")\n",
             rpc->GetDatabase().c_str());
}

static void log_AddPlotRPC(ViewerRPC *rpc, char *str)
{
    std::string plotName("?");
    PlotPluginManager *pluginManager = PlotPluginManager::Instance();
    std::string id(pluginManager->GetEnabledID(rpc->GetPlotType()));
    ScriptingPlotPluginInfo *info = pluginManager->
        GetScriptingPluginInfo(id);
    if(info != 0)
        plotName = info->GetName();
 
    SNPRINTF(str, SLEN, "AddPlot(\"%s\", \"%s\")\n",
             plotName.c_str(),
             rpc->GetVariable().c_str());
}

static void log_SetPlotFrameRangeRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "SetPlotFrameRange(%d, %d, %d)\n", 
             rpc->GetIntArg1(), rpc->GetIntArg2(), rpc->GetIntArg3());
}

static void log_DeletePlotKeyframeRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "DeletePlotKeyframe(%d, %d)\n", 
             rpc->GetIntArg1(), rpc->GetIntArg2());
}

static void log_MovePlotKeyframeRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "MovePlotKeyframe(%d, %d, %d)\n", 
             rpc->GetIntArg1(), rpc->GetIntArg2(), rpc->GetIntArg3());
}

static void log_DeleteActivePlotsRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "DeleteActivePlots()\n");

}

static void log_HideActivePlotsRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "HideActivePlots()\n");
}

static void log_DrawPlotsRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "DrawPlots()\n");
}

static void log_DisableRedrawRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "DisableRedraw()\n");
}

static void log_RedrawRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "RedrawWindow()\n");
}

static void log_SetActivePlotsRPC(ViewerRPC *rpc, char *str)
{
    int slen = SLEN;
    const intVector &ids = rpc->GetActivePlotIds();
    char *sptr = str;
    int L = SNPRINTF(sptr, slen, "SetActivePlots(");
    sptr += L, slen -= L;
    if(ids.size() > 1)
    {
        L = SNPRINTF(sptr, slen, "(");
        sptr += L, slen -= L;
    }
    for(int i = 0; i < ids.size(); ++i)
    {
        L = SNPRINTF(sptr, slen, "%d", ids[i]);
        sptr += L, slen -= L;
        if(i < ids.size() - 1)
        {
            L = SNPRINTF(sptr, slen, ", ");
            sptr += L, slen -= L;
        }
    }
    if(ids.size() > 1)
    {
        L = SNPRINTF(sptr, slen, ")");
        sptr += L, slen -= L;
    }
    SNPRINTF(sptr, slen, ")\n");
}

static void log_ChangeActivePlotsVarRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ChangeActivePlotsVar(\"%s\")\n",
             rpc->GetVariable().c_str());
}

static void log_AddOperatorRPC(ViewerRPC *rpc, char *str)
{
    std::string operatorName("?");
    OperatorPluginManager *pluginManager = OperatorPluginManager::Instance();
    std::string id(pluginManager->GetEnabledID(rpc->GetOperatorType()));
    ScriptingOperatorPluginInfo *info = pluginManager->
        GetScriptingPluginInfo(id);
    if(info != 0)
        operatorName = info->GetName();
 
    SNPRINTF(str, SLEN, "AddOperator(\"%s\")\n",
             operatorName.c_str());
}

static void log_PromoteOperatorRPC(ViewerRPC *rpc, char *str)
{
    int applyAll = viewer->GetViewerState()->GetGlobalAttributes()->GetApplyOperator() ? 1 : 0;
    SNPRINTF(str, SLEN, "PromoteOperator(%d, %d)\n", rpc->GetOperatorType(),
             applyAll);
}

static void log_DemoteOperatorRPC(ViewerRPC *rpc, char *str)
{
    int applyAll = viewer->GetViewerState()->GetGlobalAttributes()->GetApplyOperator() ? 1 : 0;
    SNPRINTF(str, SLEN, "DemoteOperator(%d, %d)\n", rpc->GetOperatorType(),
             applyAll);
}

static void log_RemoveOperatorRPC(ViewerRPC *rpc, char *str)
{
    int applyAll = viewer->GetViewerState()->GetGlobalAttributes()->GetApplyOperator() ? 1 : 0;
    SNPRINTF(str, SLEN, "RemoveOperator(%d, %d)\n", rpc->GetOperatorType(),
             applyAll);
}

static void log_RemoveLastOperatorRPC(ViewerRPC *rpc, char *str)
{
    int applyAll = viewer->GetViewerState()->GetGlobalAttributes()->GetApplyOperator() ? 1 : 0;
    SNPRINTF(str, SLEN, "RemoveLastOperator(%d)\n", applyAll);
}

static void log_RemoveAllOperatorsRPC(ViewerRPC *rpc, char *str)
{
    int applyAll = viewer->GetViewerState()->GetGlobalAttributes()->GetApplyOperator() ? 1 : 0;
    SNPRINTF(str, SLEN, "RemoveAllOperators(%d)\n", applyAll);
}

static void log_SaveWindowRPC(ViewerRPC *rpc, char *str)
{
    std::string s(PySaveWindowAttributes_GetLogString());
    s += "SetSaveWindowAttributes(SaveWindowAtts)\n";
    SNPRINTF(str, SLEN, "%sSaveWindow()\n", s.c_str());
}

static void log_SetPlotOptionsHelper(ViewerRPC *rpc, std::string &atts, 
    std::string &plotName)
{
    PlotPluginManager *pluginManager = PlotPluginManager::Instance();
    std::string id(pluginManager->GetEnabledID(rpc->GetPlotType()));
    ScriptingPlotPluginInfo *info = pluginManager->
        GetScriptingPluginInfo(id);
    if(info != 0)
    {
        char *s = info->GetLogString();
        atts = s;
        delete [] s;
        plotName = info->GetName();
        plotName += "Atts";
    }
}

static void log_SetDefaultPlotOptionsRPC(ViewerRPC *rpc, char *str)
{
    std::string atts(""), plotName("");
    log_SetPlotOptionsHelper(rpc, atts, plotName);
    atts += "SetDefaultPlotOptions(";
    atts += plotName;
    atts += ")\n";
    SNPRINTF(str, SLEN, "%s", atts.c_str());
}

static void log_SetPlotOptionsRPC(ViewerRPC *rpc, char *str)
{
    std::string atts(""), plotName("");
    log_SetPlotOptionsHelper(rpc, atts, plotName);
    atts += "SetPlotOptions(";
    atts += plotName;
    atts += ")\n";
    SNPRINTF(str, SLEN, "%s", atts.c_str());
}

static void log_SetOperatorOptionsHelper(ViewerRPC *rpc, std::string &atts, 
    std::string &operatorName)
{
    OperatorPluginManager *pluginManager = OperatorPluginManager::Instance();
    std::string id(pluginManager->GetEnabledID(rpc->GetOperatorType()));
    ScriptingOperatorPluginInfo *info = pluginManager->
        GetScriptingPluginInfo(id);
    if(info != 0)
    {
        char *s = info->GetLogString();
        atts = s;
        delete [] s;
        operatorName = info->GetName();
        operatorName += "Atts";
    }
}

static void log_SetDefaultOperatorOptionsRPC(ViewerRPC *rpc, char *str)
{
    std::string atts(""), operatorName("");
    log_SetOperatorOptionsHelper(rpc, atts, operatorName);
    atts += "SetDefaultOperatorOptions(";
    atts += operatorName;
    atts += ")\n";
    SNPRINTF(str, SLEN, "%s", atts.c_str());
}

static void log_SetOperatorOptionsRPC(ViewerRPC *rpc, char *str)
{
    std::string atts(""), operatorName("");
    log_SetOperatorOptionsHelper(rpc, atts, operatorName);
    atts += "SetOperatorOptions(";
    atts += operatorName;
    atts += ")\n";
    SNPRINTF(str, SLEN, "%s", atts.c_str());
}

static void log_AddInitializedOperatorRPC(ViewerRPC *rpc, char *str)
{
    char tmp1[SLEN], tmp2[SLEN];
    log_AddOperatorRPC(rpc, tmp1);
    log_SetOperatorOptionsRPC(rpc, tmp2);
    SNPRINTF(str, SLEN, "%s%s", tmp1, tmp2);
}

static void log_WriteConfigFileRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "WriteConfigFile()\n");
}

static void log_IconifyAllWindowsRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "IconifyAllWindows()\n");
}

static void log_DeIconifyAllWindowsRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "DeIconifyAllWindows()\n");
}

static void log_ShowAllWindowsRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ShowAllWindows()\n");
}

static void log_HideAllWindowsRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "HideAllWindows()\n");
}

static void log_SetAnnotationAttributesRPC(ViewerRPC *rpc, char *str)
{
    std::string s(PyAnnotationAttributes_GetLogString());
    SNPRINTF(str, SLEN, "%sSetAnnotationAttributes(AnnotationAtts)\n", s.c_str());
}

static void log_SetDefaultAnnotationAttributesRPC(ViewerRPC *rpc, char *str)
{
    std::string s(PyAnnotationAttributes_GetLogString());
    SNPRINTF(str, SLEN, "%sSetDefaultAnnotationAttributes(AnnotationAtts)\n", s.c_str());
}

static void log_ResetAnnotationAttributesRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("ResetAnnotationAttributes", str, MSG_UNSUPPORTED);
}

static void log_SetKeyframeAttributesRPC(ViewerRPC *rpc, char *str)
{
    std::string s(PyKeyframeAttributes_GetLogString());
    SNPRINTF(str, SLEN, "%sSetKeyframeAttributes(KeyframeAtts)\n", s.c_str());
}

// ****************************************************************************
// Method: log_SetPlotSILRestrictionRPC
//
// Purpose: 
//   Logs changes to the SIL restriction.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 11 09:55:18 PDT 2006
//
// Modifications:
//   Brad Whitlock, Wed Jan 11 09:55:42 PDT 2006
//   I removed a call to the SIL traverser's UsesAllData method because it
//   was causing a crash sometimes.
//
// ****************************************************************************

static void log_SetPlotSILRestrictionRPC(ViewerRPC *rpc, char *str)
{
    std::string s("silr = SILRestriction()\n");
    int nsets[2] = {0,0};
    avtSILRestriction_p restriction = viewer->GetPlotSILRestriction();
    avtSILRestrictionTraverser trav(restriction);
    int setid;
    for(setid = 0; setid < restriction->GetNumSets(); ++setid)
    {
        nsets[trav.UsesData(setid)?1:0]++;
    }

    if(nsets[1] == restriction->GetNumSets())
    {
        s += "silr.TurnOnAll()\n";
    }
    else if(nsets[0] < nsets[1])
    {
        // More sets were on so we'll turn them all on and then turn off sets
        // that were off.
        intVector sets;
        for(setid = 0; setid < restriction->GetNumSets(); ++setid)
        {
            if(restriction->GetSILSet(setid)->GetMapsOut().size() == 0 &&
               !trav.UsesData(setid))
            {
                sets.push_back(setid);
            }
        }

        if(sets.size() == 0)
        {
            s += "silr.TurnOffAll()\n";
        }
        else if(sets.size() == 1)
        {
            s += "silr.TurnOnAll()\n";
            char tmp[20];
            SNPRINTF(tmp, 20, "silr.TurnOffSet(%d)\n", sets[0]);
            s += tmp;
        }
        else
        {
            s += "silr.SuspendCorrectnessChecking()\n";
            s += "silr.TurnOnAll()\n";
            s += "for silSet in (";
            for(int i = 0; i < sets.size(); ++i)
            {
                char tmp[20];
                if(i < sets.size() - 1)
                    SNPRINTF(tmp, 20, "%d,", sets[i]);
                else
                    SNPRINTF(tmp, 20, "%d", sets[i]);
                s += tmp;
            }
            s += "):\n";
            s += "    silr.TurnOffSet(silSet)\n";
            s += "silr.EnableCorrectnessChecking()\n";
       }
    }
    else
    {
        // More sets were off so we'll turn them all off and then turn on sets
        // that were on.
        intVector sets;
        for(setid = 0; setid < restriction->GetNumSets(); ++setid)
        {
            if(restriction->GetSILSet(setid)->GetMapsOut().size() == 0 &&
               trav.UsesData(setid))
            {
                sets.push_back(setid);
            }
        }

        if(sets.size() == 0)
        {
            s += "silr.TurnOffAll()\n";
        }
        else if(sets.size() == 1)
        {
            s += "silr.TurnOffAll()\n";
            char tmp[20];
            SNPRINTF(tmp, 20, "silr.TurnOnSet(%d)\n", sets[0]);
            s += tmp;
        }
        else
        {
            s += "silr.SuspendCorrectnessChecking()\n";
            s += "silr.TurnOffAll()\n";
            s += "for silSet in (";
            for(int i = 0; i < sets.size(); ++i)
            {
                char tmp[20];
                if(i < sets.size() - 1)
                    SNPRINTF(tmp, 20, "%d,", sets[i]);
                else
                    SNPRINTF(tmp, 20, "%d", sets[i]);
                s += tmp;
            }
            s += "):\n";
            s += "    silr.TurnOnSet(silSet)\n";
            s += "silr.EnableCorrectnessChecking()\n";
        }
    }
    s += "SetPlotSILRestriction(silr)\n";
    SNPRINTF(str, SLEN, "%s", s.c_str());
}

static void log_SetViewCurveRPC(ViewerRPC *rpc, char *str)
{
    std::string s(PyViewCurveAttributes_GetLogString());
    SNPRINTF(str, SLEN, "%sSetViewCurve(ViewCurveAtts)\n", s.c_str());
}

static void log_SetView2DRPC(ViewerRPC *rpc, char *str)
{
    std::string s(PyView2DAttributes_GetLogString());
    SNPRINTF(str, SLEN, "%sSetView2D(View2DAtts)\n", s.c_str());
}

static void log_SetView3DRPC(ViewerRPC *rpc, char *str)
{
    std::string s(PyView3DAttributes_GetLogString());
    SNPRINTF(str, SLEN, "%sSetView3D(View3DAtts)\n", s.c_str());
}

static void log_ResetPlotOptionsRPC(ViewerRPC *rpc, char *str)
{
    std::string plotName("?");
    PlotPluginManager *pluginManager = PlotPluginManager::Instance();
    std::string id(pluginManager->GetEnabledID(rpc->GetPlotType()));
    ScriptingPlotPluginInfo *info = pluginManager->
        GetScriptingPluginInfo(id);
    if(info != 0)
        plotName = info->GetName();
 
    SNPRINTF(str, SLEN, "ResetPlotOptions(\"%s\")\n",
             plotName.c_str());
}

static void log_ResetOperatorOptionsRPC(ViewerRPC *rpc, char *str)
{
    std::string operatorName("?");
    OperatorPluginManager *pluginManager = OperatorPluginManager::Instance();
    std::string id(pluginManager->GetEnabledID(rpc->GetOperatorType()));
    ScriptingOperatorPluginInfo *info = pluginManager->
        GetScriptingPluginInfo(id);
    if(info != 0)
        operatorName = info->GetName();
 
    SNPRINTF(str, SLEN, "ResetOperatorOptions(\"%s\")\n",
             operatorName.c_str());
}

static void log_SetAppearanceRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("SetAppearance", str, MSG_UNSUPPORTED);
}

//*****************************************************************************
//  Modifications:
//    Kathleen Bonnell, Thu Aug  3 09:25:00 PDT 2006
//    Added CurveMeshVar
// 
//*****************************************************************************

static void log_ProcessExpressionsRPC(ViewerRPC *rpc, char *str)
{
    std::string exprList("");

    ExpressionList *list = viewer->GetViewerState()->GetExpressionList();
    for(int i = 0; i < list->GetNumExpressions(); ++i)
    {
        const Expression &expr = list->GetExpressions(i);
        if(expr.GetFromDB())
            continue;

        const char *fx = 0;
        if(expr.GetType() == Expression::ScalarMeshVar)
            fx = "DefineScalarExpression";
        else if(expr.GetType() == Expression::VectorMeshVar)
            fx = "DefineVectorExpression";
        else if(expr.GetType() == Expression::TensorMeshVar)
            fx = "DefineTensorExpression";
        else if(expr.GetType() == Expression::ArrayMeshVar)
            fx = "DefineArrayExpression";
        else if(expr.GetType() == Expression::CurveMeshVar)
            fx = "DefineCurveExpression";
        else if(expr.GetType() == Expression::Mesh)
            fx = "DefineMeshExpression";
        else if(expr.GetType() == Expression::Material)
            fx = "DefineMaterialExpression";
        else if(expr.GetType() == Expression::Species)
            fx = "DefineSpeciesExpression";
       
        if(fx != 0)
        {
            std::string def(expr.GetDefinition());
            if(def.size() > 0 && def[def.size()-1] == '\n')
                def = def.substr(0, def.size()-1);
            SNPRINTF(str, SLEN, "%s(\"%s\", \"%s\")\n", fx, expr.GetName().c_str(),
                     def.c_str());
            exprList += str;
        }
    }

    SNPRINTF(str, SLEN, "%s", exprList.c_str());
}

//*****************************************************************************
//  Modifications:
//    Brad Whitlock, Tue May 8 10:52:04 PDT 2007
//    Fixed so the prefixes have dots.
// 
//*****************************************************************************


static void log_SetLightListRPC(ViewerRPC *rpc, char *str)
{
    std::string s;
    LightList *lightlist = viewer->GetViewerState()->GetLightList();
    for(int i = 0; i < lightlist->NumLights(); ++i)
    {
        char objName[20], objNameDot[20], index[20];
        SNPRINTF(objName, 20, "light%d", i);
        SNPRINTF(objNameDot, 20, "light%d.", i);
        SNPRINTF(index, 20, "%d", i);

        LightAttributes L(lightlist->GetLight(i));
        s += objName;
        s += " = LightAttributes()\n";
        s += PyLightAttributes_ToString(&L, objNameDot);
        s += "SetLight(";
        s += index;
        s += ", ";
        s += objName;
        s += ")\n";
    }
    SNPRINTF(str, SLEN, "%s", s.c_str());
}

static void log_SetDefaultLightListRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("SetDefaultLightList", str, MSG_UNSUPPORTED);
}

static void log_ResetLightListRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("ResetLightList", str, MSG_UNSUPPORTED);
}

static void log_SetAnimationAttributesRPC(ViewerRPC *rpc, char *str)
{
    // Note - there's no function to set all of the annotation atts at the same
    //        so we're just setting the animation timeout for now.
    AnimationAttributes *atts = viewer->GetViewerState()->GetAnimationAttributes();
    SNPRINTF(str, SLEN, "SetAnimationTimeout(%d)\n", atts->GetTimeout());
}

static void log_SetWindowAreaRPC(ViewerRPC *rpc, char *str)
{
    int w,h,x,y;
    sscanf(rpc->GetWindowArea().c_str(), "%dx%d+%d+%d", &w, &h, &x, &y);
    SNPRINTF(str, SLEN, "SetWindowArea(%d, %d ,%d, %d)\n", w,h,x,y);
}

static void log_PrintWindowRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "PrintWindow()\n");
}

static void log_ResetViewRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ResetView()\n");
}

static void log_RecenterViewRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "RecenterView()\n");
}

static void log_ToggleMaintainViewModeRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ToggleMaintainViewMode()\n");
}

static void log_ToggleMaintainDataModeRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ToggleMaintainDataMode()\n");
}

static void log_ToggleBoundingBoxModeRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ToggleBoundingBoxMode()\n");
}

static void log_ToggleCameraViewModeRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ToggleCameraViewMode()\n");
}

static void log_TogglePerspectiveViewRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "TogglePerspectiveView()\n");
}

static void log_ToggleSpinModeRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ToggleSpinMode()\n");
}

static void log_ToggleLockTimeRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ToggleLockTime()\n");
}

static void log_ToggleLockToolsRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ToggleLockTools()\n");
}

static void log_ToggleLockViewModeRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ToggleLockViewMode()\n");
}

static void log_ToggleFullFrameRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ToggleFullFrameMode()\n");
}

static void log_UndoViewRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "UndoView()\n");
}

static void log_RedoViewRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "RedoView()\n");
}

static void log_InvertBackgroundRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "InvertBackgroundColor()\n");
}

static void log_ClearPickPointsRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ClearPickPoints()\n");
}

static void log_SetWindowModeRPC(ViewerRPC *rpc, char *str)
{
    const char *wmodes[] = {"navigate", "zone pick", "node pick", "zoom",
                            "lineout"};
    SNPRINTF(str, SLEN, "SetWindowMode(\"%s\")\n", wmodes[rpc->GetWindowMode()]);
}

static void log_EnableToolRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "EnableTool(%d, %d)\n", rpc->GetToolId(),
             rpc->GetBoolFlag());
}

static void log_CopyViewToWindowRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "CopyViewToWindow(%d, %d)\n", rpc->GetWindowLayout(),
             rpc->GetWindowId());
}

static void log_CopyLightingToWindowRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "CopyLightingToWindow(%d, %d)\n", rpc->GetWindowLayout(),
             rpc->GetWindowId());
}

static void log_CopyAnnotationsToWindowRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "CopyAnnotationsToWindow(%d, %d)\n", rpc->GetWindowLayout(),
             rpc->GetWindowId());
}

static void log_CopyPlotsToWindowRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "CopyPlotsToWindow(%d, %d)\n", rpc->GetWindowLayout(),
             rpc->GetWindowId());
}

static void log_ClearCacheRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ClearCache(\"%s\", \"%s\")\n",
             rpc->GetProgramHost().c_str(),
             rpc->GetProgramSim().c_str());
}

static void log_ClearCacheForAllEnginesRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ClearCacheForAllEngines()\n");
}

static void log_SetViewExtentsTypeRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "SetViewExtentsType(%d)\n", rpc->GetWindowLayout());
}

static void log_ClearRefLinesRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ClearReferenceLines()\n");
}

static void log_SetRenderingAttributesRPC(ViewerRPC *rpc, char *str)
{
    std::string s(PyRenderingAttributes_GetLogString());
    s += "SetRenderingAttributes(RenderingAtts)\n";
    SNPRINTF(str, SLEN, "%s", s.c_str());
}

static void log_DatabaseQueryRPC(ViewerRPC *rpc, char *str)
{
    std::string s, qName;

    if(rpc->GetIntArg3() > 0)
    {
        qName = "Global";
        qName += rpc->GetQueryName();
    }
    else
        qName = rpc->GetQueryName();

    SNPRINTF(str, SLEN, "%s(\"%s\", %d, %d, ", 
             rpc->GetBoolFlag() ? "QueryOverTime" : "Query",
             qName.c_str(),
             rpc->GetIntArg1(), rpc->GetIntArg2());
    s += str;
    const stringVector &vars = rpc->GetQueryVariables();
    if(vars.size() > 1)
        s += "(";
    for(int i = 0; i < vars.size(); ++i)
    {
        s += "\"";
        s += vars[i];
        s += "\"";
        if(i < vars.size()-1)
            s += ", ";
    }
    if(vars.size() > 1)
        s += ")";
    s += ")\n";
    SNPRINTF(str, SLEN, "%s", s.c_str());
}

static void log_PointQueryRPC(ViewerRPC *rpc, char *str)
{
    std::string s, varList;

    const stringVector &vars = rpc->GetQueryVariables();
    if(vars.size() > 1)
        varList += "(";
    for(int i = 0; i < vars.size(); ++i)
    {
        varList += "\"";
        varList += vars[i];
        varList += "\"";
        if(i < vars.size()-1)
            varList += ", ";
    }
    if(vars.size() > 1)
        varList += ")";

    if(rpc->GetQueryName() == "PickByZone")
    {
        if(rpc->GetIntArg3() > 0)
        {
            SNPRINTF(str, SLEN, "PickByGlobalZone(%d, ", 
                     rpc->GetIntArg1());
        }
        else if(rpc->GetBoolFlag())
        {
            SNPRINTF(str, SLEN, "QueryOverTime(\"PickByZone\", %d, %d, ", 
                     rpc->GetIntArg1(), rpc->GetIntArg2());
        }
        else
        {
            SNPRINTF(str, SLEN, "PickByZone(%d, %d, ", 
                     rpc->GetIntArg1(), rpc->GetIntArg2());
        }
        s += str;
        s += varList;
        s += ")\n";
        SNPRINTF(str, SLEN, "%s", s.c_str());
    }
    else if(rpc->GetQueryName() == "PickByNode")
    {
        if(rpc->GetIntArg3() > 0)
        {
            SNPRINTF(str, SLEN, "PickByGlobalNode(%d, ", 
                     rpc->GetIntArg1());
        }
        else if(rpc->GetBoolFlag())
        {
            SNPRINTF(str, SLEN, "QueryOverTime(\"PickByNode\", %d, %d, ", 
                     rpc->GetIntArg1(), rpc->GetIntArg2());
        }
        else
        {
            SNPRINTF(str, SLEN, "PickByNode(%d, %d, ", 
                     rpc->GetIntArg1(), rpc->GetIntArg2());
        }
        s += str;
        s += varList;
        s += ")\n";
        SNPRINTF(str, SLEN, "%s", s.c_str());
    }
    else if(rpc->GetQueryName() == "Pick")
    {
        SNPRINTF(str, SLEN, "Pick((%g, %g, %g), ", 
                rpc->GetQueryPoint1()[0],
                rpc->GetQueryPoint1()[1],
                rpc->GetQueryPoint1()[2]);

        s += str;
        s += varList;
        s += ")\n";
        SNPRINTF(str, SLEN, "%s", s.c_str());
    }
    else
    {
        s = "PointQuery(";
        s += rpc->GetQueryName();
        s += ")";
        MESSAGE_COMMENT(s.c_str(), str, MSG_UNSUPPORTED);
    }
}

static void log_LineQueryRPC(ViewerRPC *rpc, char *str)
{
    std::string s("LineQuery(");
    s += rpc->GetQueryName();
    s += ")";
    MESSAGE_COMMENT(s.c_str(), str, MSG_UNSUPPORTED);
}

static void log_CloneWindowRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "CloneWindow()\n");
}

static void log_SetMaterialAttributesRPC(ViewerRPC *rpc, char *str)
{
    std::string s(PyMaterialAttributes_GetLogString());
    s += "SetMaterialAttributes(MaterialAtts)\n";
    SNPRINTF(str, SLEN, "%s", s.c_str());
}

static void log_SetDefaultMaterialAttributesRPC(ViewerRPC *rpc, char *str)
{
    std::string s(PyMaterialAttributes_GetLogString());
    s += "SetDefaultMaterialAttributes(MaterialAtts)\n";
    SNPRINTF(str, SLEN, "%s", s.c_str());
}

static void log_ResetMaterialAttributesRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("ResetMaterialAttributes", str, MSG_UNSUPPORTED);
}

static void log_SetPlotDatabaseStateRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "SetPlotDatabaseState(%d, %d, %d)\n", 
             rpc->GetIntArg1(), rpc->GetIntArg2(), rpc->GetIntArg3());
}

static void log_DeletePlotDatabaseKeyframeRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "DeletePlotDatabaseKeyframe(%d, %d)\n", 
             rpc->GetIntArg1(), rpc->GetIntArg2());
}

static void log_MovePlotDatabaseKeyframeRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "MovePlotDatabaseKeyframe(%d, %d, %d)\n", 
             rpc->GetIntArg1(), rpc->GetIntArg2(), rpc->GetIntArg3());
}

static void log_ClearViewKeyframesRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ClearViewKeyframes()\n");
}

static void log_DeleteViewKeyframeRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "DeleteViewKeyframe(%d)\n", rpc->GetFrame());
}

static void log_MoveViewKeyframeRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "MoveViewKeyframe(%d, %d)\n", 
             rpc->GetIntArg1(), rpc->GetIntArg2());
}

static void log_SetViewKeyframeRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "SetViewKeyframe()\n");
}

static void log_EnableToolbarRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("EnableToolbar", str, MSG_UNSUPPORTED);
}

static void log_HideToolbarsRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "HideToolbars()\n");
}

static void log_HideToolbarsForAllWindowsRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "HideToolbars(1)\n");
}

static void log_ShowToolbarsRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ShowToolbars()\n");
}

static void log_ShowToolbarsForAllWindowsRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ShowToolbars(1)\n");
}

static void log_SetToolbarIconSizeRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("SetToolbarIconSize", str, MSG_UNSUPPORTED);
}

static void log_SaveViewRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("SaveView", str, MSG_UNSUPPORTED);
}

static void log_SetGlobalLineoutAttributesRPC(ViewerRPC *rpc, char *str)
{
    std::string s(PyGlobalLineoutAttributes_GetLogString());
    s += "SetGlobalLineoutAttributes(GlobalLineoutAtts)\n";
    SNPRINTF(str, SLEN, "%s", s.c_str());
}

static void log_SetPickAttributesRPC(ViewerRPC *rpc, char *str)
{
    std::string s(PyPickAttributes_GetLogString());
    s += "SetPickAttributes(PickAtts)\n";
    SNPRINTF(str, SLEN, "%s", s.c_str());
}

static void log_ExportColorTableRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("ExportColorTable", str, MSG_UNSUPPORTED);
}

static void log_ExportEntireStateRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "SaveSession(\"%s\")\n", rpc->GetVariable().c_str());
}

static void log_ImportEntireStateRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "RestoreSession(\"%s\", %d)\n", rpc->GetVariable().c_str(),
             rpc->GetBoolFlag()?1:0);
}

static void log_ImportEntireStateWithDifferentSourcesRPC(ViewerRPC *rpc, char *str)
{
    std::string stuple;
    const stringVector &sources = rpc->GetProgramOptions();
    stuple = "(";
    for(int i = 0; i < sources.size(); ++i)
    {
        stuple += std::string("\"") + sources[i] + std::string("\"");
        if(i < sources.size()-1)
            stuple += ",";
    }
    stuple += ")";

    SNPRINTF(str, SLEN, "RestoreSessionWithDifferentSources(\"%s\", %d, %s)\n",
             rpc->GetVariable().c_str(),
             rpc->GetBoolFlag()?1:0,
             stuple.c_str());
}

static void log_ResetPickAttributesRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ResetPickAttributes()\n");
}

static void log_AddAnnotationObjectRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("AddAnnotationObject", str, MSG_NOT_IMPLEMENTED);
}

static void log_HideActiveAnnotationObjectsRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("HideActiveAnnotationObjects", str, MSG_NOT_IMPLEMENTED);
}

static void log_DeleteActiveAnnotationObjectsRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("DeleteActiveAnnotationObjects", str, MSG_NOT_IMPLEMENTED);
}

static void log_RaiseActiveAnnotationObjectsRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("RaiseActiveAnnotationObjects", str, MSG_NOT_IMPLEMENTED);
}

static void log_LowerActiveAnnotationObjectsRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("LowerActiveAnnotationObjects", str, MSG_NOT_IMPLEMENTED);
}

static void log_SetAnnotationObjectOptionsRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("SetAnnotationObjectOptions", str, MSG_NOT_IMPLEMENTED);
}

static void log_SetDefaultAnnotationObjectListRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("SetDefaultAnnotationObjectList", str, MSG_NOT_IMPLEMENTED);
}

static void log_ResetAnnotationObjectListRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("ResetAnnotationObjectList", str, MSG_NOT_IMPLEMENTED);
}

static void log_ResetPickLetterRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ResetPickLetter()\n");
}

static void log_SetDefaultPickAttributesRPC(ViewerRPC *rpc, char *str)
{
    std::string s(PyPickAttributes_GetLogString());
    s += "SetDefaultPickAttributes(PickAtts)\n";
    SNPRINTF(str, SLEN, "%s", s.c_str());

}

static void log_ChooseCenterOfRotationRPC(ViewerRPC *rpc, char *str)
{
    if(rpc->GetBoolFlag())
    {
        SNPRINTF(str, SLEN, "ChooseCenterOfRotation(%g, %g)\n", 
                 rpc->GetQueryPoint1()[0], rpc->GetQueryPoint1()[1]);
    }
    else
    {
        SNPRINTF(str, SLEN, "ChooseCenterOfRotation()\n");
    }
}

static void log_SetCenterOfRotationRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "SetCenterOfRotation(%g, %g, %g)\n", 
             rpc->GetQueryPoint1()[0], rpc->GetQueryPoint1()[1],
             rpc->GetQueryPoint1()[2]);
}

static void log_SetQueryOverTimeAttributesRPC(ViewerRPC *rpc, char *str)
{
    std::string s(PyQueryOverTimeAttributes_GetLogString());
    s += "SetQueryOverTimeAttributes(QueryOverTimeAtts)\n";
    SNPRINTF(str, SLEN, "%s", s.c_str());
}

static void log_SetDefaultQueryOverTimeAttributesRPC(ViewerRPC *rpc, char *str)
{
    std::string s(PyQueryOverTimeAttributes_GetLogString());
    s += "SetDefaultQueryOverTimeAttributes(QueryOverTimeAtts)\n";
    SNPRINTF(str, SLEN, "%s", s.c_str());
}

static void log_ResetQueryOverTimeAttributesRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ResetQueryOverTimeAttributes()\n");
}

static void log_ResetLineoutColorRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ResetLineoutColor()\n");
}

static void log_SetInteractorAttributesRPC(ViewerRPC *rpc, char *str)
{
    std::string s(PyInteractorAttributes_GetLogString());
    s += "SetInteractorAttributes(InteractorAtts)\n";
    SNPRINTF(str, SLEN, "%s", s.c_str());
}

static void log_SetDefaultInteractorAttributesRPC(ViewerRPC *rpc, char *str)
{
    std::string s(PyInteractorAttributes_GetLogString());
    s += "SetDefaultInteractorAttributes(InteractorAtts)\n";
    SNPRINTF(str, SLEN, "%s", s.c_str());
}

static void log_ResetInteractorAttributesRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ResetInteractorAttributes()\n");
}

static void log_GetProcInfoRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("GetProcInfo", str, MSG_UNSUPPORTED);
}

static void log_SendSimulationCommandRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("SendSimulationCommand", str, MSG_UNSUPPORTED);
}

static void log_UpdateDBPluginInfoRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("UpdateDBPluginInfo", str, MSG_UNSUPPORTED);
}

static void log_ConstructDDFRPC(ViewerRPC *rpc, char *str)
{
    std::string s(PyConstructDDFAttributes_GetLogString());
    SNPRINTF(str, SLEN, "%sConstructDDFtabase(ConstructDDFAtts)\n", s.c_str());
}

static void log_ExportDBRPC(ViewerRPC *rpc, char *str)
{
    std::string s(PyExportDBAttributes_GetLogString());
    SNPRINTF(str, SLEN, "%sExportDatabase(ExportDBAtts)\n", s.c_str());
}

static void log_SetTryHarderCyclesTimesRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "SetTryHarderCyclesTimes(%d)\n", rpc->GetIntArg1());
}

static void log_SuppressQueryOutputRPC(ViewerRPC *rpc, char *str)
{
    if(rpc->GetBoolFlag())
        SNPRINTF(str, SLEN, "SuppressQueryOutputOn()\n");
    else
        SNPRINTF(str, SLEN, "SuppressQueryOutputOff()\n");
}

static void log_SetMeshManagementAttributesRPC(ViewerRPC *rpc, char *str)
{
    std::string s(PyMeshManagementAttributes_GetLogString());
    s += "SetMeshManagementAttributes(MeshManagementAtts)\n";
    SNPRINTF(str, SLEN, "%s", s.c_str());
}

static void log_SetDefaultMeshManagementAttributesRPC(ViewerRPC *rpc, char *str)
{
    std::string s(PyMeshManagementAttributes_GetLogString());
    s += "SetDefaultMeshManagementAttributes(MeshManagementAtts)\n";
    SNPRINTF(str, SLEN, "%s", s.c_str());
}

static void log_ResetMeshManagementAttributesRPC(ViewerRPC *rpc, char *str)
{
    MESSAGE_COMMENT("ResetMeshManagementAttributes", str, MSG_UNSUPPORTED);
}

static void log_ResizeWindowRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ResizeWindow(%d, %d, %d)\n", rpc->GetWindowId(),
             rpc->GetIntArg1(), rpc->GetIntArg2());
}

static void log_MoveWindowRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "MoveWindow(%d, %d, %d)\n", rpc->GetWindowId(),
             rpc->GetIntArg1(), rpc->GetIntArg2());
}

static void log_MoveAndResizeWindowRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "ResizeWindow(%d, %d, %d, %d, %d)\n", rpc->GetWindowId(),
             rpc->GetIntArg1(), rpc->GetIntArg2(), rpc->GetIntArg3(),
             rpc->GetWindowLayout());
}

static void log_RequestMetaDataRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "metadata = GetMetaData(\"%s\", %d)\n",
             rpc->GetDatabase().c_str(), rpc->GetStateNumber());
}

static void log_SetQueryFloatFormatRPC(ViewerRPC *rpc, char *str)
{
    SNPRINTF(str, SLEN, "SetQueryFloatFormat(\"%s\")\n", 
                        rpc->GetStringArg1().c_str());
}

// ****************************************************************************
// Method: LogRPCs
//
// Purpose: 
//   This is a callback function used for logging that dispatches to other
//   RPC-specific logging functions.
//
// Arguments:
//   subj : The RPC that caused the function to get called.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 10 10:05:19 PDT 2006
//
// Modifications:
//   Hank Childs, Mon Feb 13 21:39:02 PST 2006
//   Added ConstructDDFRPC.
//
//   Brad Whitlock, Fri Mar 9 17:48:59 PST 2007
//   Added RequestMetaDataRPC.
//
//   Brad Whitlock, Tue May 8 16:32:06 PST 2007
//   Added debug5 logging.
//
//   Cyrus Harrison, Tue Jan  8 08:01:51 PST 2008
//   Added SetQueryFloatFormatRPC.
//
// ****************************************************************************

void
LogRPCs(Subject *subj, void *)
{
    char str[SLEN];
    int slen = SLEN;
    bool record = true;

    if(logFile == 0) 
        return;

    // empty the string.
    str[0] = '\0';

    ViewerRPC *rpc = (ViewerRPC *)subj;
    debug5 << "Logging: " << ViewerRPC::ViewerRPCType_ToString(rpc->GetRPCType()).c_str() << endl;

    switch(rpc->GetRPCType())
    {
    case ViewerRPC::SetStateLoggingRPC:
        LogFile_SetEnabled(rpc->GetBoolFlag());
        record = false;        
        break;

    // The rpc's that we want to log.
    case ViewerRPC::AddWindowRPC:
        log_AddWindowRPC(rpc, str);
        break;
    case ViewerRPC::DeleteWindowRPC:
        log_DeleteWindowRPC(rpc, str);
        break;
    case ViewerRPC::SetWindowLayoutRPC:
        log_SetWindowLayoutRPC(rpc, str);
        break;
    case ViewerRPC::SetActiveWindowRPC:
        log_SetActiveWindowRPC(rpc, str);
        break;
    case ViewerRPC::ClearWindowRPC:
        log_ClearWindowRPC(rpc, str);
        break;
    case ViewerRPC::ClearAllWindowsRPC:
        log_ClearAllWindowsRPC(rpc, str);
        break;
    case ViewerRPC::OpenDatabaseRPC:
        log_OpenDatabaseRPC(rpc, str);
        break;
    case ViewerRPC::CloseDatabaseRPC:
        log_CloseDatabaseRPC(rpc, str);
        break;
    case ViewerRPC::ActivateDatabaseRPC:
        log_ActivateDatabaseRPC(rpc, str);
        break;
    case ViewerRPC::CheckForNewStatesRPC:
        log_CheckForNewStatesRPC(rpc, str);
        break;
    case ViewerRPC::CreateDatabaseCorrelationRPC:
        log_CreateDatabaseCorrelationRPC(rpc, str);
        break;
    case ViewerRPC::AlterDatabaseCorrelationRPC:
        log_AlterDatabaseCorrelationRPC(rpc, str);
        break;
    case ViewerRPC::DeleteDatabaseCorrelationRPC:
        log_DeleteDatabaseCorrelationRPC(rpc, str);
        break;
    case ViewerRPC::ReOpenDatabaseRPC:
        log_ReOpenDatabaseRPC(rpc, str);
        break;
    case ViewerRPC::ReplaceDatabaseRPC:
        log_ReplaceDatabaseRPC(rpc, str);
        break;
    case ViewerRPC::OverlayDatabaseRPC:
        log_OverlayDatabaseRPC(rpc, str);
        break;
    case ViewerRPC::OpenComputeEngineRPC:
        log_OpenComputeEngineRPC(rpc, str);
        break;
    case ViewerRPC::CloseComputeEngineRPC:
        log_CloseComputeEngineRPC(rpc, str);
        break;
    case ViewerRPC::AnimationSetNFramesRPC:
        log_AnimationSetNFramesRPC(rpc, str);
        break;
    case ViewerRPC::AnimationPlayRPC:
        log_AnimationPlayRPC(rpc, str);
        break;
    case ViewerRPC::AnimationReversePlayRPC:
        log_AnimationReversePlayRPC(rpc, str);
        break;
    case ViewerRPC::AnimationStopRPC:
        log_AnimationStopRPC(rpc, str);
        break;
    case ViewerRPC::TimeSliderNextStateRPC:
        log_TimeSliderNextStateRPC(rpc, str);
        break;
    case ViewerRPC::TimeSliderPreviousStateRPC:
        log_TimeSliderPreviousStateRPC(rpc, str);
        break;
    case ViewerRPC::SetTimeSliderStateRPC:
        log_SetTimeSliderStateRPC(rpc, str);
        break;
    case ViewerRPC::SetActiveTimeSliderRPC:
        log_SetActiveTimeSliderRPC(rpc, str);
        break;
    case ViewerRPC::AddPlotRPC:
        log_AddPlotRPC(rpc, str);
        break;
    case ViewerRPC::SetPlotFrameRangeRPC:
        log_SetPlotFrameRangeRPC(rpc, str);
        break;
    case ViewerRPC::DeletePlotKeyframeRPC:
        log_DeletePlotKeyframeRPC(rpc, str);
        break;
    case ViewerRPC::MovePlotKeyframeRPC:
        log_MovePlotKeyframeRPC(rpc, str);
        break;
    case ViewerRPC::DeleteActivePlotsRPC:
        log_DeleteActivePlotsRPC(rpc, str);
        break;
    case ViewerRPC::HideActivePlotsRPC:
        log_HideActivePlotsRPC(rpc, str);
        break;
    case ViewerRPC::DrawPlotsRPC:
        log_DrawPlotsRPC(rpc, str);
        break;
    case ViewerRPC::DisableRedrawRPC:
        log_DisableRedrawRPC(rpc, str);
        break;
    case ViewerRPC::RedrawRPC:
        log_RedrawRPC(rpc, str);
        break;
    case ViewerRPC::SetActivePlotsRPC:
        log_SetActivePlotsRPC(rpc, str);
        break;
    case ViewerRPC::ChangeActivePlotsVarRPC:
        log_ChangeActivePlotsVarRPC(rpc, str);
        break;
    case ViewerRPC::AddOperatorRPC:
        log_AddOperatorRPC(rpc, str);
        break;
    case ViewerRPC::AddInitializedOperatorRPC:
        log_AddInitializedOperatorRPC(rpc, str);
        break;
    case ViewerRPC::PromoteOperatorRPC:
        log_PromoteOperatorRPC(rpc, str);
        break;
    case ViewerRPC::DemoteOperatorRPC:
        log_DemoteOperatorRPC(rpc, str);
        break;
    case ViewerRPC::RemoveOperatorRPC:
        log_RemoveOperatorRPC(rpc, str);
        break;
    case ViewerRPC::RemoveLastOperatorRPC:
        log_RemoveLastOperatorRPC(rpc, str);
        break;
    case ViewerRPC::RemoveAllOperatorsRPC:
        log_RemoveAllOperatorsRPC(rpc, str);
        break;
    case ViewerRPC::SaveWindowRPC:
        log_SaveWindowRPC(rpc, str);
        break;
    case ViewerRPC::SetDefaultPlotOptionsRPC:
        log_SetDefaultPlotOptionsRPC(rpc, str);
        break;
    case ViewerRPC::SetPlotOptionsRPC:
        log_SetPlotOptionsRPC(rpc, str);
        break;
    case ViewerRPC::SetDefaultOperatorOptionsRPC:
        log_SetDefaultOperatorOptionsRPC(rpc, str);
        break;
    case ViewerRPC::SetOperatorOptionsRPC:
        log_SetOperatorOptionsRPC(rpc, str);
        break;
    case ViewerRPC::WriteConfigFileRPC:
        log_WriteConfigFileRPC(rpc, str);
        break;
    case ViewerRPC::IconifyAllWindowsRPC:
        log_IconifyAllWindowsRPC(rpc, str);
        break;
    case ViewerRPC::DeIconifyAllWindowsRPC:
        log_DeIconifyAllWindowsRPC(rpc, str);
        break;
    case ViewerRPC::ShowAllWindowsRPC:
        log_ShowAllWindowsRPC(rpc, str);
        break;
    case ViewerRPC::HideAllWindowsRPC:
        log_HideAllWindowsRPC(rpc, str);
        break;
    case ViewerRPC::SetAnnotationAttributesRPC:
        log_SetAnnotationAttributesRPC(rpc, str);
        break;
    case ViewerRPC::SetDefaultAnnotationAttributesRPC:
        log_SetDefaultAnnotationAttributesRPC(rpc, str);
        break;
    case ViewerRPC::ResetAnnotationAttributesRPC:
        log_ResetAnnotationAttributesRPC(rpc, str);
        break;
    case ViewerRPC::SetKeyframeAttributesRPC:
        log_SetKeyframeAttributesRPC(rpc, str);
        break;
    case ViewerRPC::SetPlotSILRestrictionRPC:
        log_SetPlotSILRestrictionRPC(rpc, str);
        break;
    case ViewerRPC::SetViewCurveRPC:
        log_SetViewCurveRPC(rpc, str);
        break;
    case ViewerRPC::SetView2DRPC:
        log_SetView2DRPC(rpc, str);
        break;
    case ViewerRPC::SetView3DRPC:
        log_SetView3DRPC(rpc, str);
        break;
    case ViewerRPC::ResetPlotOptionsRPC:
        log_ResetPlotOptionsRPC(rpc, str);
        break;
    case ViewerRPC::ResetOperatorOptionsRPC:
        log_ResetOperatorOptionsRPC(rpc, str);
        break;
    case ViewerRPC::SetAppearanceRPC:
        log_SetAppearanceRPC(rpc, str);
        break;
    case ViewerRPC::ProcessExpressionsRPC:
        log_ProcessExpressionsRPC(rpc, str);
        break;
    case ViewerRPC::SetLightListRPC:
        log_SetLightListRPC(rpc, str);
        break;
    case ViewerRPC::SetDefaultLightListRPC:
        log_SetDefaultLightListRPC(rpc, str);
        break;
    case ViewerRPC::ResetLightListRPC:
        log_ResetLightListRPC(rpc, str);
        break;
    case ViewerRPC::SetAnimationAttributesRPC:
        log_SetAnimationAttributesRPC(rpc, str);
        break;
    case ViewerRPC::SetWindowAreaRPC:
        log_SetWindowAreaRPC(rpc, str);
        break;
    case ViewerRPC::PrintWindowRPC:
        log_PrintWindowRPC(rpc, str);
        break;
    case ViewerRPC::ResetViewRPC:
        log_ResetViewRPC(rpc, str);
        break;
    case ViewerRPC::RecenterViewRPC:
        log_RecenterViewRPC(rpc, str);
        break;
    case ViewerRPC::ToggleMaintainViewModeRPC:
        log_ToggleMaintainViewModeRPC(rpc, str);
        break;
    case ViewerRPC::ToggleMaintainDataModeRPC:
        log_ToggleMaintainDataModeRPC(rpc, str);
        break;
    case ViewerRPC::ToggleBoundingBoxModeRPC:
        log_ToggleBoundingBoxModeRPC(rpc, str);
        break;
    case ViewerRPC::ToggleCameraViewModeRPC:
        log_ToggleCameraViewModeRPC(rpc, str);
        break;
    case ViewerRPC::TogglePerspectiveViewRPC:
        log_TogglePerspectiveViewRPC(rpc, str);
        break;
    case ViewerRPC::ToggleSpinModeRPC:
        log_ToggleSpinModeRPC(rpc, str);
        break;
    case ViewerRPC::ToggleLockTimeRPC:
        log_ToggleLockTimeRPC(rpc, str);
        break;
    case ViewerRPC::ToggleLockToolsRPC:
        log_ToggleLockToolsRPC(rpc, str);
        break;
    case ViewerRPC::ToggleLockViewModeRPC:
        log_ToggleLockViewModeRPC(rpc, str);
        break;
    case ViewerRPC::ToggleFullFrameRPC:
        log_ToggleFullFrameRPC(rpc, str);
        break;
    case ViewerRPC::UndoViewRPC:
        log_UndoViewRPC(rpc, str);
        break;
    case ViewerRPC::RedoViewRPC:
        log_RedoViewRPC(rpc, str);
        break;
    case ViewerRPC::InvertBackgroundRPC:
        log_InvertBackgroundRPC(rpc, str);
        break;
    case ViewerRPC::ClearPickPointsRPC:
        log_ClearPickPointsRPC(rpc, str);
        break;
    case ViewerRPC::SetWindowModeRPC:
        log_SetWindowModeRPC(rpc, str);
        break;
    case ViewerRPC::EnableToolRPC:
        log_EnableToolRPC(rpc, str);
        break;
    case ViewerRPC::CopyViewToWindowRPC:
        log_CopyViewToWindowRPC(rpc, str);
        break;
    case ViewerRPC::CopyLightingToWindowRPC:
        log_CopyLightingToWindowRPC(rpc, str);
        break;
    case ViewerRPC::CopyAnnotationsToWindowRPC:
        log_CopyAnnotationsToWindowRPC(rpc, str);
        break;
    case ViewerRPC::CopyPlotsToWindowRPC:
        log_CopyPlotsToWindowRPC(rpc, str);
        break;
    case ViewerRPC::ClearCacheRPC:
        log_ClearCacheRPC(rpc, str);
        break;
    case ViewerRPC::ClearCacheForAllEnginesRPC:
        log_ClearCacheForAllEnginesRPC(rpc, str);
        break;
    case ViewerRPC::SetViewExtentsTypeRPC:
        log_SetViewExtentsTypeRPC(rpc, str);
        break;
    case ViewerRPC::ClearRefLinesRPC:
        log_ClearRefLinesRPC(rpc, str);
        break;
    case ViewerRPC::SetRenderingAttributesRPC:
        log_SetRenderingAttributesRPC(rpc, str);
        break;
    case ViewerRPC::DatabaseQueryRPC:
        log_DatabaseQueryRPC(rpc, str);
        break;
    case ViewerRPC::PointQueryRPC:
        log_PointQueryRPC(rpc, str);
        break;
    case ViewerRPC::LineQueryRPC:
        log_LineQueryRPC(rpc, str);
        break;
    case ViewerRPC::CloneWindowRPC:
        log_CloneWindowRPC(rpc, str);
        break;
    case ViewerRPC::SetMaterialAttributesRPC:
        log_SetMaterialAttributesRPC(rpc, str);
        break;
    case ViewerRPC::SetDefaultMaterialAttributesRPC:
        log_SetDefaultMaterialAttributesRPC(rpc, str);
        break;
    case ViewerRPC::ResetMaterialAttributesRPC:
        log_ResetMaterialAttributesRPC(rpc, str);
        break;
    case ViewerRPC::SetPlotDatabaseStateRPC:
        log_SetPlotDatabaseStateRPC(rpc, str);
        break;
    case ViewerRPC::DeletePlotDatabaseKeyframeRPC:
        log_DeletePlotDatabaseKeyframeRPC(rpc, str);
        break;
    case ViewerRPC::MovePlotDatabaseKeyframeRPC:
        log_MovePlotDatabaseKeyframeRPC(rpc, str);
        break;
    case ViewerRPC::ClearViewKeyframesRPC:
        log_ClearViewKeyframesRPC(rpc, str);
        break;
    case ViewerRPC::DeleteViewKeyframeRPC:
        log_DeleteViewKeyframeRPC(rpc, str);
        break;
    case ViewerRPC::MoveViewKeyframeRPC:
        log_MoveViewKeyframeRPC(rpc, str);
        break;
    case ViewerRPC::SetViewKeyframeRPC:
        log_SetViewKeyframeRPC(rpc, str);
        break;
    case ViewerRPC::OpenMDServerRPC:
        log_OpenMDServerRPC(rpc, str);
        break;
    case ViewerRPC::EnableToolbarRPC:
        log_EnableToolbarRPC(rpc, str);
        break;
    case ViewerRPC::HideToolbarsRPC:
        log_HideToolbarsRPC(rpc, str);
        break;
    case ViewerRPC::HideToolbarsForAllWindowsRPC:
        log_HideToolbarsForAllWindowsRPC(rpc, str);
        break;
    case ViewerRPC::ShowToolbarsRPC:
        log_ShowToolbarsRPC(rpc, str);
        break;
    case ViewerRPC::ShowToolbarsForAllWindowsRPC:
        log_ShowToolbarsForAllWindowsRPC(rpc, str);
        break;
    case ViewerRPC::SetToolbarIconSizeRPC:
        log_SetToolbarIconSizeRPC(rpc, str);
        break;
    case ViewerRPC::SaveViewRPC:
        log_SaveViewRPC(rpc, str);
        break;
    case ViewerRPC::SetGlobalLineoutAttributesRPC:
        log_SetGlobalLineoutAttributesRPC(rpc, str);
        break;
    case ViewerRPC::SetPickAttributesRPC:
        log_SetPickAttributesRPC(rpc, str);
        break;
    case ViewerRPC::ExportColorTableRPC:
        log_ExportColorTableRPC(rpc, str);
        break;
    case ViewerRPC::ExportEntireStateRPC:
        log_ExportEntireStateRPC(rpc, str);
        break;
    case ViewerRPC::ImportEntireStateRPC:
        log_ImportEntireStateRPC(rpc, str);
        break;
    case ViewerRPC::ImportEntireStateWithDifferentSourcesRPC:
        log_ImportEntireStateWithDifferentSourcesRPC(rpc, str);
        break;
    case ViewerRPC::ResetPickAttributesRPC:
        log_ResetPickAttributesRPC(rpc, str);
        break;
    case ViewerRPC::AddAnnotationObjectRPC:
        log_AddAnnotationObjectRPC(rpc, str);
        break;
    case ViewerRPC::HideActiveAnnotationObjectsRPC:
        log_HideActiveAnnotationObjectsRPC(rpc, str);
        break;
    case ViewerRPC::DeleteActiveAnnotationObjectsRPC:
        log_DeleteActiveAnnotationObjectsRPC(rpc, str);
        break;
    case ViewerRPC::RaiseActiveAnnotationObjectsRPC:
        log_RaiseActiveAnnotationObjectsRPC(rpc, str);
        break;
    case ViewerRPC::LowerActiveAnnotationObjectsRPC:
        log_LowerActiveAnnotationObjectsRPC(rpc, str);
        break;
    case ViewerRPC::SetAnnotationObjectOptionsRPC:
        log_SetAnnotationObjectOptionsRPC(rpc, str);
        break;
    case ViewerRPC::SetDefaultAnnotationObjectListRPC:
        log_SetDefaultAnnotationObjectListRPC(rpc, str);
        break;
    case ViewerRPC::ResetAnnotationObjectListRPC:
        log_ResetAnnotationObjectListRPC(rpc, str);
        break;
    case ViewerRPC::ResetPickLetterRPC:
        log_ResetPickLetterRPC(rpc, str);
        break;
    case ViewerRPC::SetDefaultPickAttributesRPC:
        log_SetDefaultPickAttributesRPC(rpc, str);
        break;
    case ViewerRPC::ChooseCenterOfRotationRPC:
        log_ChooseCenterOfRotationRPC(rpc, str);
        break;
    case ViewerRPC::SetCenterOfRotationRPC:
        log_SetCenterOfRotationRPC(rpc, str);
        break;
    case ViewerRPC::SetQueryOverTimeAttributesRPC:
        log_SetQueryOverTimeAttributesRPC(rpc, str);
        break;
    case ViewerRPC::SetDefaultQueryOverTimeAttributesRPC:
        log_SetDefaultQueryOverTimeAttributesRPC(rpc, str);
        break;
    case ViewerRPC::ResetQueryOverTimeAttributesRPC:
        log_ResetQueryOverTimeAttributesRPC(rpc, str);
        break;
    case ViewerRPC::ResetLineoutColorRPC:
        log_ResetLineoutColorRPC(rpc, str);
        break;
    case ViewerRPC::SetInteractorAttributesRPC:
        log_SetInteractorAttributesRPC(rpc, str);
        break;
    case ViewerRPC::SetDefaultInteractorAttributesRPC:
        log_SetDefaultInteractorAttributesRPC(rpc, str);
        break;
    case ViewerRPC::ResetInteractorAttributesRPC:
        log_ResetInteractorAttributesRPC(rpc, str);
        break;
    case ViewerRPC::GetProcInfoRPC:
        log_GetProcInfoRPC(rpc, str);
        break;
    case ViewerRPC::SendSimulationCommandRPC:
        log_SendSimulationCommandRPC(rpc, str);
        break;
    case ViewerRPC::UpdateDBPluginInfoRPC:
        log_UpdateDBPluginInfoRPC(rpc, str);
        break;
    case ViewerRPC::ExportDBRPC:
        log_ExportDBRPC(rpc, str);
        break;
    case ViewerRPC::SetTryHarderCyclesTimesRPC:
        log_SetTryHarderCyclesTimesRPC(rpc, str);
        break;
    case ViewerRPC::SuppressQueryOutputRPC:
        log_SuppressQueryOutputRPC(rpc, str);
        break;
    case ViewerRPC::SetMeshManagementAttributesRPC:
        log_SetMeshManagementAttributesRPC(rpc, str);
        break;
    case ViewerRPC::SetDefaultMeshManagementAttributesRPC:
        log_SetDefaultMeshManagementAttributesRPC(rpc, str);
        break;
    case ViewerRPC::ResetMeshManagementAttributesRPC:
        log_ResetMeshManagementAttributesRPC(rpc, str);
        break;
    case ViewerRPC::ResizeWindowRPC:
        log_ResizeWindowRPC(rpc, str);
        break;
    case ViewerRPC::MoveWindowRPC:
        log_MoveWindowRPC(rpc, str);
        break;
    case ViewerRPC::MoveAndResizeWindowRPC:
        log_MoveAndResizeWindowRPC(rpc, str);
        break;
    case ViewerRPC::ConstructDDFRPC:
        log_ConstructDDFRPC(rpc, str);
        break;
    case ViewerRPC::RequestMetaDataRPC:
        log_RequestMetaDataRPC(rpc, str);
        break;
    case ViewerRPC::SetQueryFloatFormatRPC:
        log_SetQueryFloatFormatRPC(rpc, str);
        break;
        
    // RPCs that we don't want to log:
    case ViewerRPC::CloseRPC:
    case ViewerRPC::DetachRPC:
    case ViewerRPC::OpenClientRPC:
    case ViewerRPC::ConnectToMetaDataServerRPC:
    case ViewerRPC::UpdateColorTableRPC:

        // ... more ...
        record = false;
        break;

    // RPCs that we want to log but do not require anything special.
    default:
        {
        // Do the default behavior of stripping "RPC" from the name and 
        // making a function call from the rpc name.
        std::string rpcName(ViewerRPC::ViewerRPCType_ToString(rpc->GetRPCType()));
        SNPRINTF(str, SLEN, "# MAINTENANCE ISSUE: %s is not handled in "
                "Logging.C. Please contact a VisIt developer.\n", rpcName.c_str());
        debug5 << str;
        }
        break;
    }

    if(record && logLevel == 0)
    {
        // Add the string to the macro string
        if(macroRecord)
            macroString += str;

        // Write to the log
        if(logFile != 0)
            fprintf(logFile, "%s", str);
    }
}

static const char *beginSpontaneousComment = "# Begin spontaneous state\n";
static const char *endSpontaneousComment = "# End spontaneous state\n\n";

#if 0
//
// Use this function someday to log spontaneous state changes for objects
// that we need to watch.
//
void
SpontaneousStateLogger(const std::string &s)
{
    if(LogFile_GetEnabled() && logLevel == 0)
    {
        // Objects that might need to be watched for spontaneous state:
        //
        // DatabaseCorrelationList
        //

        if(logFile != 0)
        {
            fprintf(logFile, beginSpontaneousComment);
            fprintf(logFile, "%s", s.c_str());
            fprintf(logFile, endSpontaneousComment);
        }

        if(macroRecord)
        {
            macroString += beginSpontaneousComment;
            macroString += s;
            macroString += endSpontaneousComment;
        }
    }
}
#endif

// ****************************************************************************
// Method: SS_log_ViewCurve
//
// Purpose: 
//   This is a callback function for when ViewCurve state from the viewer makes
//   its way to the CLI as a result of a direct user interaction.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 10 11:45:16 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
SS_log_ViewCurve(const std::string &s)
{
    if(LogFile_GetEnabled() && logLevel == 0 && 
       viewer->GetViewerState()->GetWindowInformation()->GetViewDimension() == 1)
    {
        const char *v = "SetViewCurve(ViewCurveAtts)\n";

        if(logFile != 0)
        {
            fprintf(logFile, beginSpontaneousComment);
            fprintf(logFile, "%s", s.c_str());
            fprintf(logFile, v);
            fprintf(logFile, endSpontaneousComment);
        }

        if(macroRecord)
        {
            macroString += beginSpontaneousComment;
            macroString += s;
            macroString += v;
            macroString += endSpontaneousComment;
        }
    }
}

// ****************************************************************************
// Method: SS_log_View2D
//
// Purpose: 
//   This is a callback function for when View2D state from the viewer makes its
//   way to the CLI as a result of a direct user interaction.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 10 11:45:16 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
SS_log_View2D(const std::string &s)
{
    if(LogFile_GetEnabled() && logLevel == 0 && 
       viewer->GetViewerState()->GetWindowInformation()->GetViewDimension() == 2)
    {
        const char *v = "SetView2D(View2DAtts)\n";

        if(logFile != 0)
        {
            fprintf(logFile, beginSpontaneousComment);
            fprintf(logFile, "%s", s.c_str());
            fprintf(logFile, v);
            fprintf(logFile, endSpontaneousComment);
        }

        if(macroRecord)
        {
            macroString += beginSpontaneousComment;
            macroString += s;
            macroString += v;
            macroString += endSpontaneousComment;
        }
    }
}

// ****************************************************************************
// Method: SS_log_View3D
//
// Purpose: 
//   This is a callback function for when View3D state from the viewer makes its
//   way to the CLI as a result of a direct user interaction.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 10 11:45:16 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
SS_log_View3D(const std::string &s)
{
    if(LogFile_GetEnabled() && logLevel == 0 && 
       viewer->GetViewerState()->GetWindowInformation()->GetViewDimension() == 3)
    {
        const char *v = "SetView3D(View3DAtts)\n";

        if(logFile != 0)
        {
            fprintf(logFile, beginSpontaneousComment);
            fprintf(logFile, "%s", s.c_str());
            fprintf(logFile, v);
            fprintf(logFile, endSpontaneousComment);
        }

        if(macroRecord)
        {
            macroString += beginSpontaneousComment;
            macroString += s;
            macroString += v;
            macroString += endSpontaneousComment;
        }
    }
}
