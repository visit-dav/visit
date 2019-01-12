/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package visitclient;

import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonPrimitive;
import java.io.OutputStreamWriter;
import java.util.ArrayList;

/**
 *
 * @author hari
 */
public class ViewerState
{
    public ArrayList<AttributeSubject> states;

    ViewerState()
    {
        states = new ArrayList<AttributeSubject>();
    }

    synchronized boolean update(JsonObject jo)
    {
        //if it does not have id, then class is not updated properly.
        if(!jo.has("id")) return false;

        JsonElement e = jo.get("id");
        int id = e.getAsInt();

        if(id < states.size())
            states.get(id).update(jo);
        else
        {
            int diff = id-states.size();

            states.add(new AttributeSubject());
            for(int i = 0; i < diff; ++i)
                states.add(new AttributeSubject());

            states.get(id).update(jo);
        }

        return true;
    }
    
    synchronized AttributeSubject get(int index)
    {
        if(index < states.size())
            return states.get(index);
        return null;
    }
    
    synchronized JsonElement get(int index, String key)
    {
        if(index >= 0 && index < states.size())
            return states.get(index).get(key);
        return null;
    }
    
    synchronized void set(int index, String key, Boolean value)
    {
        set(index, key, new JsonPrimitive(value));
    }
    
    synchronized void set(int index, String key, Number value)
    {
        set(index, key, new JsonPrimitive(value));
    }
    
    
    synchronized void set(int index, String key, String value)
    {
        set(index, key, new JsonPrimitive(value));
    }
    
    synchronized void set(int index, String key, JsonElement value)
    {
        if(index >= 0 && index < states.size())
            states.get(index).set(key, value);
    }

    synchronized void notify(int index)
    {
        if(index >= 0 && index < states.size())
            states.get(index).notify(output);
    }
        
    synchronized void setConnection(OutputStreamWriter o)
    {
        output = o;
    }
    
