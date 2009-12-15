/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
#include <Python.h>

#include <ViewerProxy.h>
#include <ViewerRPC.h>
#include <PlotPluginManager.h>
#include <PlotPluginInfo.h>
#include <OperatorPluginManager.h>
#include <OperatorPluginInfo.h>
#include <DebugStream.h>
#include <TimingsManager.h>
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
#include <PyViewAxisArrayAttributes.h>
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
        fprintf(logFile, "# Visit %s log file\n", VISIT_VERSION);
        fprintf(logFile, "ScriptVersion = \"%s\"\n", VISIT_VERSION);
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

static std::string MESSAGE_COMMENT(const char *name, int reason)
{
    std::string s;

    if(reason == MSG_NOT_IMPLEMENTED)
    {
        s = (std::string("# Logging for ") + name) + " is not implemented yet.\n";
    }
    else if(reason == MSG_UNSUPPORTED)
    {
        s = (std::string("# The ") + name) + " RPC is not supported in the VisIt module "
                 "so it will not be logged.\n";
    }
    return s;
}


static std::string log_AddWindowRPC(ViewerRPC *rpc)
{
    return std::string("AddWindow()\n");
}

static std::string log_DeleteWindowRPC(ViewerRPC *rpc)
{
    return std::string("DeleteWindow()\n");
}

static std::string log_SetWindowLayoutRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "SetWindowLayout(%d)\n", rpc->GetWindowLayout());
    return std::string(str);
}

static std::string log_SetActiveWindowRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "SetActiveWindow(%d)\n", rpc->GetWindowId());
    return std::string(str);
}

static std::string log_ClearWindowRPC(ViewerRPC *rpc)
{
    return std::string("ClearWindow()\n");
}

static std::string log_ClearAllWindowsRPC(ViewerRPC *rpc)
{
    return std::string("ClearAllWindows()\n");
}

static std::string log_OpenDatabaseRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "OpenDatabase(\"%s\", %d)\n",
             rpc->GetDatabase().c_str(),
             rpc->GetIntArg1());
    return std::string(str);
}

static std::string log_CloseDatabaseRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "OpenDatabase(\"%s\")\n",
             rpc->GetDatabase().c_str());
    return std::string(str);
}

static std::string log_ActivateDatabaseRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "ActivateDatabase(\"%s\")\n",
             rpc->GetDatabase().c_str());
    return std::string(str);
}

static std::string log_CheckForNewStatesRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "CheckForNewStates(\"%s\")\n",
             rpc->GetDatabase().c_str());
    return std::string(str);
}

static std::string log_CreateDatabaseCorrelationRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    std::string s("CreateDatabaseCorrelation(\"");
    s += rpc->GetDatabase();
    s += "\",(";
    const stringVector &dbs = rpc->GetProgramOptions();
    for(unsigned int i = 0; i < dbs.size(); ++i)
    {
        s += dbs[i];
        if(i < dbs.size() - 1)
            s += ", ";
    }
    SNPRINTF(str, SLEN, "%s), %d)\n", s.c_str(), rpc->GetIntArg1());
    return std::string(str);     
}

static std::string log_AlterDatabaseCorrelationRPC(ViewerRPC *rpc)
{
     char str[SLEN];
     std::string s("AlterDatabaseCorrelation(\"");
     s += rpc->GetDatabase();
     s += "\",(";
     const stringVector &dbs = rpc->GetProgramOptions();
     for(unsigned int i = 0; i < dbs.size(); ++i)
     {
         s += dbs[i];
         if(i < dbs.size() - 1)
             s += ", ";
     }
     SNPRINTF(str, SLEN, "%s), %d)\n", s.c_str(), rpc->GetIntArg1());
     return std::string(str);
}

static std::string log_DeleteDatabaseCorrelationRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "DeleteDatabaseCorrelation(\"%s\")\n", 
             rpc->GetDatabase().c_str());
    return std::string(str);
}

static std::string log_ReOpenDatabaseRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "OpenDatabase(\"%s\", %d)\n",
             rpc->GetDatabase().c_str(),
             rpc->GetIntArg1());
    return std::string(str);
}

static std::string log_ReplaceDatabaseRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "ReplaceDatabase(\"%s\", %d)\n",
             rpc->GetDatabase().c_str(),
             rpc->GetIntArg1());
    return std::string(str);
}

static std::string log_OverlayDatabaseRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "OverlayDatabase(\"%s\")\n", 
             rpc->GetDatabase().c_str());
    return std::string(str);
}

static std::string log_OpenComponentHelper(ViewerRPC *rpc, const char *mName)
{
    const stringVector &argv = rpc->GetProgramOptions();
    char str[SLEN];

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
        for(unsigned int i = 0; i < argv.size(); ++i)
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
    return std::string(str);
}

static std::string log_OpenComputeEngineRPC(ViewerRPC *rpc)
{
    return log_OpenComponentHelper(rpc, "OpenComputeEngine");
}

static std::string log_OpenMDServerRPC(ViewerRPC *rpc)
{
    return log_OpenComponentHelper(rpc, "OpenMDServer");
}

static std::string log_CloseComputeEngineRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "CloseComputeEngine(\"%s\", \"%s\")\n",
             rpc->GetProgramHost().c_str(),
             rpc->GetProgramSim().c_str());
    return std::string(str);
}

static std::string log_AnimationSetNFramesRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "AnimationSetNFrames(%d)\n", rpc->GetNFrames());
    return std::string(str);
}

static std::string log_AnimationPlayRPC(ViewerRPC *rpc)
{
    return MESSAGE_COMMENT("AnimationPlay", MSG_UNSUPPORTED);
}

static std::string log_AnimationReversePlayRPC(ViewerRPC *rpc)
{
    return MESSAGE_COMMENT("AnimationReversePlay", MSG_UNSUPPORTED);
}

static std::string log_AnimationStopRPC(ViewerRPC *rpc)
{
    return MESSAGE_COMMENT("AnimationStop", MSG_UNSUPPORTED);
}

static std::string log_TimeSliderNextStateRPC(ViewerRPC *rpc)
{
    return std::string("TimeSliderNextState()\n");
}

static std::string log_TimeSliderPreviousStateRPC(ViewerRPC *rpc)
{
    return std::string("TimeSliderPreviousState()\n");
}

static std::string log_SetTimeSliderStateRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "SetTimeSliderState(%d)\n", rpc->GetStateNumber());
    return std::string(str);
}

static std::string log_SetActiveTimeSliderRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "SetActiveTimeSlider(\"%s\")\n",
             rpc->GetDatabase().c_str());
    return std::string(str);
}

static std::string log_AddPlotRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    std::string plotName("?");
    PlotPluginManager *pluginManager = viewer->GetPlotPluginManager();
    std::string id(pluginManager->GetEnabledID(rpc->GetPlotType()));
    ScriptingPlotPluginInfo *info = pluginManager->
        GetScriptingPluginInfo(id);
    if(info != 0)
        plotName = info->GetName();
 
    int  inheritSILRestriction = viewer->GetViewerState()->GetGlobalAttributes()->GetNewPlotsInheritSILRestriction();
    int  applyOperator = viewer->GetViewerState()->GetGlobalAttributes()->GetApplyOperator() ? 1: 0;
    SNPRINTF(str, SLEN, "AddPlot(\"%s\", \"%s\", %d, %d)\n",
             plotName.c_str(),
             rpc->GetVariable().c_str(),
             inheritSILRestriction,
             applyOperator);
    return std::string(str);
}

static std::string log_SetPlotFrameRangeRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "SetPlotFrameRange(%d, %d, %d)\n", 
             rpc->GetIntArg1(), rpc->GetIntArg2(), rpc->GetIntArg3());
    return std::string(str);
}

static std::string log_DeletePlotKeyframeRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "DeletePlotKeyframe(%d, %d)\n", 
             rpc->GetIntArg1(), rpc->GetIntArg2());
    return std::string(str);
}

static std::string log_MovePlotKeyframeRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "MovePlotKeyframe(%d, %d, %d)\n", 
             rpc->GetIntArg1(), rpc->GetIntArg2(), rpc->GetIntArg3());
    return std::string(str);
}

