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
#include <Python.h>
#include <ViewerRPC.h>
#include <PyViewerRPC.h>

//
// Convenience methods for helping turn ViewerRPC data into a tuple
//

static PyObject *
ViewerRPC_no_args()
{
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
ViewerRPC_one_int(int val)
{
    PyObject *tuple = PyTuple_New(1);
    PyTuple_SET_ITEM(tuple, 0, PyLong_FromLong((long)val));
    return tuple;
}

static PyObject *
ViewerRPC_one_bool(bool val)
{
    PyObject *tuple = PyTuple_New(1);
    PyTuple_SET_ITEM(tuple, 0, PyLong_FromLong(val?1L:0L));
    return tuple;
}

static PyObject *
ViewerRPC_one_string(const std::string &val)
{
    PyObject *tuple = PyTuple_New(1);
    PyTuple_SET_ITEM(tuple, 0, PyString_FromString(val.c_str()));
    return tuple;
}

static PyObject *
ViewerRPC_two_int(int a, int b)
{
    PyObject *tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyLong_FromLong((long)a));
    PyTuple_SET_ITEM(tuple, 1, PyLong_FromLong((long)b));
    return tuple;
}

static PyObject *
ViewerRPC_two_string(const std::string &a, const std::string &b)
{
    PyObject *tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyString_FromString(a.c_str()));
    PyTuple_SET_ITEM(tuple, 1, PyString_FromString(b.c_str()));
    return tuple;
}

static PyObject *
ViewerRPC_three_int(int a, int b, int c)
{
    PyObject *tuple = PyTuple_New(3);
    PyTuple_SET_ITEM(tuple, 0, PyLong_FromLong((long)a));
    PyTuple_SET_ITEM(tuple, 1, PyLong_FromLong((long)b));
    PyTuple_SET_ITEM(tuple, 2, PyLong_FromLong((long)c));
    return tuple;
}

static PyObject *
PyTuple_FromIntVector(const intVector &vec)
{
    PyObject *tuple = 0;
    if(vec.size() == 0)
    {
        PyObject *tuple = PyTuple_New(1);
        PyTuple_SET_ITEM(tuple, 0, PyLong_FromLong(0L));
    }
    else
    {
        tuple = PyTuple_New(vec.size());
        for(int i = 0; i < vec.size(); ++i)
            PyTuple_SET_ITEM(tuple, i, PyLong_FromLong((long)vec[i]));
    }
    return tuple;
}

static PyObject *
PyTuple_FromDoubleVector(const doubleVector &vec)
{
    PyObject *tuple = 0;
    if(vec.size() == 0)
    {
        PyObject *tuple = PyTuple_New(1);
        PyTuple_SET_ITEM(tuple, 0, PyFloat_FromDouble(0.));
    }
    else
    {
        tuple = PyTuple_New(vec.size());
        for(int i = 0; i < vec.size(); ++i)
            PyTuple_SET_ITEM(tuple, i, PyFloat_FromDouble(vec[i]));
    }
    return tuple;
}

static PyObject *
PyTuple_FromDoubleArray(const double *vec, int len)
{
    PyObject *tuple = PyTuple_New(len);
    for(int i = 0; i < len; ++i)
        PyTuple_SET_ITEM(tuple, i, PyFloat_FromDouble(vec[i]));
    return tuple;
}

static PyObject *
PyTuple_FromStringVector(const stringVector &vec)
{
    PyObject *tuple = 0;
    if(vec.size() == 0)
    {
        PyObject *tuple = PyTuple_New(1);
        PyTuple_SET_ITEM(tuple, 0, PyString_FromString(""));
    }
    else
    {
        tuple = PyTuple_New(vec.size());
        for(int i = 0; i < vec.size(); ++i)
            PyTuple_SET_ITEM(tuple, i, PyString_FromString(vec[i].c_str()));
    }
    return tuple;
}

static PyObject *
ViewerRPC_wrap_ViewerRPC(ViewerRPC *rpc)
{
    // Wrap the ViewerRPC and stash it into a tuple.
    PyObject *tuple = PyTuple_New(1);
    PyTuple_SET_ITEM(tuple, 0, PyViewerRPC_Wrap(rpc));
    return tuple;
}

////////////////////////////////////////////////////////////////////////////////
//
// Functions to turn ViewerRPC into tuple data that looks the same as the 
// argument lists to the methods in ViewerMethods.h
//
////////////////////////////////////////////////////////////////////////////////

static PyObject *args_CloseRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_DetachRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_AddWindowRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_DeleteWindowRPC(ViewerRPC *) { return ViewerRPC_no_args(); }

static PyObject *args_SetWindowLayoutRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_int(rpc->GetWindowLayout());
}

static PyObject *args_SetActiveWindowRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_int(rpc->GetWindowId());
}

static PyObject *args_ClearWindowRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_bool(rpc->GetBoolFlag());
}

static PyObject *args_ClearAllWindowsRPC(ViewerRPC *) { return ViewerRPC_no_args(); }

static PyObject *args_OpenDatabaseRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(4);
    PyTuple_SET_ITEM(tuple, 0, PyString_FromString(rpc->GetDatabase().c_str()));
    PyTuple_SET_ITEM(tuple, 1, PyLong_FromLong(rpc->GetIntArg1()));
    PyTuple_SET_ITEM(tuple, 2, PyLong_FromLong(rpc->GetBoolFlag()?1L:0L));
    PyTuple_SET_ITEM(tuple, 3, PyString_FromString(rpc->GetStringArg1().c_str()));
    return tuple; 
}

static PyObject *args_CloseDatabaseRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_string(rpc->GetDatabase());
}

static PyObject *args_ActivateDatabaseRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_string(rpc->GetDatabase());
}

static PyObject *args_CheckForNewStatesRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_string(rpc->GetDatabase());
}

static PyObject *args_CreateDatabaseCorrelationRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(4);
    PyTuple_SET_ITEM(tuple, 0, PyString_FromString(rpc->GetDatabase().c_str()));
    PyTuple_SET_ITEM(tuple, 1, PyTuple_FromStringVector(rpc->GetProgramOptions()));
    PyTuple_SET_ITEM(tuple, 2, PyLong_FromLong(rpc->GetIntArg1()));
    PyTuple_SET_ITEM(tuple, 3, PyLong_FromLong(rpc->GetIntArg2()));
    return tuple; 
}

static PyObject *args_AlterDatabaseCorrelationRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(4);
    PyTuple_SET_ITEM(tuple, 0, PyString_FromString(rpc->GetDatabase().c_str()));
    PyTuple_SET_ITEM(tuple, 1, PyTuple_FromStringVector(rpc->GetProgramOptions()));
    PyTuple_SET_ITEM(tuple, 2, PyLong_FromLong(rpc->GetIntArg1()));
    PyTuple_SET_ITEM(tuple, 3, PyLong_FromLong(rpc->GetIntArg2()));
    return tuple; 
}

static PyObject *args_DeleteDatabaseCorrelationRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_string(rpc->GetDatabase());
}

static PyObject *args_ReOpenDatabaseRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyString_FromString(rpc->GetDatabase().c_str()));
    PyTuple_SET_ITEM(tuple, 1, PyLong_FromLong(rpc->GetIntArg1()));
    return tuple;
}

static PyObject *args_ReplaceDatabaseRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyString_FromString(rpc->GetDatabase().c_str()));
    PyTuple_SET_ITEM(tuple, 1, PyLong_FromLong(rpc->GetIntArg1()));
    return tuple;
}

static PyObject *args_OverlayDatabaseRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_string(rpc->GetDatabase());
}

static PyObject *args_OpenComputeEngineRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyString_FromString(rpc->GetProgramHost().c_str()));
    PyTuple_SET_ITEM(tuple, 1, PyTuple_FromStringVector(rpc->GetProgramOptions()));
    return tuple;
}

static PyObject *args_CloseComputeEngineRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyString_FromString(rpc->GetProgramHost().c_str()));
    PyTuple_SET_ITEM(tuple, 1, PyString_FromString(rpc->GetProgramSim().c_str()));
    return tuple;
}

static PyObject *args_AnimationSetNFramesRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_int(rpc->GetNFrames());
}

static PyObject *args_AnimationPlayRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_AnimationReversePlayRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_AnimationStopRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_TimeSliderNextStateRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_TimeSliderPreviousStateRPC(ViewerRPC *) { return ViewerRPC_no_args(); }

static PyObject *args_SetTimeSliderStateRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_int(rpc->GetStateNumber());
}

static PyObject *args_SetActiveTimeSliderRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_string(rpc->GetDatabase());
}

static PyObject *args_AddPlotRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyLong_FromLong(rpc->GetPlotType()));
    PyTuple_SET_ITEM(tuple, 1, PyString_FromString(rpc->GetVariable().c_str()));
    return tuple;
}

static PyObject *args_SetPlotFrameRangeRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(3);
    PyTuple_SET_ITEM(tuple, 0, PyLong_FromLong(rpc->GetIntArg1()));
    PyTuple_SET_ITEM(tuple, 1, PyLong_FromLong(rpc->GetIntArg2()));
    PyTuple_SET_ITEM(tuple, 2, PyLong_FromLong(rpc->GetIntArg3()));
    return tuple;
}

static PyObject *args_DeletePlotKeyframeRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyLong_FromLong(rpc->GetIntArg1()));
    PyTuple_SET_ITEM(tuple, 1, PyLong_FromLong(rpc->GetIntArg2()));
    return tuple;
}

static PyObject *args_MovePlotKeyframeRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(3);
    PyTuple_SET_ITEM(tuple, 0, PyLong_FromLong(rpc->GetIntArg1()));
    PyTuple_SET_ITEM(tuple, 1, PyLong_FromLong(rpc->GetIntArg2()));
    PyTuple_SET_ITEM(tuple, 2, PyLong_FromLong(rpc->GetIntArg3()));
    return tuple;
}

static PyObject *args_DeleteActivePlotsRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_HideActivePlotsRPC(ViewerRPC *) { return ViewerRPC_no_args(); }

static PyObject *args_DrawPlotsRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_bool(rpc->GetBoolFlag());
}

static PyObject *args_DisableRedrawRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_RedrawRPC(ViewerRPC *) { return ViewerRPC_no_args(); }

static PyObject *args_SetActivePlotsRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyTuple_FromIntVector(rpc->GetActivePlotIds()));
    PyTuple_SET_ITEM(tuple, 1, PyLong_FromLong(rpc->GetBoolFlag()?1L:0L));
    return tuple;
}

static PyObject *args_ChangeActivePlotsVarRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_string(rpc->GetVariable());
}

static PyObject *args_AddOperatorRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyLong_FromLong(rpc->GetOperatorType()));
    PyTuple_SET_ITEM(tuple, 1, PyLong_FromLong(rpc->GetBoolFlag()?1L:0L));
    return tuple;
}

static PyObject *args_AddInitializedOperatorRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_int(rpc->GetOperatorType());
}

static PyObject *args_PromoteOperatorRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_int(rpc->GetOperatorType());
}

static PyObject *args_DemoteOperatorRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_int(rpc->GetOperatorType());
}

static PyObject *args_RemoveOperatorRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_int(rpc->GetOperatorType());
}

static PyObject *args_RemoveLastOperatorRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_RemoveAllOperatorsRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SaveWindowRPC(ViewerRPC *) { return ViewerRPC_no_args(); }

static PyObject *args_SetDefaultPlotOptionsRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_int(rpc->GetPlotType());
}

static PyObject *args_SetPlotOptionsRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_int(rpc->GetPlotType());
}

static PyObject *args_SetDefaultOperatorOptionsRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_int(rpc->GetOperatorType());
}

static PyObject *args_SetOperatorOptionsRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_int(rpc->GetOperatorType());
}

static PyObject *args_WriteConfigFileRPC(ViewerRPC *) { return ViewerRPC_no_args(); }

static PyObject *args_ConnectToMetaDataServerRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyString_FromString(rpc->GetProgramHost().c_str()));
    PyTuple_SET_ITEM(tuple, 1, PyTuple_FromStringVector(rpc->GetProgramOptions()));
    return tuple;
}

static PyObject *args_IconifyAllWindowsRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_DeIconifyAllWindowsRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ShowAllWindowsRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_HideAllWindowsRPC(ViewerRPC *) { return ViewerRPC_no_args(); }

static PyObject *args_UpdateColorTableRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_string(rpc->GetColorTableName());
}

static PyObject *args_SetAnnotationAttributesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetDefaultAnnotationAttributesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ResetAnnotationAttributesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetKeyframeAttributesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetPlotSILRestrictionRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetViewCurveRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetView2DRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetView3DRPC(ViewerRPC *) { return ViewerRPC_no_args(); }

