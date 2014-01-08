# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  annotation.py
#
#  Tests:      mesh      - 3D unstructured, single domain
#              plots     - pc, subset
#              operators - none
#              selection - none
#
#  Defect ID:  none
#
#  Programmer: Brad Whitlock
#  Date:       Mon Jun 3 13:25:18 PST 2002
#
#  Modifications:
#    Kathleen Bonnell, Thu Jul 18 10:33:07 PDT 2002
#    Added test for combinations of ticks/labels/gridlines to ensure
#    that they all play together nicely.  Removed test of userInfo, as it will
#    always have a different timestamp than the baseline image.
#
#    Kathleen Bonnell, Thu Sep  5 10:55:47 PDT 2002 
#    Changed the variable format for SubsetPlots, to reflect new interface
#    which allows subsets of groups. 
#
#    Eric Brugger, Mon Nov 11 14:09:08 PST 2002
#    Changed the test suite extensively since the 2d annotations were
#    enhanced significantly and the 3d annotation field names were changed
#    to match.
#
#    Hank Childs, Tue Nov 19 15:45:12 PST 2002
#    Changed Aslice to Slice to account for interface change in slicing.
#
#    Kathleen Bonnell, Thu Aug 28 14:34:57 PDT 2003
#    Remove compound var name from Subset plots.
#
#    Eric Brugger, Tue Nov 25 15:12:15 PST 2003
#    Added tests for axis labeling with narrow 2d windows.
#
#    Kathleen Bonnell, Wed May  5 08:11:53 PDT 2004
#    Modified slice atts to get same picture since defaults have changed. 
#
#    Kathleen Bonnell, Mon Sep 14 15:01:25 PDT 2009
#    Removed deprecated annotation settings.
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Kathleen Biagas, Wed Jan  8 13:39:46 PST 2014
#    Added 2d tests 26 & 27, gridlines with a different saved window size.
#
# ----------------------------------------------------------------------------

TurnOnAllAnnotations()
OpenDatabase(silo_data_path("globe.silo"))

AddPlot("Pseudocolor", "u")
AddOperator("Slice")
slice = SliceAttributes()
slice.project2d = 1
slice.SetAxisType(slice.XAxis)
slice.SetFlip(1)
SetOperatorOptions(slice)
DrawPlots()

# Test gradient backgrounds
a = GetAnnotationAttributes()
a.gradientColor1 = (0, 255, 255)
a.gradientColor2 = (0, 0, 255)
a.gradientBackgroundStyle = 0 # TopToBottom
a.backgroundMode = 1 # Gradient
SetAnnotationAttributes(a)
Test("annot_01")
a.gradientBackgroundStyle = 4 # Radial
SetAnnotationAttributes(a)
Test("annot_02")

# Test setting background/foreground colors
a.backgroundColor = (0, 0, 0)
a.foregroundColor = (255, 255, 255)
a.backgroundMode = 0 # Solid
SetAnnotationAttributes(a)
Test("annot_03")

# Turn off the database
a.databaseInfoFlag = 0
SetAnnotationAttributes(a)
Test("annot_04")

a.databaseInfoFlag = 1

# Turn off the legend
a.legendInfoFlag = 0
SetAnnotationAttributes(a)
Test("annot_05")

a.legendInfoFlag = 1

##
## Test 2D options
##

# Turn off the 2D axes
a.axes2D.visible = 0
SetAnnotationAttributes(a)
Test("annot2d_01")

a.axes2D.visible = 1

# Test 2D axis labels
a.axes2D.xAxis.label.visible = 0
a.axes2D.yAxis.label.visible = 0
SetAnnotationAttributes(a)
Test("annot2d_02")

a.axes2D.xAxis.label.visible = 1
a.axes2D.yAxis.label.visible = 0
SetAnnotationAttributes(a)
Test("annot2d_03")

a.axes2D.xAxis.label.visible = 0
a.axes2D.yAxis.label.visible = 1
SetAnnotationAttributes(a)
Test("annot2d_04")

