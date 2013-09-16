
function AttributeSubject()
{
    var callbackList = [];
    var api = null;
    var data = null;
    var id = -1;
    var typename = "";

    this.update = function(rawData)
    {
        if(rawData.hasOwnProperty("api")) {
            api = rawData;
            id = api["id"];
            typename = api["typename"];
        }
        else
        {
            data = rawData;
            for(var i = 0; i < callbackList.length; ++i) {
                callbackList[i](this);
            }
        }
    }

    this.set = function(key, value) 
    {
        if(api["api"].hasOwnProperty(key)) {
           var index = api["api"][key];         
           data["contents"][index] = value;
        }
    }

    this.get = function(key) {
        var index = api["api"][key];
        return data["contents"][index];
    }

    this.notify = function(conn) {
        conn.send(JSON.stringify(data));
    }

    this.getId = function() {
        return id;
    }

    this.getTypename = function() {
        return typename;
    }

    this.toString = function() {
        return JSON.stringify(api) + "\n" + JSON.stringify(data);
    }
    this.registerCallback = function(cb) {
        callbackList.push(cb);
    }
}

function ViewerState() 
{
    var states = [];
    var typenameToState = {};
    var output = null;

    this.update = function(rawData) 
    {
        
        //if it does not have id, then class is not updated properly.
        if(!rawData.hasOwnProperty("id")) return false;

        var id = rawData["id"];
        
        if(id < states.length)
            states[id].update(rawData);
        else
        {
            var diff = id - states.length;

            states.push(new AttributeSubject());
            for(var i = 0; i < diff; ++i)
                states.push(new AttributeSubject());

            states[id].update(rawData);
        }

        /// create map
        typenameToState[states[id].getTypename()] = states[id].getId();
        
        return true;
    }

    this.getIndexFromTypename = function(typename) {
        return typenameToState[typename];
    }

    this.registerCallback = function(key, callback) 
    {
        var index = vp.GetViewerState().getIndexFromTypename(key);
        var subject = vp.GetViewerState().getAttributeSubject(index);
        subject.registerCallback(callback);
    }

    this.get = function(index, key) {
        if(index >= 0 && index < states.length)
            return states[index].get(key);
        return null;
    }

    this.set = function(index, key, value) {
        if(index >= 0 && index < states.length)
            return states[index].set(key,value);
        return null;
    }

    this.notify = function(index)
    {
        if(index >= 0 && index < states.length)
            states[index].notify(output);
    }
        
    this.setConnection = function(conn)
    {
        output = conn;
    }

    this.getAttributeSubject = function(index) {
        if(index >= 0 && index < states.length)
            return states[index];
        
        return null;
    }

    this.getStates = function() {
        return states;
    }
}

function ViewerMethods(state) 
{
    var viewerState = state;
    var windowId = 1;
    
    this.InvertBackgroundColor = function ()
    {
        this.SetActiveWindow(windowId);
        viewerState.set(0,"RPCType",RPCType.InvertBackgroundRPC);
        viewerState.notify(0);
    }

    this.SetActiveWindow = function(id)
    {
        viewerState.set(0,"RPCType",RPCType.SetActiveWindowRPC);
        viewerState.set(0,"windowId", id);
        viewerState.set(0,"boolFlag", false);
        viewerState.notify(0);
    }

    this.OpenDatabase = function(str)
    {
        viewerState.set(0,"RPCType",RPCType.OpenDatabaseRPC);
        viewerState.set(0,"intArg1",0);
        viewerState.set(0,"boolFlag",true);
        viewerState.set(0,"stringArg1","");
        viewerState.set(0,"database",str);
        viewerState.notify(0);
    }

    this.DrawPlots = function ()
    {
        viewerState.set(0,"RPCType",RPCType.DrawPlotsRPC);
        viewerState.notify(0);
    }

    this.AddPseudocolorPlot = function (variable)
    {
        this.SetActiveWindow(windowId);
        viewerState.set(0,"RPCType",RPCType.AddPlotRPC);
        viewerState.set(0,"plotType",11);
        viewerState.set(0,"variable",variable);
        viewerState.notify(0);
    }

    this.AddContourPlot = function (variable)
    {
        this.SetActiveWindow(windowId);
        viewerState.set(0,"RPCType",RPCType.AddPlotRPC);
        viewerState.set(0,"plotType",1);
        viewerState.set(0,"variable",variable);
        viewerState.notify(0);
    }

    this.DeleteActivePlots = function ()
    {
        this.SetActiveWindow(windowId);
        viewerState.set(0,"RPCType",RPCType.DeleteActivePlotsRPC); //DeleteActivePlots
        viewerState.notify(0);
    }

    this.GetViewUp = function()
    {
       var View3DIndex = viewerState.getIndexFromTypename("View3DAttributes");
       var viewUp = viewerState.get(View3DIndex, "viewUp");
       var viewNormal = viewerState.get(View3DIndex, "viewNormal");
       return [viewUp, viewNormal];
    }

    this.UpdateView = function(up, normal)
    {
       var View3DIndex = viewerState.getIndexFromTypename("View3DAttributes");

       //todo: modify this to update two actions at the same time..
       this.SetActiveWindow(windowId);
    
       //var viewUp = viewerState.get(View3DIndex, "viewUp");
       //var viewNormal = viewerState.get(View3DIndex, "viewNormal");
       
       //var newViewUp = modelMat.row(2).elements;
       //var newViewNormal = modelMat.row(3).elements; 

       //var up = [ newViewUp[0], newViewUp[1], newViewUp[2] ];
       //var normal = [ newViewNormal[0], newViewNormal[1], newViewNormal[2] ];

       //alert(up + " " + normal);
       viewerState.set(View3DIndex,"viewUp", up);
       viewerState.set(View3DIndex,"viewNormal", normal);

       //todo: modify this to update two actions at the same time..
       viewerState.notify(View3DIndex);
       this.SetView3D();
    }

    this.SetView3D = function()
    {
       viewerState.set(0,"RPCType",RPCType.SetView3DRPC);
       viewerState.notify(0);
    }

    this.DisconnectClient = function ()
    {
        viewerState.set(0,"RPCType",RPCType.DetachRPC);
        viewerState.notify(0);
    }
}