static PyObject *args_ResetPlotOptionsRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_int(rpc->GetPlotType());
}

static PyObject *args_ResetOperatorOptionsRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_int(rpc->GetOperatorType());
}

static PyObject *args_SetAppearanceRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ProcessExpressionsRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetLightListRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetDefaultLightListRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ResetLightListRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetAnimationAttributesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }

static PyObject *args_SetWindowAreaRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_string(rpc->GetWindowArea());
}

static PyObject *args_PrintWindowRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ResetViewRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_RecenterViewRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ToggleMaintainViewModeRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ToggleMaintainDataModeRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ToggleBoundingBoxModeRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ToggleCameraViewModeRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_TogglePerspectiveViewRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ToggleSpinModeRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ToggleLockTimeRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ToggleLockToolsRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ToggleLockViewModeRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ToggleFullFrameRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_UndoViewRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_RedoViewRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_InvertBackgroundRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ClearPickPointsRPC(ViewerRPC *) { return ViewerRPC_no_args(); }

static PyObject *args_SetWindowModeRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_int(rpc->GetWindowMode());
}

static PyObject *args_EnableToolRPC(ViewerRPC *rpc)
{
    return ViewerRPC_two_int(rpc->GetToolId(), rpc->GetBoolFlag()?1:0);
}

static PyObject *args_CopyViewToWindowRPC(ViewerRPC *rpc)
{
    return ViewerRPC_two_int(rpc->GetWindowLayout(), rpc->GetWindowId());
}

static PyObject *args_CopyLightingToWindowRPC(ViewerRPC *rpc)
{
    return ViewerRPC_two_int(rpc->GetWindowLayout(), rpc->GetWindowId());
}

static PyObject *args_CopyAnnotationsToWindowRPC(ViewerRPC *rpc)
{
    return ViewerRPC_two_int(rpc->GetWindowLayout(), rpc->GetWindowId());
}

static PyObject *args_CopyPlotsToWindowRPC(ViewerRPC *rpc)
{
    return ViewerRPC_two_int(rpc->GetWindowLayout(), rpc->GetWindowId());
}

static PyObject *args_ClearCacheRPC(ViewerRPC *rpc) 
{
    return ViewerRPC_two_string(rpc->GetProgramHost(), rpc->GetProgramSim());
}

static PyObject *args_ClearCacheForAllEnginesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }

static PyObject *args_SetViewExtentsTypeRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_int(rpc->GetWindowLayout());
}

static PyObject *args_ClearRefLinesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetRenderingAttributesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }

static PyObject *args_DatabaseQueryRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(8);
    PyTuple_SET_ITEM(tuple, 0, PyString_FromString(rpc->GetQueryName().c_str()));
    PyTuple_SET_ITEM(tuple, 1, PyTuple_FromStringVector(rpc->GetQueryVariables()));
    PyTuple_SET_ITEM(tuple, 2, PyLong_FromLong((long)rpc->GetIntArg1()));
    PyTuple_SET_ITEM(tuple, 3, PyLong_FromLong((long)rpc->GetIntArg2()));
    PyTuple_SET_ITEM(tuple, 4, PyLong_FromLong(rpc->GetBoolFlag()?1L:0L));
    PyTuple_SET_ITEM(tuple, 5, PyLong_FromLong((long)rpc->GetIntArg3()));
    PyTuple_SET_ITEM(tuple, 6, PyTuple_FromDoubleVector(rpc->GetDoubleArg1()));
    PyTuple_SET_ITEM(tuple, 7, PyTuple_FromDoubleVector(rpc->GetDoubleArg2()));
    return tuple;
}

static PyObject *args_PointQueryRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(7);
    PyTuple_SET_ITEM(tuple, 0, PyString_FromString(rpc->GetQueryName().c_str()));
    PyTuple_SET_ITEM(tuple, 1, PyTuple_FromDoubleArray(rpc->GetQueryPoint1(), 3));
    PyTuple_SET_ITEM(tuple, 2, PyTuple_FromStringVector(rpc->GetQueryVariables()));
    PyTuple_SET_ITEM(tuple, 3, PyLong_FromLong(rpc->GetBoolFlag()?1L:0L));
    PyTuple_SET_ITEM(tuple, 4, PyLong_FromLong((long)rpc->GetIntArg1()));
    PyTuple_SET_ITEM(tuple, 5, PyLong_FromLong((long)rpc->GetIntArg2()));
    PyTuple_SET_ITEM(tuple, 6, PyLong_FromLong((long)rpc->GetIntArg3()));
    return tuple;
}

static PyObject *args_LineQueryRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(6);
    PyTuple_SET_ITEM(tuple, 0, PyString_FromString(rpc->GetQueryName().c_str()));
    PyTuple_SET_ITEM(tuple, 1, PyTuple_FromDoubleArray(rpc->GetQueryPoint1(), 3));
    PyTuple_SET_ITEM(tuple, 2, PyTuple_FromDoubleArray(rpc->GetQueryPoint2(), 3));
    PyTuple_SET_ITEM(tuple, 3, PyTuple_FromStringVector(rpc->GetQueryVariables()));
    PyTuple_SET_ITEM(tuple, 4, PyLong_FromLong((long)rpc->GetIntArg1()));
    PyTuple_SET_ITEM(tuple, 5, PyLong_FromLong(rpc->GetBoolFlag()?1L:0L));
    return tuple;
}

static PyObject *args_CloneWindowRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetMaterialAttributesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetDefaultMaterialAttributesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ResetMaterialAttributesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }

static PyObject *args_SetPlotDatabaseStateRPC(ViewerRPC *rpc)
{
    return ViewerRPC_three_int(rpc->GetIntArg1(), rpc->GetIntArg2(), rpc->GetIntArg3());
}

static PyObject *args_DeletePlotDatabaseKeyframeRPC(ViewerRPC *rpc)
{
    return ViewerRPC_two_int(rpc->GetIntArg1(), rpc->GetIntArg2());
}

static PyObject *args_MovePlotDatabaseKeyframeRPC(ViewerRPC *rpc)
{
    return ViewerRPC_three_int(rpc->GetIntArg1(), rpc->GetIntArg2(), rpc->GetIntArg3());
}

static PyObject *args_ClearViewKeyframesRPC(ViewerRPC *rpc) { return ViewerRPC_no_args(); }

static PyObject *args_DeleteViewKeyframeRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_int(rpc->GetFrame());
}

