/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
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
"SetViewCurveRPC", "SetView2DRPC", "SetView3DRPC", 
"ResetPlotOptionsRPC", "ResetOperatorOptionsRPC", "SetAppearanceRPC", 
"ProcessExpressionsRPC", "SetLightListRPC", "SetDefaultLightListRPC", 
"ResetLightListRPC", "SetAnimationAttributesRPC", "SetWindowAreaRPC", 
"PrintWindowRPC", "ResetViewRPC", "RecenterViewRPC", 
"ToggleMaintainViewModeRPC", "ToggleMaintainDataModeRPC", "ToggleBoundingBoxModeRPC", 
"ToggleCameraViewModeRPC", "TogglePerspectiveViewRPC", "ToggleSpinModeRPC", 
"ToggleLockTimeRPC", "ToggleLockToolsRPC", "ToggleLockViewModeRPC", 
"ToggleFullFrameRPC", "UndoViewRPC", "RedoViewRPC", 
"InvertBackgroundRPC", "ClearPickPointsRPC", "SetWindowModeRPC", 
"EnableToolRPC", "CopyViewToWindowRPC", "CopyLightingToWindowRPC", 
"CopyAnnotationsToWindowRPC", "CopyPlotsToWindowRPC", "ClearCacheRPC", 
"ClearCacheForAllEnginesRPC", "SetViewExtentsTypeRPC", "ClearRefLinesRPC", 
"SetRenderingAttributesRPC", "DatabaseQueryRPC", "PointQueryRPC", 
"LineQueryRPC", "CloneWindowRPC", "SetMaterialAttributesRPC", 
"SetDefaultMaterialAttributesRPC", "ResetMaterialAttributesRPC", "SetPlotDatabaseStateRPC", 
"DeletePlotDatabaseKeyframeRPC", "MovePlotDatabaseKeyframeRPC", "ClearViewKeyframesRPC", 
"DeleteViewKeyframeRPC", "MoveViewKeyframeRPC", "SetViewKeyframeRPC", 
"OpenMDServerRPC", "EnableToolbarRPC", "HideToolbarsRPC", 
"HideToolbarsForAllWindowsRPC", "ShowToolbarsRPC", "ShowToolbarsForAllWindowsRPC", 
"SetToolbarIconSizeRPC", "SaveViewRPC", "SetGlobalLineoutAttributesRPC", 
"SetPickAttributesRPC", "ExportColorTableRPC", "ExportEntireStateRPC", 
"ImportEntireStateRPC", "ImportEntireStateWithDifferentSourcesRPC", "ResetPickAttributesRPC", 
"AddAnnotationObjectRPC", "HideActiveAnnotationObjectsRPC", "DeleteActiveAnnotationObjectsRPC", 
"RaiseActiveAnnotationObjectsRPC", "LowerActiveAnnotationObjectsRPC", "SetAnnotationObjectOptionsRPC", 
"SetDefaultAnnotationObjectListRPC", "ResetAnnotationObjectListRPC", "ResetPickLetterRPC", 
"SetDefaultPickAttributesRPC", "ChooseCenterOfRotationRPC", "SetCenterOfRotationRPC", 
"SetQueryOverTimeAttributesRPC", "SetDefaultQueryOverTimeAttributesRPC", "ResetQueryOverTimeAttributesRPC", 
"ResetLineoutColorRPC", "SetInteractorAttributesRPC", "SetDefaultInteractorAttributesRPC", 
"ResetInteractorAttributesRPC", "GetProcInfoRPC", "SendSimulationCommandRPC", 
"UpdateDBPluginInfoRPC", "ExportDBRPC", "SetTryHarderCyclesTimesRPC", 
"OpenClientRPC", "SuppressQueryOutputRPC", "SetMeshManagementAttributesRPC", 
"SetDefaultMeshManagementAttributesRPC", "ResetMeshManagementAttributesRPC", "ResizeWindowRPC", 
"MoveWindowRPC", "MoveAndResizeWindowRPC", "SetStateLoggingRPC", 
"ConstructDDFRPC", "UpdatePlotInfoAttsRPC", "RequestMetaDataRPC", 
"MaxRPC"};

