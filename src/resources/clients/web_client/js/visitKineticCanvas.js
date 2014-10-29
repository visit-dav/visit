var debugTextArea = document.getElementById("debugTextArea");


var isEventSupported = (function(){
    var TAGNAMES = {
      'select':'input','change':'input',
      'submit':'form','reset':'form',
      'error':'img','load':'img','abort':'img'
    }
    function isEventSupported(eventName) {
      var el = document.createElement(TAGNAMES[eventName] || 'div');
      eventName = 'on' + eventName;
      var isSupported = (eventName in el);
      if (!isSupported) {
        el.setAttribute(eventName, 'return;');
        isSupported = typeof el[eventName] == 'function';
      }
      el = null;
      return isSupported;
    }
    return isEventSupported;
  })();


function VisItCanvas(container_div_id, screenWidth, screenHeight)
{

    var ctrlKey = false;
    var shiftKey = false;
    var m_screenWidth = screenWidth;
    var m_screenHeight = screenHeight;

    var m_moving = false;

    var m_start_x, m_start_y;

    var m_divId = container_div_id;

    var radius = 1.0;

    var vp = null;

    var m_stage = new Kinetic.Stage({
                    container: m_divId,
                    width: m_screenWidth,
                    height: m_screenHeight
                    });

    var layer = new Kinetic.Layer();
    var imageObj = new Image();
                
    imageObj.onload = function() {
        layer.removeChildren();

        kineticImage = new Kinetic.Image({
                x: 0, y: 0,
                image: imageObj,
                width: m_stage.getWidth(),
                height: m_stage.getHeight()
                });

        kineticImage.setListening(false);
    
        layer.add(kineticImage);
        m_stage.add(layer);
    };

    var borderLayer = new Kinetic.Layer();
            
    var border = new Kinetic.Rect({
                        width: m_stage.getWidth(),
                        height: m_stage.getHeight(),
                        stroke: 'black',
                        strokeWidth: 1
                 });

    border.on('mousedown touchstart', function(e) {
        moveStart(e);
    });
    border.on('mousemove touchmove', function(e) {
        move(e);
    });
    border.on('mouseup touchend', function(e) {
        moveStop(e);
    });

    borderLayer.add(border);
    m_stage.add(borderLayer);

    keyPress = function(e) {
        var evtobj = window.event? event : e;
        ctrlKey = evtobj.ctrlKey;
        shiftKey = evtobj.shiftKey;
    }

    window.addEventListener('keydown',keyPress);
    window.addEventListener('keyup',keyPress);

    /*
     * Start the movement
     */
    moveStart = function(e) {

        var elem = border.getCanvas().element.parentElement;

        var x = (e.pageX - elem.offsetLeft);
        var y = (e.pageY - elem.offsetTop);

        if(isEventSupported('touchstart') && e instanceof TouchEvent)
        {
            xe = e.touches[0];
            x = (xe.pageX - elem.offsetLeft - m_screenWidth/2.0 ) / radius;
            y = -(xe.pageY - elem.offsetTop - m_screenHeight/2.0) / radius;
        }
  
        m_moving = true;
        m_start_x = x;
        m_start_y = y;
        //console.log("started moving");
     }

    move = function(e) {
        if(!m_moving) {
            return;
        }

        //debugTextArea.value += "moving\n";
        var elem = border.getCanvas().element.parentElement;

        var x = (e.pageX - elem.offsetLeft);
        var y = (e.pageY - elem.offsetTop);
        
        if(isEventSupported('touchstart') && e instanceof TouchEvent)
        {
            xe = e.touches[0];
            x = (xe.pageX - elem.offsetLeft - m_screenWidth/2.0 ) / radius;
            y = -(xe.pageY - elem.offsetTop - m_screenHeight/2.0) / radius;
        }
        e.preventDefault();
        //console.log("moving");
    }

    moveStop = function(e) {
        m_moving = false;

        //debugTextArea.value += "moving\n";
        var elem = border.getCanvas().element.parentElement;

        var x = (e.pageX - elem.offsetLeft);
        var y = (e.pageY - elem.offsetTop);

        if(isEventSupported('touchstart') && e instanceof TouchEvent)
        {
            xe = e.touches[0];
            x = (xe.pageX - elem.offsetLeft - m_screenWidth/2.0 ) / radius;
            y = -(xe.pageY - elem.offsetTop - m_screenHeight/2.0) / radius;
        }

        //console.log("stopped moving");
        //console.log(m_start_x + " " + m_start_y + " " + x +  " " + y);

        var start_x = m_start_x / m_stage.getWidth();
        var start_y = m_start_y / m_stage.getHeight();
        var end_x = x / m_stage.getWidth();
        var end_y = y / m_stage.getHeight();

        vp.GetViewerMethods().UpdateMouseActions(1, "LeftPress", start_x, start_y, end_x, end_y, ctrlKey, shiftKey);
        vp.GetViewerMethods().UpdateMouseActions(1, "Move", start_x, start_y, end_x, end_y, ctrlKey, shiftKey);
        vp.GetViewerMethods().UpdateMouseActions(1, "LeftRelease", start_x, start_y, end_x, end_y, ctrlKey, shiftKey);
    }

    this.setVisItConnection = function(conn) {
        vp = conn;
        vp.GetViewerState().registerCallback("ViewerClientInformation",
                                             [this.updateViewer, this] );
        vp.GetViewerMethods().RegisterNewWindow(1);
    }

    this.updateViewer = function(obj) 
    {
        var vars = obj.get("vars");
        var supportedFormats = obj.get("supportedFormats");

        for(var i = 0; i < 1 /*vars.length*/; ++i) {
            if(vars[i].get("format") === supportedFormats.indexOf("Image")) 
            {
                var image  = "data:image/jpeg;base64,";
                image += vars[i].get("data");
                imageObj.src = image;
            }
        }
    }
}