a.axes2D.xAxis.label.visible = 1
a.axes2D.yAxis.label.visible = 1

# Test 2D axis titles
a.axes2D.xAxis.title.visible = 0
a.axes2D.yAxis.title.visible = 0
SetAnnotationAttributes(a)
Test("annot2d_05")

a.axes2D.xAxis.title.visible = 1
a.axes2D.yAxis.title.visible = 0
SetAnnotationAttributes(a)
Test("annot2d_06")

a.axes2D.xAxis.title.visible = 0
a.axes2D.yAxis.title.visible = 1
SetAnnotationAttributes(a)
Test("annot2d_07")

a.axes2D.xAxis.title.visible = 1
a.axes2D.yAxis.title.visible = 1

# Test 2D gridlines
a.axes2D.xAxis.grid = 1
a.axes2D.yAxis.grid = 0
SetAnnotationAttributes(a)
Test("annot2d_08")

a.axes2D.xAxis.grid = 0
a.axes2D.yAxis.grid = 1
SetAnnotationAttributes(a)
Test("annot2d_09")

a.axes2D.xAxis.grid = 1
a.axes2D.yAxis.grid = 1
SetAnnotationAttributes(a)
Test("annot2d_10")

a.axes2D.xAxis.grid = 0
a.axes2D.yAxis.grid = 0

# Test auto set ticks off
a.axes2D.autoSetTicks = 0
a.axes2D.xAxis.tickMarks.majorMinimum = -8.
a.axes2D.xAxis.tickMarks.majorMaximum = 8.
a.axes2D.xAxis.tickMarks.majorSpacing = 4.
a.axes2D.xAxis.tickMarks.minorSpacing = 1.
a.axes2D.yAxis.tickMarks.majorMinimum = -9.
a.axes2D.yAxis.tickMarks.majorMaximum = 9.
a.axes2D.yAxis.tickMarks.majorSpacing = 3.
a.axes2D.yAxis.tickMarks.minorSpacing = 1.
a.axes2D.tickAxes = a.axes2D.All  # All
a.axes2D.xAxis.grid = 1
a.axes2D.yAxis.grid = 1
SetAnnotationAttributes(a)
Test("annot2d_11")

a.axes2D.autoSetTicks = 1
a.axes2D.tickAxes = a.axes2D.BottomLeft # BottomLeft
a.axes2D.xAxis.grid = 0
a.axes2D.yAxis.grid = 0

# Test tick locations
a.axes2D.tickLocation = a.axes2D.Inside # Inside
SetAnnotationAttributes(a)
Test("annot2d_12")

a.axes2D.tickLocation = a.axes2D.Both # Both
SetAnnotationAttributes(a)
Test("annot2d_13")

a.axes2D.tickLocation = 1 # Outside

# Test font sizes
a.axes2D.xAxis.label.font.scale = 1.5
a.axes2D.yAxis.label.font.scale = 1.5
a.axes2D.xAxis.title.font.scale = 1.5
a.axes2D.yAxis.title.font.scale = 1.5
SetAnnotationAttributes(a)
Test("annot2d_14")

a.axes2D.xAxis.label.font.scale = 2
a.axes2D.yAxis.label.font.scale = 2 
a.axes2D.xAxis.title.font.scale = 2 
a.axes2D.yAxis.title.font.scale = 2 
SetAnnotationAttributes(a)
Test("annot2d_15")

a.axes2D.xAxis.label.font.scale = 1
a.axes2D.yAxis.label.font.scale = 1
a.axes2D.xAxis.title.font.scale = 1 
a.axes2D.yAxis.title.font.scale = 1

# Test labeling locations
a.axes2D.tickAxes = a.axes2D.Off 
SetAnnotationAttributes(a)
Test("annot2d_16")

a.axes2D.tickAxes = a.axes2D.Bottom
SetAnnotationAttributes(a)
Test("annot2d_17")

