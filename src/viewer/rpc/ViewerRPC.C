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

#include <ViewerRPC.h>
#include <DataNode.h>
#include <snprintf.h>

//
// Enum conversion methods for ViewerRPC::ViewerRPCType
//

static const char *ViewerRPCType_strings[] = {
"CloseRPC", "DetachRPC", "AddWindowRPC", 
"DeleteWindowRPC", "SetWindowLayoutRPC", "SetActiveWindowRPC", 
"ClearWindowRPC", "ClearAllWindowsRPC", "OpenDatabaseRPC", 
"CloseDatabaseRPC", "ActivateDatabaseRPC", "CheckForNewStatesRPC", 
"CreateDatabaseCorrelationRPC", "AlterDatabaseCorrelationRPC", "DeleteDatabaseCorrelationRPC", 
"ReOpenDatabaseRPC", "ReplaceDatabaseRPC", "OverlayDatabaseRPC", 
"OpenComputeEngineRPC", "CloseComputeEngineRPC", "AnimationSetNFramesRPC", 
"AnimationPlayRPC", "AnimationReversePlayRPC", "AnimationStopRPC", 
"TimeSliderNextStateRPC", "TimeSliderPreviousStateRPC", "SetTimeSliderStateRPC", 
"SetActiveTimeSliderRPC", "AddPlotRPC", "SetPlotFrameRangeRPC", 
"DeletePlotKeyframeRPC", "MovePlotKeyframeRPC", "DeleteActivePlotsRPC", 
"HideActivePlotsRPC", "DrawPlotsRPC", "DisableRedrawRPC", 
"RedrawRPC", "SetActivePlotsRPC", "ChangeActivePlotsVarRPC", 
"AddOperatorRPC", "AddInitializedOperatorRPC", "PromoteOperatorRPC", 
"DemoteOperatorRPC", "RemoveOperatorRPC", "RemoveLastOperatorRPC", 
"RemoveAllOperatorsRPC", "SaveWindowRPC", "SetDefaultPlotOptionsRPC", 
"SetPlotOptionsRPC", "SetDefaultOperatorOptionsRPC", "SetOperatorOptionsRPC", 
"WriteConfigFileRPC", "ConnectToMetaDataServerRPC", "IconifyAllWindowsRPC", 
"DeIconifyAllWindowsRPC", "ShowAllWindowsRPC", "HideAllWindowsRPC", 
"UpdateColorTableRPC", "SetAnnotationAttributesRPC", "SetDefaultAnnotationAttributesRPC", 
"ResetAnnotationAttributesRPC", "SetKeyframeAttributesRPC", "SetPlotSILRestrictionRPC", 
"SetViewAxisArrayRPC", "SetViewCurveRPC", "SetView2DRPC", 
"SetView3DRPC", "ResetPlotOptionsRPC", "ResetOperatorOptionsRPC", 
"SetAppearanceRPC", "ProcessExpressionsRPC", "SetLightListRPC", 
"SetDefaultLightListRPC", "ResetLightListRPC", "SetAnimationAttributesRPC", 
"SetWindowAreaRPC", "PrintWindowRPC", "ResetViewRPC", 
"RecenterViewRPC", "ToggleMaintainViewModeRPC", "ToggleBoundingBoxModeRPC", 
"ToggleCameraViewModeRPC", "TogglePerspectiveViewRPC", "ToggleSpinModeRPC", 
"ToggleLockTimeRPC", "ToggleLockToolsRPC", "ToggleLockViewModeRPC", 
"ToggleFullFrameRPC", "UndoViewRPC", "RedoViewRPC", 
"InvertBackgroundRPC", "ClearPickPointsRPC", "SetWindowModeRPC", 
"EnableToolRPC", "SetToolUpdateModeRPC", "CopyViewToWindowRPC", 
"CopyLightingToWindowRPC", "CopyAnnotationsToWindowRPC", "CopyPlotsToWindowRPC", 
"ClearCacheRPC", "ClearCacheForAllEnginesRPC", "SetViewExtentsTypeRPC", 
"ClearRefLinesRPC", "SetRenderingAttributesRPC", "DatabaseQueryRPC", 
"PointQueryRPC", "LineQueryRPC", "CloneWindowRPC", 
"SetMaterialAttributesRPC", "SetDefaultMaterialAttributesRPC", "ResetMaterialAttributesRPC", 
"SetPlotDatabaseStateRPC", "DeletePlotDatabaseKeyframeRPC", "MovePlotDatabaseKeyframeRPC", 
"ClearViewKeyframesRPC", "DeleteViewKeyframeRPC", "MoveViewKeyframeRPC", 
"SetViewKeyframeRPC", "OpenMDServerRPC", "EnableToolbarRPC", 
"HideToolbarsRPC", "HideToolbarsForAllWindowsRPC", "ShowToolbarsRPC", 
"ShowToolbarsForAllWindowsRPC", "SetToolbarIconSizeRPC", "SaveViewRPC", 
"SetGlobalLineoutAttributesRPC", "SetPickAttributesRPC", "ExportColorTableRPC", 
"ExportEntireStateRPC", "ImportEntireStateRPC", "ImportEntireStateWithDifferentSourcesRPC", 
"ResetPickAttributesRPC", "AddAnnotationObjectRPC", "HideActiveAnnotationObjectsRPC", 
"DeleteActiveAnnotationObjectsRPC", "RaiseActiveAnnotationObjectsRPC", "LowerActiveAnnotationObjectsRPC", 
"SetAnnotationObjectOptionsRPC", "SetDefaultAnnotationObjectListRPC", "ResetAnnotationObjectListRPC", 
"ResetPickLetterRPC", "SetDefaultPickAttributesRPC", "ChooseCenterOfRotationRPC", 
"SetCenterOfRotationRPC", "SetQueryOverTimeAttributesRPC", "SetDefaultQueryOverTimeAttributesRPC", 
"ResetQueryOverTimeAttributesRPC", "ResetLineoutColorRPC", "SetInteractorAttributesRPC", 
"SetDefaultInteractorAttributesRPC", "ResetInteractorAttributesRPC", "GetProcInfoRPC", 
"SendSimulationCommandRPC", "UpdateDBPluginInfoRPC", "ExportDBRPC", 
"SetTryHarderCyclesTimesRPC", "OpenClientRPC", "OpenGUIClientRPC", 
"OpenCLIClientRPC", "SuppressQueryOutputRPC", "SetQueryFloatFormatRPC", 
"SetMeshManagementAttributesRPC", "SetDefaultMeshManagementAttributesRPC", "ResetMeshManagementAttributesRPC", 
"ResizeWindowRPC", "MoveWindowRPC", "MoveAndResizeWindowRPC", 
"SetStateLoggingRPC", "ConstructDDFRPC", "RequestMetaDataRPC", 
"SetTreatAllDBsAsTimeVaryingRPC", "SetCreateMeshQualityExpressionsRPC", "SetCreateTimeDerivativeExpressionsRPC", 
"SetCreateVectorMagnitudeExpressionsRPC", "CopyActivePlotsRPC", "SetPlotFollowsTimeRPC", 
"TurnOffAllLocksRPC", "SetDefaultFileOpenOptionsRPC", "SetSuppressMessagesRPC", 
"ApplyNamedSelectionRPC", "CreateNamedSelectionRPC", "DeleteNamedSelectionRPC", 
"LoadNamedSelectionRPC", "SaveNamedSelectionRPC", "MenuQuitRPC", 
"SetPlotDescriptionRPC", "MovePlotOrderTowardFirstRPC", "MovePlotOrderTowardLastRPC", 
"SetPlotOrderToFirstRPC", "SetPlotOrderToLastRPC", "UndoLastActionRPC", 
"RedoLastActionRPC", "MaxRPC"};