static PyObject *args_MoveViewKeyframeRPC(ViewerRPC *rpc)
{
    return ViewerRPC_two_int(rpc->GetIntArg1(), rpc->GetIntArg2());
}

static PyObject *args_SetViewKeyframeRPC(ViewerRPC *) { return ViewerRPC_no_args(); }

static PyObject *args_OpenMDServerRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyString_FromString(rpc->GetProgramHost().c_str()));
    PyTuple_SET_ITEM(tuple, 1, PyTuple_FromStringVector(rpc->GetProgramOptions()));
    return tuple;
}

static PyObject *args_EnableToolbarRPC(ViewerRPC *rpc)
{
    return ViewerRPC_two_int(rpc->GetToolId(), rpc->GetBoolFlag()?1:0);
}

static PyObject *args_HideToolbarsRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_HideToolbarsForAllWindowsRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ShowToolbarsRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ShowToolbarsForAllWindowsRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetToolbarIconSizeRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SaveViewRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetGlobalLineoutAttributesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetPickAttributesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }

static PyObject *args_ExportColorTableRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_string(rpc->GetColorTableName());
}

static PyObject *args_ExportEntireStateRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_string(rpc->GetVariable());
}

static PyObject *args_ImportEntireStateRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyString_FromString(rpc->GetVariable().c_str()));
    PyTuple_SET_ITEM(tuple, 1, PyLong_FromLong(rpc->GetBoolFlag()?1L:0L));
    return tuple;
}

static PyObject *args_ImportEntireStateWithDifferentSourcesRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(3);
    PyTuple_SET_ITEM(tuple, 0, PyString_FromString(rpc->GetVariable().c_str()));
    PyTuple_SET_ITEM(tuple, 1, PyLong_FromLong(rpc->GetBoolFlag()?1L:0L));
    PyTuple_SET_ITEM(tuple, 2, PyTuple_FromStringVector(rpc->GetProgramOptions()));
    return tuple;
}

static PyObject *args_ResetPickAttributesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }

static PyObject *args_AddAnnotationObjectRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyLong_FromLong(rpc->GetIntArg1()));
    PyTuple_SET_ITEM(tuple, 1, PyString_FromString(rpc->GetStringArg1().c_str()));
    return tuple;
}

static PyObject *args_HideActiveAnnotationObjectsRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_DeleteActiveAnnotationObjectsRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_RaiseActiveAnnotationObjectsRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_LowerActiveAnnotationObjectsRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetAnnotationObjectOptionsRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetDefaultAnnotationObjectListRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ResetAnnotationObjectListRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ResetPickLetterRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetDefaultPickAttributesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }

static PyObject *args_ChooseCenterOfRotationRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyLong_FromLong(rpc->GetBoolFlag()?1L:0L));
    PyTuple_SET_ITEM(tuple, 1, PyTuple_FromDoubleArray(rpc->GetQueryPoint1(), 3));
    return tuple;
}

static PyObject *args_SetCenterOfRotationRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(1);
    PyTuple_SET_ITEM(tuple, 0, PyTuple_FromDoubleArray(rpc->GetQueryPoint1(), 3));
    return tuple;
}

static PyObject *args_SetQueryOverTimeAttributesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetDefaultQueryOverTimeAttributesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ResetQueryOverTimeAttributesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ResetLineoutColorRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetInteractorAttributesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetDefaultInteractorAttributesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ResetInteractorAttributesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }

static PyObject *args_GetProcInfoRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(3);
    PyTuple_SET_ITEM(tuple, 0, PyLong_FromLong((long)rpc->GetIntArg1()));
    PyTuple_SET_ITEM(tuple, 1, PyString_FromString(rpc->GetProgramHost().c_str()));
    PyTuple_SET_ITEM(tuple, 2, PyTuple_FromStringVector(rpc->GetProgramOptions()));
    return tuple;
}

static PyObject *args_SendSimulationCommandRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(4);
    PyTuple_SET_ITEM(tuple, 0, PyString_FromString(rpc->GetProgramHost().c_str()));
    PyTuple_SET_ITEM(tuple, 1, PyString_FromString(rpc->GetProgramSim().c_str()));
    PyTuple_SET_ITEM(tuple, 2, PyString_FromString(rpc->GetStringArg1().c_str()));
    PyTuple_SET_ITEM(tuple, 3, PyString_FromString(rpc->GetStringArg2().c_str()));
    return tuple;
}

static PyObject *args_UpdateDBPluginInfoRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_string(rpc->GetProgramHost());
}

static PyObject *args_ExportDBRPC(ViewerRPC *) { return ViewerRPC_no_args(); }

static PyObject *args_SetTryHarderCyclesTimesRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_int(rpc->GetIntArg1());
}

static PyObject *args_OpenClientRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(3);
    PyTuple_SET_ITEM(tuple, 0, PyString_FromString(rpc->GetDatabase().c_str()));
    PyTuple_SET_ITEM(tuple, 1, PyString_FromString(rpc->GetProgramHost().c_str()));
    PyTuple_SET_ITEM(tuple, 2, PyTuple_FromStringVector(rpc->GetProgramOptions()));
    return tuple;
}

static PyObject *args_SuppressQueryOutputRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_bool(rpc->GetBoolFlag());
}

static PyObject *args_SetQueryFloatFormatRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_string(rpc->GetStringArg1());
}

static PyObject *args_SetMeshManagementAttributesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetDefaultMeshManagementAttributesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ResetMeshManagementAttributesRPC(ViewerRPC *) { return ViewerRPC_no_args(); }

static PyObject *args_ResizeWindowRPC(ViewerRPC *rpc)
{
    return ViewerRPC_three_int(rpc->GetWindowId(), rpc->GetIntArg1(), rpc->GetIntArg2());
}

static PyObject *args_MoveWindowRPC(ViewerRPC *rpc)
{
    return ViewerRPC_three_int(rpc->GetWindowId(), rpc->GetIntArg1(), rpc->GetIntArg2());
}

static PyObject *args_MoveAndResizeWindowRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(5);
    PyTuple_SET_ITEM(tuple, 0, PyLong_FromLong((long)rpc->GetWindowId()));
    PyTuple_SET_ITEM(tuple, 1, PyLong_FromLong((long)rpc->GetIntArg1()));
    PyTuple_SET_ITEM(tuple, 2, PyLong_FromLong((long)rpc->GetIntArg2()));
    PyTuple_SET_ITEM(tuple, 3, PyLong_FromLong((long)rpc->GetIntArg3()));
    PyTuple_SET_ITEM(tuple, 4, PyLong_FromLong((long)rpc->GetWindowLayout()));
    return tuple;
}

