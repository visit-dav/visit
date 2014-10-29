
/*
    Each API variable has a few elements
    "name" : { (req) "attrId" : attribute, "type" : attribute_type,
               (opt) "api" : API, possibly ToolTips, etc..}}
*/

var visitRPC = {};

function AttributeSubject()
{
    var callbackList = [];
    var api = null;
    var data = null;
    var id = -1;
    var typename = "";

    this.getEntireState = function() 
    {
        return data;
    }

    this.setEntireState = function(_data)
    {
        data = _data;
    }

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
                var callbackObj = callbackList[i];
                if(callbackObj instanceof  Array){
                    callbackObj[0].apply(callbackObj[1], [this]);
                }
                else {
                    callbackObj(this);
                }
            }
        }
    }

    this.update_internal_api = function(iapi, idata) {
        api = { "api" : iapi };
        data = { "contents" : idata };
        id = -1;
        typename = "";
    }

    this.set = function(key, value) 
    {
        if(api["api"].hasOwnProperty(key)) {
           var index = api["api"][key]["attrId"];         
           data["contents"][index] = value;
        }
    }

    this.get = function(key) {
        var index = api["api"][key]["attrId"];
        var type = api["api"][key]["type"];

        if(type.indexOf("AttributeGroup") >= 0) {
            if(type.indexOf("List") >= 0|| type.indexOf("Vector") >= 0) {
                var subjs = [];

                for(var i = 0; i < data["contents"][index].length; ++i) {
                    var subj = new AttributeSubject();
                    subj.update_internal_api(api["api"][key]["api"], 
                                             data["contents"][index][i]);
                    subjs.push(subj);
                }
                return subjs;
            } 
            else {
                var subj = new AttributeSubject();
                subj.update_internal_api(api["api"][key]["api"], 
                                         data["contents"][index]);
                return subj;
            }
        }

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
    var syncTag = -1;
    var sync_queue = [];
    var initialized = false;

    this.update = function(rawData, forceUpdate) 
    {
        forceUpdate = typeof forceUpdate !== 'undefined' ? forceUpdate : false;
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
        if(index >= 0 && index < states.length) {
            /// waiting on a previous call...
            if(syncTag >= 0) {
                sync_queue.push([index,JSON.stringify(states[index].getEntireState())]);
            } else {
                states[index].notify(output);
                this.synchronize();
            }
        }
    }
        
    this.setConnection = function(conn)
    {
        output = conn;
    }

    this.registerCallbacks = function() {
        this.registerCallback("SyncAttributes", [this.syncCallback, this]);
        initialized = true;
    }

    this.getAttributeSubject = function(index) {
        if(index >= 0 && index < states.length)
            return states[index];
        
        return null;
    }

    this.getStates = function() {
        return states;
    }

    this.syncCallback = function(arg) {
        
        var tag = arg.get("syncTag"); 

        if(syncTag != tag) return;

        //console.log("handling: " + tag);
        syncTag = -1;
        if(sync_queue.length > 0) {
            var obj = sync_queue[0];
            sync_queue.shift();

            var index = obj[0];
            var data = jQuery.parseJSON(obj[1]);

            states[index].setEntireState(data);
            this.notify(index);
        }
    }

    this.synchronize = function(arg) {
        var tag = Math.floor((Math.random() * 10000) + 1);
        var sync_index = this.getIndexFromTypename("SyncAttributes");
        var syncAtts = this.getAttributeSubject(sync_index);

        //console.log("creating: " + tag);
        syncTag = tag;
        syncAtts.set("syncTag", tag);
        syncAtts.notify(output);
    }
}

