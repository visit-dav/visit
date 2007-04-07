# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  offscreensave.py
#
#  Tests:      save window options
#              mesh      - 3D unstructured, multiple domains
#              plots     - pc, mesh 
#              operators - slice 
#
#  Programmer: Mark C. Miller 
#  Date:       29Mar04 
#
#  Modifications:
#
#    Mark C. Miller, Tue May 11 20:21:24 PDT 2004
#    Changed scalable rendering controls to use activation mode
#
#    Mark C. Miller, Thu Jan  6 15:09:06 PST 2005
#    Added a test that involves annotation objects
#
# ----------------------------------------------------------------------------

OpenDatabase("../data/multi_ucd3d.silo")
AddPlot("Mesh", "mesh1")
AddPlot("Pseudocolor", "d")
DrawPlots()

view=GetView3D()
view.viewNormal=(-0.528567, 0.374238, 0.761946)
view.viewUp=(0.200044, 0.927212, -0.316639)
SetView3D(view)
Test("offscreen_01")

# build some savewindow attributes that DO NOT
# use screen capture
swa=SaveWindowAttributes()
swa.screenCapture=0
swa.width=400
swa.height=400

# simple test to make sure we get all the annoations
# when we save window using off-screen save
Test("offscreen_02",swa)

# now, test saving at a different size
swa.width=600
swa.height=600
Test("offscreen_03",swa)

# Ok, now diddle with annoations a bit and do some
# more saves
swa.width=300
swa.height=300
a = AnnotationAttributes()
a.userInfoFlag = 0
a.databaseInfoFlag = 0
SetAnnotationAttributes(a)
Test("offscreen_04",swa)
a.legendInfoFlag = 0
SetAnnotationAttributes(a)
Test("offscreen_05",swa)

# Ok, now put the window into SR mode
ra = GetRenderingAttributes()
ra.scalableActivationMode = ra.Always
SetRenderingAttributes(ra)
Test("offscreen_06",swa)

# diddle some more with annoations
a.databaseInfoFlag = 1
a.legendInfoFlag = 1
SetAnnotationAttributes(a)
Test("offscreen_07",swa)

# Now, use screen capture mode to save the window
Test("offscreen_08")

DeleteAllPlots()

OpenDatabase("../data/curv2d.silo")

# Test off screen save of a 2D window
AddPlot("Mesh", "curvmesh2d")
AddPlot("Pseudocolor", "d")
DrawPlots()
swa.width=400
swa.height=400
Test("offscreen_09",swa)

# Test off screen save of a full frame 2D window
view2=GetView2D()
view2.fullFrameActivationMode=view2.On
SetView2D(view2)
swa.width=500
swa.height=500
Test("offscreen_10",swa)

# Now test some annotation objects

DeleteAllPlots()

OpenDatabase("../data/multi_ucd3d.silo")
AddPlot("Mesh", "mesh1")
AddPlot("Pseudocolor", "d")
DrawPlots()

view=GetView3D()
view.viewNormal=(-0.528567, 0.374238, 0.761946)
view.viewUp=(0.200044, 0.927212, -0.316639)
SetView3D(view)

# Create a time slider
slider = CreateAnnotationObject("TimeSlider")
slider.position = (0.5, 0.5)
swa.width=300
swa.height=300
Test("offscreen_11",swa)

Exit()