static PyObject *args_SetStateLoggingRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_ConstructDDFRPC(ViewerRPC *) { return ViewerRPC_no_args(); }

static PyObject *args_UpdatePlotInfoAttsRPC(ViewerRPC *rpc)
{
    return ViewerRPC_two_int(rpc->GetWindowId(), rpc->GetIntArg1());
}

static PyObject *args_RequestMetaDataRPC(ViewerRPC *rpc)
{
    PyObject *tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyString_FromString(rpc->GetDatabase().c_str()));
    PyTuple_SET_ITEM(tuple, 1, PyLong_FromLong((long)rpc->GetStateNumber()));
    return tuple;
}

static PyObject *args_SetTreatAllDBsAsTimeVaryingRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_int(rpc->GetIntArg1());
}

static PyObject *args_SetCreateMeshQualityExpressionsRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_int(rpc->GetIntArg1());
}

static PyObject *args_SetCreateTimeDerivativeExpressionsRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_int(rpc->GetIntArg1());
}

static PyObject *args_SetCreateVectorMagnitudeExpressionsRPC(ViewerRPC *rpc)
{
    return ViewerRPC_one_int(rpc->GetIntArg1());
}

static PyObject *args_CopyActivePlotsRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetPlotFollowsTimeRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_TurnOffAllLocksRPC(ViewerRPC *) { return ViewerRPC_no_args(); }
static PyObject *args_SetDefaultFileOpenOptionsRPC(ViewerRPC *) { return ViewerRPC_no_args(); }

// ****************************************************************************
// Method: args_ViewerRPC
//
// Purpose: 
//   This function creates a tuple of arguments for the input rpc, exposing
//   only the relevant pieces of the ViewerRPC. The order of data in the tuple
//   matches the signatures of the ViewerMethods.h methods.
//
// Arguments:
//   rpc : The rpc that for which we're getting arguments.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 15:11:27 PST 2008
//
// Modifications:
//   
// ****************************************************************************

