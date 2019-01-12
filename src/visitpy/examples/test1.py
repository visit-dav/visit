# ----------------------------------------------------------------------------
#  Test script that creates most of our standard plots.
#
#  Modifications:
#    Hank Childs, Tue May  7 09:27:44 PDT 2002
#    Updated for 0.9.9.  Made it use a file in the /data directory.
#    Also update view setting methods to meet new interface.
#
#    Brad Whitlock, Tue Apr 1 10:11:35 PDT 2003
#    Updated for 1.1.3.
#
#    Eric Brugger, Fri Aug 29 16:31:47 PDT 2003
#    I modified the script to use the new interface for setting views.
#
# ----------------------------------------------------------------------------

import os

# Check the version.
ScriptVersion = "1.2.1"
if(Version() != ScriptVersion):
    print "This script is for VisIt %s. It may not work with version %s" % (ScriptVersion, Version())

view = View3DAttributes()

def test1():
    SetWindowLayout(8)
    
    # The default view.
    view.viewNormal = (-0.381295, 0.357243, 0.852638)
    view.focus = (0, 0, 0)
    view.viewUp = (0.159941, 0.9339, -0.319766)
    view.viewAngle = 30
    view.parallelScale = 17.3205
    view.nearPlane = -20.
    view.farPlane = 81.9615
    view.perspective = 1
    
    # The plot->db mapping.
    dbs   = {"Contour"     : "globe.silo",
             "Mesh"        : "globe.silo",
             "Pseudocolor" : "globe.silo",
             "Subset"      : "globe.silo",
             "Surface"     : "curv2d.silo",
             "Vector"      : "globe.silo",
             "Volume"      : "globe.silo"}
    
    # The plot->var mapping.
    plotvars = {"Contour"     : ["t"],
                "Mesh"        : ["mesh1"],
                "Pseudocolor" : ["v"],
                "Subset"      : ["domains(mesh1)", "mat1(mesh1)"],
                "Surface"     : ["d"],
                "Vector"      : ["vel"],
                "Volume"      : ["t"]}
    
    # This function adds the plot to the window.
    def AddPlotToWindow(p, v):
        AddPlot(p, v)
        DisableRedraw()
        DrawPlots()
        SetView3D(view)
        RedrawWindow()
    
    # Iterate through the list of plot plugins and do the plot using the
    # appropriate variable and database.
    pp = PlotPlugins()
    i = 0
    for p in pp:
        if(dbs.has_key(p)):
            for var in plotvars[p]:
                SetActiveWindow(i + 1)
                OpenDatabase("../../data/%s" % dbs[p])
                AddPlotToWindow(p, var)
                i = i + 1
        else:
            print "The plot type %s is not supported by this script!" % p


if(not os.path.isfile("../../data/globe.silo")):
    print "This script requires the file globe.silo to be built in the data directory"
elif(not os.path.isfile("../../data/curv2d.silo")):
    print "This script requires the file curv2d.silo to be built in the data directory"
else:
    test1()
