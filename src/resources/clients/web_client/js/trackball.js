/*
 * Created for CS352, Calvin College Computer Science
 *
 * Peter Plantinga -- March 2011
 * Harry Plantinga -- March 2011
 */

var modelMat, oldModelMat;  // current, old modelview matrix
var projectionMat;
var p1;
var trackball = {
  screenWidth:  200,
  screenHeight: 200,
  radius:       50,
  moving:   false,
};

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

trackball.init = function (stage) {  

  trackball.screenWidth = stage.getWidth();
  trackball.screenHeight = stage.getHeight();
  trackball.radius = trackball.screenWidth/4.0;
  // load dataset
  // trackball.load();

  modelMat = oldModelMat = $M([[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]]);

  var borderLayer = new Kinetic.Layer();
  var border = new Kinetic.Rect({
          width: stage.getWidth(),
          height: stage.getHeight(),
          stroke: 'black',
          strokeWidth: 1
  });

  border.on('mousedown touchstart', function(e) {
    trackball.moveStart(e);
  });
  border.on('mousemove touchmove', function(e) {
    trackball.move(e);
  });
  border.on('mouseup touchend', function(e) {
    trackball.moveStop(e);
  });

  borderLayer.add(border);
  stage.add(borderLayer);

  trackball.border = border;
  trackball.layer = borderLayer;
}

/*
 * Start the movement
 */
trackball.moveStart = function(e) {
  var elem = trackball.border.getCanvas().element.parentElement;

  var x = (e.pageX - elem.offsetLeft - trackball.screenWidth/2.0 ) / trackball.radius;
  var y = -(e.pageY - elem.offsetTop - trackball.screenHeight/2.0) / trackball.radius;

  if(isEventSupported('touchstart') && e instanceof TouchEvent)
  {
     xe = e.touches[0];
     x = (xe.pageX - elem.offsetLeft - trackball.screenWidth/2.0 ) / trackball.radius;
     y = -(xe.pageY - elem.offsetTop - trackball.screenHeight/2.0) / trackball.radius;
  }
  var z = Math.sqrt(Math.max(1 - x * x - y * y, 0.0));
  p1 = $V([x,y,z]);
  
  var values  = vp.GetViewerMethods().GetViewUp();

  /*
  oldModelMat.row(2).elements[0] = values[0][0];
  oldModelMat.row(2).elements[1] = values[0][1];
  oldModelMat.row(2).elements[2] = values[0][2];

  oldModelMat.row(3).elements[0] = values[0][0];
  oldModelMat.row(3).elements[1] = values[0][1];
  oldModelMat.row(2).elements[2] = values[0][2];
  */
  trackball.moving = true;
}

/*
 * Multiply the vectors by a rotation matrix
 */
trackball.move = function(e) {
  if(trackball.moving) {
  //debugTextArea.value += "moving\n";
  var elem = trackball.border.getCanvas().element.parentElement;

    var x = (e.pageX - elem.offsetLeft - trackball.screenWidth/2.0 ) / trackball.radius;
    var y = -(e.pageY - elem.offsetTop - trackball.screenHeight/2.0) / trackball.radius;
  if(isEventSupported('touchstart') && e instanceof TouchEvent)
  {
     xe = e.touches[0];
     x = (xe.pageX - elem.offsetLeft - trackball.screenWidth/2.0 ) / trackball.radius;
     y = -(xe.pageY - elem.offsetTop - trackball.screenHeight/2.0) / trackball.radius;
  }
    var z = Math.sqrt(Math.max(1 - x * x - y * y, 0));
    var p2 = $V([x,y,z]);
    var n = p1.cross(p2);
    var th = p1.angleFrom(p2);
    modelMat = trackball.Rotate4(th, n).x(oldModelMat);
    //vp.UpdateView(modelMat);
    e.preventDefault();
  }
}

/*
 * Stop moving and prepend the rotation matrix to the old matrix
 */
trackball.moveStop = function(e) {
  //debugTextArea.value += "end\n";
  oldModelMat = modelMat;
  trackball.moving = false;

  var newViewUp = modelMat.row(2).elements;
  var newViewNormal = modelMat.row(3).elements; 

  var up = [ newViewUp[0], newViewUp[1], newViewUp[2] ];
  var normal = [ newViewNormal[0], newViewNormal[1], newViewNormal[2] ];

  vp.GetViewerMethods().UpdateView(up, normal);
  //vp.SetView3D();
}

trackball.compare = function(face1, face2) {
  var sum = 0;
  for(var i = 0; i < face1.indices.length; i++) {
    sum += modelMat.x(vertices[face1.indices[i]]).e(3);
  }
  avg1 = sum / face1.indices.length;
  sum = 0;
  for(var i = 0; i < face2.indices.length; i++) {
    sum += modelMat.x(vertices[face2.indices[i]]).e(3);
  }
  avg2 = sum / face2.indices.length;
  return avg1 - avg2;
}

/*
 * Homogeneous 3D rotation
 */ 
trackball.Rotate4 = function(theta,n) {
    var m1 = Matrix.Rotation(theta,n);
    return $M([ 
        [m1.e(1,1), m1.e(1,2), m1.e(1,3), 0],
        [m1.e(2,1), m1.e(2,2), m1.e(2,3), 0],
        [m1.e(3,1), m1.e(3,2), m1.e(3,3), 0],
        [0, 0, 0, 1]]);
}

/*
 * Homogenous Scaling
 */
trackball.Scale = function() {
  return $M([
    [1, 0, 0, 0],
    [0, 1, 0, 0],
    [0, 0, 1, 0],
    [0, 0, 0, 100/100]]);; //100/$('#zoomSlider').val()]]);
}

/*
 * Homogenous Perspective matrix
 */
trackball.Perspective = function() {
  return $M([
    [1, 0, 0, 0],
    [0, 1, 0, 0],
    [0, 0, 1, 0],
    [0, 0, -10/40, 1]]); //-10/$('#perspectiveSlider').val(), 1]]);
}
   
/*
 * Compute outward normal
 */
trackball.normal = function(face) {
  e1 = vertices[face.indices[1]].subtract(vertices[face.indices[0]]);
  e1 = trackball.transform($V([e1.e(1), e1.e(2), e1.e(3), 1]));
  e2 = vertices[face.indices[2]].subtract(vertices[face.indices[1]]);
  e2 = trackball.transform($V([e2.e(1), e2.e(2), e2.e(3), 1]));
  return e1.cross(e2);
}

/*
 * Transform the vertex
 */
trackball.transform = function (vertex) {
  var v = trackball.Scale().x(modelMat.x(vertex));
  return $V([v.e(1), v.e(2), v.e(3)]).multiply(1/v.e(4));
}
