# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  ANALYZE.py
#
#  Tests:      mesh      - 3D rectilinear, single domain
#              plots     - Pseudocolor, Contour
#              operators - Slice
#
#  Programmer: Brad Whitlock
#  Date:       Fri Dec 5 09:50:05 PDT 2003
#
#  Modifications:
#    Brad Whitlock, Wed Mar 31 09:13:47 PDT 2004
#    Added code to clear the engine cache to reduce memory usage.
#
#    Brad Whitlock, Wed Mar 9 09:12:05 PDT 2005
#    Updated so it does not use removed, deprecated functions.
#
# ----------------------------------------------------------------------------

# Set up the annotations
a = AnnotationAttributes()
a.userInfoFlag = 0
a.axesFlag = 0
a.bboxFlag = 0
a.backgroundMode = a.Solid
a.backgroundColor = (0, 0, 0, 255)
a.foregroundColor = (255, 255, 255, 255)
SetAnnotationAttributes(a)

# Create a plot using the large database
dbs = ("../data/ANALYZE_test_data/s01_anatomy_stripped.hdr",\
       "../data/ANALYZE_test_data/s01_epi_r01.visit")
OpenDatabase(dbs[0])
AddPlot("Contour", "Variable")
c = ContourAttributes()
c.contourMethod = c.Value
c.contourValue = (40)
c.colorType = c.ColorBySingleColor
c.singleColor = (255,240,220,255)
SetPlotOptions(c)
DrawPlots()

# Set the view
v = View3DAttributes()
v.viewNormal = (0, 0, -1)
v.focus = (0, 0, 0)
v.viewUp = (0, -1, 0)
v.viewAngle = 30
v.parallelScale = 399.965
v.nearPlane = -799.93
v.farPlane = 799.93
v.imagePan = (0, 0)
v.imageZoom = 2.26061
v.perspective = 1
v.eyeAngle = 2
SetView3D(v)

Test("ANALYZE00")

# Look at it from another view
v.viewNormal = (1, 0, 0)
SetView3D(v)
Test("ANALYZE01")

# Look at it from the top
v.viewNormal = (0, -1, 0)
v.viewUp = (-1, 0, 0)
SetView3D(v)
Test("ANALYZE02")

# Look at a new database
ResetView()
DeleteAllPlots()
CloseDatabase(dbs[0])
ClearCache("localhost")

OpenDatabase(dbs[1])
AddPlot("Pseudocolor", "Variable")
p = PseudocolorAttributes()
p.minFlag = 1
p.min = 0
p.maxFlag = 1
p.max = 1300
p.colorTableName = "gray"
p.scaling = p.Skew
p.skewFactor = 0.5
SetPlotOptions(p)

# Slice it.
AddOperator("Slice")
s = SliceAttributes()
s.originType = s.Point
s.originPoint = (0, 0, 0)
s.normal = (0, -1, 0)
s.upAxis = (0, 0, 1)
s.project2d = 1
s.axisType = s.ZAxis
SetOperatorOptions(s)
DrawPlots()

# Add a time slider
slider = CreateAnnotationObject("TimeSlider")
slider.text = "Progress"
slider.height = 0.1

# Animate through the rest of the frames
testnum = 3
for i in range(TimeSliderGetNStates()):
    SetTimeSliderState(i)
    Test("ANALYZE%02d" % testnum)
    testnum = testnum + 1

Exit()

