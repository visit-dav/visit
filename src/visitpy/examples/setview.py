# ----------------------------------------------------------------------------
#  Test script that changes animates an isosurface by changing its iso-value.
#
#  Modifications:
#    Hank Childs, Tue May  7 09:27:44 PDT 2002
#    Updated for 0.9.9.  Made it use a file in the /data directory.
#    Also update view setting methods to meet new interface.
#
#    Brad Whitlock, Tue Apr 1 09:55:28 PDT 2003
#    Updated for 1.1.3.
#
# ----------------------------------------------------------------------------

import os
from math import sin, cos, sqrt

# Check the version.
ScriptVersion = "1.1.3"
if(Version() != ScriptVersion):
    print "This script is for VisIt %s. It may not work with version %s" % (ScriptVersion, Version())

def setview():
    OpenDatabase("localhost:../../data/globe.silo")
    AddPlot("Pseudocolor", "u")
    DrawPlots()
    a = GetView3D()
    print a

    for angle in range(360):
        radian = float(angle) * (2. * 3.14159 / 360.)
        x = cos(radian)
        z = sin(radian)
        y = z
        h = sqrt(x*x + y*y + z*z)
        x = x / h
        y = y / h
        z = z / h
        a.viewNormal = (x, y, z)
        a.nearPlane = -40.
        a.farPlane = 40.
        SetView3D(a)

if(not os.path.isfile("../../data/globe.silo")):
    print "This script requires the file globe.silo to be built in the data directory"
else:
    setview()
