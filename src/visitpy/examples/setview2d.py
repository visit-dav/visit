# ----------------------------------------------------------------------------
#  Test script that changes animates an isosurface by changing its iso-value.
#
#  Modifications:
#    Hank Childs, Tue May  7 09:27:44 PDT 2002
#    Updated for 0.9.9.  Made it use a file in the /data directory.
#    Also update view setting methods to meet new interface.
#
#    Brad Whitlock, Tue Apr 1 09:57:04 PDT 2003
#    Updated for 1.1.3.
#
#    Eric Brugger, Fri Aug 29 16:30:46 PDT 2003
#    I modified the script to use the new interface for setting views.
#
# ----------------------------------------------------------------------------

import os
from math import sin, cos, sqrt

# Check the version.
ScriptVersion = "1.2.1"
if(Version() != ScriptVersion):
    print("This script is for VisIt %s. It may not work with version %s" % (ScriptVersion, Version()))

def setview2d():
    # Disable redraw until we have things set up.
    DisableRedraw()

    # Set the annotation attributes.
    annot = AnnotationAttributes()
    annot.axesTickLocation2D = annot.Both
    annot.backgroundColor = (0, 0, 0)
    annot.foregroundColor = (255, 255, 255)
    annot.backgroundMode = annot.Solid
    SetAnnotationAttributes(annot)

    # Open a database and do some plots.
    OpenDatabase("localhost:../../data/curv2d.silo")
    AddPlot("Pseudocolor", "d")
    AddPlot("Mesh", "curvmesh2d")
    m = MeshAttributes()
    m.meshColor = (128,128,128)
    SetPlotOptions(m)
    AddPlot("Contour", "d")
    c = ContourAttributes()
    c.lineWidth = 3
    SetPlotOptions(c)
    DrawPlots()
    
    # Set the view.
    v0 = View2DAttributes()
    v0.windowCoords = (-4.71769, -1.18606, 0.543024, 4.07465)
    v0.viewportCoords = (0.22, 0.9, 0.1, 0.9)
    SetView2D(v0)
    RedrawWindow()
    
    # Create a new view.
    v1 = View2DAttributes()
    v1.windowCoords = (1.59449, 5.12612, 0.0458612, 3.57749)
    v1.viewportCoords = (0.22, 0.9, 0.1, 0.9)
    
    # Pan from view v0 to v1.
    for i in range(20):
        t = float(i) / (20. - 1.)
        view = (((1. - t) * v0) + (t * v1))
        SetView2D(view)
    
    # Create a new view.
    v2 = View2DAttributes()
    v2.windowCoords = (1.9001, 3.1167, 0.198349, 1.41495)
    v2.viewportCoords = (0.22, 0.9, 0.1, 0.9)
    
    # Go from view v1 to v2.
    for i in range(10):
        t = float(i) / (10. - 1.)
        view = (((1. - t) * v1) + (t * v2))
        SetView2D(view)



if(not os.path.isfile("../../data/curv2d.silo")):
    print("This script requires the file curv2d.silo to be built in the data directory")
else:
    setview2d()
