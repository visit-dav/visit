# ----------------------------------------------------------------------------
#  Test script that changes animates an isosurface by changing its iso-value.
#
#  Modifications:
#    Hank Childs, Tue May  7 09:27:44 PDT 2002
#    Updated for 0.9.9.  Made it use a file in the /data directory.
#
#    Brad Whitlock, Tue Apr 1 09:48:05 PDT 2003
#    Updated for 1.1.3.
#
#    Eric Brugger, Tue Sep  2 12:04:59 PDT 2003
#    I updated the script for 1.2.1.
#
# ----------------------------------------------------------------------------

import os

# Check the version.
ScriptVersion = "1.2.1"
if(Version() != ScriptVersion):
    print "This script is for VisIt %s. It may not work with version %s" % (ScriptVersion, Version())

def movingcontour():
    # Disable redraws
    DisableRedraw()

    # Open a database and do a pc plot.
    OpenDatabase("localhost:../../data/globe.silo")
    AddPlot("Contour", "u")
    DrawPlots()
    
    # Set the view
    v = GetView3D()
    v.SetViewNormal(-0.528889, 0.367702, 0.7649)
    v.SetViewUp(0.176641, 0.929226, -0.324558)
    v.SetParallelScale(17.3205)
    v.SetPerspective(1)
    SetView3D(v)
    
    # Redraw the window
    #RedrawWindow()
    
    # Create a contour and move it through the data
    c = ContourAttributes()
    c.contourMethod = 2
    for i in range(0, 100, 5):
        if(i == 0):
            c.contourPercent = 0
        elif(i < 94):
            c.contourPercent = (i-5, i, i+5)
        else:
            c.contourPercent = 99
        SetPlotOptions(c)
        # Redraw the window the first time. This also enables future redraws.
        if(i == 0):
            RedrawWindow()


if(not os.path.isfile("../../data/globe.silo")):
    print "This script requires the file globe.silo to be built in the data directory"
else:
    movingcontour()
