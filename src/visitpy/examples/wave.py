# ----------------------------------------------------------------------------
#  Test script that plays through the wave database.
#
#  Modifications:
#    Hank Childs, Tue May  7 09:27:44 PDT 2002
#    Updated for 0.9.9.  Made it use a file in the /data directory.
#    Also update view setting methods to meet new interface.
#
#    Brad Whitlock, Tue Apr 1 10:22:17 PDT 2003
#    Updated for 1.1.3.
#
# ----------------------------------------------------------------------------

import os

# Check the version.
ScriptVersion = "1.1.3"
if(Version() != ScriptVersion):
    print "This script is for VisIt %s. It may not work with version %s" % (ScriptVersion, Version())

def wave():
    # Open the file and do some plots.
    OpenDatabase("../../data/wave.visit")
    AddPlot("Pseudocolor", "pressure")
    AddPlot("Mesh", "quadmesh")
    DisableRedraw()
    DrawPlots()
    
    # Set the view.
    v = ViewAttributes()
    v.viewNormal = (-0.630384, 0.624746, 0.46077)
    v.focus = (4.64837, 0.0923395, 2.03837)
    v.viewUp = (0.531596, 0.779962, -0.330249)
    v.viewAngle = 30
    v.setScale = 1
    v.parallelScale = 5.6009
    v.nearPlane = -30
    v.farPlane = 100
    v.perspective = 1
    RedrawWindow()
    
    # Create the final view
    v2 = ViewAttributes()
    v2.viewNormal = (0.585721, 0.364706, 0.723824)
    v2.focus = (6.67418, -0.256771, 0.561709)
    v2.viewUp = (-0.161539, 0.927655, -0.336691)
    v2.viewAngle = 30
    v2.setScale = 1
    v2.parallelScale = 3.47772
    v2.nearPlane = -30
    v2.farPlane = 100
    v2.perspective = 1
    
    # Iterate through the time sequence.
    for i in range(1, GetDatabaseNStates()):
        t = float(i) / float(GetDatabaseNStates() - 1)
        newview = (t * v2) + ((1. - t) * v)
        DisableRedraw()
        AnimationSetFrame(i)
        SetView3D(newview)
        RedrawWindow()

if(not os.path.isfile("../../data/wave.visit")):
    print "This script requires the file wave.visit to be built in the data directory"
else:
    wave()