function ViewerMethods(state) 
{
    var viewerState = state;
    var windowId = 1;
    
    this.InvertBackgroundColor = function ()
    {
        this.SetActiveWindow(windowId);
        viewerState.set(0,"RPCType",visitRPC["InvertBackgroundRPC"]);
        viewerState.notify(0);
    }

    this.SetActiveWindow = function(id)
    {
        viewerState.set(0,"RPCType",visitRPC["SetActiveWindowRPC"]);
        viewerState.set(0,"windowId", id);
        viewerState.set(0,"boolFlag", false);
        viewerState.notify(0);
    }

    this.OpenDatabase = function(str)
    {
        viewerState.set(0,"RPCType",visitRPC["OpenDatabaseRPC"]);
        viewerState.set(0,"intArg1",0);
        viewerState.set(0,"boolFlag",true);
        viewerState.set(0,"stringArg1","");
        viewerState.set(0,"database",str);
        viewerState.notify(0);
    }

    this.DrawPlots = function ()
    {
        viewerState.set(0,"RPCType",visitRPC["DrawPlotsRPC"]);
        viewerState.notify(0);
    }

    getEnabledId = function(plot_type, name) {
        var names = viewerState.get(14, "name");
        var types = viewerState.get(14, "type");
        var enabled = viewerState.get(14, "enabled");

        // alert(names);
        // alert(types);
        // alert(enabled);

        var mapper = [];

        for (i = 0; i < names.length; ++i) {
            if (enabled[i] == 1 && plot_type == types[i])
                mapper.push(names[i]);
        }

        mapper.sort();

        for (i = 0; i < mapper.length; ++i) {
            //alert(name + " " + mapper[i]);
            if (name == mapper[i])
                return i;
        }

        return -1;
    }


    this.AddPlot = function(name, variable) {
        var id = getEnabledId("plot",name);

        if(id < 0) return;

        this.SetActiveWindow(windowId);
        viewerState.set(0,"RPCType",visitRPC["AddPlotRPC"]);
        viewerState.set(0,"plotType",id);
        viewerState.set(0,"variable",variable);
        viewerState.notify(0);
    }

    this.AddPseudocolorPlot = function (variable)
    {
        this.AddPlot("Pseudocolor", variable);
    }

    this.AddContourPlot = function (variable)
    {
        this.AddPlot("Contour", variable);
    }

    this.DeleteActivePlots = function ()
    {
        this.SetActiveWindow(windowId);
        viewerState.set(0,"RPCType",visitRPC["DeleteActivePlotsRPC"]); //DeleteActivePlots
        viewerState.notify(0);
    }

    this.RegisterNewWindow = function(windowId) {

        var node = {};
        node["action"] = "RegisterNewWindow";
        node["windowId"] = windowId;

        viewerState.set(0,"RPCType",visitRPC["ExportRPC"]);
        viewerState.set(0,"stringArg1", JSON.stringify(node));
        viewerState.notify(0);
    }

    this.UpdateMouseActions = function(windowId, button,   
                                       start_dx, start_dy,
                                       end_dx, end_dy, 
                                       isCtrlPressed, isShiftPressed) {

        var node = {};
        node["action"] = "UpdateMouseActions";
        node["mouseButton"] = button;
        node["windowId"] = windowId;
        node["start_dx"] = start_dx;
        node["start_dy"] = start_dy;
        node["end_dx"] = end_dx;
        node["end_dy"] = end_dy;
        node["ctrl"] = isCtrlPressed;
        node["shift"] = isShiftPressed;

        viewerState.set(0,"RPCType",visitRPC["ExportRPC"]);
        viewerState.set(0,"stringArg1", JSON.stringify(node));
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
       viewerState.set(0,"RPCType",visitRPC["SetView3DRPC"]);
       viewerState.notify(0);
    }

    this.DisconnectClient = function ()
    {
        viewerState.set(0,"RPCType",visitRPC["DetachRPC"]);
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
    var numberOfVisItStates = 140;

    var viewerState = new ViewerState();
    var viewerMethods = new ViewerMethods(viewerState);
 
    var viewerClientAttributesId = 51;
    var viewerClientInformation = 52;

    var hasInitialized = false;
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
                handshake["canRender"] = "image";
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
        debug("Initializing VisIt: ");

        visitState = jQuery.parseJSON(data);
        var uri = "ws://" + visitState.host + ":" + visitState.port;

        var rpc_array = visitState["rpc_array"];

        /// create dictionary
        for(i = 0; i < rpc_array.length; ++i) {
            visitRPC[rpc_array[i]] = i;
        }

        numberOfVisItStates = visitState["numStates"];
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
            data = data.replace(/\\\\/g,"\\");
            obj = jQuery.parseJSON(data)
        }
        catch(err)
        {
           debug("failed to parse: " + data);
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
        if(hasInitialized == false && 
           viewerState.getStates().length == numberOfVisItStates) {
            initializedCallback();
            viewerState.registerCallbacks();
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