a.axes2D.tickAxes = a.axes2D.Left 
SetAnnotationAttributes(a)
Test("annot2d_18")

a.axes2D.tickAxes = a.axes2D.All
SetAnnotationAttributes(a)
Test("annot2d_19")

a.axes2D.tickAxes = a.axes2D.BottomLeft

# Test how the 2D flags play together
# Especially with gridlines on.  Changing
# databases so can also test that gridline
# length is correct in each direction.

DeleteAllPlots()
OpenDatabase(silo_data_path("curv2d.silo"))

AddPlot("Pseudocolor", "u")
DrawPlots()
ResetView()

# Turn on y axis labels, x grid lines
# and turn off ticks.
a.axes2D.xAxis.label.visible = 0
a.axes2D.yAxis.label.visible = 1
a.axes2D.xAxis.grid = 1
a.axes2D.yAxis.grid = 0
a.axes2D.tickAxes = a.axes2D.Off
SetAnnotationAttributes(a)
Test("annot2d_20")

# Turn on y axis labels, bottom ticks marks,
# and x and y gridlines 
a.axes2D.xAxis.grid = 1
a.axes2D.yAxis.grid = 1
a.axes2D.tickAxes = a.axes2D.Bottom
a.axes2D.tickLocation = a.axes2D.Outside
a.axes2D.xAxis.label.visible = 0
a.axes2D.yAxis.label.visible = 1
SetAnnotationAttributes(a)
Test("annot2d_21")

# Test how the gridlines work by themselves 
# Turn off labels and ticks, 
a.axes2D.tickAxes = a.axes2D.Off
a.axes2D.xAxis.label.visible = 0
a.axes2D.yAxis.label.visible = 0
SetAnnotationAttributes(a)
Test("annot2d_22")

# Test the 2d axes line width.
a.axes2D.tickAxes = a.axes2D.BottomLeft
a.axes2D.tickLocation = a.axes2D.Outside
a.axes2D.lineWidth = 2
SetAnnotationAttributes(a)
Test("annot2d_23")

# Test axis labeling with a narrow window.
a.axes2D.xAxis.label.visible = 1
a.axes2D.yAxis.label.visible = 1
a.axes2D.xAxis.grid = 0
a.axes2D.yAxis.grid = 0
a.axes2D.lineWidth = 0
SetAnnotationAttributes(a)

v = View2DAttributes()
v.viewportCoords = (0.2, 0.95, 0.15, 0.95)
v.windowCoords = (-4.68395, 4.89802, 2.5, 2.5001)
SetView2D(v)
Test("annot2d_24")

v.windowCoords = (0., 0.0001, 0.216897, 4.925)
SetView2D(v)
Test("annot2d_25")

##
## Gridlines with a changed window size (#1676)
##
ResetView()
DeleteAllPlots()
OpenDatabase(silo_data_path("multi_rect2d.silo"))
AddPlot("Pseudocolor", "d")
DrawPlots()
a.axes2D.xAxis.grid = 1
a.axes2D.yAxis.grid = 1
SetAnnotationAttributes(a)
Test("annot2d_26")
swa_orig = SaveWindowAttributes()
swa = SaveWindowAttributes()
swa.height = 800
swa.width = 600
swa.resConstraint = swa.NoConstraint
Test("annot2d_27", swa)

##
## Test 3D options
##

DeleteAllPlots()
OpenDatabase(silo_data_path("globe.silo"))

AddPlot("Subset", "mat1")
DrawPlots()
v = View3DAttributes()
v.viewNormal = (-0.527602, 0.329431, 0.783014)
v.focus = (0, 0, 0)
v.viewUp = (0.115312, 0.940986, -0.318195)
v.viewAngle = 30
v.parallelScale = 17.3205
v.nearPlane = -34.641
v.farPlane = 34.641
v.perspective = 1
SetView3D(v)

# Turn off the triad.
a.axes3D.visible = 1
a.axes3D.bboxFlag = 1
a.axes3D.triadFlag = 0
SetAnnotationAttributes(a)
Test("annot3d_01")