static std::string log_DeleteActivePlotsRPC(ViewerRPC *rpc)
{
    return std::string("DeleteActivePlots()\n");
}

static std::string log_HideActivePlotsRPC(ViewerRPC *rpc)
{
    return std::string("HideActivePlots()\n");
}

static std::string log_DrawPlotsRPC(ViewerRPC *rpc)
{
    return std::string("DrawPlots()\n");
}

static std::string log_DisableRedrawRPC(ViewerRPC *rpc)
{
    return std::string("DisableRedraw()\n");
}

static std::string log_RedrawRPC(ViewerRPC *rpc)
{
    return std::string("RedrawWindow()\n");
}

static std::string log_SetActivePlotsRPC(ViewerRPC *rpc)
{
    char str[SLEN];
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
    for(unsigned int i = 0; i < ids.size(); ++i)
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
    return std::string(str);
}

static std::string log_ChangeActivePlotsVarRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "ChangeActivePlotsVar(\"%s\")\n",
             rpc->GetVariable().c_str());
    return std::string(str);
}

static std::string log_AddOperatorRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    std::string operatorName("?");
    OperatorPluginManager *pluginManager = viewer->GetOperatorPluginManager();
    std::string id(pluginManager->GetEnabledID(rpc->GetOperatorType()));
    ScriptingOperatorPluginInfo *info = pluginManager->
        GetScriptingPluginInfo(id);
    if(info != 0)
        operatorName = info->GetName();
 
    int applyAll = viewer->GetViewerState()->GetGlobalAttributes()->GetApplyOperator() ? 1 : 0;
    SNPRINTF(str, SLEN, "AddOperator(\"%s\", %d)\n",
             operatorName.c_str(),
             applyAll);
    return std::string(str);
}

static std::string log_PromoteOperatorRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    int applyAll = viewer->GetViewerState()->GetGlobalAttributes()->GetApplyOperator() ? 1 : 0;
    SNPRINTF(str, SLEN, "PromoteOperator(%d, %d)\n", rpc->GetOperatorType(),
             applyAll);
    return std::string(str);
}

static std::string log_DemoteOperatorRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    int applyAll = viewer->GetViewerState()->GetGlobalAttributes()->GetApplyOperator() ? 1 : 0;
    SNPRINTF(str, SLEN, "DemoteOperator(%d, %d)\n", rpc->GetOperatorType(),
             applyAll);
    return std::string(str);
}

static std::string log_RemoveOperatorRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    int applyAll = viewer->GetViewerState()->GetGlobalAttributes()->GetApplyOperator() ? 1 : 0;
    SNPRINTF(str, SLEN, "RemoveOperator(%d, %d)\n", rpc->GetOperatorType(),
             applyAll);
    return std::string(str);
}

static std::string log_RemoveLastOperatorRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    int applyAll = viewer->GetViewerState()->GetGlobalAttributes()->GetApplyOperator() ? 1 : 0;
    SNPRINTF(str, SLEN, "RemoveLastOperator(%d)\n", applyAll);
    return std::string(str);
}

static std::string log_RemoveAllOperatorsRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    int applyAll = viewer->GetViewerState()->GetGlobalAttributes()->GetApplyOperator() ? 1 : 0;
    SNPRINTF(str, SLEN, "RemoveAllOperators(%d)\n", applyAll);
    return std::string(str);
}

static std::string log_SaveWindowRPC(ViewerRPC *rpc)
{
    std::string s(PySaveWindowAttributes_GetLogString());
    s += "SetSaveWindowAttributes(SaveWindowAtts)\n";
    return s;
}