std::string
ViewerRPC::ViewerRPCType_ToString(ViewerRPC::ViewerRPCType t)
{
    int index = int(t);
    if(index < 0 || index >= 194) index = 0;
    return ViewerRPCType_strings[index];
}

std::string
ViewerRPC::ViewerRPCType_ToString(int t)
{
    int index = (t < 0 || t >= 194) ? 0 : t;
    return ViewerRPCType_strings[index];
}

bool
ViewerRPC::ViewerRPCType_FromString(const std::string &s, ViewerRPC::ViewerRPCType &val)
{
    val = ViewerRPC::CloseRPC;
    for(int i = 0; i < 194; ++i)
    {
        if(s == ViewerRPCType_strings[i])
        {
            val = (ViewerRPCType)i;
            return true;
        }
    }
    return false;
}

// ****************************************************************************
// Method: ViewerRPC::ViewerRPC
//
// Purpose: 
//   Init utility for the ViewerRPC class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void ViewerRPC::Init()
{
    RPCType = CloseRPC;
    windowLayout = 1;
    windowId = 0;
    windowMode = 0;
    nFrames = 0;
    stateNumber = 0;
    frameRange[0] = 0;
    frameRange[1] = 0;
    frame = 0;
    plotType = 0;
    operatorType = 0;
    queryPoint1[0] = 0;
    queryPoint1[1] = 0;
    queryPoint1[2] = 0;
    queryPoint2[0] = 0;
    queryPoint2[1] = 0;
    queryPoint2[2] = 0;
    toolId = 0;
    boolFlag = false;
    intArg1 = 0;
    intArg2 = 0;
    intArg3 = 0;
    doubleArg1.push_back(0);
    doubleArg2.push_back(0);
    toolUpdateMode = 1;

    ViewerRPC::SelectAll();
}

// ****************************************************************************
// Method: ViewerRPC::ViewerRPC
//
// Purpose: 
//   Copy utility for the ViewerRPC class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void ViewerRPC::Copy(const ViewerRPC &obj)
{
    RPCType = obj.RPCType;
    windowLayout = obj.windowLayout;
    windowId = obj.windowId;
    windowMode = obj.windowMode;
    windowArea = obj.windowArea;
    database = obj.database;
    programHost = obj.programHost;
    programSim = obj.programSim;
    programOptions = obj.programOptions;
    nFrames = obj.nFrames;
    stateNumber = obj.stateNumber;
    frameRange[0] = obj.frameRange[0];
    frameRange[1] = obj.frameRange[1];

    frame = obj.frame;
    plotType = obj.plotType;
    operatorType = obj.operatorType;
    variable = obj.variable;
    activePlotIds = obj.activePlotIds;
    activeOperatorIds = obj.activeOperatorIds;
    expandedPlotIds = obj.expandedPlotIds;
    colorTableName = obj.colorTableName;
    queryName = obj.queryName;
    queryPoint1[0] = obj.queryPoint1[0];
    queryPoint1[1] = obj.queryPoint1[1];
    queryPoint1[2] = obj.queryPoint1[2];

    queryPoint2[0] = obj.queryPoint2[0];
    queryPoint2[1] = obj.queryPoint2[1];
    queryPoint2[2] = obj.queryPoint2[2];

    queryVariables = obj.queryVariables;
    toolId = obj.toolId;
    boolFlag = obj.boolFlag;
    intArg1 = obj.intArg1;
    intArg2 = obj.intArg2;
    intArg3 = obj.intArg3;
    stringArg1 = obj.stringArg1;
    stringArg2 = obj.stringArg2;
    doubleArg1 = obj.doubleArg1;
    doubleArg2 = obj.doubleArg2;
    toolUpdateMode = obj.toolUpdateMode;

    ViewerRPC::SelectAll();
}