# Turn off the bounding box
a.axes3D.triadFlag = 1
a.axes3D.bboxFlag = 0
SetAnnotationAttributes(a)
Test("annot3d_02")

# Turn some axes off
a.axes3D.bboxFlag = 1
a.axes3D.xAxis.label.visible = 0
a.axes3D.yAxis.label.visible = 1
a.axes3D.zAxis.label.visible = 0
a.axes3D.xAxis.tickMarks.visible = 0
a.axes3D.yAxis.tickMarks.visible = 1
a.axes3D.zAxis.tickMarks.visible = 0
SetAnnotationAttributes(a)
Test("annot3d_03")

# Turn on all gridlines
a.axes3D.xAxis.label.visible = 1
a.axes3D.yAxis.label.visible = 1
a.axes3D.zAxis.label.visible = 1
a.axes3D.xAxis.tickMarks.visible = 1
a.axes3D.yAxis.tickMarks.visible = 1
a.axes3D.zAxis.tickMarks.visible = 1
a.axes3D.xAxis.grid = 1
a.axes3D.yAxis.grid = 1
a.axes3D.zAxis.grid = 1
SetAnnotationAttributes(a)
Test("annot3d_04")

# Turn off titles, labels and ticks
# want to ensure gridlines work alone
a.axes3D.xAxis.label.visible = 0
a.axes3D.yAxis.label.visible = 0
a.axes3D.zAxis.label.visible = 0
a.axes3D.xAxis.title.visible = 0
a.axes3D.yAxis.title.visible = 0
a.axes3D.zAxis.title.visible = 0
a.axes3D.xAxis.tickMarks.visible = 0
a.axes3D.yAxis.tickMarks.visible = 0
a.axes3D.zAxis.tickMarks.visible = 0
SetAnnotationAttributes(a)
Test("annot3d_05")

# Mix up which ticks, titles, labels, 
# or gridlines are turned on
a.axes3D.xAxis.label.visible = 1
a.axes3D.yAxis.label.visible = 0
a.axes3D.zAxis.label.visible = 0
a.axes3D.xAxis.title.visible = 0
a.axes3D.yAxis.title.visible = 0
a.axes3D.zAxis.title.visible = 1
a.axes3D.xAxis.tickMarks.visible = 0
a.axes3D.yAxis.tickMarks.visible = 1
a.axes3D.zAxis.tickMarks.visible = 0
a.axes3D.xAxis.grid = 0
a.axes3D.yAxis.grid = 0
a.axes3D.zAxis.grid = 1
SetAnnotationAttributes(a)
Test("annot3d_06")

a.axes3D.xAxis.label.visible = 1
a.axes3D.yAxis.label.visible = 1
a.axes3D.zAxis.label.visible = 1
a.axes3D.xAxis.title.visible = 1
a.axes3D.yAxis.title.visible = 1
a.axes3D.zAxis.title.visible = 1
a.axes3D.xAxis.tickMarks.visible = 1
a.axes3D.yAxis.tickMarks.visible = 1
a.axes3D.zAxis.tickMarks.visible = 1
a.axes3D.xAxis.grid = 0
a.axes3D.yAxis.grid = 0
a.axes3D.zAxis.grid = 0

# Change the 3D axis type
a.axes3D.axesType = a.axes3D.StaticEdges
SetAnnotationAttributes(a)
Test("annot3d_07")

# Change the 3D axis type
a.axes3D.axesType = a.axes3D.FurthestTriad
SetAnnotationAttributes(a)
Test("annot3d_08")

# Change the 3D axis type
a.axes3D.axesType = a.axes3D.OutsideEdges
SetAnnotationAttributes(a)
Test("annot3d_09")

# Change the 3D axis type
a.axes3D.axesType = a.axes3D.StaticTriad
SetAnnotationAttributes(a)
Test("annot3d_10")

Exit()