static void log_SetPlotOptionsHelper(ViewerRPC *rpc, std::string &atts, 
    std::string &plotName)
{
    PlotPluginManager *pluginManager = viewer->GetPlotPluginManager();
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

static std::string log_SetDefaultPlotOptionsRPC(ViewerRPC *rpc)
{
    std::string atts(""), plotName("");
    log_SetPlotOptionsHelper(rpc, atts, plotName);
    atts += "SetDefaultPlotOptions(";
    atts += plotName;
    atts += ")\n";
    return atts;
}

static std::string log_SetPlotOptionsRPC(ViewerRPC *rpc)
{
    std::string atts(""), plotName("");
    log_SetPlotOptionsHelper(rpc, atts, plotName);
    atts += "SetPlotOptions(";
    atts += plotName;
    atts += ")\n";
    return atts;
}

static void log_SetOperatorOptionsHelper(ViewerRPC *rpc, std::string &atts, 
    std::string &operatorName)
{
    OperatorPluginManager *pluginManager = viewer->GetOperatorPluginManager();
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

static std::string log_SetDefaultOperatorOptionsRPC(ViewerRPC *rpc)
{
    std::string atts(""), operatorName("");
    log_SetOperatorOptionsHelper(rpc, atts, operatorName);
    atts += "SetDefaultOperatorOptions(";
    atts += operatorName;
    atts += ")\n";
    return atts;
}

static std::string log_SetOperatorOptionsRPC(ViewerRPC *rpc)
{
    std::string atts(""), operatorName("");
    bool  applyOperator = viewer->GetViewerState()->GetGlobalAttributes()->GetApplyOperator();
    log_SetOperatorOptionsHelper(rpc, atts, operatorName);
    atts += "SetOperatorOptions(";
    atts += operatorName;
    atts += ", ";
    atts += (applyOperator ? "1" : "0");
    atts += ")\n";
    return atts;
}

static std::string log_AddInitializedOperatorRPC(ViewerRPC *rpc)
{
    std::string s;
    s = log_AddOperatorRPC(rpc);
    s += log_SetOperatorOptionsRPC(rpc);
    return s;
}

static std::string log_WriteConfigFileRPC(ViewerRPC *rpc)
{
    return std::string("WriteConfigFile()\n");
}

static std::string log_IconifyAllWindowsRPC(ViewerRPC *rpc)
{
    return std::string("IconifyAllWindows()\n");
}

static std::string log_DeIconifyAllWindowsRPC(ViewerRPC *rpc)
{
    return std::string("DeIconifyAllWindows()\n");
}

static std::string log_ShowAllWindowsRPC(ViewerRPC *rpc)
{
    return std::string("ShowAllWindows()\n");
}

static std::string log_HideAllWindowsRPC(ViewerRPC *rpc)
{
    return std::string("HideAllWindows()\n");
}

static std::string log_SetAnnotationAttributesRPC(ViewerRPC *rpc)
{
    std::string s(PyAnnotationAttributes_GetLogString());
    s += std::string("SetAnnotationAttributes(AnnotationAtts)\n");
    return s;
}

static std::string log_SetDefaultAnnotationAttributesRPC(ViewerRPC *rpc)
{
    std::string s(PyAnnotationAttributes_GetLogString());
    s += std::string("SetDefaultAnnotationAttributes(AnnotationAtts)\n");
    return s;
}

static std::string log_ResetAnnotationAttributesRPC(ViewerRPC *rpc)
{
    return MESSAGE_COMMENT("ResetAnnotationAttributes", MSG_UNSUPPORTED);
}

static std::string log_SetKeyframeAttributesRPC(ViewerRPC *rpc)
{
    std::string s(PyKeyframeAttributes_GetLogString());
    s += std::string("SetKeyframeAttributes(KeyframeAtts)\n");
    return s;
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
//   Hank Childs, Mon Dec 14 13:12:58 PST 2009
//   Reverse ordering for two conditions in an if test.  One is cheap, the
//   other expensive.  By reversing them, the overall time is much faster
//   (because of short circuiting).
//
// ****************************************************************************

static std::string log_SetPlotSILRestrictionRPC(ViewerRPC *rpc)
{
    int t1 = visitTimer->StartTimer();
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
            if(!trav.UsesData(setid) &&
               restriction->GetSILSet(setid)->GetMapsOut().size() == 0)
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
            for(unsigned int i = 0; i < sets.size(); ++i)
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
            if(trav.UsesData(setid) &&
               restriction->GetSILSet(setid)->GetMapsOut().size() == 0)
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
            for(unsigned int i = 0; i < sets.size(); ++i)
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
    bool applyAll = viewer->GetViewerState()->GetGlobalAttributes()->GetApplySelection();
    s += "SetPlotSILRestriction(silr ,";
    s += (applyAll ? "1" : "0");
    s += ")\n";
    visitTimer->StopTimer(t1, "Setting up log string for sil restrictions");
    return s;
}

static std::string log_SetViewAxisArrayRPC(ViewerRPC *rpc)
{
    std::string s(PyViewAxisArrayAttributes_GetLogString());
    s += std::string("SetViewAxisArray(ViewAxisArrayAtts)\n");
    return s;
}

static std::string log_SetViewCurveRPC(ViewerRPC *rpc)
{
    std::string s(PyViewCurveAttributes_GetLogString());
    s += std::string("SetViewCurve(ViewCurveAtts)\n");
    return s;
}

static std::string log_SetView2DRPC(ViewerRPC *rpc)
{
    std::string s(PyView2DAttributes_GetLogString());
    s += std::string("SetView2D(View2DAtts)\n");
    return s;
}

static std::string log_SetView3DRPC(ViewerRPC *rpc)
{
    std::string s(PyView3DAttributes_GetLogString());
    s += std::string("SetView3D(View3DAtts)\n");
    return s;
}

static std::string log_ResetPlotOptionsRPC(ViewerRPC *rpc)
{
    std::string plotName("?");
    PlotPluginManager *pluginManager = viewer->GetPlotPluginManager();
    std::string id(pluginManager->GetEnabledID(rpc->GetPlotType()));
    ScriptingPlotPluginInfo *info = pluginManager->
        GetScriptingPluginInfo(id);
    if(info != 0)
        plotName = info->GetName();
 
    return std::string("ResetPlotOptions(\"") + plotName + std::string("\")\n");
}

static std::string log_ResetOperatorOptionsRPC(ViewerRPC *rpc)
{
    std::string operatorName("?");
    OperatorPluginManager *pluginManager = viewer->GetOperatorPluginManager();
    std::string id(pluginManager->GetEnabledID(rpc->GetOperatorType()));
    ScriptingOperatorPluginInfo *info = pluginManager->
        GetScriptingPluginInfo(id);
    if(info != 0)
        operatorName = info->GetName();
 
    int  applyAll = viewer->GetViewerState()->GetGlobalAttributes()->GetApplyOperator() ? 1: 0;
    char tmp[20];
    SNPRINTF(tmp, 20, "\", %d)\n", applyAll);
    return std::string("ResetOperatorOptions(\"") + operatorName + std::string(tmp);
}

static std::string log_SetAppearanceRPC(ViewerRPC *rpc)
{
    return MESSAGE_COMMENT("SetAppearance", MSG_UNSUPPORTED);
}

//*****************************************************************************
//  Modifications:
//    Kathleen Bonnell, Thu Aug  3 09:25:00 PDT 2006
//    Added CurveMeshVar
// 
//*****************************************************************************

static std::string log_ProcessExpressionsRPC(ViewerRPC *rpc)
{
    std::string exprList("");
    char str[SLEN];

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

    return exprList;
}

//*****************************************************************************
//  Modifications:
//    Brad Whitlock, Tue May 8 10:52:04 PDT 2007
//    Fixed so the prefixes have dots.
// 
//*****************************************************************************


static std::string log_SetLightListRPC(ViewerRPC *rpc)
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
    return s;
}

static std::string log_SetDefaultLightListRPC(ViewerRPC *rpc)
{
    return MESSAGE_COMMENT("SetDefaultLightList", MSG_UNSUPPORTED);
}

static std::string log_ResetLightListRPC(ViewerRPC *rpc)
{
    return MESSAGE_COMMENT("ResetLightList", MSG_UNSUPPORTED);
}

static std::string log_SetAnimationAttributesRPC(ViewerRPC *rpc)
{
    // Note - there's no function to set all of the annotation atts at the same
    //        so we're just setting the animation timeout for now.
    AnimationAttributes *atts = viewer->GetViewerState()->GetAnimationAttributes();
    char str[SLEN];
    SNPRINTF(str, SLEN, "SetAnimationTimeout(%d)\n", atts->GetTimeout());
    return std::string(str);
}

static std::string log_SetWindowAreaRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    int w,h,x,y;
    sscanf(rpc->GetWindowArea().c_str(), "%dx%d+%d+%d", &w, &h, &x, &y);
    SNPRINTF(str, SLEN, "SetWindowArea(%d, %d ,%d, %d)\n", w,h,x,y);
    return std::string(str);
}

static std::string log_PrintWindowRPC(ViewerRPC *rpc)
{
    return std::string("PrintWindow()\n");
}

static std::string log_ResetViewRPC(ViewerRPC *rpc)
{
    return std::string("ResetView()\n");
}

static std::string log_RecenterViewRPC(ViewerRPC *rpc)
{
    return std::string("RecenterView()\n");
}

static std::string log_ToggleMaintainViewModeRPC(ViewerRPC *rpc)
{
    return std::string("ToggleMaintainViewMode()\n");
}

static std::string log_ToggleMaintainDataModeRPC(ViewerRPC *rpc)
{
    return std::string("ToggleMaintainDataMode()\n");
}

static std::string log_ToggleBoundingBoxModeRPC(ViewerRPC *rpc)
{
    return std::string("ToggleBoundingBoxMode()\n");
}

static std::string log_ToggleCameraViewModeRPC(ViewerRPC *rpc)
{
    return std::string("ToggleCameraViewMode()\n");
}

static std::string log_TogglePerspectiveViewRPC(ViewerRPC *rpc)
{
    return std::string("TogglePerspectiveView()\n");
}

static std::string log_ToggleSpinModeRPC(ViewerRPC *rpc)
{
    return std::string("ToggleSpinMode()\n");
}

static std::string log_ToggleLockTimeRPC(ViewerRPC *rpc)
{
    return std::string("ToggleLockTime()\n");
}

static std::string log_ToggleLockToolsRPC(ViewerRPC *rpc)
{
    return std::string("ToggleLockTools()\n");
}

static std::string log_ToggleLockViewModeRPC(ViewerRPC *rpc)
{
    return std::string("ToggleLockViewMode()\n");
}

static std::string log_ToggleFullFrameRPC(ViewerRPC *rpc)
{
    return std::string("ToggleFullFrameMode()\n");
}

static std::string log_UndoViewRPC(ViewerRPC *rpc)
{
    return std::string("UndoView()\n");
}

static std::string log_RedoViewRPC(ViewerRPC *rpc)
{
    return std::string("RedoView()\n");
}

static std::string log_InvertBackgroundRPC(ViewerRPC *rpc)
{
    return std::string("InvertBackgroundColor()\n");
}

static std::string log_ClearPickPointsRPC(ViewerRPC *rpc)
{
    return std::string("ClearPickPoints()\n");
}

static std::string log_SetWindowModeRPC(ViewerRPC *rpc)
{
    const char *wmodes[] = {"navigate", "zone pick", "node pick", "zoom",
                            "lineout", "spreadsheet pick"};
    return (std::string("SetWindowMode(\"") + wmodes[rpc->GetWindowMode()]) + "\")\n";
}

static std::string log_EnableToolRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "EnableTool(%d, %d)\n", rpc->GetToolId(),
             rpc->GetBoolFlag());
    return std::string(str);
}

static std::string log_CopyViewToWindowRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "CopyViewToWindow(%d, %d)\n", rpc->GetWindowLayout(),
             rpc->GetWindowId());
    return std::string(str);
}

static std::string log_CopyLightingToWindowRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "CopyLightingToWindow(%d, %d)\n", rpc->GetWindowLayout(),
             rpc->GetWindowId());
    return std::string(str);
}

static std::string log_CopyAnnotationsToWindowRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "CopyAnnotationsToWindow(%d, %d)\n", rpc->GetWindowLayout(),
             rpc->GetWindowId());
    return std::string(str);
}

static std::string log_CopyPlotsToWindowRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "CopyPlotsToWindow(%d, %d)\n", rpc->GetWindowLayout(),
             rpc->GetWindowId());
    return std::string(str);
}

static std::string log_ClearCacheRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "ClearCache(\"%s\", \"%s\")\n",
             rpc->GetProgramHost().c_str(),
             rpc->GetProgramSim().c_str());
    return std::string(str);
}