PyObject *
args_ViewerRPC(ViewerRPC *rpc)
{
    PyObject *args = 0;

    // Assemble the arguments appropriately.
    switch(rpc->GetRPCType())
    {
    case ViewerRPC::CloseRPC:
        args = args_CloseRPC(rpc);
        break;
    case ViewerRPC::DetachRPC:
        args = args_DetachRPC(rpc);
        break;
    case ViewerRPC::AddWindowRPC:
        args = args_AddWindowRPC(rpc);
        break;
    case ViewerRPC::DeleteWindowRPC:
        args = args_DeleteWindowRPC(rpc);
        break;
    case ViewerRPC::SetWindowLayoutRPC:
        args = args_SetWindowLayoutRPC(rpc);
        break;
    case ViewerRPC::SetActiveWindowRPC:
        args = args_SetActiveWindowRPC(rpc);
        break;
    case ViewerRPC::ClearWindowRPC:
        args = args_ClearWindowRPC(rpc);
        break;
    case ViewerRPC::ClearAllWindowsRPC:
        args = args_ClearAllWindowsRPC(rpc);
        break;
    case ViewerRPC::OpenDatabaseRPC:
        args = args_OpenDatabaseRPC(rpc);
        break;
    case ViewerRPC::CloseDatabaseRPC:
        args = args_CloseDatabaseRPC(rpc);
        break;
    case ViewerRPC::ActivateDatabaseRPC:
        args = args_ActivateDatabaseRPC(rpc);
        break;
    case ViewerRPC::CheckForNewStatesRPC:
        args = args_CheckForNewStatesRPC(rpc);
        break;
    case ViewerRPC::CreateDatabaseCorrelationRPC:
        args = args_CreateDatabaseCorrelationRPC(rpc);
        break;
    case ViewerRPC::AlterDatabaseCorrelationRPC:
        args = args_AlterDatabaseCorrelationRPC(rpc);
        break;
    case ViewerRPC::DeleteDatabaseCorrelationRPC:
        args = args_DeleteDatabaseCorrelationRPC(rpc);
        break;
    case ViewerRPC::ReOpenDatabaseRPC:
        args = args_ReOpenDatabaseRPC(rpc);
        break;
    case ViewerRPC::ReplaceDatabaseRPC:
        args = args_ReplaceDatabaseRPC(rpc);
        break;
    case ViewerRPC::OverlayDatabaseRPC:
        args = args_OverlayDatabaseRPC(rpc);
        break;
    case ViewerRPC::OpenComputeEngineRPC:
        args = args_OpenComputeEngineRPC(rpc);
        break;
    case ViewerRPC::CloseComputeEngineRPC:
        args = args_CloseComputeEngineRPC(rpc);
        break;
    case ViewerRPC::AnimationSetNFramesRPC:
        args = args_AnimationSetNFramesRPC(rpc);
        break;
    case ViewerRPC::AnimationPlayRPC:
        args = args_AnimationPlayRPC(rpc);
        break;
    case ViewerRPC::AnimationReversePlayRPC:
        args = args_AnimationReversePlayRPC(rpc);
        break;
    case ViewerRPC::AnimationStopRPC:
        args = args_AnimationStopRPC(rpc);
        break;
    case ViewerRPC::TimeSliderNextStateRPC:
        args = args_TimeSliderNextStateRPC(rpc);
        break;
    case ViewerRPC::TimeSliderPreviousStateRPC:
        args = args_TimeSliderPreviousStateRPC(rpc);
        break;
    case ViewerRPC::SetTimeSliderStateRPC:
        args = args_SetTimeSliderStateRPC(rpc);
        break;
    case ViewerRPC::SetActiveTimeSliderRPC:
        args = args_SetActiveTimeSliderRPC(rpc);
        break;
    case ViewerRPC::AddPlotRPC:
        args = args_AddPlotRPC(rpc);
        break;
    case ViewerRPC::SetPlotFrameRangeRPC:
        args = args_SetPlotFrameRangeRPC(rpc);
        break;
    case ViewerRPC::DeletePlotKeyframeRPC:
        args = args_DeletePlotKeyframeRPC(rpc);
        break;
    case ViewerRPC::MovePlotKeyframeRPC:
        args = args_MovePlotKeyframeRPC(rpc);
        break;
    case ViewerRPC::DeleteActivePlotsRPC:
        args = args_DeleteActivePlotsRPC(rpc);
        break;
    case ViewerRPC::HideActivePlotsRPC:
        args = args_HideActivePlotsRPC(rpc);
        break;
    case ViewerRPC::DrawPlotsRPC:
        args = args_DrawPlotsRPC(rpc);
        break;
    case ViewerRPC::DisableRedrawRPC:
        args = args_DisableRedrawRPC(rpc);
        break;
    case ViewerRPC::RedrawRPC:
        args = args_RedrawRPC(rpc);
        break;
    case ViewerRPC::SetActivePlotsRPC:
        args = args_SetActivePlotsRPC(rpc);
        break;
    case ViewerRPC::ChangeActivePlotsVarRPC:
        args = args_ChangeActivePlotsVarRPC(rpc);
        break;
    case ViewerRPC::AddOperatorRPC:
        args = args_AddOperatorRPC(rpc);
        break;
    case ViewerRPC::AddInitializedOperatorRPC:
        args = args_AddInitializedOperatorRPC(rpc);
        break;
    case ViewerRPC::PromoteOperatorRPC:
        args = args_PromoteOperatorRPC(rpc);
        break;
    case ViewerRPC::DemoteOperatorRPC:
        args = args_DemoteOperatorRPC(rpc);
        break;
    case ViewerRPC::RemoveOperatorRPC:
        args = args_RemoveOperatorRPC(rpc);
        break;
    case ViewerRPC::RemoveLastOperatorRPC:
        args = args_RemoveLastOperatorRPC(rpc);
        break;
    case ViewerRPC::RemoveAllOperatorsRPC:
        args = args_RemoveAllOperatorsRPC(rpc);
        break;
    case ViewerRPC::SaveWindowRPC:
        args = args_SaveWindowRPC(rpc);
        break;
    case ViewerRPC::SetDefaultPlotOptionsRPC:
        args = args_SetDefaultPlotOptionsRPC(rpc);
        break;
    case ViewerRPC::SetPlotOptionsRPC:
        args = args_SetPlotOptionsRPC(rpc);
        break;
    case ViewerRPC::SetDefaultOperatorOptionsRPC:
        args = args_SetDefaultOperatorOptionsRPC(rpc);
        break;
    case ViewerRPC::SetOperatorOptionsRPC:
        args = args_SetOperatorOptionsRPC(rpc);
        break;
    case ViewerRPC::WriteConfigFileRPC:
        args = args_WriteConfigFileRPC(rpc);
        break;
    case ViewerRPC::ConnectToMetaDataServerRPC:
        args = args_ConnectToMetaDataServerRPC(rpc);
        break;
    case ViewerRPC::IconifyAllWindowsRPC:
        args = args_IconifyAllWindowsRPC(rpc);
        break;
    case ViewerRPC::DeIconifyAllWindowsRPC:
        args = args_DeIconifyAllWindowsRPC(rpc);
        break;
    case ViewerRPC::ShowAllWindowsRPC:
        args = args_ShowAllWindowsRPC(rpc);
        break;
    case ViewerRPC::HideAllWindowsRPC:
        args = args_HideAllWindowsRPC(rpc);
        break;
    case ViewerRPC::UpdateColorTableRPC:
        args = args_UpdateColorTableRPC(rpc);
        break;
    case ViewerRPC::SetAnnotationAttributesRPC:
        args = args_SetAnnotationAttributesRPC(rpc);
        break;
    case ViewerRPC::SetDefaultAnnotationAttributesRPC:
        args = args_SetDefaultAnnotationAttributesRPC(rpc);
        break;
    case ViewerRPC::ResetAnnotationAttributesRPC:
        args = args_ResetAnnotationAttributesRPC(rpc);
        break;
    case ViewerRPC::SetKeyframeAttributesRPC:
        args = args_SetKeyframeAttributesRPC(rpc);
        break;
    case ViewerRPC::SetPlotSILRestrictionRPC:
        args = args_SetPlotSILRestrictionRPC(rpc);
        break;
    case ViewerRPC::SetViewCurveRPC:
        args = args_SetViewCurveRPC(rpc);
        break;
    case ViewerRPC::SetView2DRPC:
        args = args_SetView2DRPC(rpc);
        break;
    case ViewerRPC::SetView3DRPC:
        args = args_SetView3DRPC(rpc);
        break;
    case ViewerRPC::ResetPlotOptionsRPC:
        args = args_ResetPlotOptionsRPC(rpc);
        break;
    case ViewerRPC::ResetOperatorOptionsRPC:
        args = args_ResetOperatorOptionsRPC(rpc);
        break;
    case ViewerRPC::SetAppearanceRPC:
        args = args_SetAppearanceRPC(rpc);
        break;
    case ViewerRPC::ProcessExpressionsRPC:
        args = args_ProcessExpressionsRPC(rpc);
        break;
    case ViewerRPC::SetLightListRPC:
        args = args_SetLightListRPC(rpc);
        break;
    case ViewerRPC::SetDefaultLightListRPC:
        args = args_SetDefaultLightListRPC(rpc);
        break;
    case ViewerRPC::ResetLightListRPC:
        args = args_ResetLightListRPC(rpc);
        break;
    case ViewerRPC::SetAnimationAttributesRPC:
        args = args_SetAnimationAttributesRPC(rpc);
        break;
    case ViewerRPC::SetWindowAreaRPC:
        args = args_SetWindowAreaRPC(rpc);
        break;
    case ViewerRPC::PrintWindowRPC:
        args = args_PrintWindowRPC(rpc);
        break;
    case ViewerRPC::ResetViewRPC:
        args = args_ResetViewRPC(rpc);
        break;
    case ViewerRPC::RecenterViewRPC:
        args = args_RecenterViewRPC(rpc);
        break;
    case ViewerRPC::ToggleMaintainViewModeRPC:
        args = args_ToggleMaintainViewModeRPC(rpc);
        break;
    case ViewerRPC::ToggleMaintainDataModeRPC:
        args = args_ToggleMaintainDataModeRPC(rpc);
        break;
    case ViewerRPC::ToggleBoundingBoxModeRPC:
        args = args_ToggleBoundingBoxModeRPC(rpc);
        break;
    case ViewerRPC::ToggleCameraViewModeRPC:
        args = args_ToggleCameraViewModeRPC(rpc);
        break;
    case ViewerRPC::TogglePerspectiveViewRPC:
        args = args_TogglePerspectiveViewRPC(rpc);
        break;
    case ViewerRPC::ToggleSpinModeRPC:
        args = args_ToggleSpinModeRPC(rpc);
        break;
    case ViewerRPC::ToggleLockTimeRPC:
        args = args_ToggleLockTimeRPC(rpc);
        break;
    case ViewerRPC::ToggleLockToolsRPC:
        args = args_ToggleLockToolsRPC(rpc);
        break;
    case ViewerRPC::ToggleLockViewModeRPC:
        args = args_ToggleLockViewModeRPC(rpc);
        break;
    case ViewerRPC::ToggleFullFrameRPC:
        args = args_ToggleFullFrameRPC(rpc);
        break;
    case ViewerRPC::UndoViewRPC:
        args = args_UndoViewRPC(rpc);
        break;
    case ViewerRPC::RedoViewRPC:
        args = args_RedoViewRPC(rpc);
        break;
    case ViewerRPC::InvertBackgroundRPC:
        args = args_InvertBackgroundRPC(rpc);
        break;
    case ViewerRPC::ClearPickPointsRPC:
        args = args_ClearPickPointsRPC(rpc);
        break;
    case ViewerRPC::SetWindowModeRPC:
        args = args_SetWindowModeRPC(rpc);
        break;
    case ViewerRPC::EnableToolRPC:
        args = args_EnableToolRPC(rpc);
        break;
    case ViewerRPC::CopyViewToWindowRPC:
        args = args_CopyViewToWindowRPC(rpc);
        break;
    case ViewerRPC::CopyLightingToWindowRPC:
        args = args_CopyLightingToWindowRPC(rpc);
        break;
    case ViewerRPC::CopyAnnotationsToWindowRPC:
        args = args_CopyAnnotationsToWindowRPC(rpc);
        break;
    case ViewerRPC::CopyPlotsToWindowRPC:
        args = args_CopyPlotsToWindowRPC(rpc);
        break;
    case ViewerRPC::ClearCacheRPC:
        args = args_ClearCacheRPC(rpc);
        break;
    case ViewerRPC::ClearCacheForAllEnginesRPC:
        args = args_ClearCacheForAllEnginesRPC(rpc);
        break;
    case ViewerRPC::SetViewExtentsTypeRPC:
        args = args_SetViewExtentsTypeRPC(rpc);
        break;
    case ViewerRPC::ClearRefLinesRPC:
        args = args_ClearRefLinesRPC(rpc);
        break;
    case ViewerRPC::SetRenderingAttributesRPC:
        args = args_SetRenderingAttributesRPC(rpc);
        break;
    case ViewerRPC::DatabaseQueryRPC:
        args = args_DatabaseQueryRPC(rpc);
        break;
    case ViewerRPC::PointQueryRPC:
        args = args_PointQueryRPC(rpc);
        break;
    case ViewerRPC::LineQueryRPC:
        args = args_LineQueryRPC(rpc);
        break;
    case ViewerRPC::CloneWindowRPC:
        args = args_CloneWindowRPC(rpc);
        break;
    case ViewerRPC::SetMaterialAttributesRPC:
        args = args_SetMaterialAttributesRPC(rpc);
        break;
    case ViewerRPC::SetDefaultMaterialAttributesRPC:
        args = args_SetDefaultMaterialAttributesRPC(rpc);
        break;
    case ViewerRPC::ResetMaterialAttributesRPC:
        args = args_ResetMaterialAttributesRPC(rpc);
        break;
    case ViewerRPC::SetPlotDatabaseStateRPC:
        args = args_SetPlotDatabaseStateRPC(rpc);
        break;
    case ViewerRPC::DeletePlotDatabaseKeyframeRPC:
        args = args_DeletePlotDatabaseKeyframeRPC(rpc);
        break;
    case ViewerRPC::MovePlotDatabaseKeyframeRPC:
        args = args_MovePlotDatabaseKeyframeRPC(rpc);
        break;
    case ViewerRPC::ClearViewKeyframesRPC:
        args = args_ClearViewKeyframesRPC(rpc);
        break;
    case ViewerRPC::DeleteViewKeyframeRPC:
        args = args_DeleteViewKeyframeRPC(rpc);
        break;
    case ViewerRPC::MoveViewKeyframeRPC:
        args = args_MoveViewKeyframeRPC(rpc);
        break;
    case ViewerRPC::SetViewKeyframeRPC:
        args = args_SetViewKeyframeRPC(rpc);
        break;
    case ViewerRPC::OpenMDServerRPC:
        args = args_OpenMDServerRPC(rpc);
        break;
    case ViewerRPC::EnableToolbarRPC:
        args = args_EnableToolbarRPC(rpc);
        break;
    case ViewerRPC::HideToolbarsRPC:
        args = args_HideToolbarsRPC(rpc);
        break;
    case ViewerRPC::HideToolbarsForAllWindowsRPC:
        args = args_HideToolbarsForAllWindowsRPC(rpc);
        break;
    case ViewerRPC::ShowToolbarsRPC:
        args = args_ShowToolbarsRPC(rpc);
        break;
    case ViewerRPC::ShowToolbarsForAllWindowsRPC:
        args = args_ShowToolbarsForAllWindowsRPC(rpc);
        break;
    case ViewerRPC::SetToolbarIconSizeRPC:
        args = args_SetToolbarIconSizeRPC(rpc);
        break;
    case ViewerRPC::SaveViewRPC:
        args = args_SaveViewRPC(rpc);
        break;
    case ViewerRPC::SetGlobalLineoutAttributesRPC:
        args = args_SetGlobalLineoutAttributesRPC(rpc);
        break;
    case ViewerRPC::SetPickAttributesRPC:
        args = args_SetPickAttributesRPC(rpc);
        break;
    case ViewerRPC::ExportColorTableRPC:
        args = args_ExportColorTableRPC(rpc);
        break;
    case ViewerRPC::ExportEntireStateRPC:
        args = args_ExportEntireStateRPC(rpc);
        break;
    case ViewerRPC::ImportEntireStateRPC:
        args = args_ImportEntireStateRPC(rpc);
        break;
    case ViewerRPC::ImportEntireStateWithDifferentSourcesRPC:
        args = args_ImportEntireStateWithDifferentSourcesRPC(rpc);
        break;
    case ViewerRPC::ResetPickAttributesRPC:
        args = args_ResetPickAttributesRPC(rpc);
        break;
    case ViewerRPC::AddAnnotationObjectRPC:
        args = args_AddAnnotationObjectRPC(rpc);
        break;
    case ViewerRPC::HideActiveAnnotationObjectsRPC:
        args = args_HideActiveAnnotationObjectsRPC(rpc);
        break;
    case ViewerRPC::DeleteActiveAnnotationObjectsRPC:
        args = args_DeleteActiveAnnotationObjectsRPC(rpc);
        break;
    case ViewerRPC::RaiseActiveAnnotationObjectsRPC:
        args = args_RaiseActiveAnnotationObjectsRPC(rpc);
        break;
    case ViewerRPC::LowerActiveAnnotationObjectsRPC:
        args = args_LowerActiveAnnotationObjectsRPC(rpc);
        break;
    case ViewerRPC::SetAnnotationObjectOptionsRPC:
        args = args_SetAnnotationObjectOptionsRPC(rpc);
        break;
    case ViewerRPC::SetDefaultAnnotationObjectListRPC:
        args = args_SetDefaultAnnotationObjectListRPC(rpc);
        break;
    case ViewerRPC::ResetAnnotationObjectListRPC:
        args = args_ResetAnnotationObjectListRPC(rpc);
        break;
    case ViewerRPC::ResetPickLetterRPC:
        args = args_ResetPickLetterRPC(rpc);
        break;
    case ViewerRPC::SetDefaultPickAttributesRPC:
        args = args_SetDefaultPickAttributesRPC(rpc);
        break;
    case ViewerRPC::ChooseCenterOfRotationRPC:
        args = args_ChooseCenterOfRotationRPC(rpc);
        break;
    case ViewerRPC::SetCenterOfRotationRPC:
        args = args_SetCenterOfRotationRPC(rpc);
        break;
    case ViewerRPC::SetQueryOverTimeAttributesRPC:
        args = args_SetQueryOverTimeAttributesRPC(rpc);
        break;
    case ViewerRPC::SetDefaultQueryOverTimeAttributesRPC:
        args = args_SetDefaultQueryOverTimeAttributesRPC(rpc);
        break;
    case ViewerRPC::ResetQueryOverTimeAttributesRPC:
        args = args_ResetQueryOverTimeAttributesRPC(rpc);
        break;
    case ViewerRPC::ResetLineoutColorRPC:
        args = args_ResetLineoutColorRPC(rpc);
        break;
    case ViewerRPC::SetInteractorAttributesRPC:
        args = args_SetInteractorAttributesRPC(rpc);
        break;
    case ViewerRPC::SetDefaultInteractorAttributesRPC:
        args = args_SetDefaultInteractorAttributesRPC(rpc);
        break;
    case ViewerRPC::ResetInteractorAttributesRPC:
        args = args_ResetInteractorAttributesRPC(rpc);
        break;
    case ViewerRPC::GetProcInfoRPC:
        args = args_GetProcInfoRPC(rpc);
        break;
    case ViewerRPC::SendSimulationCommandRPC:
        args = args_SendSimulationCommandRPC(rpc);
        break;
    case ViewerRPC::UpdateDBPluginInfoRPC:
        args = args_UpdateDBPluginInfoRPC(rpc);
        break;
    case ViewerRPC::ExportDBRPC:
        args = args_ExportDBRPC(rpc);
        break;
    case ViewerRPC::SetTryHarderCyclesTimesRPC:
        args = args_SetTryHarderCyclesTimesRPC(rpc);
        break;
    case ViewerRPC::OpenClientRPC:
        args = args_OpenClientRPC(rpc);
        break;
    case ViewerRPC::SuppressQueryOutputRPC:
        args = args_SuppressQueryOutputRPC(rpc);
        break;
    case ViewerRPC::SetQueryFloatFormatRPC:
        args = args_SetQueryFloatFormatRPC(rpc);
        break;
    case ViewerRPC::SetMeshManagementAttributesRPC:
        args = args_SetMeshManagementAttributesRPC(rpc);
        break;
    case ViewerRPC::SetDefaultMeshManagementAttributesRPC:
        args = args_SetDefaultMeshManagementAttributesRPC(rpc);
        break;
    case ViewerRPC::ResetMeshManagementAttributesRPC:
        args = args_ResetMeshManagementAttributesRPC(rpc);
        break;
    case ViewerRPC::ResizeWindowRPC:
        args = args_ResizeWindowRPC(rpc);
        break;
    case ViewerRPC::MoveWindowRPC:
        args = args_MoveWindowRPC(rpc);
        break;
    case ViewerRPC::MoveAndResizeWindowRPC:
        args = args_MoveAndResizeWindowRPC(rpc);
        break;
    case ViewerRPC::SetStateLoggingRPC:
        args = args_SetStateLoggingRPC(rpc);
        break;
    case ViewerRPC::ConstructDDFRPC:
        args = args_ConstructDDFRPC(rpc);
        break;
    case ViewerRPC::UpdatePlotInfoAttsRPC:
        args = args_UpdatePlotInfoAttsRPC(rpc);
        break;
    case ViewerRPC::RequestMetaDataRPC:
        args = args_RequestMetaDataRPC(rpc);
        break;
    case ViewerRPC::SetTreatAllDBsAsTimeVaryingRPC:
        args = args_SetTreatAllDBsAsTimeVaryingRPC(rpc);
        break;
    case ViewerRPC::SetCreateMeshQualityExpressionsRPC:
        args = args_SetCreateMeshQualityExpressionsRPC(rpc);
        break;
    case ViewerRPC::SetCreateTimeDerivativeExpressionsRPC:
        args = args_SetCreateTimeDerivativeExpressionsRPC(rpc);
        break;
    case ViewerRPC::SetCreateVectorMagnitudeExpressionsRPC:
        args = args_SetCreateVectorMagnitudeExpressionsRPC(rpc);
        break;
    case ViewerRPC::CopyActivePlotsRPC:
        args = args_CopyActivePlotsRPC(rpc);
        break;
    case ViewerRPC::SetPlotFollowsTimeRPC:
        args = args_SetPlotFollowsTimeRPC(rpc);
        break;
    case ViewerRPC::TurnOffAllLocksRPC:
        args = args_TurnOffAllLocksRPC(rpc);
        break;
    case ViewerRPC::SetDefaultFileOpenOptionsRPC:
        args = args_SetDefaultFileOpenOptionsRPC(rpc);
        break;
    default:
        // The ViewerRPC is not handled so just wrap the whole ViewerRPC and pass
        // that to the user's callback.
        args = ViewerRPC_wrap_ViewerRPC(rpc);
    }

    return args;
}
