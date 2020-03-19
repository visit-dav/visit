# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  timeslider.py
#
#  Tests:      mesh        - 2D curvilinear, single domain
#              plots       - Pseudocolor
#              annotations - Time slider
#
#  Defect ID:  VisIt00003746
#
#  Programmer: Brad Whitlock
#  Date:       Thu Dec 4 14:39:21 PST 2003
#
#  Modifications:
#    Brad Whitlock, Wed Mar 9 09:15:30 PDT 2005
#    Removed deprecated functions.
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Update due to change 
#
#    Brad Whitlock, Wed Sep 28 14:12:00 PDT 2011
#    Move the time slider around a couple times.
# ----------------------------------------------------------------------------

# Set up the annotation colors, etc.
a = GetAnnotationAttributes()
a.backgroundColor = (0, 0, 0, 255)
a.foregroundColor = (255, 255, 255, 255)
a.backgroundMode = a.Solid
SetAnnotationAttributes(a)

# Open up the database and create a plot.
OpenDatabase(data_path("pdb_test_data/allinone00.pdb"))

AddPlot("Pseudocolor", "mesh/a")
SetTimeSliderState(4)
DrawPlots()

# Create a time slider
slider = CreateAnnotationObject("TimeSlider")
Test("timeslider00")

# Turn it off
slider.visible = 0
Test("timeslider01")

# Turn it back on and move and resize it
slider.visible = 1
slider.width = 0.60
slider.height = 0.1
slider.position = (0.2, 0.03)
Test("timeslider02")

# Change the colors
slider.startColor = (255, 0, 0, 255)
slider.endColor = (255, 255, 0, 255)
Test("timeslider03")

# Change the rounding mode
slider.rounded = 0
Test("timeslider04")

# Change the shading mode
slider.shaded = 0
Test("timeslider05")

# Change the text
slider.text = "The time is: $time"
Test("timeslider06")
SetTimeSliderState(14)
Test("timeslider07")

# Change the text color
slider.shaded = 1
slider.rounded = 1
slider.useForegroundForTextColor = 0
slider.textColor = (0, 0, 255, 255)
Test("timeslider08")

# Create a new slider
SetTimeSliderState(18)
slider2 = CreateAnnotationObject("TimeSlider","slider_obj")
slider2.position = (0.3, 0.8)
slider2.height = 0.15
Test("timeslider09")

# Delete the first slider.
slider.Delete()
Test("timeslider10")

# Create an alias to the second slider using the GetAnnotationObject call.
foo = GetAnnotationObject("slider_obj")
foo.startColor = (255, 0, 255, 255)
foo.endColor = (0, 255, 255, 100)
Test("timeslider11")

# Delete the annotation using the alias.
foo.Delete()
Test("timeslider12")

# Delete all plots and open a new database
SetTimeSliderState(0)
DeleteAllPlots()
OpenDatabase(silo_data_path("globe.silo"))

AddPlot("Pseudocolor", "speed")
AddPlot("Mesh", "mesh1")
DrawPlots()
v0 = View3DAttributes()
v0.viewNormal = (-0.556371, 0.348221, 0.754449)
v0.focus = (0, 0, 0)
v0.viewUp = (0.185162, 0.93708, -0.295968)
v0.viewAngle = 30
v0.parallelScale = 17.3205
v0.nearPlane = -34.641
v0.farPlane = 34.641
v0.imagePan = (0, 0)
v0.imageZoom = 1.20945
v0.perspective = 1
v0.eyeAngle = 2

v1 = View3DAttributes()
v1.viewNormal = (0.501427, 0.560716, 0.658915)
v1.focus = (0, 0, 0)
v1.viewUp = (-0.272851, 0.825194, -0.494577)
v1.viewAngle = 30
v1.parallelScale = 17.3205
v1.nearPlane = -34.641
v1.farPlane = 34.641
v1.imagePan = (0, 0)
v1.imageZoom = 1.52784
v1.perspective = 1
v1.eyeAngle = 2

# Create a new slider
slider = CreateAnnotationObject("TimeSlider")
slider.position = (0.2, 0.02)
slider.width = 0.6
slider.height = 0.08
slider.percentComplete = 0
slider.timeDisplay = slider.UserSpecified
slider.text = "Using percentComplete manually"
testnum = 13
nsteps = 10
moves = {4 : (0.02, 0.5), 8 : (0.2, 0.9)}
# Set the percentComplete manually
for i in range(nsteps):
    t = float(i) / float(nsteps - 1)
    slider.percentComplete = t * 100.
    if i in list(moves.keys()):
        slider.position = moves[i]
    v = v0 * (1. - t) + v1 * t
    SetView3D(v)
    Test("timeslider%02d" % testnum)
    testnum = testnum + 1

Exit()