function VisItProxy()
{
    if(!jQuery)
    {
        alert("JQuery is required for VisIt to run properly.. attempting workaround..");
        var script = document.createElement('script');
        script.src = "https://ajax.googleapis.com/ajax/libs/jquery/1.7.2/jquery.min.js";
        script.type = 'text/javascript';
        document.getElementsByTagName('head')[0].appendChild(script);
    }

    var visitImage = document.getElementById("visitImage");
   
    /// initial handshake
    var hostname = "localhost";
    var port = "9002";
    var password = "bob";
    var wsUri = "ws://" + hostname + ":" + port;

    var userName = "";
    var windowId = 1;
    var windowWidth = 800;
    var windowHeight = 800;

    /// visit variables
    var websocket = null;
    var inputConnection = null;
    var outputConnection = null;
    var visitState = null;

    var viewerState = new ViewerState();
    var viewerMethods = new ViewerMethods(viewerState);
 
    var viewerClientAttributesId = 51;
    var viewerClientInformation = 52;

    var hasInitialized = false;
    var lastInitializedId = 140; //TODO: Get this information from header..    
    var initializedCallback = null;

    this.connect = function(host, prt, passwd, uname, wid, width, height, iCallback)
    {
        hostname = host;
        port = prt;
        password = passwd;
        userName = uname;
        wsUri = "ws://" + hostname + ":" + port;

        windowId = parseInt(wid);
        if(windowId == 0)
            windowId = 1;

        windowWidth = width;
        windowHeight = height;

        initializedCallback = iCallback;
        this.initWebSocket();
    };
        
    this.initWebSocket = function ()
    {
         try {
             this.stopWebSocket(); /// stop previous one..

             if (typeof MozWebSocket == 'function')
                 WebSocket = MozWebSocket;

             websocket = new WebSocket(wsUri);
             websocket.onopen = function (evt) 
             {
                debug("Sending initial password..");
                /// once connected send password
                var handshake = {};
                handshake["password"] = password;
                handshake["canRender"] = true;
                handshake["geometry"] = windowWidth + "x" + windowHeight;
                handshake["name"] = userName;
                handshake["windowIds"] = new Array();
                handshake["windowIds"][0] = parseInt(windowId);
                websocket.send( JSON.stringify(handshake) );
             };
             websocket.onclose = function (evt) 
             {
                debug("Disconnecting initial connection..");
                //connectToVisIt(uri);
             };
             websocket.onmessage = function (evt) 
             {
                initializeVisIt(evt.data);
             };
             websocket.onerror = function (evt)
             {
                debug('ERROR: ' + evt.data);
             };
         } catch (exception) {
             debug('ERROR: ' + exception);
         }
    }

    
    function connectToVisIt(uri)
    {
        try {
            /// setup input connection
            debug("connectToVisIt: " + uri);

            inputConnection = new WebSocket(uri);

            inputConnection.onopen = function (evt)
            {
                debug("Incoming Connection Connected");
            };
            inputConnection.onclose = function (evt)
            {
                debug("Incoming Connection Disconnected");
            };
            inputConnection.onmessage = function (evt) 
            {
                parseInput(evt.data);
            };
            inputConnection.onerror = function (evt) 
            {
                debug('ERROR: ' + evt.data);
            };

            /// setup output connection
            outputConnection = new WebSocket(uri);

            outputConnection.onopen = function (evt)
            {
                debug("Outgoing Connection Connected");
                viewerState.setConnection(outputConnection);
            };
            outputConnection.onclose = function (evt)
            {
                debug("Outgoing Connection Disconnected");
            };
            outputConnection.onmessage = function (evt)
            {
                if(!visitState.hasOwnProperty("socketKey"))
                {
                    debug("Outgoing write header");
                    readHeader(evt.data);
                    writeHeader();
                }
                else
                {
                    debug("outgoing: " + evt.data);
                }
            };
            outputConnection.onerror = function (evt)
            {
                debug('ERROR: ' + evt.data);
            };

        } catch (exception) {
            debug('ERROR: ' + exception);
        }
    }

    this.stopWebSocket = function ()
    {
        if(inputConnection && outputConnection)
            this.DisconnectClient();

        visitState = null;
        
        if (websocket) websocket.close();
        if (inputConnection) inputConnection.close();
        if (outputConnection) outputConnection.close();
    }

    this.checkSocket = function ()
    {
        if ( websocket )
         debug("state " + convertState(websocket.readyState));
        if ( inputConnection )
         debug("state " + convertState(inputConnection.readyState));
        if ( outputConnection )
         debug("state " + convertState(outputConnection.readyState));
    }

    function convertState(state)
    {
        var stateStr = "";
        switch (state) {
        case 0:
            stateStr = "CONNECTING";
            break;
        case 1:
            stateStr = "OPEN";
            break;
        case 2:
            stateStr = "CLOSING";
            break;
        case 3:
            stateStr = "CLOSED";
            break;
        default:
            stateStr = "UNKNOWN";
            break;
        }
        return stateStr;
    }

    /////////////////////////////////////////////////

    function initializeVisIt(data)
    {
        debug("Initializing VisIt: " + data);
        visitState = jQuery.parseJSON(data);
        var uri = "ws://" + visitState.host + ":" + visitState.port;
        debug(uri);
        //websocket.close();
        connectToVisIt(uri);
    }

    function readHeader(data)
    {
        debug("Reading header");
        var visitHeader = jQuery.parseJSON(data)
        visitState["socketKey"] = visitHeader.socketKey;
    }

    function writeHeader()
    {
        /// now write outgoing header
        var key = {};
        key["version"] = visitState.version;
        key["securityKey"] = visitState.securityKey;
        key["socketKey"] = visitState.socketKey;

        key = JSON.stringify(key);
        debug("Writing header: " + key);
        inputConnection.send(key);
    }

    function parseInput(data)
    {
        var obj = null;
        try{
            obj = jQuery.parseJSON(data)
        }
        catch(err)
        {
           //alert(data);
           return;
        }

        if(obj.hasOwnProperty("version") && obj.hasOwnProperty("socketKey"))
        {
            readHeader(data);
            writeHeader();
            return;
        }

        viewerState.update(obj);

        /// TODO: have a robust mechanism for initialization..
        if(hasInitialized == false && viewerState.getStates().length == lastInitializedId) {
            initializedCallback();
            hasInitialized = true;
        }

    }

    //// VisIt functions /////

    this.GetViewerState = function ()
    {
        return viewerState;
    }

    this.GetViewerMethods = function ()
    {
        return viewerMethods;
    }

    this.GetInputConnection = function ()
    {
        return inputConnection;
    }

    this.GetOutputConnection = function ()
    {
        return outputConnection;
    }
    
/*
    this.SendMessage = function (message)
    {
        outmessage = "Message: " + userName + " : " + message;
        viewerState.set(viewerClientAttributesId,"state", "message");
        viewerState.set(viewerClientAttributesId,"message", outmessage);
        notify(viewerClientAttributesId);

        // add message to debug log (local message)..
        debug(outmessage);
    }
*/
}