// Type map format string
const char *ViewerRPC::TypeMapFormatString = VIEWERRPC_TMFS;
const AttributeGroup::private_tmfs_t ViewerRPC::TmfsStruct = {VIEWERRPC_TMFS};


// ****************************************************************************
// Method: ViewerRPC::ViewerRPC
//
// Purpose: 
//   Default constructor for the ViewerRPC class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

ViewerRPC::ViewerRPC() : 
    AttributeSubject(ViewerRPC::TypeMapFormatString)
{
    ViewerRPC::Init();
}

// ****************************************************************************
// Method: ViewerRPC::ViewerRPC
//
// Purpose: 
//   Constructor for the derived classes of ViewerRPC class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

ViewerRPC::ViewerRPC(private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    ViewerRPC::Init();
}

// ****************************************************************************
// Method: ViewerRPC::ViewerRPC
//
// Purpose: 
//   Copy constructor for the ViewerRPC class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

ViewerRPC::ViewerRPC(const ViewerRPC &obj) : 
    AttributeSubject(ViewerRPC::TypeMapFormatString)
{
    ViewerRPC::Copy(obj);
}

// ****************************************************************************
// Method: ViewerRPC::ViewerRPC
//
// Purpose: 
//   Copy constructor for derived classes of the ViewerRPC class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

ViewerRPC::ViewerRPC(const ViewerRPC &obj, private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    ViewerRPC::Copy(obj);
}

// ****************************************************************************
// Method: ViewerRPC::~ViewerRPC
//
// Purpose: 
//   Destructor for the ViewerRPC class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

ViewerRPC::~ViewerRPC()
{
    // nothing here
}

// ****************************************************************************
// Method: ViewerRPC::operator = 
//
// Purpose: 
//   Assignment operator for the ViewerRPC class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

ViewerRPC& 
ViewerRPC::operator = (const ViewerRPC &obj)
{
    if (this == &obj) return *this;

    ViewerRPC::Copy(obj);

    return *this;
}

// ****************************************************************************
// Method: ViewerRPC::operator == 
//
// Purpose: 
//   Comparison operator == for the ViewerRPC class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerRPC::operator == (const ViewerRPC &obj) const
{
    // Compare the frameRange arrays.
    bool frameRange_equal = true;
    for(int i = 0; i < 2 && frameRange_equal; ++i)
        frameRange_equal = (frameRange[i] == obj.frameRange[i]);

    // Compare the queryPoint1 arrays.
    bool queryPoint1_equal = true;
    for(int i = 0; i < 3 && queryPoint1_equal; ++i)
        queryPoint1_equal = (queryPoint1[i] == obj.queryPoint1[i]);

    // Compare the queryPoint2 arrays.
    bool queryPoint2_equal = true;
    for(int i = 0; i < 3 && queryPoint2_equal; ++i)
        queryPoint2_equal = (queryPoint2[i] == obj.queryPoint2[i]);

    // Create the return value
    return ((RPCType == obj.RPCType) &&
            (windowLayout == obj.windowLayout) &&
            (windowId == obj.windowId) &&
            (windowMode == obj.windowMode) &&
            (windowArea == obj.windowArea) &&
            (database == obj.database) &&
            (programHost == obj.programHost) &&
            (programSim == obj.programSim) &&
            (programOptions == obj.programOptions) &&
            (nFrames == obj.nFrames) &&
            (stateNumber == obj.stateNumber) &&
            frameRange_equal &&
            (frame == obj.frame) &&
            (plotType == obj.plotType) &&
            (operatorType == obj.operatorType) &&
            (variable == obj.variable) &&
            (activePlotIds == obj.activePlotIds) &&
            (activeOperatorIds == obj.activeOperatorIds) &&
            (expandedPlotIds == obj.expandedPlotIds) &&
            (colorTableName == obj.colorTableName) &&
            (queryName == obj.queryName) &&
            queryPoint1_equal &&
            queryPoint2_equal &&
            (queryVariables == obj.queryVariables) &&
            (toolId == obj.toolId) &&
            (boolFlag == obj.boolFlag) &&
            (intArg1 == obj.intArg1) &&
            (intArg2 == obj.intArg2) &&
            (intArg3 == obj.intArg3) &&
            (stringArg1 == obj.stringArg1) &&
            (stringArg2 == obj.stringArg2) &&
            (doubleArg1 == obj.doubleArg1) &&
            (doubleArg2 == obj.doubleArg2) &&
            (toolUpdateMode == obj.toolUpdateMode));
}

// ****************************************************************************
// Method: ViewerRPC::operator != 
//
// Purpose: 
//   Comparison operator != for the ViewerRPC class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerRPC::operator != (const ViewerRPC &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: ViewerRPC::TypeName
//
// Purpose: 
//   Type name method for the ViewerRPC class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