std::string
ViewerRPC::ViewerRPCType_ToString(ViewerRPC::ViewerRPCType t)
{
    int index = int(t);
    if(index < 0 || index >= 169) index = 0;
    return ViewerRPCType_strings[index];
}

std::string
ViewerRPC::ViewerRPCType_ToString(int t)
{
    int index = (t < 0 || t >= 169) ? 0 : t;
    return ViewerRPCType_strings[index];
}

bool
ViewerRPC::ViewerRPCType_FromString(const std::string &s, ViewerRPC::ViewerRPCType &val)
{
    val = ViewerRPC::CloseRPC;
    for(int i = 0; i < 169; ++i)
    {
        if(s == ViewerRPCType_strings[i])
        {
            val = (ViewerRPCType)i;
            return true;
        }
    }
    return false;
}

// Type map format string
const char *ViewerRPC::TypeMapFormatString = "iiiisssss*iiIiiisi*i*i*ssDDs*ibiiissd*d*";

// ****************************************************************************
// Method: ViewerRPC::ViewerRPC
//
// Purpose: 
//   Constructor for the ViewerRPC class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue May 8 16:46:45 PST 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerRPC::ViewerRPC() : 
    AttributeSubject(ViewerRPC::TypeMapFormatString)
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
// Creation:   Tue May 8 16:46:45 PST 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerRPC::ViewerRPC(const ViewerRPC &obj) : 
    AttributeSubject(ViewerRPC::TypeMapFormatString)
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

    SelectAll();
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
// Creation:   Tue May 8 16:46:45 PST 2007
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
// Creation:   Tue May 8 16:46:45 PST 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerRPC& 
ViewerRPC::operator = (const ViewerRPC &obj)
{
    if (this == &obj) return *this;
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

    SelectAll();
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
// Creation:   Tue May 8 16:46:45 PST 2007
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerRPC::operator == (const ViewerRPC &obj) const
{
    int i;

    // Compare the frameRange arrays.
    bool frameRange_equal = true;
    for(i = 0; i < 2 && frameRange_equal; ++i)
        frameRange_equal = (frameRange[i] == obj.frameRange[i]);

    // Compare the queryPoint1 arrays.
    bool queryPoint1_equal = true;
    for(i = 0; i < 3 && queryPoint1_equal; ++i)
        queryPoint1_equal = (queryPoint1[i] == obj.queryPoint1[i]);

    // Compare the queryPoint2 arrays.
    bool queryPoint2_equal = true;
    for(i = 0; i < 3 && queryPoint2_equal; ++i)
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
            (doubleArg2 == obj.doubleArg2));
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
// Creation:   Tue May 8 16:46:45 PST 2007
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
// Creation:   Tue May 8 16:46:45 PST 2007
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
// Creation:   Tue May 8 16:46:45 PST 2007
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
// Creation:   Tue May 8 16:46:45 PST 2007
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
// Creation:   Tue May 8 16:46:45 PST 2007
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
// Creation:   Tue May 8 16:46:45 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
ViewerRPC::SelectAll()
{
    Select(0, (void *)&RPCType);
    Select(1, (void *)&windowLayout);
    Select(2, (void *)&windowId);
    Select(3, (void *)&windowMode);
    Select(4, (void *)&windowArea);
    Select(5, (void *)&database);
    Select(6, (void *)&programHost);
    Select(7, (void *)&programSim);
    Select(8, (void *)&programOptions);
    Select(9, (void *)&nFrames);
    Select(10, (void *)&stateNumber);
    Select(11, (void *)frameRange, 2);
    Select(12, (void *)&frame);
    Select(13, (void *)&plotType);
    Select(14, (void *)&operatorType);
    Select(15, (void *)&variable);
    Select(16, (void *)&activePlotIds);
    Select(17, (void *)&activeOperatorIds);
    Select(18, (void *)&expandedPlotIds);
    Select(19, (void *)&colorTableName);
    Select(20, (void *)&queryName);
    Select(21, (void *)queryPoint1, 3);
    Select(22, (void *)queryPoint2, 3);
    Select(23, (void *)&queryVariables);
    Select(24, (void *)&toolId);
    Select(25, (void *)&boolFlag);
    Select(26, (void *)&intArg1);
    Select(27, (void *)&intArg2);
    Select(28, (void *)&intArg3);
    Select(29, (void *)&stringArg1);
    Select(30, (void *)&stringArg2);
    Select(31, (void *)&doubleArg1);
    Select(32, (void *)&doubleArg2);
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
ViewerRPC::SetRPCType(ViewerRPC::ViewerRPCType RPCType_)
{
    RPCType = RPCType_;
    Select(0, (void *)&RPCType);
}

void
ViewerRPC::SetWindowLayout(int windowLayout_)
{
    windowLayout = windowLayout_;
    Select(1, (void *)&windowLayout);
}

void
ViewerRPC::SetWindowId(int windowId_)
{
    windowId = windowId_;
    Select(2, (void *)&windowId);
}

void
ViewerRPC::SetWindowMode(int windowMode_)
{
    windowMode = windowMode_;
    Select(3, (void *)&windowMode);
}

void
ViewerRPC::SetWindowArea(const std::string &windowArea_)
{
    windowArea = windowArea_;
    Select(4, (void *)&windowArea);
}

void
ViewerRPC::SetDatabase(const std::string &database_)
{
    database = database_;
    Select(5, (void *)&database);
}

void
ViewerRPC::SetProgramHost(const std::string &programHost_)
{
    programHost = programHost_;
    Select(6, (void *)&programHost);
}

void
ViewerRPC::SetProgramSim(const std::string &programSim_)
{
    programSim = programSim_;
    Select(7, (void *)&programSim);
}

void
ViewerRPC::SetProgramOptions(const stringVector &programOptions_)
{
    programOptions = programOptions_;
    Select(8, (void *)&programOptions);
}

void
ViewerRPC::SetNFrames(int nFrames_)
{
    nFrames = nFrames_;
    Select(9, (void *)&nFrames);
}

void
ViewerRPC::SetStateNumber(int stateNumber_)
{
    stateNumber = stateNumber_;
    Select(10, (void *)&stateNumber);
}

void
ViewerRPC::SetFrameRange(const int *frameRange_)
{
    frameRange[0] = frameRange_[0];
    frameRange[1] = frameRange_[1];
    Select(11, (void *)frameRange, 2);
}

void
ViewerRPC::SetFrame(int frame_)
{
    frame = frame_;
    Select(12, (void *)&frame);
}

void
ViewerRPC::SetPlotType(int plotType_)
{
    plotType = plotType_;
    Select(13, (void *)&plotType);
}

void
ViewerRPC::SetOperatorType(int operatorType_)
{
    operatorType = operatorType_;
    Select(14, (void *)&operatorType);
}

void
ViewerRPC::SetVariable(const std::string &variable_)
{
    variable = variable_;
    Select(15, (void *)&variable);
}

void
ViewerRPC::SetActivePlotIds(const intVector &activePlotIds_)
{
    activePlotIds = activePlotIds_;
    Select(16, (void *)&activePlotIds);
}

void
ViewerRPC::SetActiveOperatorIds(const intVector &activeOperatorIds_)
{
    activeOperatorIds = activeOperatorIds_;
    Select(17, (void *)&activeOperatorIds);
}

void
ViewerRPC::SetExpandedPlotIds(const intVector &expandedPlotIds_)
{
    expandedPlotIds = expandedPlotIds_;
    Select(18, (void *)&expandedPlotIds);
}

void
ViewerRPC::SetColorTableName(const std::string &colorTableName_)
{
    colorTableName = colorTableName_;
    Select(19, (void *)&colorTableName);
}

void
ViewerRPC::SetQueryName(const std::string &queryName_)
{
    queryName = queryName_;
    Select(20, (void *)&queryName);
}

void
ViewerRPC::SetQueryPoint1(const double *queryPoint1_)
{
    queryPoint1[0] = queryPoint1_[0];
    queryPoint1[1] = queryPoint1_[1];
    queryPoint1[2] = queryPoint1_[2];
    Select(21, (void *)queryPoint1, 3);
}

void
ViewerRPC::SetQueryPoint2(const double *queryPoint2_)
{
    queryPoint2[0] = queryPoint2_[0];
    queryPoint2[1] = queryPoint2_[1];
    queryPoint2[2] = queryPoint2_[2];
    Select(22, (void *)queryPoint2, 3);
}

void
ViewerRPC::SetQueryVariables(const stringVector &queryVariables_)
{
    queryVariables = queryVariables_;
    Select(23, (void *)&queryVariables);
}

void
ViewerRPC::SetToolId(int toolId_)
{
    toolId = toolId_;
    Select(24, (void *)&toolId);
}

void
ViewerRPC::SetBoolFlag(bool boolFlag_)
{
    boolFlag = boolFlag_;
    Select(25, (void *)&boolFlag);
}

void
ViewerRPC::SetIntArg1(int intArg1_)
{
    intArg1 = intArg1_;
    Select(26, (void *)&intArg1);
}

void
ViewerRPC::SetIntArg2(int intArg2_)
{
    intArg2 = intArg2_;
    Select(27, (void *)&intArg2);
}

void
ViewerRPC::SetIntArg3(int intArg3_)
{
    intArg3 = intArg3_;
    Select(28, (void *)&intArg3);
}

void
ViewerRPC::SetStringArg1(const std::string &stringArg1_)
{
    stringArg1 = stringArg1_;
    Select(29, (void *)&stringArg1);
}

void
ViewerRPC::SetStringArg2(const std::string &stringArg2_)
{
    stringArg2 = stringArg2_;
    Select(30, (void *)&stringArg2);
}

void
ViewerRPC::SetDoubleArg1(const doubleVector &doubleArg1_)
{
    doubleArg1 = doubleArg1_;
    Select(31, (void *)&doubleArg1);
}

void
ViewerRPC::SetDoubleArg2(const doubleVector &doubleArg2_)
{
    doubleArg2 = doubleArg2_;
    Select(32, (void *)&doubleArg2);
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

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
ViewerRPC::SelectWindowArea()
{
    Select(4, (void *)&windowArea);
}

void
ViewerRPC::SelectDatabase()
{
    Select(5, (void *)&database);
}

void
ViewerRPC::SelectProgramHost()
{
    Select(6, (void *)&programHost);
}

void
ViewerRPC::SelectProgramSim()
{
    Select(7, (void *)&programSim);
}

void
ViewerRPC::SelectProgramOptions()
{
    Select(8, (void *)&programOptions);
}

void
ViewerRPC::SelectFrameRange()
{
    Select(11, (void *)frameRange, 2);
}

void
ViewerRPC::SelectVariable()
{
    Select(15, (void *)&variable);
}

void
ViewerRPC::SelectActivePlotIds()
{
    Select(16, (void *)&activePlotIds);
}

void
ViewerRPC::SelectActiveOperatorIds()
{
    Select(17, (void *)&activeOperatorIds);
}

void
ViewerRPC::SelectExpandedPlotIds()
{
    Select(18, (void *)&expandedPlotIds);
}

void
ViewerRPC::SelectColorTableName()
{
    Select(19, (void *)&colorTableName);
}

void
ViewerRPC::SelectQueryName()
{
    Select(20, (void *)&queryName);
}

void
ViewerRPC::SelectQueryPoint1()
{
    Select(21, (void *)queryPoint1, 3);
}

void
ViewerRPC::SelectQueryPoint2()
{
    Select(22, (void *)queryPoint2, 3);
}

void
ViewerRPC::SelectQueryVariables()
{
    Select(23, (void *)&queryVariables);
}

void
ViewerRPC::SelectStringArg1()
{
    Select(29, (void *)&stringArg1);
}

void
ViewerRPC::SelectStringArg2()
{
    Select(30, (void *)&stringArg2);
}

void
ViewerRPC::SelectDoubleArg1()
{
    Select(31, (void *)&doubleArg1);
}

void
ViewerRPC::SelectDoubleArg2()
{
    Select(32, (void *)&doubleArg2);
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
// Creation:   Tue May 8 16:46:45 PST 2007
//
// Modifications:
//   
// ****************************************************************************

std::string
ViewerRPC::GetFieldName(int index) const
{
    switch (index)
    {
        case 0:  return "RPCType";
        case 1:  return "windowLayout";
        case 2:  return "windowId";
        case 3:  return "windowMode";
        case 4:  return "windowArea";
        case 5:  return "database";
        case 6:  return "programHost";
        case 7:  return "programSim";
        case 8:  return "programOptions";
        case 9:  return "nFrames";
        case 10:  return "stateNumber";
        case 11:  return "frameRange";
        case 12:  return "frame";
        case 13:  return "plotType";
        case 14:  return "operatorType";
        case 15:  return "variable";
        case 16:  return "activePlotIds";
        case 17:  return "activeOperatorIds";
        case 18:  return "expandedPlotIds";
        case 19:  return "colorTableName";
        case 20:  return "queryName";
        case 21:  return "queryPoint1";
        case 22:  return "queryPoint2";
        case 23:  return "queryVariables";
        case 24:  return "toolId";
        case 25:  return "boolFlag";
        case 26:  return "intArg1";
        case 27:  return "intArg2";
        case 28:  return "intArg3";
        case 29:  return "stringArg1";
        case 30:  return "stringArg2";
        case 31:  return "doubleArg1";
        case 32:  return "doubleArg2";
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
// Creation:   Tue May 8 16:46:45 PST 2007
//
// Modifications:
//   
// ****************************************************************************

AttributeGroup::FieldType
ViewerRPC::GetFieldType(int index) const
{
    switch (index)
    {
        case 0:  return FieldType_enum;
        case 1:  return FieldType_int;
        case 2:  return FieldType_int;
        case 3:  return FieldType_int;
        case 4:  return FieldType_string;
        case 5:  return FieldType_string;
        case 6:  return FieldType_string;
        case 7:  return FieldType_string;
        case 8:  return FieldType_stringVector;
        case 9:  return FieldType_int;
        case 10:  return FieldType_int;
        case 11:  return FieldType_intArray;
        case 12:  return FieldType_int;
        case 13:  return FieldType_int;
        case 14:  return FieldType_int;
        case 15:  return FieldType_string;
        case 16:  return FieldType_intVector;
        case 17:  return FieldType_intVector;
        case 18:  return FieldType_intVector;
        case 19:  return FieldType_string;
        case 20:  return FieldType_string;
        case 21:  return FieldType_doubleArray;
        case 22:  return FieldType_doubleArray;
        case 23:  return FieldType_stringVector;
        case 24:  return FieldType_int;
        case 25:  return FieldType_bool;
        case 26:  return FieldType_int;
        case 27:  return FieldType_int;
        case 28:  return FieldType_int;
        case 29:  return FieldType_string;
        case 30:  return FieldType_string;
        case 31:  return FieldType_doubleVector;
        case 32:  return FieldType_doubleVector;
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
// Creation:   Tue May 8 16:46:45 PST 2007
//
// Modifications:
//   
// ****************************************************************************

std::string
ViewerRPC::GetFieldTypeName(int index) const
{
    switch (index)
    {
        case 0:  return "enum";
        case 1:  return "int";
        case 2:  return "int";
        case 3:  return "int";
        case 4:  return "string";
        case 5:  return "string";
        case 6:  return "string";
        case 7:  return "string";
        case 8:  return "stringVector";
        case 9:  return "int";
        case 10:  return "int";
        case 11:  return "intArray";
        case 12:  return "int";
        case 13:  return "int";
        case 14:  return "int";
        case 15:  return "string";
        case 16:  return "intVector";
        case 17:  return "intVector";
        case 18:  return "intVector";
        case 19:  return "string";
        case 20:  return "string";
        case 21:  return "doubleArray";
        case 22:  return "doubleArray";
        case 23:  return "stringVector";
        case 24:  return "int";
        case 25:  return "bool";
        case 26:  return "int";
        case 27:  return "int";
        case 28:  return "int";
        case 29:  return "string";
        case 30:  return "string";
        case 31:  return "doubleVector";
        case 32:  return "doubleVector";
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
// Creation:   Tue May 8 16:46:45 PST 2007
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerRPC::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    int i;

    const ViewerRPC &obj = *((const ViewerRPC*)rhs);
    bool retval = false;
    switch (index_)
    {
    case 0:
        {  // new scope
        retval = (RPCType == obj.RPCType);
        }
        break;
    case 1:
        {  // new scope
        retval = (windowLayout == obj.windowLayout);
        }
        break;
    case 2:
        {  // new scope
        retval = (windowId == obj.windowId);
        }
        break;
    case 3:
        {  // new scope
        retval = (windowMode == obj.windowMode);
        }
        break;
    case 4:
        {  // new scope
        retval = (windowArea == obj.windowArea);
        }
        break;
    case 5:
        {  // new scope
        retval = (database == obj.database);
        }
        break;
    case 6:
        {  // new scope
        retval = (programHost == obj.programHost);
        }
        break;
    case 7:
        {  // new scope
        retval = (programSim == obj.programSim);
        }
        break;
    case 8:
        {  // new scope
        retval = (programOptions == obj.programOptions);
        }
        break;
    case 9:
        {  // new scope
        retval = (nFrames == obj.nFrames);
        }
        break;
    case 10:
        {  // new scope
        retval = (stateNumber == obj.stateNumber);
        }
        break;
    case 11:
        {  // new scope
        // Compare the frameRange arrays.
        bool frameRange_equal = true;
        for(i = 0; i < 2 && frameRange_equal; ++i)
            frameRange_equal = (frameRange[i] == obj.frameRange[i]);

        retval = frameRange_equal;
        }
        break;
    case 12:
        {  // new scope
        retval = (frame == obj.frame);
        }
        break;
    case 13:
        {  // new scope
        retval = (plotType == obj.plotType);
        }
        break;
    case 14:
        {  // new scope
        retval = (operatorType == obj.operatorType);
        }
        break;
    case 15:
        {  // new scope
        retval = (variable == obj.variable);
        }
        break;
    case 16:
        {  // new scope
        retval = (activePlotIds == obj.activePlotIds);
        }
        break;
    case 17:
        {  // new scope
        retval = (activeOperatorIds == obj.activeOperatorIds);
        }
        break;
    case 18:
        {  // new scope
        retval = (expandedPlotIds == obj.expandedPlotIds);
        }
        break;
    case 19:
        {  // new scope
        retval = (colorTableName == obj.colorTableName);
        }
        break;
    case 20:
        {  // new scope
        retval = (queryName == obj.queryName);
        }
        break;
    case 21:
        {  // new scope
        // Compare the queryPoint1 arrays.
        bool queryPoint1_equal = true;
        for(i = 0; i < 3 && queryPoint1_equal; ++i)
            queryPoint1_equal = (queryPoint1[i] == obj.queryPoint1[i]);

        retval = queryPoint1_equal;
        }
        break;
    case 22:
        {  // new scope
        // Compare the queryPoint2 arrays.
        bool queryPoint2_equal = true;
        for(i = 0; i < 3 && queryPoint2_equal; ++i)
            queryPoint2_equal = (queryPoint2[i] == obj.queryPoint2[i]);

        retval = queryPoint2_equal;
        }
        break;
    case 23:
        {  // new scope
        retval = (queryVariables == obj.queryVariables);
        }
        break;
    case 24:
        {  // new scope
        retval = (toolId == obj.toolId);
        }
        break;
    case 25:
        {  // new scope
        retval = (boolFlag == obj.boolFlag);
        }
        break;
    case 26:
        {  // new scope
        retval = (intArg1 == obj.intArg1);
        }
        break;
    case 27:
        {  // new scope
        retval = (intArg2 == obj.intArg2);
        }
        break;
    case 28:
        {  // new scope
        retval = (intArg3 == obj.intArg3);
        }
        break;
    case 29:
        {  // new scope
        retval = (stringArg1 == obj.stringArg1);
        }
        break;
    case 30:
        {  // new scope
        retval = (stringArg2 == obj.stringArg2);
        }
        break;
    case 31:
        {  // new scope
        retval = (doubleArg1 == obj.doubleArg1);
        }
        break;
    case 32:
        {  // new scope
        retval = (doubleArg2 == obj.doubleArg2);
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