var RPCType = {
        CloseRPC : 0,
        DetachRPC : 1,
        AddWindowRPC : 2,
        DeleteWindowRPC : 3,
        SetWindowLayoutRPC : 4,
        SetActiveWindowRPC : 5,
        ClearWindowRPC : 6,
        ClearAllWindowsRPC : 7,
        OpenDatabaseRPC : 8,
        CloseDatabaseRPC : 9,
        ActivateDatabaseRPC : 10,
        CheckForNewStatesRPC : 11,
        CreateDatabaseCorrelationRPC : 12,
        AlterDatabaseCorrelationRPC : 13,
        DeleteDatabaseCorrelationRPC : 14,
        ReOpenDatabaseRPC : 15,
        ReplaceDatabaseRPC : 16,
        OverlayDatabaseRPC : 17,
        OpenComputeEngineRPC : 18,
        CloseComputeEngineRPC : 19,
        AnimationSetNFramesRPC : 20,
        AnimationPlayRPC : 21,
        AnimationReversePlayRPC : 22,
        AnimationStopRPC : 23,
        TimeSliderNextStateRPC : 24,
        TimeSliderPreviousStateRPC : 25,
        SetTimeSliderStateRPC : 26,
        SetActiveTimeSliderRPC : 27,
        AddPlotRPC : 28,
        SetPlotFrameRangeRPC : 29,
        DeletePlotKeyframeRPC : 30,
        MovePlotKeyframeRPC : 31,
        DeleteActivePlotsRPC : 32,
        HideActivePlotsRPC : 33,
        DrawPlotsRPC : 34,
        DisableRedrawRPC : 35,
        RedrawRPC : 36,
        SetActivePlotsRPC : 37,
        ChangeActivePlotsVarRPC : 38,
        AddOperatorRPC : 39,
        AddInitializedOperatorRPC : 40,
        PromoteOperatorRPC : 41,
        DemoteOperatorRPC : 42,
        RemoveOperatorRPC : 43,
        RemoveLastOperatorRPC : 44,
        RemoveAllOperatorsRPC : 45,
        SaveWindowRPC : 46,
        SetDefaultPlotOptionsRPC : 47,
        SetPlotOptionsRPC : 48,
        SetDefaultOperatorOptionsRPC : 49,
        SetOperatorOptionsRPC : 50,
        WriteConfigFileRPC : 51,
        ConnectToMetaDataServerRPC : 52,
        IconifyAllWindowsRPC : 53,
        DeIconifyAllWindowsRPC : 54,
        ShowAllWindowsRPC : 55,
        HideAllWindowsRPC : 56,
        UpdateColorTableRPC : 57,
        SetAnnotationAttributesRPC : 58,
        SetDefaultAnnotationAttributesRPC : 59,
        ResetAnnotationAttributesRPC : 60,
        SetKeyframeAttributesRPC : 61,
        SetPlotSILRestrictionRPC : 62,
        SetViewAxisArrayRPC : 63,
        SetViewCurveRPC : 64,
        SetView2DRPC : 65,
        SetView3DRPC : 66,
        ResetPlotOptionsRPC : 67,
        ResetOperatorOptionsRPC : 68,
        SetAppearanceRPC : 69,
        ProcessExpressionsRPC : 70,
        SetLightListRPC : 71,
        SetDefaultLightListRPC : 72,
        ResetLightListRPC : 73,
        SetAnimationAttributesRPC : 74,
        SetWindowAreaRPC : 75,
        PrintWindowRPC : 76,
        ResetViewRPC : 77,
        RecenterViewRPC : 78,
        ToggleAllowPopupRPC : 79,
        ToggleMaintainViewModeRPC : 80,
        ToggleBoundingBoxModeRPC : 81,
        ToggleCameraViewModeRPC : 82,
        TogglePerspectiveViewRPC : 83,
        ToggleSpinModeRPC : 84,
        ToggleLockTimeRPC : 85,
        ToggleLockToolsRPC : 86,
        ToggleLockViewModeRPC : 87,
        ToggleFullFrameRPC : 88,
        UndoViewRPC : 89,
        RedoViewRPC : 90,
        InvertBackgroundRPC : 91,
        ClearPickPointsRPC : 92,
        SetWindowModeRPC : 93,
        EnableToolRPC  : 94,
        SetToolUpdateModeRPC : 95,
        CopyViewToWindowRPC : 96,
        CopyLightingToWindowRPC : 97,
        CopyAnnotationsToWindowRPC : 98,
        CopyPlotsToWindowRPC : 99,
        ClearCacheRPC : 100,
        ClearCacheForAllEnginesRPC : 101,
        SetViewExtentsTypeRPC : 102,
        ClearRefLinesRPC : 103,
        SetRenderingAttributesRPC : 104,
        QueryRPC : 105,
        CloneWindowRPC : 106,
        SetMaterialAttributesRPC : 107,
        SetDefaultMaterialAttributesRPC : 108,
        ResetMaterialAttributesRPC : 109,
        SetPlotDatabaseStateRPC : 110,
        DeletePlotDatabaseKeyframeRPC : 111,
        MovePlotDatabaseKeyframeRPC : 112,
        ClearViewKeyframesRPC : 113,
        DeleteViewKeyframeRPC : 114,
        MoveViewKeyframeRPC : 115,
        SetViewKeyframeRPC : 116,
        OpenMDServerRPC : 117,
        EnableToolbarRPC : 118,
        HideToolbarsRPC : 119,
        HideToolbarsForAllWindowsRPC : 120,
        ShowToolbarsRPC : 121,
        ShowToolbarsForAllWindowsRPC : 122,
        SetToolbarIconSizeRPC : 123,
        SaveViewRPC : 124,
        SetGlobalLineoutAttributesRPC : 125,
        SetPickAttributesRPC : 126,
        ExportColorTableRPC : 127,
        ExportEntireStateRPC : 128,
        ImportEntireStateRPC : 129,
        ImportEntireStateWithDifferentSourcesRPC : 130,
        ResetPickAttributesRPC : 131,
        AddAnnotationObjectRPC : 132,
        HideActiveAnnotationObjectsRPC : 133,
        DeleteActiveAnnotationObjectsRPC : 134,
        RaiseActiveAnnotationObjectsRPC : 135,
        LowerActiveAnnotationObjectsRPC : 136,
        SetAnnotationObjectOptionsRPC : 137,
        SetDefaultAnnotationObjectListRPC : 138,
        ResetAnnotationObjectListRPC : 139,
        ResetPickLetterRPC : 140,
        SetDefaultPickAttributesRPC : 141,
        ChooseCenterOfRotationRPC : 142,
        SetCenterOfRotationRPC : 143,
        SetQueryOverTimeAttributesRPC : 144,
        SetDefaultQueryOverTimeAttributesRPC : 145,
        ResetQueryOverTimeAttributesRPC : 146,
        ResetLineoutColorRPC : 147,
        SetInteractorAttributesRPC : 148,
        SetDefaultInteractorAttributesRPC : 149,
        ResetInteractorAttributesRPC : 150,
        GetProcInfoRPC : 151,
        SendSimulationCommandRPC : 152,
        UpdateDBPluginInfoRPC : 153,
        ExportDBRPC : 154,
        SetTryHarderCyclesTimesRPC : 155,
        OpenClientRPC : 156,
        OpenGUIClientRPC : 157,
        OpenCLIClientRPC : 158,
        SuppressQueryOutputRPC : 159,
        SetQueryFloatFormatRPC : 160,
        SetMeshManagementAttributesRPC : 161,
        SetDefaultMeshManagementAttributesRPC : 162,
        ResetMeshManagementAttributesRPC : 163,
        ResizeWindowRPC : 164,
        MoveWindowRPC : 165,
        MoveAndResizeWindowRPC : 166,
        SetStateLoggingRPC : 167,
        ConstructDataBinningRPC : 168,
        RequestMetaDataRPC : 169,
        SetTreatAllDBsAsTimeVaryingRPC : 170,
        SetCreateMeshQualityExpressionsRPC : 171,
        SetCreateTimeDerivativeExpressionsRPC : 172,
        SetCreateVectorMagnitudeExpressionsRPC : 173,
        CopyActivePlotsRPC : 174,
        SetPlotFollowsTimeRPC : 175,
        TurnOffAllLocksRPC : 176,
        SetDefaultFileOpenOptionsRPC : 177,
        SetSuppressMessagesRPC : 178,
        ApplyNamedSelectionRPC : 179,
        CreateNamedSelectionRPC : 180,
        DeleteNamedSelectionRPC : 181,
        LoadNamedSelectionRPC : 182,
        SaveNamedSelectionRPC : 183,
        SetNamedSelectionAutoApplyRPC : 184,
        UpdateNamedSelectionRPC : 185,
        InitializeNamedSelectionVariablesRPC : 186,
        MenuQuitRPC : 187,
        SetPlotDescriptionRPC : 188,
        MovePlotOrderTowardFirstRPC : 189,
        MovePlotOrderTowardLastRPC : 190,
        SetPlotOrderToFirstRPC : 191,
        SetPlotOrderToLastRPC : 192,
        RenamePickLabelRPC : 193,
        GetQueryParametersRPC : 194,
        DDTConnectRPC : 195,
        DDTFocusRPC : 196,
        ReleaseToDDTRPC : 197,
        ExportRPC : 198,
        MaxRPC : 199
    }