    public enum RPCType
    {
        CloseRPC,
        DetachRPC,
        AddWindowRPC,
        DeleteWindowRPC,
        SetWindowLayoutRPC,
        SetActiveWindowRPC,
        ClearWindowRPC,
        ClearAllWindowsRPC,
        OpenDatabaseRPC,
        CloseDatabaseRPC,
        ActivateDatabaseRPC,
        CheckForNewStatesRPC,
        CreateDatabaseCorrelationRPC,
        AlterDatabaseCorrelationRPC,
        DeleteDatabaseCorrelationRPC,
        ReOpenDatabaseRPC,
        ReplaceDatabaseRPC,
        OverlayDatabaseRPC,
        OpenComputeEngineRPC,
        CloseComputeEngineRPC,
        AnimationSetNFramesRPC,
        AnimationPlayRPC,
        AnimationReversePlayRPC,
        AnimationStopRPC,
        TimeSliderNextStateRPC,
        TimeSliderPreviousStateRPC,
        SetTimeSliderStateRPC,
        SetActiveTimeSliderRPC,
        AddPlotRPC,
        SetPlotFrameRangeRPC,
        DeletePlotKeyframeRPC,
        MovePlotKeyframeRPC,
        DeleteActivePlotsRPC,
        HideActivePlotsRPC,
        DrawPlotsRPC,
        DisableRedrawRPC,
        RedrawRPC,
        SetActivePlotsRPC,
        ChangeActivePlotsVarRPC,
        AddOperatorRPC,
        AddInitializedOperatorRPC,
        PromoteOperatorRPC,
        DemoteOperatorRPC,
        RemoveOperatorRPC,
        RemoveLastOperatorRPC,
        RemoveAllOperatorsRPC,
        SaveWindowRPC,
        SetDefaultPlotOptionsRPC,
        SetPlotOptionsRPC,
        SetDefaultOperatorOptionsRPC,
        SetOperatorOptionsRPC,
        WriteConfigFileRPC,
        ConnectToMetaDataServerRPC,
        IconifyAllWindowsRPC,
        DeIconifyAllWindowsRPC,
        ShowAllWindowsRPC,
        HideAllWindowsRPC,
        UpdateColorTableRPC,
        SetAnnotationAttributesRPC,
        SetDefaultAnnotationAttributesRPC,
        ResetAnnotationAttributesRPC,
        SetKeyframeAttributesRPC,
        SetPlotSILRestrictionRPC,
        SetViewAxisArrayRPC,
        SetViewCurveRPC,
        SetView2DRPC,
        SetView3DRPC,
        ResetPlotOptionsRPC,
        ResetOperatorOptionsRPC,
        SetAppearanceRPC,
        ProcessExpressionsRPC,
        SetLightListRPC,
        SetDefaultLightListRPC,
        ResetLightListRPC,
        SetAnimationAttributesRPC,
        SetWindowAreaRPC,
        PrintWindowRPC,
        ResetViewRPC,
        RecenterViewRPC,
        ToggleAllowPopupRPC,
        ToggleMaintainViewModeRPC,
        ToggleBoundingBoxModeRPC,
        ToggleCameraViewModeRPC,
        TogglePerspectiveViewRPC,
        ToggleSpinModeRPC,
        ToggleLockTimeRPC,
        ToggleLockToolsRPC,
        ToggleLockViewModeRPC,
        ToggleFullFrameRPC,
        UndoViewRPC,
        RedoViewRPC,
        InvertBackgroundRPC,
        ClearPickPointsRPC,
        SetWindowModeRPC,
        EnableToolRPC,
        SetToolUpdateModeRPC,
        CopyViewToWindowRPC,
        CopyLightingToWindowRPC,
        CopyAnnotationsToWindowRPC,
        CopyPlotsToWindowRPC,
        ClearCacheRPC,
        ClearCacheForAllEnginesRPC,
        SetViewExtentsTypeRPC,
        ClearRefLinesRPC,
        SetRenderingAttributesRPC,
        QueryRPC,
        CloneWindowRPC,
        SetMaterialAttributesRPC,
        SetDefaultMaterialAttributesRPC,
        ResetMaterialAttributesRPC,
        SetPlotDatabaseStateRPC,
        DeletePlotDatabaseKeyframeRPC,
        MovePlotDatabaseKeyframeRPC,
        ClearViewKeyframesRPC,
        DeleteViewKeyframeRPC,
        MoveViewKeyframeRPC,
        SetViewKeyframeRPC,
        OpenMDServerRPC,
        EnableToolbarRPC,
        HideToolbarsRPC,
        HideToolbarsForAllWindowsRPC,
        ShowToolbarsRPC,
        ShowToolbarsForAllWindowsRPC,
        SetToolbarIconSizeRPC,
        SaveViewRPC,
        SetGlobalLineoutAttributesRPC,
        SetPickAttributesRPC,
        ExportColorTableRPC,
        ExportEntireStateRPC,
        ImportEntireStateRPC,
        ImportEntireStateWithDifferentSourcesRPC,
        ResetPickAttributesRPC,
        AddAnnotationObjectRPC,
        HideActiveAnnotationObjectsRPC,
        DeleteActiveAnnotationObjectsRPC,
        RaiseActiveAnnotationObjectsRPC,
        LowerActiveAnnotationObjectsRPC,
        SetAnnotationObjectOptionsRPC,
        SetDefaultAnnotationObjectListRPC,
        ResetAnnotationObjectListRPC,
        ResetPickLetterRPC,
        SetDefaultPickAttributesRPC,
        ChooseCenterOfRotationRPC,
        SetCenterOfRotationRPC,
        SetQueryOverTimeAttributesRPC,
        SetDefaultQueryOverTimeAttributesRPC,
        ResetQueryOverTimeAttributesRPC,
        ResetLineoutColorRPC,
        SetInteractorAttributesRPC,
        SetDefaultInteractorAttributesRPC,
        ResetInteractorAttributesRPC,
        GetProcInfoRPC,
        SendSimulationCommandRPC,
        UpdateDBPluginInfoRPC,
        ExportDBRPC,
        SetTryHarderCyclesTimesRPC,
        OpenClientRPC,
        OpenGUIClientRPC,
        OpenCLIClientRPC,
        SuppressQueryOutputRPC,
        SetQueryFloatFormatRPC,
        SetMeshManagementAttributesRPC,
        SetDefaultMeshManagementAttributesRPC,
        ResetMeshManagementAttributesRPC,
        ResizeWindowRPC,
        MoveWindowRPC,
        MoveAndResizeWindowRPC,
        SetStateLoggingRPC,
        ConstructDataBinningRPC,
        RequestMetaDataRPC,
        SetTreatAllDBsAsTimeVaryingRPC,
        SetCreateMeshQualityExpressionsRPC,
        SetCreateTimeDerivativeExpressionsRPC,
        SetCreateVectorMagnitudeExpressionsRPC,
        CopyActivePlotsRPC,
        SetPlotFollowsTimeRPC,
        TurnOffAllLocksRPC,
        SetDefaultFileOpenOptionsRPC,
        SetSuppressMessagesRPC,
        ApplyNamedSelectionRPC,
        CreateNamedSelectionRPC,
        DeleteNamedSelectionRPC,
        LoadNamedSelectionRPC,
        SaveNamedSelectionRPC,
        SetNamedSelectionAutoApplyRPC,
        UpdateNamedSelectionRPC,
        InitializeNamedSelectionVariablesRPC,
        MenuQuitRPC,
        SetPlotDescriptionRPC,
        MovePlotOrderTowardFirstRPC,
        MovePlotOrderTowardLastRPC,
        SetPlotOrderToFirstRPC,
        SetPlotOrderToLastRPC,
        RenamePickLabelRPC,
        GetQueryParametersRPC,
        DDTConnectRPC,
        DDTFocusRPC,
        ReleaseToDDTRPC,
        ExportRPC,
        MaxRPC
    }
    
    private OutputStreamWriter output;
}