static std::string log_ClearCacheForAllEnginesRPC(ViewerRPC *rpc)
{
    return std::string("ClearCacheForAllEngines()\n");
}

static std::string log_SetViewExtentsTypeRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "SetViewExtentsType(%d)\n", rpc->GetWindowLayout());
    return std::string(str);
}

static std::string log_ClearRefLinesRPC(ViewerRPC *rpc)
{
    return std::string("ClearReferenceLines()\n");
}

static std::string log_SetRenderingAttributesRPC(ViewerRPC *rpc)
{
    std::string s(PyRenderingAttributes_GetLogString());
    s += "SetRenderingAttributes(RenderingAtts)\n";
    return s;
}

static std::string log_DatabaseQueryRPC(ViewerRPC *rpc)
{
    std::string s, qName;
    char str[SLEN];

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
    for(unsigned int i = 0; i < vars.size(); ++i)
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
    return s;
}

static std::string log_PointQueryRPC(ViewerRPC *rpc)
{
    std::string s, varList;
    char str[SLEN];
    const stringVector &vars = rpc->GetQueryVariables();
    if(vars.size() > 1)
        varList += "(";
    for(unsigned int i = 0; i < vars.size(); ++i)
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
    }
    else
    {
        s = "PointQuery(";
        s += rpc->GetQueryName();
        s += ")";
        s = MESSAGE_COMMENT(s.c_str(), MSG_UNSUPPORTED);
    }
    return s;
}

static std::string log_LineQueryRPC(ViewerRPC *rpc)
{
    std::string s("LineQuery(");
    s += rpc->GetQueryName();
    s += ")";
    return MESSAGE_COMMENT(s.c_str(), MSG_UNSUPPORTED);
}

static std::string log_CloneWindowRPC(ViewerRPC *rpc)
{
    return std::string("CloneWindow()\n");
}

static std::string log_SetMaterialAttributesRPC(ViewerRPC *rpc)
{
    std::string s(PyMaterialAttributes_GetLogString());
    s += "SetMaterialAttributes(MaterialAtts)\n";
    return s;
}

static std::string log_SetDefaultMaterialAttributesRPC(ViewerRPC *rpc)
{
    std::string s(PyMaterialAttributes_GetLogString());
    s += "SetDefaultMaterialAttributes(MaterialAtts)\n";
    return s;
}

static std::string log_ResetMaterialAttributesRPC(ViewerRPC *rpc)
{
    return MESSAGE_COMMENT("ResetMaterialAttributes", MSG_UNSUPPORTED);
}

static std::string log_SetPlotDatabaseStateRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "SetPlotDatabaseState(%d, %d, %d)\n", 
             rpc->GetIntArg1(), rpc->GetIntArg2(), rpc->GetIntArg3());
    return std::string(str);
}

static std::string log_DeletePlotDatabaseKeyframeRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "DeletePlotDatabaseKeyframe(%d, %d)\n", 
             rpc->GetIntArg1(), rpc->GetIntArg2());
    return std::string(str);
}

static std::string log_MovePlotDatabaseKeyframeRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "MovePlotDatabaseKeyframe(%d, %d, %d)\n", 
             rpc->GetIntArg1(), rpc->GetIntArg2(), rpc->GetIntArg3());
    return std::string(str);
}

static std::string log_ClearViewKeyframesRPC(ViewerRPC *rpc)
{
    return std::string("ClearViewKeyframes()\n");
}

static std::string log_DeleteViewKeyframeRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "DeleteViewKeyframe(%d)\n", rpc->GetFrame());
    return std::string(str);
}

static std::string log_MoveViewKeyframeRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "MoveViewKeyframe(%d, %d)\n", 
             rpc->GetIntArg1(), rpc->GetIntArg2());
    return std::string(str);
}

static std::string log_SetViewKeyframeRPC(ViewerRPC *rpc)
{
    return std::string("SetViewKeyframe()\n");
}

static std::string log_EnableToolbarRPC(ViewerRPC *rpc)
{
    return MESSAGE_COMMENT("EnableToolbar", MSG_UNSUPPORTED);
}

static std::string log_HideToolbarsRPC(ViewerRPC *rpc)
{
    return std::string("HideToolbars()\n");
}

static std::string log_HideToolbarsForAllWindowsRPC(ViewerRPC *rpc)
{
    return std::string("HideToolbars(1)\n");
}

static std::string log_ShowToolbarsRPC(ViewerRPC *rpc)
{
    return std::string("ShowToolbars()\n");
}

static std::string log_ShowToolbarsForAllWindowsRPC(ViewerRPC *rpc)
{
    return std::string("ShowToolbars(1)\n");
}

static std::string log_SetToolbarIconSizeRPC(ViewerRPC *rpc)
{
    return MESSAGE_COMMENT("SetToolbarIconSize", MSG_UNSUPPORTED);
}

static std::string log_SaveViewRPC(ViewerRPC *rpc)
{
    return MESSAGE_COMMENT("SaveView", MSG_UNSUPPORTED);
}

static std::string log_SetGlobalLineoutAttributesRPC(ViewerRPC *rpc)
{
    std::string s(PyGlobalLineoutAttributes_GetLogString());
    s += "SetGlobalLineoutAttributes(GlobalLineoutAtts)\n";
    return s;
}

static std::string log_SetPickAttributesRPC(ViewerRPC *rpc)
{
    std::string s(PyPickAttributes_GetLogString());
    s += "SetPickAttributes(PickAtts)\n";
    return s;
}

static std::string log_ExportColorTableRPC(ViewerRPC *rpc)
{
    return MESSAGE_COMMENT("ExportColorTable", MSG_UNSUPPORTED);
}

static std::string log_ExportEntireStateRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "SaveSession(\"%s\")\n", rpc->GetVariable().c_str());
    return std::string(str);
}

static std::string log_ImportEntireStateRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "RestoreSession(\"%s\", %d)\n", rpc->GetVariable().c_str(),
             rpc->GetBoolFlag()?1:0);
    return std::string(str);
}

static std::string log_ImportEntireStateWithDifferentSourcesRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    std::string stuple;
    const stringVector &sources = rpc->GetProgramOptions();
    stuple = "(";
    for(unsigned int i = 0; i < sources.size(); ++i)
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
    return std::string(str);
}

static std::string log_ResetPickAttributesRPC(ViewerRPC *rpc)
{
    return std::string("ResetPickAttributes()\n");
}

static std::string log_AddAnnotationObjectRPC(ViewerRPC *rpc)
{
    return MESSAGE_COMMENT("AddAnnotationObject", MSG_NOT_IMPLEMENTED);
}

static std::string log_HideActiveAnnotationObjectsRPC(ViewerRPC *rpc)
{
    return MESSAGE_COMMENT("HideActiveAnnotationObjects", MSG_NOT_IMPLEMENTED);
}

static std::string log_DeleteActiveAnnotationObjectsRPC(ViewerRPC *rpc)
{
    return MESSAGE_COMMENT("DeleteActiveAnnotationObjects", MSG_NOT_IMPLEMENTED);
}

static std::string log_RaiseActiveAnnotationObjectsRPC(ViewerRPC *rpc)
{
    return MESSAGE_COMMENT("RaiseActiveAnnotationObjects", MSG_NOT_IMPLEMENTED);
}

static std::string log_LowerActiveAnnotationObjectsRPC(ViewerRPC *rpc)
{
    return MESSAGE_COMMENT("LowerActiveAnnotationObjects", MSG_NOT_IMPLEMENTED);
}

static std::string log_SetAnnotationObjectOptionsRPC(ViewerRPC *rpc)
{
    return MESSAGE_COMMENT("SetAnnotationObjectOptions", MSG_NOT_IMPLEMENTED);
}

static std::string log_SetDefaultAnnotationObjectListRPC(ViewerRPC *rpc)
{
    return MESSAGE_COMMENT("SetDefaultAnnotationObjectList", MSG_NOT_IMPLEMENTED);
}

static std::string log_ResetAnnotationObjectListRPC(ViewerRPC *rpc)
{
    return MESSAGE_COMMENT("ResetAnnotationObjectList", MSG_NOT_IMPLEMENTED);
}

static std::string log_ResetPickLetterRPC(ViewerRPC *rpc)
{
    return std::string("ResetPickLetter()\n");
}

