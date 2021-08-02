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
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Eric Brugger, Mon Aug  2 09:42:50 PDT 2021
#    Added test of 16384 x 16384 image save.
#
# ----------------------------------------------------------------------------

import hashlib

TurnOnAllAnnotations()
OpenDatabase(silo_data_path("multi_ucd3d.silo"))

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

OpenDatabase(silo_data_path("curv2d.silo"))


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

OpenDatabase(silo_data_path("multi_ucd3d.silo"))

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

# Test saving a 16384 x 16384 image
slider.Delete()
DeleteAllPlots()

OpenDatabase(silo_data_path("curv2d.silo"))

AddPlot("Pseudocolor", "d")
DrawPlots()

view2=GetView2D()
view2.fullFrameActivationMode=view2.Off
SetView2D(view2)

swa.width = 16384
swa.height = 16384
swa.fileName = "image_16384x16384"
swa.family = 0
SetSaveWindowAttributes(swa)
SaveWindow()

# Comparing md5 sum instead of image, since the image is large.
md5_hash = hashlib.md5()
with open("image_16384x16384.png","rb") as f:
    # Read and update hash in chunks of 4K
    for byte_block in iter(lambda: f.read(4096),b""):
        md5_hash.update(byte_block)
TestValueEQ("md5 hash for 16384x16384 image", md5_hash.hexdigest(), "9196b516c25ecbeac1fab4cd54ee0c59")

Exit()
