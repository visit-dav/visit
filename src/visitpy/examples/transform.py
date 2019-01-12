# ----------------------------------------------------------------------------
#  Test script that changes uses the Clip and Transform operators.
#
#  Modifications:
#    Hank Childs, Tue May  7 09:27:44 PDT 2002
#    Updated for 0.9.9.  Made it use a file in the /data directory.
#    Also update view setting methods to meet new interface.
#
#    Brad Whitlock, Tue Apr 1 10:16:33 PDT 2003
#    I updated the script for 1.1.3.
#
#    Eric Brugger, Fri Aug 29 16:32:12 PDT 2003
#    I modified the script to use the new interface for setting views.
#
# ----------------------------------------------------------------------------

import os

# Check the version.
ScriptVersion = "1.2.1"
if(Version() != ScriptVersion):
    print "This script is for VisIt %s. It may not work with version %s" % (ScriptVersion, Version())

def transform():
    # Prevent redraws
    DisableRedraw()
    
    # Open a database and do a pc plot.
    OpenDatabase("localhost:../../data/globe.silo")
    AddPlot("Pseudocolor", "u")
    
    # Add an clip operator to the plot
    clip = ClipAttributes()
    clip.plane2Status = 1
    SetDefaultOperatorOptions(clip)
    AddOperator("Clip")
    
    # Create another plot to cover the hole and add operators to it.
    AddPlot("Pseudocolor", "u")
    pc = PseudocolorAttributes()
    pc.legendFlag = 0
    SetPlotOptions(pc)
    clip2 = ClipAttributes()
    clip2.plane2Status = 1
    clip2.planeInverse = 1
    SetDefaultOperatorOptions(clip2)
    AddOperator("Clip")
    AddOperator("Transform")
    DrawPlots()
    
    # Set the initial view
    v0 = View3DAttributes()
    v0.viewNormal = (0.49658, 0.271031, 0.824591)
    v0.focus = (0, 0, 0)
    v0.viewUp = (0.0462416, 0.94039, -0.336939)
    v0.viewAngle = 30
    v0.parallelScale = 17.3205
    v0.nearPlane = -40.
    v0.farPlane = 40.
    v0.perspective = 1
    SetView3D(v0)
    
    # Set the final view
    v1 = View3DAttributes()
    v1.viewNormal = (0.383878, 0.582957, 0.7161)
    v1.focus = (2.5, 2.5, 0)
    v1.viewUp = (-0.247477, 0.812098, -0.528442)
    v1.viewAngle = 30
    v1.parallelScale = 24.0057
    v1.nearPlane = -40.
    v1.farPlane = 40.
    v1.perspective = 1
    
    # Redraw the window
    RedrawWindow()
    SaveWindow()
    
    # Create the "control points" for the transform
    
    # Control point 1.
    axis0 = [0., 1., 0.]
    rotate0 = 0.
    translate0 = [0., 0., 0.]
    
    # Control point 2.
    axis1 = [0., 1., 0.]
    rotate1 = 90.
    translate1 = [17., 2.5, 0.]
    
    # Control point 3.
    axis2 = [0., 1., 0.]
    rotate2 = 180.
    translate2 = [15., 5., 0.]
    
    # Control point 3.
    axis3 = [0., 0., 1.]
    rotate3 = 180.
    translate3 = [12.5, 12., 0.]
    
    translate_temp = [0., 0., 0.]
    axis_temp = [0., 1., 0.]
    
    # Do a loop to move the second shifted plot and change the view.
    dX = 10
    dY = 10
    transform = TransformAttributes()
    transform.doRotate = 1
    transform.doTranslate = 1
    
    for i in range(50):
        t = (float(i) + 1.) / 50.
        t2 = t * t
        t3 = t * t2
        omt = 1. - t
        omt2 = omt * omt
        omt3 = omt * omt2
        DisableRedraw()
        # Set the rotateAmount in the transform
        c0 = omt3 * rotate0
        c1 = 3. * t * omt2 * rotate1
        c2 = 3. * t2 * omt * rotate2
        c3 = t3 * rotate3
        transform.rotateAmount = c0 + c1 + c2 + c3
        # Compute the axis and the translation
        for j in range(3):
            # Compute the translation
            c0 = omt3 * translate0[j]
            c1 = 3. * t * omt2 * translate1[j]
            c2 = 3. * t2 * omt * translate2[j]
            c3 = t3 * translate3[j]
            translate_temp[j] = c0 + c1 + c2 + c3
            # Compute the axis
            c0 = omt3 * axis0[j]
            c1 = 3. * t * omt2 * axis1[j]
            c2 = 3. * t2 * omt * axis2[j]
            c3 = t3 * axis3[j]
            axis_temp[j] = c0 + c1 + c2 + c3
        # Set the translation and the axis using the computed values.
        transform.translateX = translate_temp[0]
        transform.translateY = translate_temp[1]
        transform.translateZ = translate_temp[2]
        transform.rotateAxis = (axis_temp[0], axis_temp[1], axis_temp[2])
        SetOperatorOptions(transform)
        # Set the view
        view = (((1. - t) * v0) + (t * v1))
        SetView3D(view)
        RedrawWindow()
        SaveWindow()


if(not os.path.isfile("../../data/globe.silo")):
    print "This script requires the file globe.silo to be built in the data directory"
else:
    transform()