const std::string
ViewerRPC::TypeName() const
{
    return "ViewerRPC";
}

// ****************************************************************************
// Method: ViewerRPC::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the ViewerRPC class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerRPC::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const ViewerRPC *tmp = (const ViewerRPC *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: ViewerRPC::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the ViewerRPC class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
ViewerRPC::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new ViewerRPC(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: ViewerRPC::NewInstance
//
// Purpose: 
//   NewInstance method for the ViewerRPC class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
ViewerRPC::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new ViewerRPC(*this);
    else
        retval = new ViewerRPC;

    return retval;
}

// ****************************************************************************
// Method: ViewerRPC::SelectAll
//
// Purpose: 
//   Selects all attributes.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void
ViewerRPC::SelectAll()
{
    Select(ID_RPCType,           (void *)&RPCType);
    Select(ID_windowLayout,      (void *)&windowLayout);
    Select(ID_windowId,          (void *)&windowId);
    Select(ID_windowMode,        (void *)&windowMode);
    Select(ID_windowArea,        (void *)&windowArea);
    Select(ID_database,          (void *)&database);
    Select(ID_programHost,       (void *)&programHost);
    Select(ID_programSim,        (void *)&programSim);
    Select(ID_programOptions,    (void *)&programOptions);
    Select(ID_nFrames,           (void *)&nFrames);
    Select(ID_stateNumber,       (void *)&stateNumber);
    Select(ID_frameRange,        (void *)frameRange, 2);
    Select(ID_frame,             (void *)&frame);
    Select(ID_plotType,          (void *)&plotType);
    Select(ID_operatorType,      (void *)&operatorType);
    Select(ID_variable,          (void *)&variable);
    Select(ID_activePlotIds,     (void *)&activePlotIds);
    Select(ID_activeOperatorIds, (void *)&activeOperatorIds);
    Select(ID_expandedPlotIds,   (void *)&expandedPlotIds);
    Select(ID_colorTableName,    (void *)&colorTableName);
    Select(ID_queryName,         (void *)&queryName);
    Select(ID_queryPoint1,       (void *)queryPoint1, 3);
    Select(ID_queryPoint2,       (void *)queryPoint2, 3);
    Select(ID_queryVariables,    (void *)&queryVariables);
    Select(ID_toolId,            (void *)&toolId);
    Select(ID_boolFlag,          (void *)&boolFlag);
    Select(ID_intArg1,           (void *)&intArg1);
    Select(ID_intArg2,           (void *)&intArg2);
    Select(ID_intArg3,           (void *)&intArg3);
    Select(ID_stringArg1,        (void *)&stringArg1);
    Select(ID_stringArg2,        (void *)&stringArg2);
    Select(ID_doubleArg1,        (void *)&doubleArg1);
    Select(ID_doubleArg2,        (void *)&doubleArg2);
    Select(ID_toolUpdateMode,    (void *)&toolUpdateMode);
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
ViewerRPC::SetRPCType(ViewerRPC::ViewerRPCType RPCType_)
{
    RPCType = RPCType_;
    Select(ID_RPCType, (void *)&RPCType);
}

void
ViewerRPC::SetWindowLayout(int windowLayout_)
{
    windowLayout = windowLayout_;
    Select(ID_windowLayout, (void *)&windowLayout);
}

void
ViewerRPC::SetWindowId(int windowId_)
{
    windowId = windowId_;
    Select(ID_windowId, (void *)&windowId);
}

void
ViewerRPC::SetWindowMode(int windowMode_)
{
    windowMode = windowMode_;
    Select(ID_windowMode, (void *)&windowMode);
}

void
ViewerRPC::SetWindowArea(const std::string &windowArea_)
{
    windowArea = windowArea_;
    Select(ID_windowArea, (void *)&windowArea);
}

void
ViewerRPC::SetDatabase(const std::string &database_)
{
    database = database_;
    Select(ID_database, (void *)&database);
}

void
ViewerRPC::SetProgramHost(const std::string &programHost_)
{
    programHost = programHost_;
    Select(ID_programHost, (void *)&programHost);
}

void
ViewerRPC::SetProgramSim(const std::string &programSim_)
{
    programSim = programSim_;
    Select(ID_programSim, (void *)&programSim);
}

void
ViewerRPC::SetProgramOptions(const stringVector &programOptions_)
{
    programOptions = programOptions_;
    Select(ID_programOptions, (void *)&programOptions);
}

void
ViewerRPC::SetNFrames(int nFrames_)
{
    nFrames = nFrames_;
    Select(ID_nFrames, (void *)&nFrames);
}

void
ViewerRPC::SetStateNumber(int stateNumber_)
{
    stateNumber = stateNumber_;
    Select(ID_stateNumber, (void *)&stateNumber);
}

void
ViewerRPC::SetFrameRange(const int *frameRange_)
{
    frameRange[0] = frameRange_[0];
    frameRange[1] = frameRange_[1];
    Select(ID_frameRange, (void *)frameRange, 2);
}

void
ViewerRPC::SetFrame(int frame_)
{
    frame = frame_;
    Select(ID_frame, (void *)&frame);
}

void
ViewerRPC::SetPlotType(int plotType_)
{
    plotType = plotType_;
    Select(ID_plotType, (void *)&plotType);
}

void
ViewerRPC::SetOperatorType(int operatorType_)
{
    operatorType = operatorType_;
    Select(ID_operatorType, (void *)&operatorType);
}

void
ViewerRPC::SetVariable(const std::string &variable_)
{
    variable = variable_;
    Select(ID_variable, (void *)&variable);
}

void
ViewerRPC::SetActivePlotIds(const intVector &activePlotIds_)
{
    activePlotIds = activePlotIds_;
    Select(ID_activePlotIds, (void *)&activePlotIds);
}

void
ViewerRPC::SetActiveOperatorIds(const intVector &activeOperatorIds_)
{
    activeOperatorIds = activeOperatorIds_;
    Select(ID_activeOperatorIds, (void *)&activeOperatorIds);
}

void
ViewerRPC::SetExpandedPlotIds(const intVector &expandedPlotIds_)
{
    expandedPlotIds = expandedPlotIds_;
    Select(ID_expandedPlotIds, (void *)&expandedPlotIds);
}

void
ViewerRPC::SetColorTableName(const std::string &colorTableName_)
{
    colorTableName = colorTableName_;
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
ViewerRPC::SetQueryName(const std::string &queryName_)
{
    queryName = queryName_;
    Select(ID_queryName, (void *)&queryName);
}

void
ViewerRPC::SetQueryPoint1(const double *queryPoint1_)
{
    queryPoint1[0] = queryPoint1_[0];
    queryPoint1[1] = queryPoint1_[1];
    queryPoint1[2] = queryPoint1_[2];
    Select(ID_queryPoint1, (void *)queryPoint1, 3);
}

void
ViewerRPC::SetQueryPoint2(const double *queryPoint2_)
{
    queryPoint2[0] = queryPoint2_[0];
    queryPoint2[1] = queryPoint2_[1];
    queryPoint2[2] = queryPoint2_[2];
    Select(ID_queryPoint2, (void *)queryPoint2, 3);
}

void
ViewerRPC::SetQueryVariables(const stringVector &queryVariables_)
{
    queryVariables = queryVariables_;
    Select(ID_queryVariables, (void *)&queryVariables);
}

void
ViewerRPC::SetToolId(int toolId_)
{
    toolId = toolId_;
    Select(ID_toolId, (void *)&toolId);
}

void
ViewerRPC::SetBoolFlag(bool boolFlag_)
{
    boolFlag = boolFlag_;
    Select(ID_boolFlag, (void *)&boolFlag);
}

void
ViewerRPC::SetIntArg1(int intArg1_)
{
    intArg1 = intArg1_;
    Select(ID_intArg1, (void *)&intArg1);
}

void
ViewerRPC::SetIntArg2(int intArg2_)
{
    intArg2 = intArg2_;
    Select(ID_intArg2, (void *)&intArg2);
}

void
ViewerRPC::SetIntArg3(int intArg3_)
{
    intArg3 = intArg3_;
    Select(ID_intArg3, (void *)&intArg3);
}

void
ViewerRPC::SetStringArg1(const std::string &stringArg1_)
{
    stringArg1 = stringArg1_;
    Select(ID_stringArg1, (void *)&stringArg1);
}

void
ViewerRPC::SetStringArg2(const std::string &stringArg2_)
{
    stringArg2 = stringArg2_;
    Select(ID_stringArg2, (void *)&stringArg2);
}

void
ViewerRPC::SetDoubleArg1(const doubleVector &doubleArg1_)
{
    doubleArg1 = doubleArg1_;
    Select(ID_doubleArg1, (void *)&doubleArg1);
}

void
ViewerRPC::SetDoubleArg2(const doubleVector &doubleArg2_)
{
    doubleArg2 = doubleArg2_;
    Select(ID_doubleArg2, (void *)&doubleArg2);
}

void
ViewerRPC::SetToolUpdateMode(int toolUpdateMode_)
{
    toolUpdateMode = toolUpdateMode_;
    Select(ID_toolUpdateMode, (void *)&toolUpdateMode);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

ViewerRPC::ViewerRPCType
ViewerRPC::GetRPCType() const
{
    return ViewerRPCType(RPCType);
}

int
ViewerRPC::GetWindowLayout() const
{
    return windowLayout;
}

int
ViewerRPC::GetWindowId() const
{
    return windowId;
}

int
ViewerRPC::GetWindowMode() const
{
    return windowMode;
}

const std::string &
ViewerRPC::GetWindowArea() const
{
    return windowArea;
}

std::string &
ViewerRPC::GetWindowArea()
{
    return windowArea;
}

const std::string &
ViewerRPC::GetDatabase() const
{
    return database;
}

std::string &
ViewerRPC::GetDatabase()
{
    return database;
}

const std::string &
ViewerRPC::GetProgramHost() const
{
    return programHost;
}

std::string &
ViewerRPC::GetProgramHost()
{
    return programHost;
}

const std::string &
ViewerRPC::GetProgramSim() const
{
    return programSim;
}

std::string &
ViewerRPC::GetProgramSim()
{
    return programSim;
}

const stringVector &
ViewerRPC::GetProgramOptions() const
{
    return programOptions;
}

stringVector &
ViewerRPC::GetProgramOptions()
{
    return programOptions;
}

int
ViewerRPC::GetNFrames() const
{
    return nFrames;
}

int
ViewerRPC::GetStateNumber() const
{
    return stateNumber;
}

const int *
ViewerRPC::GetFrameRange() const
{
    return frameRange;
}

int *
ViewerRPC::GetFrameRange()
{
    return frameRange;
}

int
ViewerRPC::GetFrame() const
{
    return frame;
}

int
ViewerRPC::GetPlotType() const
{
    return plotType;
}

int
ViewerRPC::GetOperatorType() const
{
    return operatorType;
}

const std::string &
ViewerRPC::GetVariable() const
{
    return variable;
}

std::string &
ViewerRPC::GetVariable()
{
    return variable;
}

const intVector &
ViewerRPC::GetActivePlotIds() const
{
    return activePlotIds;
}

intVector &
ViewerRPC::GetActivePlotIds()
{
    return activePlotIds;
}

const intVector &
ViewerRPC::GetActiveOperatorIds() const
{
    return activeOperatorIds;
}

intVector &
ViewerRPC::GetActiveOperatorIds()
{
    return activeOperatorIds;
}

const intVector &
ViewerRPC::GetExpandedPlotIds() const
{
    return expandedPlotIds;
}

intVector &
ViewerRPC::GetExpandedPlotIds()
{
    return expandedPlotIds;
}

const std::string &
ViewerRPC::GetColorTableName() const
{
    return colorTableName;
}

std::string &
ViewerRPC::GetColorTableName()
{
    return colorTableName;
}

const std::string &
ViewerRPC::GetQueryName() const
{
    return queryName;
}

std::string &
ViewerRPC::GetQueryName()
{
    return queryName;
}

const double *
ViewerRPC::GetQueryPoint1() const
{
    return queryPoint1;
}

double *
ViewerRPC::GetQueryPoint1()
{
    return queryPoint1;
}

const double *
ViewerRPC::GetQueryPoint2() const
{
    return queryPoint2;
}

double *
ViewerRPC::GetQueryPoint2()
{
    return queryPoint2;
}

const stringVector &
ViewerRPC::GetQueryVariables() const
{
    return queryVariables;
}

stringVector &
ViewerRPC::GetQueryVariables()
{
    return queryVariables;
}

int
ViewerRPC::GetToolId() const
{
    return toolId;
}

bool
ViewerRPC::GetBoolFlag() const
{
    return boolFlag;
}

int
ViewerRPC::GetIntArg1() const
{
    return intArg1;
}

int
ViewerRPC::GetIntArg2() const
{
    return intArg2;
}

int
ViewerRPC::GetIntArg3() const
{
    return intArg3;
}

const std::string &
ViewerRPC::GetStringArg1() const
{
    return stringArg1;
}

std::string &
ViewerRPC::GetStringArg1()
{
    return stringArg1;
}

const std::string &
ViewerRPC::GetStringArg2() const
{
    return stringArg2;
}

std::string &
ViewerRPC::GetStringArg2()
{
    return stringArg2;
}

const doubleVector &
ViewerRPC::GetDoubleArg1() const
{
    return doubleArg1;
}

doubleVector &
ViewerRPC::GetDoubleArg1()
{
    return doubleArg1;
}

const doubleVector &
ViewerRPC::GetDoubleArg2() const
{
    return doubleArg2;
}

doubleVector &
ViewerRPC::GetDoubleArg2()
{
    return doubleArg2;
}

int
ViewerRPC::GetToolUpdateMode() const
{
    return toolUpdateMode;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
ViewerRPC::SelectWindowArea()
{
    Select(ID_windowArea, (void *)&windowArea);
}

void
ViewerRPC::SelectDatabase()
{
    Select(ID_database, (void *)&database);
}

void
ViewerRPC::SelectProgramHost()
{
    Select(ID_programHost, (void *)&programHost);
}

void
ViewerRPC::SelectProgramSim()
{
    Select(ID_programSim, (void *)&programSim);
}

void
ViewerRPC::SelectProgramOptions()
{
    Select(ID_programOptions, (void *)&programOptions);
}

void
ViewerRPC::SelectFrameRange()
{
    Select(ID_frameRange, (void *)frameRange, 2);
}

void
ViewerRPC::SelectVariable()
{
    Select(ID_variable, (void *)&variable);
}

void
ViewerRPC::SelectActivePlotIds()
{
    Select(ID_activePlotIds, (void *)&activePlotIds);
}

void
ViewerRPC::SelectActiveOperatorIds()
{
    Select(ID_activeOperatorIds, (void *)&activeOperatorIds);
}

void
ViewerRPC::SelectExpandedPlotIds()
{
    Select(ID_expandedPlotIds, (void *)&expandedPlotIds);
}

void
ViewerRPC::SelectColorTableName()
{
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
ViewerRPC::SelectQueryName()
{
    Select(ID_queryName, (void *)&queryName);
}

void
ViewerRPC::SelectQueryPoint1()
{
    Select(ID_queryPoint1, (void *)queryPoint1, 3);
}

void
ViewerRPC::SelectQueryPoint2()
{
    Select(ID_queryPoint2, (void *)queryPoint2, 3);
}

void
ViewerRPC::SelectQueryVariables()
{
    Select(ID_queryVariables, (void *)&queryVariables);
}

void
ViewerRPC::SelectStringArg1()
{
    Select(ID_stringArg1, (void *)&stringArg1);
}

void
ViewerRPC::SelectStringArg2()
{
    Select(ID_stringArg2, (void *)&stringArg2);
}

void
ViewerRPC::SelectDoubleArg1()
{
    Select(ID_doubleArg1, (void *)&doubleArg1);
}

void
ViewerRPC::SelectDoubleArg2()
{
    Select(ID_doubleArg2, (void *)&doubleArg2);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ViewerRPC::GetFieldName
//
// Purpose: 
//   This method returns the name of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

std::string
ViewerRPC::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_RPCType:           return "RPCType";
    case ID_windowLayout:      return "windowLayout";
    case ID_windowId:          return "windowId";
    case ID_windowMode:        return "windowMode";
    case ID_windowArea:        return "windowArea";
    case ID_database:          return "database";
    case ID_programHost:       return "programHost";
    case ID_programSim:        return "programSim";
    case ID_programOptions:    return "programOptions";
    case ID_nFrames:           return "nFrames";
    case ID_stateNumber:       return "stateNumber";
    case ID_frameRange:        return "frameRange";
    case ID_frame:             return "frame";
    case ID_plotType:          return "plotType";
    case ID_operatorType:      return "operatorType";
    case ID_variable:          return "variable";
    case ID_activePlotIds:     return "activePlotIds";
    case ID_activeOperatorIds: return "activeOperatorIds";
    case ID_expandedPlotIds:   return "expandedPlotIds";
    case ID_colorTableName:    return "colorTableName";
    case ID_queryName:         return "queryName";
    case ID_queryPoint1:       return "queryPoint1";
    case ID_queryPoint2:       return "queryPoint2";
    case ID_queryVariables:    return "queryVariables";
    case ID_toolId:            return "toolId";
    case ID_boolFlag:          return "boolFlag";
    case ID_intArg1:           return "intArg1";
    case ID_intArg2:           return "intArg2";
    case ID_intArg3:           return "intArg3";
    case ID_stringArg1:        return "stringArg1";
    case ID_stringArg2:        return "stringArg2";
    case ID_doubleArg1:        return "doubleArg1";
    case ID_doubleArg2:        return "doubleArg2";
    case ID_toolUpdateMode:    return "toolUpdateMode";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: ViewerRPC::GetFieldType
//
// Purpose: 
//   This method returns the type of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

AttributeGroup::FieldType
ViewerRPC::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_RPCType:           return FieldType_enum;
    case ID_windowLayout:      return FieldType_int;
    case ID_windowId:          return FieldType_int;
    case ID_windowMode:        return FieldType_int;
    case ID_windowArea:        return FieldType_string;
    case ID_database:          return FieldType_string;
    case ID_programHost:       return FieldType_string;
    case ID_programSim:        return FieldType_string;
    case ID_programOptions:    return FieldType_stringVector;
    case ID_nFrames:           return FieldType_int;
    case ID_stateNumber:       return FieldType_int;
    case ID_frameRange:        return FieldType_intArray;
    case ID_frame:             return FieldType_int;
    case ID_plotType:          return FieldType_int;
    case ID_operatorType:      return FieldType_int;
    case ID_variable:          return FieldType_string;
    case ID_activePlotIds:     return FieldType_intVector;
    case ID_activeOperatorIds: return FieldType_intVector;
    case ID_expandedPlotIds:   return FieldType_intVector;
    case ID_colorTableName:    return FieldType_string;
    case ID_queryName:         return FieldType_string;
    case ID_queryPoint1:       return FieldType_doubleArray;
    case ID_queryPoint2:       return FieldType_doubleArray;
    case ID_queryVariables:    return FieldType_stringVector;
    case ID_toolId:            return FieldType_int;
    case ID_boolFlag:          return FieldType_bool;
    case ID_intArg1:           return FieldType_int;
    case ID_intArg2:           return FieldType_int;
    case ID_intArg3:           return FieldType_int;
    case ID_stringArg1:        return FieldType_string;
    case ID_stringArg2:        return FieldType_string;
    case ID_doubleArg1:        return FieldType_doubleVector;
    case ID_doubleArg2:        return FieldType_doubleVector;
    case ID_toolUpdateMode:    return FieldType_int;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: ViewerRPC::GetFieldTypeName
//
// Purpose: 
//   This method returns the name of a field type given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

std::string
ViewerRPC::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_RPCType:           return "enum";
    case ID_windowLayout:      return "int";
    case ID_windowId:          return "int";
    case ID_windowMode:        return "int";
    case ID_windowArea:        return "string";
    case ID_database:          return "string";
    case ID_programHost:       return "string";
    case ID_programSim:        return "string";
    case ID_programOptions:    return "stringVector";
    case ID_nFrames:           return "int";
    case ID_stateNumber:       return "int";
    case ID_frameRange:        return "intArray";
    case ID_frame:             return "int";
    case ID_plotType:          return "int";
    case ID_operatorType:      return "int";
    case ID_variable:          return "string";
    case ID_activePlotIds:     return "intVector";
    case ID_activeOperatorIds: return "intVector";
    case ID_expandedPlotIds:   return "intVector";
    case ID_colorTableName:    return "string";
    case ID_queryName:         return "string";
    case ID_queryPoint1:       return "doubleArray";
    case ID_queryPoint2:       return "doubleArray";
    case ID_queryVariables:    return "stringVector";
    case ID_toolId:            return "int";
    case ID_boolFlag:          return "bool";
    case ID_intArg1:           return "int";
    case ID_intArg2:           return "int";
    case ID_intArg3:           return "int";
    case ID_stringArg1:        return "string";
    case ID_stringArg2:        return "string";
    case ID_doubleArg1:        return "doubleVector";
    case ID_doubleArg2:        return "doubleVector";
    case ID_toolUpdateMode:    return "int";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: ViewerRPC::FieldsEqual
//
// Purpose: 
//   This method compares two fields and return true if they are equal.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerRPC::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const ViewerRPC &obj = *((const ViewerRPC*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_RPCType:
        {  // new scope
        retval = (RPCType == obj.RPCType);
        }
        break;
    case ID_windowLayout:
        {  // new scope
        retval = (windowLayout == obj.windowLayout);
        }
        break;
    case ID_windowId:
        {  // new scope
        retval = (windowId == obj.windowId);
        }
        break;
    case ID_windowMode:
        {  // new scope
        retval = (windowMode == obj.windowMode);
        }
        break;
    case ID_windowArea:
        {  // new scope
        retval = (windowArea == obj.windowArea);
        }
        break;
    case ID_database:
        {  // new scope
        retval = (database == obj.database);
        }
        break;
    case ID_programHost:
        {  // new scope
        retval = (programHost == obj.programHost);
        }
        break;
    case ID_programSim:
        {  // new scope
        retval = (programSim == obj.programSim);
        }
        break;
    case ID_programOptions:
        {  // new scope
        retval = (programOptions == obj.programOptions);
        }
        break;
    case ID_nFrames:
        {  // new scope
        retval = (nFrames == obj.nFrames);
        }
        break;
    case ID_stateNumber:
        {  // new scope
        retval = (stateNumber == obj.stateNumber);
        }
        break;
    case ID_frameRange:
        {  // new scope
        // Compare the frameRange arrays.
        bool frameRange_equal = true;
        for(int i = 0; i < 2 && frameRange_equal; ++i)
            frameRange_equal = (frameRange[i] == obj.frameRange[i]);

        retval = frameRange_equal;
        }
        break;
    case ID_frame:
        {  // new scope
        retval = (frame == obj.frame);
        }
        break;
    case ID_plotType:
        {  // new scope
        retval = (plotType == obj.plotType);
        }
        break;
    case ID_operatorType:
        {  // new scope
        retval = (operatorType == obj.operatorType);
        }
        break;
    case ID_variable:
        {  // new scope
        retval = (variable == obj.variable);
        }
        break;
    case ID_activePlotIds:
        {  // new scope
        retval = (activePlotIds == obj.activePlotIds);
        }
        break;
    case ID_activeOperatorIds:
        {  // new scope
        retval = (activeOperatorIds == obj.activeOperatorIds);
        }
        break;
    case ID_expandedPlotIds:
        {  // new scope
        retval = (expandedPlotIds == obj.expandedPlotIds);
        }
        break;
    case ID_colorTableName:
        {  // new scope
        retval = (colorTableName == obj.colorTableName);
        }
        break;
    case ID_queryName:
        {  // new scope
        retval = (queryName == obj.queryName);
        }
        break;
    case ID_queryPoint1:
        {  // new scope
        // Compare the queryPoint1 arrays.
        bool queryPoint1_equal = true;
        for(int i = 0; i < 3 && queryPoint1_equal; ++i)
            queryPoint1_equal = (queryPoint1[i] == obj.queryPoint1[i]);

        retval = queryPoint1_equal;
        }
        break;
    case ID_queryPoint2:
        {  // new scope
        // Compare the queryPoint2 arrays.
        bool queryPoint2_equal = true;
        for(int i = 0; i < 3 && queryPoint2_equal; ++i)
            queryPoint2_equal = (queryPoint2[i] == obj.queryPoint2[i]);

        retval = queryPoint2_equal;
        }
        break;
    case ID_queryVariables:
        {  // new scope
        retval = (queryVariables == obj.queryVariables);
        }
        break;
    case ID_toolId:
        {  // new scope
        retval = (toolId == obj.toolId);
        }
        break;
    case ID_boolFlag:
        {  // new scope
        retval = (boolFlag == obj.boolFlag);
        }
        break;
    case ID_intArg1:
        {  // new scope
        retval = (intArg1 == obj.intArg1);
        }
        break;
    case ID_intArg2:
        {  // new scope
        retval = (intArg2 == obj.intArg2);
        }
        break;
    case ID_intArg3:
        {  // new scope
        retval = (intArg3 == obj.intArg3);
        }
        break;
    case ID_stringArg1:
        {  // new scope
        retval = (stringArg1 == obj.stringArg1);
        }
        break;
    case ID_stringArg2:
        {  // new scope
        retval = (stringArg2 == obj.stringArg2);
        }
        break;
    case ID_doubleArg1:
        {  // new scope
        retval = (doubleArg1 == obj.doubleArg1);
        }
        break;
    case ID_doubleArg2:
        {  // new scope
        retval = (doubleArg2 == obj.doubleArg2);
        }
        break;
    case ID_toolUpdateMode:
        {  // new scope
        retval = (toolUpdateMode == obj.toolUpdateMode);
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
//  Method:  ViewerRPC::SetWindowArea
//
//  Purpose:
//    Sets the window area used by the viewer.
//
//  Programmer:  Brad Whitlock
//  Creation:    Fri Dec 20 14:18:12 PST 2002
//
// ****************************************************************************
void
ViewerRPC::SetWindowArea(int x, int y, int w, int h)
{
    char str[50];
    SNPRINTF(str, 50, "%dx%d+%d+%d", w, h, x, y);
    SetWindowArea(str);
}

