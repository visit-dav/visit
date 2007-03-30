# ****************************************************************************
# Program: renderatts.py
#
# Purpose: 
#   Tries stereo rendering and antialiased rendering. It also shows how to
#   access information about the vis window using the GetWindowInformation
#   function.
#
# Note:       
#
# Programmer: Brad Whitlock
# Creation:   Tue Sep 24 09:42:53 PDT 2002
#
# Modifications:
#   Brad Whitlock, Tue Apr 1 09:49:47 PDT 2003
#   I updated the script to 1.1.3.
#
# ****************************************************************************

import os

# Check the version.
ScriptVersion = "1.1.3"
if(Version() != ScriptVersion):
    print "This script is for VisIt %s. It may not work with version %s" % (ScriptVersion, Version())

def renderatts():
    OpenDatabase("localhost:../../data/globe.silo")

    # Create a pseudocolor plot
    AddPlot("Pseudocolor", "u")
    p = PseudocolorAttributes()
    p.opacity = 0.3
    SetPlotOptions(p)

    # Create a mesh plot.
    AddPlot("Mesh", "mesh1")
    m = MeshAttributes()
    m.lineWidth = 1
    SetPlotOptions(m)
    DrawPlots()

    # Set the view
    v = ViewAttributes()
    v.viewNormal = (0.493964, 0.388211, 0.778005)
    v.focus = (0, 0, 0)
    v.viewUp = (-0.208148, 0.921571, -0.327692)
    v.viewAngle = 30
    v.setScale = 1
    v.parallelScale = 15.6706
    v.nearPlane = -34.641
    v.farPlane = 34.641
    v.perspective = 1
    SetView3D(v)

    # Save the window
    SaveWindow()

    # Now try stereo rendering with the red/blue mode.
    r = GetRenderingAttributes()
    r.stereoRendering = 1
    r.stereoType = r.RedBlue
    SetRenderingAttributes(r)
    SaveWindow()

    # Try anti-aliasing
    r.stereoRendering = 0
    r.antialiasing = 1
    SetRenderingAttributes(r)
    SaveWindow()

    # Turn off antialiasing
    r.antialiasing = 0
    SetRenderingAttributes(r)

    # Print out the extents for the plot.
    info = GetWindowInformation()
    print "Plot extents are: (%g, %g) (%g, %g) (%g, %g)" % info.extents

    # Set the view centering mode to "actual"
    SetViewExtentsType("actual")

    # Apply a threshold operator to the plots and print out the extents.
    t = ThresholdAttributes()
    t.lbound = -10.
    t.ubound = 0.
    t.variable = "u"
    SetDefaultOperatorOptions(t)
    AddOperator("Threshold", 1)
    SaveWindow()

    # Print out the extents for the plot.
    info = GetWindowInformation()
    print "Plot extents after thesholding are: (%g, %g) (%g, %g) (%g, %g)" % info.extents

if(not os.path.isfile("../../data/globe.silo")):
    print "This script requires the file globe.silo to be built in the data directory"
else:
    renderatts()
