# ----------------------------------------------------------------------------
#  Test script that changes animates an isosurface by changing its iso-value.
#
#  Modifications:
#    Hank Childs, Tue May  7 09:27:44 PDT 2002
#    Updated for 0.9.9.  Made it use a file in the /data directory.
#    Also update view setting methods to meet new interface.
#
#    Brad Whitlock, Tue Apr 1 10:00:22 PDT 2003
#    Updated for 1.1.3.
#
# ----------------------------------------------------------------------------

import os

# Check the version.
ScriptVersion = "1.1.3"
if(Version() != ScriptVersion):
    print "This script is for VisIt %s. It may not work with version %s" % (ScriptVersion, Version())

# Define a function to print out the categories and the sets in them.
def PrintCategories(silr):
    for cats in silr.Categories():
        print "Category \"%s\"" % cats
        for sets in silr.SetsInCategory(cats):
            print "\tset[%d] = %s" % (sets, silr.SetName(sets))

# Define a function that adds a subset plot and turns off the sets one by one.
def AddSubsetPlotAndTurnOff(subset, category):
    OpenDatabase("../../data/multi_ucd3d.silo")
    AddPlot("Subset", subset)
    DisableRedraw()
    DrawPlots()
    v = ViewAttributes()
    v.camera = (0.325857, -0.666083, -0.670933)
    v.focus = (-0.620844, 3.45846, 8.74694)
    v.viewUp = (-0.414813, -0.73844, 0.531636)
    v.viewAngle = 30
    v.setScale = 1
    v.parallelScale = 10.1669
    v.nearPlane = -50
    v.farPlane = 50
    v.perspective = 1
    v.windowCoords = (0, 1, 0, 1)
    v.viewportCoords = (0.2, 0.8, 0.2, 0.8)
    SetView3D(v)
    RedrawWindow()
    # Create a new SIL restriction and make the domains disappear one
    # at a time.
    silr2 = SILRestriction()
    PrintCategories(silr2)
    for sets in silr2.SetsInCategory(category):
        silr2.TurnSet(sets, 0)
        SetPlotSILRestriction(silr2)
    # Turn on all the sets.
    silr2.TurnOnAll()
    SetPlotSILRestriction(silr2)

def silselect():
    # Set the window layout to 4.
    SetWindowLayout(4)
    
    # Create a good 2d view.
    v = ViewAttributes()
    v.camera = (0, 0, -1)
    v.focus = (0, 0, 0)
    v.viewUp = (0, 0, 0)
    v.viewAngle = 30
    v.setScale = 0
    v.parallelScale = 1
    v.nearPlane = -50
    v.farPlane = 50
    v.perspective = 1
    v.windowCoords = (-4.04508, 4.04508, 1.17557, 4.75528)
    v.viewportCoords = (0.1, 0.9, 0.2, 0.9)
    
    # Create a domain subset plot.
    OpenDatabase("../../data/multi_curv2d.silo")
    AddPlot("Subset", "domains(mesh1)")
    
    # Create a SIL restriction based on the selected plot.
    silr = SILRestriction()
    PrintCategories(silr)
    
    # Turn off alternate domains.
    onoff = 0
    for sets in silr.SetsInCategory('domains'):
        if(onoff == 0):
            silr.TurnOffSet(sets)
            onoff = 1
        else:
            onoff = 0
    SetPlotSILRestriction(silr)
    SetView2D(v)
    DrawPlots()
    
    # Do a material subset plot in window 2.
    SetActiveWindow(2)
    OpenDatabase("../../data/multi_curv2d.silo")
    AddPlot("Subset", "mat1(mesh1)")
    silr.TurnOnAll()
    silr.TurnOffSet(silr.SetsInCategory('mat1')[1])
    SetPlotSILRestriction(silr)
    SetView2D(v)
    DrawPlots()
    
    # Do a domain subset plot in window 3.
    SetActiveWindow(3)
    AddSubsetPlotAndTurnOff('domains(mesh1)', 'domains')
    
    # Do a material subset plot in window 4.
    SetActiveWindow(4)
    AddSubsetPlotAndTurnOff('mat1(mesh1)', 'mat1')


if(not os.path.isfile("../../data/multi_curv2d.silo")):
    print "This script requires the file multi_curv2d.silo to be built in the data directory"
elif(not os.path.isfile("../../data/multi_ucd3d.silo")):
    print "This script requires the file multi_ucd3d.silo to be built in the data directory"
else:
    silselect()
