
function VisItProxy()
{
    /// add support imports 
    var script = document.createElement('script');
    script.src = "https://ajax.googleapis.com/ajax/libs/jquery/1.7.2/jquery.min.js";
    script.type = 'text/javascript';
    document.getElementsByTagName('head')[0].appendChild(script);

    var threescript = document.createElement('script');
    threescript.src = "three.min.js";
    threescript.type = 'text/javascript';

    var visitImage = document.getElementById("visitImage");
    var debugTextArea = document.getElementById("debugTextArea");

    /// initial handshake
    var hostname = "localhost";
    var port = "9002";
    var password = "bob";
    var wsUri = "ws://" + hostname + ":" + port;

    /// visit variables
    var websocket = null;
    var inputConnection = null;
    var outputConnection = null;
    var visitState = null;
    var viewerState = [];
    var viewerApi = [];


    var loaderCallback = null;

    this.SetLoaderCallback = function(l)
    {
        loaderCallback = l;
    }

    this.connect = function(host,prt,passwd)
    {
        hostname = host;
        port = prt;
        password = passwd;
        wsUri = "ws://" + hostname + ":" + port;
        this.initWebSocket();
    };

    function debug(message)
    {
        debugTextArea.value += message + "\n";
        debugTextArea.scrollTop = debugTextArea.scrollHeight;
    }

    this.showAllClasses = function ()
    {
        $.each(viewerApi, function(key,value)
        {
            debug("key: " + key + " " + viewerApi[key].typename);
        });
    }

    this.inspectClass = function ()
    {
        var key = document.getElementById("inspect").value;
        debug("api: " + JSON.stringify(viewerApi[key]));
        debug("data: " + JSON.stringify(viewerState[key]));
    }
            
    this.clearScreen = function ()
    {
        debugTextArea.value = "";
        debugTextArea.scrollTop = debugTextArea.scrollHeight;
    }
            
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

    this.disconnectClient = function ()
    {
        setContents(0,"RPCType",1);
        notify(0);
    }

    this.stopWebSocket = function ()
    {
        if(inputConnection && outputConnection)
            this.disconnectClient();

        visitState = null;
        if (websocket)
         websocket.close();
        if (inputConnection)
         inputConnection.close();
        if (outputConnection)
         outputConnection.close();
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
        debug("initializing VisIt: " + data);
        visitState = jQuery.parseJSON(data);
        uri = "ws://" + visitState.host + ":" + visitState.port;
        debug(uri);
        //websocket.close();
        connectToVisIt(uri);
    }

    function readHeader(data)
    {
        debug("reading header");
        visitHeader = jQuery.parseJSON(data)
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
        debug("writing header: " + key);
        inputConnection.send(key);
    }

    function getKey(id,key)
    {
        return viewerApi[id].api.data[key];
    }

    function getContents(id,key)
    {
        obj = viewerState[id];
        return obj.contents.data[getKey(id,key)];
    }

    function setContents(id,key,value)
    {
        viewerState[id].contents.data[getKey(id,key)] = value;
    }

    function parseInput(data)
    {
        obj = jQuery.parseJSON(data);

        if(obj.hasOwnProperty("version") && obj.hasOwnProperty("socketKey"))
        {
            readHeader(data);
            writeHeader();
            return;
        }

        if(obj.hasOwnProperty("api"))
        {
            //fill out api..
            if(viewerApi.hasOwnProperty(obj.id))
                jQuery.extend(viewerApi[obj.id],obj);
            else
                viewerApi[obj.id] = obj;
        }
        else
        {
            //fill out contents..
            if(viewerState.hasOwnProperty(obj.id))
                jQuery.extend(viewerState[obj.id],obj);
            else
                viewerState[obj.id] = obj;

            //debug("typename: " + obj.typename);
            /// 38 is QueryAttributes, result must be an ImageData
            if(obj.typename === "QueryAttributes" && getContents(obj.id,"resultsMessage") === "ImageData")
            {
                updateImage();
                //updateLoader();
            }
        }
    }

    function updateLoader()
    {
        var vtkdataset = getContents(38,"defaultVars")[0];
        loaderCallback(vtkdataset);
    }

    function updateImage()
    {
        //obj = viewerState[38];
        //var image  = "data:image/png;base64,";
        //var image  = "data:image/jpg;base64,";
        var image  = "data:image/jpeg;base64,";
        //image += obj.contents.data[getContent(obj.id,"defaultVars")][0];
        image += getContents(38,"defaultVars")[0];
        visitImage.src = image;
    }


            //// VisIt functions /////

    function notify(index)
    {
        if(!outputConnection) return;
        //debug(outputConnection);
        outputConnection.send(JSON.stringify(viewerState[index]));
        //inputConnection.send(JSON.stringify(viewerState[index]));
    }

    this.GetViewerState = function ()
    {
        return viewerState;
    }

    this.GetInputConnection = function ()
    {
        return inputConnection;
    }

    this.GetOutputConnection = function ()
    {
        return outputConnection;
    }

    this.InvertBackgroundColor = function ()
    {
        //viewerState[0].contents.data.RPCType = 91; //InvertBackgroundColorRPC
        setContents(0,"RPCType",91); //InvertBackgroundColor
        notify(0);
    }


    function openDatabaseWF(str)
    {
        //viewerState[0].contents.data.RPCType = 8; //OpenDatabaseRPC
        //viewerState[0].contents.data.intArg1 = 0;
        //viewerState[0].contents.data.boolFlag = true;
        //viewerState[0].contents.data.stringArg1 = "";
        //viewerState[0].contents.data.database = str;

        setContents(0,"RPCType",8); //OpenDatabaseRPC
        setContents(0,"intArg1",0);
        setContents(0,"boolFlag",true);
        setContents(0,"stringArg1","");
        setContents(0,"database",str);
        notify(0);
    }

    this.OpenDatabase = function ()
    {
        var filename = document.getElementById("database").value;

        if(filename !== "")
        {
            debug("loading filename: " + filename);
            openDatabaseWF(filename);
        }
    }

    this.DrawPlots = function ()
    {
        //viewerState[0].contents.data.RPCType = 34;
        setContents(0,"RPCType",34); //DrawPlotsRPC
        notify(0);
    }

    this.AddPseudocolorPlot = function ()
    {
        var variable = document.getElementById("variable").value;

        if(variable === "")
        {
         alert("please add variable");
         return;
        }

        //viewerState[0].contents.data.RPCType = 28;
        //viewerState[0].contents.data.plotType = 11;
        //viewerState[0].contents.data.variable = variable;
        setContents(0,"RPCType",28);
        setContents(0,"plotType",11);
        setContents(0,"variable",variable);
        notify(0);
    }

    this.AddContourPlot = function ()
    {
        var variable = document.getElementById("variable").value;

        if(variable === "")
        {
         alert("please add variable");
         return;
        }

        //viewerState[0].contents.data.RPCType = 28;
        //viewerState[0].contents.data.plotType = 1;
        //viewerState[0].contents.data.variable = variable;

        setContents(0,"RPCType",28);
        setContents(0,"plotType",1);
        setContents(0,"variable",variable);
        notify(0);
    }

    this.DeleteActivePlots = function ()
    {
        //viewerState[0].contents.data.RPCType = 32;
        setContents(0,"RPCType",32); //DeleteActivePlots
        notify(0);
    }
}