static std::string log_SetDefaultPickAttributesRPC(ViewerRPC *rpc)
{
    std::string s(PyPickAttributes_GetLogString());
    s += "SetDefaultPickAttributes(PickAtts)\n";
    return s;

}

static std::string log_ChooseCenterOfRotationRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    if(rpc->GetBoolFlag())
    {
        SNPRINTF(str, SLEN, "ChooseCenterOfRotation(%g, %g)\n", 
                 rpc->GetQueryPoint1()[0], rpc->GetQueryPoint1()[1]);
    }
    else
    {
        SNPRINTF(str, SLEN, "ChooseCenterOfRotation()\n");
    }
    return std::string(str);
}

static std::string log_SetCenterOfRotationRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "SetCenterOfRotation(%g, %g, %g)\n", 
             rpc->GetQueryPoint1()[0], rpc->GetQueryPoint1()[1],
             rpc->GetQueryPoint1()[2]);
    return std::string(str);
}

static std::string log_SetQueryOverTimeAttributesRPC(ViewerRPC *rpc)
{
    std::string s(PyQueryOverTimeAttributes_GetLogString());
    s += "SetQueryOverTimeAttributes(QueryOverTimeAtts)\n";
    return s;
}

static std::string log_SetDefaultQueryOverTimeAttributesRPC(ViewerRPC *rpc)
{
    std::string s(PyQueryOverTimeAttributes_GetLogString());
    s += "SetDefaultQueryOverTimeAttributes(QueryOverTimeAtts)\n";
    return s;
}

static std::string log_ResetQueryOverTimeAttributesRPC(ViewerRPC *rpc)
{
    return std::string("ResetQueryOverTimeAttributes()\n");
}

static std::string log_ResetLineoutColorRPC(ViewerRPC *rpc)
{
    return std::string("ResetLineoutColor()\n");
}

static std::string log_SetInteractorAttributesRPC(ViewerRPC *rpc)
{
    std::string s(PyInteractorAttributes_GetLogString());
    s += "SetInteractorAttributes(InteractorAtts)\n";
    return s;
}

static std::string log_SetDefaultInteractorAttributesRPC(ViewerRPC *rpc)
{
    std::string s(PyInteractorAttributes_GetLogString());
    s += "SetDefaultInteractorAttributes(InteractorAtts)\n";
    return s;
}

static std::string log_ResetInteractorAttributesRPC(ViewerRPC *rpc)
{
    return std::string("ResetInteractorAttributes()\n");
}

static std::string log_GetProcInfoRPC(ViewerRPC *rpc)
{
    return MESSAGE_COMMENT("GetProcInfo", MSG_UNSUPPORTED);
}

static std::string log_SendSimulationCommandRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    if (rpc->GetStringArg2().length() > 0)
        SNPRINTF(str, SLEN,
                "SendSimulationCommand(\"%s\", \"%s\", \"%s\", \"%s\")\n",
                 rpc->GetProgramHost().c_str(), rpc->GetProgramSim().c_str(),
                 rpc->GetStringArg1().c_str(), rpc->GetStringArg2().c_str());
    else
        SNPRINTF(str, SLEN, "SendSimulationCommand(\"%s\", \"%s\", \"%s\")\n",
                 rpc->GetProgramHost().c_str(), rpc->GetProgramSim().c_str(),
                 rpc->GetStringArg1().c_str());
    return std::string(str);
}

static std::string log_UpdateDBPluginInfoRPC(ViewerRPC *rpc)
{
    return MESSAGE_COMMENT("UpdateDBPluginInfo", MSG_UNSUPPORTED);
}

static std::string log_ConstructDDFRPC(ViewerRPC *rpc)
{
    std::string s(PyConstructDDFAttributes_GetLogString());
    s += "ConstructDDFtabase(ConstructDDFAtts)\n";
    return s;
}

static std::string log_ExportDBRPC(ViewerRPC *rpc)
{
    std::string s(PyExportDBAttributes_GetLogString());
    s += "ExportDatabase(ExportDBAtts)\n";
    return s;
}

static std::string log_SetTryHarderCyclesTimesRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "SetTryHarderCyclesTimes(%d)\n", rpc->GetIntArg1());
    return std::string(str);
}

static std::string log_SuppressQueryOutputRPC(ViewerRPC *rpc)
{
    std::string s;
    if(rpc->GetBoolFlag())
        s = std::string("SuppressQueryOutputOn()\n");
    else
        s = std::string("SuppressQueryOutputOff()\n");
    return s;
}

static std::string log_SetMeshManagementAttributesRPC(ViewerRPC *rpc)
{
    std::string s(PyMeshManagementAttributes_GetLogString());
    s += "SetMeshManagementAttributes(MeshManagementAtts)\n";
    return s;
}

static std::string log_SetDefaultMeshManagementAttributesRPC(ViewerRPC *rpc)
{
    std::string s(PyMeshManagementAttributes_GetLogString());
    s += "SetDefaultMeshManagementAttributes(MeshManagementAtts)\n";
    return s;
}

static std::string log_ResetMeshManagementAttributesRPC(ViewerRPC *rpc)
{
    return MESSAGE_COMMENT("ResetMeshManagementAttributes", MSG_UNSUPPORTED);
}

static std::string log_ResizeWindowRPC(ViewerRPC *rpc)
{
    char str[SLEN]; 
    SNPRINTF(str, SLEN, "ResizeWindow(%d, %d, %d)\n", rpc->GetWindowId(),
             rpc->GetIntArg1(), rpc->GetIntArg2());
    return std::string(str);
}

static std::string log_MoveWindowRPC(ViewerRPC *rpc)
{
    char str[SLEN]; 
    SNPRINTF(str, SLEN, "MoveWindow(%d, %d, %d)\n", rpc->GetWindowId(),
             rpc->GetIntArg1(), rpc->GetIntArg2());
    return std::string(str);
}

static std::string log_MoveAndResizeWindowRPC(ViewerRPC *rpc)
{
    char str[SLEN]; 
    SNPRINTF(str, SLEN, "ResizeWindow(%d, %d, %d, %d, %d)\n", rpc->GetWindowId(),
             rpc->GetIntArg1(), rpc->GetIntArg2(), rpc->GetIntArg3(),
             rpc->GetWindowLayout());
    return std::string(str);
}

static std::string log_RequestMetaDataRPC(ViewerRPC *rpc)
{
    char str[SLEN]; 
    SNPRINTF(str, SLEN, "metadata = GetMetaData(\"%s\", %d)\n",
             rpc->GetDatabase().c_str(), rpc->GetStateNumber());
    return std::string(str);
}

static std::string log_SetQueryFloatFormatRPC(ViewerRPC *rpc)
{
    char str[SLEN]; 
    SNPRINTF(str, SLEN, "SetQueryFloatFormat(\"%s\")\n", 
                        rpc->GetStringArg1().c_str());
    return std::string(str);
}

static std::string log_CreateNamedSelectionRPC(ViewerRPC *rpc)
{
    return std::string("CreateNamedSelection(\"") + rpc->GetStringArg1() + "\")\n";
}

static std::string log_DeleteNamedSelectionRPC(ViewerRPC *rpc)
{
    return std::string("DeleteNamedSelection(\"") + rpc->GetStringArg1() + "\")\n"; 
}

static std::string log_LoadNamedSelectionRPC(ViewerRPC *rpc)
{
    return std::string("LoadNamedSelection(\"") + rpc->GetStringArg1() + "\")\n";
}

static std::string log_SaveNamedSelectionRPC(ViewerRPC *rpc)
{
    return std::string("SaveNamedSelection(\"") + rpc->GetStringArg1() + "\")\n"; 
}

static std::string log_ApplyNamedSelectionRPC(ViewerRPC *rpc)
{
    return std::string("ApplyNamedSelection(\"") + rpc->GetStringArg1() + "\")\n"; 
}

static std::string log_SetPlotDescriptionRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "SetPlotDescription(%d, \"%s\")\n", rpc->GetIntArg1(), rpc->GetStringArg1().c_str());
    return std::string(str);
}

static std::string log_MovePlotOrderTowardLastRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "MovePlotOrderTowardLast(%d)\n", rpc->GetIntArg1());
    return std::string(str);
}

static std::string log_MovePlotOrderTowardFirstRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "MovePlotOrderTowardFirst(%d)\n", rpc->GetIntArg1());
    return std::string(str);
}

static std::string log_SetPlotOrderToLastRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "SetPlotOrderToLast(%d)\n", rpc->GetIntArg1());
    return std::string(str);
}

static std::string log_SetPlotOrderToFirstRPC(ViewerRPC *rpc)
{
    char str[SLEN];
    SNPRINTF(str, SLEN, "SetPlotOrderToFirst(%d)\n", rpc->GetIntArg1());
    return std::string(str);
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
//   Brad Whitlock, Tue Oct 20 16:19:15 PDT 2009
//   I added the new RPCs that control the plot list ordering.
//
//   Brad Whitlock, Mon Nov  9 14:59:03 PST 2009
//   I made it use strings more instead of a fixed sized buffer so we would
//   be less likely to cut off longer strings such as plot attributes.
//
// ****************************************************************************

void
LogRPCs(Subject *subj, void *)
{
    std::string str;
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
        str = log_AddWindowRPC(rpc);
        break;
    case ViewerRPC::DeleteWindowRPC:
        str = log_DeleteWindowRPC(rpc);
        break;
    case ViewerRPC::SetWindowLayoutRPC:
        str = log_SetWindowLayoutRPC(rpc);
        break;
    case ViewerRPC::SetActiveWindowRPC:
        str = log_SetActiveWindowRPC(rpc);
        break;
    case ViewerRPC::ClearWindowRPC:
        str = log_ClearWindowRPC(rpc);
        break;
    case ViewerRPC::ClearAllWindowsRPC:
        str = log_ClearAllWindowsRPC(rpc);
        break;
    case ViewerRPC::OpenDatabaseRPC:
        str = log_OpenDatabaseRPC(rpc);
        break;
    case ViewerRPC::CloseDatabaseRPC:
        str = log_CloseDatabaseRPC(rpc);
        break;
    case ViewerRPC::ActivateDatabaseRPC:
        str = log_ActivateDatabaseRPC(rpc);
        break;
    case ViewerRPC::CheckForNewStatesRPC:
        str = log_CheckForNewStatesRPC(rpc);
        break;
    case ViewerRPC::CreateDatabaseCorrelationRPC:
        str = log_CreateDatabaseCorrelationRPC(rpc);
        break;
    case ViewerRPC::AlterDatabaseCorrelationRPC:
        str = log_AlterDatabaseCorrelationRPC(rpc);
        break;
    case ViewerRPC::DeleteDatabaseCorrelationRPC:
        str = log_DeleteDatabaseCorrelationRPC(rpc);
        break;
    case ViewerRPC::ReOpenDatabaseRPC:
        str = log_ReOpenDatabaseRPC(rpc);
        break;
    case ViewerRPC::ReplaceDatabaseRPC:
        str = log_ReplaceDatabaseRPC(rpc);
        break;
    case ViewerRPC::OverlayDatabaseRPC:
        str = log_OverlayDatabaseRPC(rpc);
        break;
    case ViewerRPC::OpenComputeEngineRPC:
        str = log_OpenComputeEngineRPC(rpc);
        break;
    case ViewerRPC::CloseComputeEngineRPC:
        str = log_CloseComputeEngineRPC(rpc);
        break;
    case ViewerRPC::AnimationSetNFramesRPC:
        str = log_AnimationSetNFramesRPC(rpc);
        break;
    case ViewerRPC::AnimationPlayRPC:
        str = log_AnimationPlayRPC(rpc);
        break;
    case ViewerRPC::AnimationReversePlayRPC:
        str = log_AnimationReversePlayRPC(rpc);
        break;
    case ViewerRPC::AnimationStopRPC:
        str = log_AnimationStopRPC(rpc);
        break;
    case ViewerRPC::TimeSliderNextStateRPC:
        str = log_TimeSliderNextStateRPC(rpc);
        break;
    case ViewerRPC::TimeSliderPreviousStateRPC:
        str = log_TimeSliderPreviousStateRPC(rpc);
        break;
    case ViewerRPC::SetTimeSliderStateRPC:
        str = log_SetTimeSliderStateRPC(rpc);
        break;
    case ViewerRPC::SetActiveTimeSliderRPC:
        str = log_SetActiveTimeSliderRPC(rpc);
        break;
    case ViewerRPC::AddPlotRPC:
        str = log_AddPlotRPC(rpc);
        break;
    case ViewerRPC::SetPlotFrameRangeRPC:
        str = log_SetPlotFrameRangeRPC(rpc);
        break;
    case ViewerRPC::DeletePlotKeyframeRPC:
        str = log_DeletePlotKeyframeRPC(rpc);
        break;
    case ViewerRPC::MovePlotKeyframeRPC:
        str = log_MovePlotKeyframeRPC(rpc);
        break;
    case ViewerRPC::DeleteActivePlotsRPC:
        str = log_DeleteActivePlotsRPC(rpc);
        break;
    case ViewerRPC::HideActivePlotsRPC:
        str = log_HideActivePlotsRPC(rpc);
        break;
    case ViewerRPC::DrawPlotsRPC:
        str = log_DrawPlotsRPC(rpc);
        break;
    case ViewerRPC::DisableRedrawRPC:
        str = log_DisableRedrawRPC(rpc);
        break;
    case ViewerRPC::RedrawRPC:
        str = log_RedrawRPC(rpc);
        break;
    case ViewerRPC::SetActivePlotsRPC:
        str = log_SetActivePlotsRPC(rpc);
        break;
    case ViewerRPC::ChangeActivePlotsVarRPC:
        str = log_ChangeActivePlotsVarRPC(rpc);
        break;
    case ViewerRPC::AddOperatorRPC:
        str = log_AddOperatorRPC(rpc);
        break;
    case ViewerRPC::AddInitializedOperatorRPC:
        str = log_AddInitializedOperatorRPC(rpc);
        break;
    case ViewerRPC::PromoteOperatorRPC:
        str = log_PromoteOperatorRPC(rpc);
        break;
    case ViewerRPC::DemoteOperatorRPC:
        str = log_DemoteOperatorRPC(rpc);
        break;
    case ViewerRPC::RemoveOperatorRPC:
        str = log_RemoveOperatorRPC(rpc);
        break;
    case ViewerRPC::RemoveLastOperatorRPC:
        str = log_RemoveLastOperatorRPC(rpc);
        break;
    case ViewerRPC::RemoveAllOperatorsRPC:
        str = log_RemoveAllOperatorsRPC(rpc);
        break;
    case ViewerRPC::SaveWindowRPC:
        str = log_SaveWindowRPC(rpc);
        break;
    case ViewerRPC::SetDefaultPlotOptionsRPC:
        str = log_SetDefaultPlotOptionsRPC(rpc);
        break;
    case ViewerRPC::SetPlotOptionsRPC:
        str = log_SetPlotOptionsRPC(rpc);
        break;
    case ViewerRPC::SetDefaultOperatorOptionsRPC:
        str = log_SetDefaultOperatorOptionsRPC(rpc);
        break;
    case ViewerRPC::SetOperatorOptionsRPC:
        str = log_SetOperatorOptionsRPC(rpc);
        break;
    case ViewerRPC::WriteConfigFileRPC:
        str = log_WriteConfigFileRPC(rpc);
        break;
    case ViewerRPC::IconifyAllWindowsRPC:
        str = log_IconifyAllWindowsRPC(rpc);
        break;
    case ViewerRPC::DeIconifyAllWindowsRPC:
        str = log_DeIconifyAllWindowsRPC(rpc);
        break;
    case ViewerRPC::ShowAllWindowsRPC:
        str = log_ShowAllWindowsRPC(rpc);
        break;
    case ViewerRPC::HideAllWindowsRPC:
        str = log_HideAllWindowsRPC(rpc);
        break;
    case ViewerRPC::SetAnnotationAttributesRPC:
        str = log_SetAnnotationAttributesRPC(rpc);
        break;
    case ViewerRPC::SetDefaultAnnotationAttributesRPC:
        str = log_SetDefaultAnnotationAttributesRPC(rpc);
        break;
    case ViewerRPC::ResetAnnotationAttributesRPC:
        str = log_ResetAnnotationAttributesRPC(rpc);
        break;
    case ViewerRPC::SetKeyframeAttributesRPC:
        str = log_SetKeyframeAttributesRPC(rpc);
        break;
    case ViewerRPC::SetPlotSILRestrictionRPC:
        str = log_SetPlotSILRestrictionRPC(rpc);
        break;
    case ViewerRPC::SetViewAxisArrayRPC:
        str = log_SetViewAxisArrayRPC(rpc);
        break;
    case ViewerRPC::SetViewCurveRPC:
        str = log_SetViewCurveRPC(rpc);
        break;
    case ViewerRPC::SetView2DRPC:
        str = log_SetView2DRPC(rpc);
        break;
    case ViewerRPC::SetView3DRPC:
        str = log_SetView3DRPC(rpc);
        break;
    case ViewerRPC::ResetPlotOptionsRPC:
        str = log_ResetPlotOptionsRPC(rpc);
        break;
    case ViewerRPC::ResetOperatorOptionsRPC:
        str = log_ResetOperatorOptionsRPC(rpc);
        break;
    case ViewerRPC::SetAppearanceRPC:
        str = log_SetAppearanceRPC(rpc);
        break;
    case ViewerRPC::ProcessExpressionsRPC:
        str = log_ProcessExpressionsRPC(rpc);
        break;
    case ViewerRPC::SetLightListRPC:
        str = log_SetLightListRPC(rpc);
        break;
    case ViewerRPC::SetDefaultLightListRPC:
        str = log_SetDefaultLightListRPC(rpc);
        break;
    case ViewerRPC::ResetLightListRPC:
        str = log_ResetLightListRPC(rpc);
        break;
    case ViewerRPC::SetAnimationAttributesRPC:
        str = log_SetAnimationAttributesRPC(rpc);
        break;
    case ViewerRPC::SetWindowAreaRPC:
        str = log_SetWindowAreaRPC(rpc);
        break;
    case ViewerRPC::PrintWindowRPC:
        str = log_PrintWindowRPC(rpc);
        break;
    case ViewerRPC::ResetViewRPC:
        str = log_ResetViewRPC(rpc);
        break;
    case ViewerRPC::RecenterViewRPC:
        str = log_RecenterViewRPC(rpc);
        break;
    case ViewerRPC::ToggleMaintainViewModeRPC:
        str = log_ToggleMaintainViewModeRPC(rpc);
        break;
    case ViewerRPC::ToggleMaintainDataModeRPC:
        str = log_ToggleMaintainDataModeRPC(rpc);
        break;
    case ViewerRPC::ToggleBoundingBoxModeRPC:
        str = log_ToggleBoundingBoxModeRPC(rpc);
        break;
    case ViewerRPC::ToggleCameraViewModeRPC:
        str = log_ToggleCameraViewModeRPC(rpc);
        break;
    case ViewerRPC::TogglePerspectiveViewRPC:
        str = log_TogglePerspectiveViewRPC(rpc);
        break;
    case ViewerRPC::ToggleSpinModeRPC:
        str = log_ToggleSpinModeRPC(rpc);
        break;
    case ViewerRPC::ToggleLockTimeRPC:
        str = log_ToggleLockTimeRPC(rpc);
        break;
    case ViewerRPC::ToggleLockToolsRPC:
        str = log_ToggleLockToolsRPC(rpc);
        break;
    case ViewerRPC::ToggleLockViewModeRPC:
        str = log_ToggleLockViewModeRPC(rpc);
        break;
    case ViewerRPC::ToggleFullFrameRPC:
        str = log_ToggleFullFrameRPC(rpc);
        break;
    case ViewerRPC::UndoViewRPC:
        str = log_UndoViewRPC(rpc);
        break;
    case ViewerRPC::RedoViewRPC:
        str = log_RedoViewRPC(rpc);
        break;
    case ViewerRPC::InvertBackgroundRPC:
        str = log_InvertBackgroundRPC(rpc);
        break;
    case ViewerRPC::ClearPickPointsRPC:
        str = log_ClearPickPointsRPC(rpc);
        break;
    case ViewerRPC::SetWindowModeRPC:
        str = log_SetWindowModeRPC(rpc);
        break;
    case ViewerRPC::EnableToolRPC:
        str = log_EnableToolRPC(rpc);
        break;
    case ViewerRPC::CopyViewToWindowRPC:
        str = log_CopyViewToWindowRPC(rpc);
        break;
    case ViewerRPC::CopyLightingToWindowRPC:
        str = log_CopyLightingToWindowRPC(rpc);
        break;
    case ViewerRPC::CopyAnnotationsToWindowRPC:
        str = log_CopyAnnotationsToWindowRPC(rpc);
        break;
    case ViewerRPC::CopyPlotsToWindowRPC:
        str = log_CopyPlotsToWindowRPC(rpc);
        break;
    case ViewerRPC::ClearCacheRPC:
        str = log_ClearCacheRPC(rpc);
        break;
    case ViewerRPC::ClearCacheForAllEnginesRPC:
        str = log_ClearCacheForAllEnginesRPC(rpc);
        break;
    case ViewerRPC::SetViewExtentsTypeRPC:
        str = log_SetViewExtentsTypeRPC(rpc);
        break;
    case ViewerRPC::ClearRefLinesRPC:
        str = log_ClearRefLinesRPC(rpc);
        break;
    case ViewerRPC::SetRenderingAttributesRPC:
        str = log_SetRenderingAttributesRPC(rpc);
        break;
    case ViewerRPC::DatabaseQueryRPC:
        str = log_DatabaseQueryRPC(rpc);
        break;
    case ViewerRPC::PointQueryRPC:
        str = log_PointQueryRPC(rpc);
        break;
    case ViewerRPC::LineQueryRPC:
        str = log_LineQueryRPC(rpc);
        break;
    case ViewerRPC::CloneWindowRPC:
        str = log_CloneWindowRPC(rpc);
        break;
    case ViewerRPC::SetMaterialAttributesRPC:
        str = log_SetMaterialAttributesRPC(rpc);
        break;
    case ViewerRPC::SetDefaultMaterialAttributesRPC:
        str = log_SetDefaultMaterialAttributesRPC(rpc);
        break;
    case ViewerRPC::ResetMaterialAttributesRPC:
        str = log_ResetMaterialAttributesRPC(rpc);
        break;
    case ViewerRPC::SetPlotDatabaseStateRPC:
        str = log_SetPlotDatabaseStateRPC(rpc);
        break;
    case ViewerRPC::DeletePlotDatabaseKeyframeRPC:
        str = log_DeletePlotDatabaseKeyframeRPC(rpc);
        break;
    case ViewerRPC::MovePlotDatabaseKeyframeRPC:
        str = log_MovePlotDatabaseKeyframeRPC(rpc);
        break;
    case ViewerRPC::ClearViewKeyframesRPC:
        str = log_ClearViewKeyframesRPC(rpc);
        break;
    case ViewerRPC::DeleteViewKeyframeRPC:
        str = log_DeleteViewKeyframeRPC(rpc);
        break;
    case ViewerRPC::MoveViewKeyframeRPC:
        str = log_MoveViewKeyframeRPC(rpc);
        break;
    case ViewerRPC::SetViewKeyframeRPC:
        str = log_SetViewKeyframeRPC(rpc);
        break;
    case ViewerRPC::OpenMDServerRPC:
        str = log_OpenMDServerRPC(rpc);
        break;
    case ViewerRPC::EnableToolbarRPC:
        str = log_EnableToolbarRPC(rpc);
        break;
    case ViewerRPC::HideToolbarsRPC:
        str = log_HideToolbarsRPC(rpc);
        break;
    case ViewerRPC::HideToolbarsForAllWindowsRPC:
        str = log_HideToolbarsForAllWindowsRPC(rpc);
        break;
    case ViewerRPC::ShowToolbarsRPC:
        str = log_ShowToolbarsRPC(rpc);
        break;
    case ViewerRPC::ShowToolbarsForAllWindowsRPC:
        str = log_ShowToolbarsForAllWindowsRPC(rpc);
        break;
    case ViewerRPC::SetToolbarIconSizeRPC:
        str = log_SetToolbarIconSizeRPC(rpc);
        break;
    case ViewerRPC::SaveViewRPC:
        str = log_SaveViewRPC(rpc);
        break;
    case ViewerRPC::SetGlobalLineoutAttributesRPC:
        str = log_SetGlobalLineoutAttributesRPC(rpc);
        break;
    case ViewerRPC::SetPickAttributesRPC:
        str = log_SetPickAttributesRPC(rpc);
        break;
    case ViewerRPC::ExportColorTableRPC:
        str = log_ExportColorTableRPC(rpc);
        break;
    case ViewerRPC::ExportEntireStateRPC:
        str = log_ExportEntireStateRPC(rpc);
        break;
    case ViewerRPC::ImportEntireStateRPC:
        str = log_ImportEntireStateRPC(rpc);
        break;
    case ViewerRPC::ImportEntireStateWithDifferentSourcesRPC:
        str = log_ImportEntireStateWithDifferentSourcesRPC(rpc);
        break;
    case ViewerRPC::ResetPickAttributesRPC:
        str = log_ResetPickAttributesRPC(rpc);
        break;
    case ViewerRPC::AddAnnotationObjectRPC:
        str = log_AddAnnotationObjectRPC(rpc);
        break;
    case ViewerRPC::HideActiveAnnotationObjectsRPC:
        str = log_HideActiveAnnotationObjectsRPC(rpc);
        break;
    case ViewerRPC::DeleteActiveAnnotationObjectsRPC:
        str = log_DeleteActiveAnnotationObjectsRPC(rpc);
        break;
    case ViewerRPC::RaiseActiveAnnotationObjectsRPC:
        str = log_RaiseActiveAnnotationObjectsRPC(rpc);
        break;
    case ViewerRPC::LowerActiveAnnotationObjectsRPC:
        str = log_LowerActiveAnnotationObjectsRPC(rpc);
        break;
    case ViewerRPC::SetAnnotationObjectOptionsRPC:
        str = log_SetAnnotationObjectOptionsRPC(rpc);
        break;
    case ViewerRPC::SetDefaultAnnotationObjectListRPC:
        str = log_SetDefaultAnnotationObjectListRPC(rpc);
        break;
    case ViewerRPC::ResetAnnotationObjectListRPC:
        str = log_ResetAnnotationObjectListRPC(rpc);
        break;
    case ViewerRPC::ResetPickLetterRPC:
        str = log_ResetPickLetterRPC(rpc);
        break;
    case ViewerRPC::SetDefaultPickAttributesRPC:
        str = log_SetDefaultPickAttributesRPC(rpc);
        break;
    case ViewerRPC::ChooseCenterOfRotationRPC:
        str = log_ChooseCenterOfRotationRPC(rpc);
        break;
    case ViewerRPC::SetCenterOfRotationRPC:
        str = log_SetCenterOfRotationRPC(rpc);
        break;
    case ViewerRPC::SetQueryOverTimeAttributesRPC:
        str = log_SetQueryOverTimeAttributesRPC(rpc);
        break;
    case ViewerRPC::SetDefaultQueryOverTimeAttributesRPC:
        str = log_SetDefaultQueryOverTimeAttributesRPC(rpc);
        break;
    case ViewerRPC::ResetQueryOverTimeAttributesRPC:
        str = log_ResetQueryOverTimeAttributesRPC(rpc);
        break;
    case ViewerRPC::ResetLineoutColorRPC:
        str = log_ResetLineoutColorRPC(rpc);
        break;
    case ViewerRPC::SetInteractorAttributesRPC:
        str = log_SetInteractorAttributesRPC(rpc);
        break;
    case ViewerRPC::SetDefaultInteractorAttributesRPC:
        str = log_SetDefaultInteractorAttributesRPC(rpc);
        break;
    case ViewerRPC::ResetInteractorAttributesRPC:
        str = log_ResetInteractorAttributesRPC(rpc);
        break;
    case ViewerRPC::GetProcInfoRPC:
        str = log_GetProcInfoRPC(rpc);
        break;
    case ViewerRPC::SendSimulationCommandRPC:
        str = log_SendSimulationCommandRPC(rpc);
        break;
    case ViewerRPC::UpdateDBPluginInfoRPC:
        str = log_UpdateDBPluginInfoRPC(rpc);
        break;
    case ViewerRPC::ExportDBRPC:
        str = log_ExportDBRPC(rpc);
        break;
    case ViewerRPC::SetTryHarderCyclesTimesRPC:
        str = log_SetTryHarderCyclesTimesRPC(rpc);
        break;
    case ViewerRPC::SuppressQueryOutputRPC:
        str = log_SuppressQueryOutputRPC(rpc);
        break;
    case ViewerRPC::SetMeshManagementAttributesRPC:
        str = log_SetMeshManagementAttributesRPC(rpc);
        break;
    case ViewerRPC::SetDefaultMeshManagementAttributesRPC:
        str = log_SetDefaultMeshManagementAttributesRPC(rpc);
        break;
    case ViewerRPC::ResetMeshManagementAttributesRPC:
        str = log_ResetMeshManagementAttributesRPC(rpc);
        break;
    case ViewerRPC::ResizeWindowRPC:
        str = log_ResizeWindowRPC(rpc);
        break;
    case ViewerRPC::MoveWindowRPC:
        str = log_MoveWindowRPC(rpc);
        break;
    case ViewerRPC::MoveAndResizeWindowRPC:
        str = log_MoveAndResizeWindowRPC(rpc);
        break;
    case ViewerRPC::ConstructDDFRPC:
        str = log_ConstructDDFRPC(rpc);
        break;
    case ViewerRPC::RequestMetaDataRPC:
        str = log_RequestMetaDataRPC(rpc);
        break;
    case ViewerRPC::SetQueryFloatFormatRPC:
        str = log_SetQueryFloatFormatRPC(rpc);
        break;
    case ViewerRPC::CreateNamedSelectionRPC:
        str = log_CreateNamedSelectionRPC(rpc);
        break;
    case ViewerRPC::DeleteNamedSelectionRPC:
        str = log_DeleteNamedSelectionRPC(rpc);
        break;
    case ViewerRPC::LoadNamedSelectionRPC:
        str = log_LoadNamedSelectionRPC(rpc);
        break;
    case ViewerRPC::SaveNamedSelectionRPC:
        str = log_SaveNamedSelectionRPC(rpc);
        break;
    case ViewerRPC::ApplyNamedSelectionRPC:
        str = log_ApplyNamedSelectionRPC(rpc);
        break;
    case ViewerRPC::SetPlotDescriptionRPC:
        str = log_SetPlotDescriptionRPC(rpc);
        break;
    case ViewerRPC::MovePlotOrderTowardFirstRPC:
        str = log_MovePlotOrderTowardFirstRPC(rpc);
        break;
    case ViewerRPC::MovePlotOrderTowardLastRPC:
        str = log_MovePlotOrderTowardLastRPC(rpc);
        break;
    case ViewerRPC::SetPlotOrderToFirstRPC:
        str = log_SetPlotOrderToFirstRPC(rpc);
        break;
    case ViewerRPC::SetPlotOrderToLastRPC:
        str = log_SetPlotOrderToLastRPC(rpc);
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
        str = (std::string("# MAINTENANCE ISSUE: ") + rpcName) + " is not handled in "
                "Logging.C. Please contact a VisIt developer.\n";
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
            fprintf(logFile, "%s", str.c_str());
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
// Method: SS_log_ViewAxisArray
//
// Purpose: 
//   This is a callback function for when ViewAxisArray state from the viewer
//   makes its way to the CLI as a result of a direct user interaction.
//
// Note:  view dimension of 4 is indicative of axis array view!
//
// Programmer: Jeremy Meredith
// Creation:   February  4, 2008
//
// Modifications:
//   
// ****************************************************************************

void
SS_log_ViewAxisArray(const std::string &s)
{
    if(LogFile_GetEnabled() && logLevel == 0 && 
       viewer->GetViewerState()->GetWindowInformation()->GetViewDimension() == 4)
    {
        const char *v = "SetViewAxisArray(ViewAxisArrayAtts)\n";

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
