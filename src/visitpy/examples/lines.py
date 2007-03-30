# ----------------------------------------------------------------------------
#  Test script that changes animates an isosurface by changing its iso-value.
#
#  Modifications:
#    Hank Childs, Tue May  7 09:27:44 PDT 2002
#    Updated for 0.9.9.  Made it use a file in the /data directory.
#    Updated view setting routines to meet new interface.
#
#    Brad Whitlock, Tue Apr 1 09:40:34 PDT 2003
#    Updated for 1.1.3.
#
# ----------------------------------------------------------------------------

import os

# Check the version.
ScriptVersion = "1.1.3"
if(Version() != ScriptVersion):
    print "This script is for VisIt %s. It may not work with version %s" % (ScriptVersion, Version())

def lines():
    SetWindowLayout(1)
    OpenDatabase("../../data/spring.dat")
    AddPlot("Subset", "lines(Lines)")
    AddOperator("Tube")
    t = TubeAttributes()
    t.width = 0.25
    SetOperatorOptions(t)
    DisableRedraw()
    DrawPlots()
    
    # Set the view.
    v = ViewAttributes()
    v.camera = (-0.410342, 0.533276, -0.739754)
    v.focus = (0.454296, -0.69183, 4.2213)
    v.viewUp = (0.365615, 0.83935, 0.402266)
    v.viewAngle = 30
    v.setScale = 1
    v.parallelScale = 4.02425
    v.nearPlane = -20.2
    v.farPlane = 20.2139
    v.perspective = 1
    v.windowCoords = (0, 1, 0, 1)
    v.viewportCoords = (0.2, 0.8, 0.2, 0.8)
    SetView3D(v)
    
    RedrawWindow()

if 'Tube' not in OperatorPlugins():
    print "This script requires the Tube operator plugin!"
elif (not os.path.isfile("../../data/spring.dat")):
    print "This script requires the file spring.dat to exist in the data directory"
else:
    lines()
