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
# ----------------------------------------------------------------------------

TurnOnAllAnnotations()
OpenDatabase("../data/globe.silo")
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
a.axesFlag2D = 0
SetAnnotationAttributes(a)
Test("annot2d_01")

a.axesFlag2D = 1

# Test 2D axis labels
a.xAxisLabels2D = 0
a.yAxisLabels2D = 0
SetAnnotationAttributes(a)
Test("annot2d_02")

a.xAxisLabels2D = 1
a.yAxisLabels2D = 0
SetAnnotationAttributes(a)
Test("annot2d_03")

a.xAxisLabels2D = 0
a.yAxisLabels2D = 1
SetAnnotationAttributes(a)
Test("annot2d_04")

a.xAxisLabels2D = 1
a.yAxisLabels2D = 1

# Test 2D axis titles
a.xAxisTitle2D = 0
a.yAxisTitle2D = 0
SetAnnotationAttributes(a)
Test("annot2d_05")

a.xAxisTitle2D = 1
a.yAxisTitle2D = 0
SetAnnotationAttributes(a)
Test("annot2d_06")

a.xAxisTitle2D = 0
a.yAxisTitle2D = 1
SetAnnotationAttributes(a)
Test("annot2d_07")

a.xAxisTitle2D = 1
a.yAxisTitle2D = 1

# Test 2D gridlines
a.xGridLines2D = 1
a.yGridLines2D = 0
SetAnnotationAttributes(a)
Test("annot2d_08")

a.xGridLines2D = 0
a.yGridLines2D = 1
SetAnnotationAttributes(a)
Test("annot2d_09")

a.xGridLines2D = 1
a.yGridLines2D = 1
SetAnnotationAttributes(a)
Test("annot2d_10")

a.xGridLines2D = 0
a.yGridLines2D = 0

# Test auto set ticks off
a.axesAutoSetTicks2D = 0
a.xMajorTickMinimum2D = -8.
a.xMajorTickMaximum2D = 8.
a.xMajorTickSpacing2D = 4.
a.xMinorTickSpacing2D = 1.
a.yMajorTickMinimum2D = -9.
a.yMajorTickMaximum2D = 9.
a.yMajorTickSpacing2D = 3.
a.yMinorTickSpacing2D = 1.
a.axesTicks2D = 4 # All
a.xGridLines2D = 1
a.yGridLines2D = 1
SetAnnotationAttributes(a)
Test("annot2d_11")

a.axesAutoSetTicks2D = 1
a.axesTicks2D = 3 # BottomLeft
a.xGridLines2D = 0
a.yGridLines2D = 0

# Test tick locations
a.axesTickLocation2D = 0 # Inside
SetAnnotationAttributes(a)
Test("annot2d_12")

a.axesTickLocation2D = 2 # Both
SetAnnotationAttributes(a)
Test("annot2d_13")

a.axesTickLocation2D = 1 # Outside

# Test font sizes
a.xLabelFontHeight2D = 0.03
a.yLabelFontHeight2D = 0.03
a.xTitleFontHeight2D = 0.03
a.yTitleFontHeight2D = 0.03
SetAnnotationAttributes(a)
Test("annot2d_14")

a.xLabelFontHeight2D = 0.04
a.yLabelFontHeight2D = 0.04
a.xTitleFontHeight2D = 0.04
a.yTitleFontHeight2D = 0.04
SetAnnotationAttributes(a)
Test("annot2d_15")

a.xLabelFontHeight2D = 0.02
a.yLabelFontHeight2D = 0.02
a.xTitleFontHeight2D = 0.02
a.yTitleFontHeight2D = 0.02

# Test labeling locations
a.axesTicks2D = 0 # Off
SetAnnotationAttributes(a)
Test("annot2d_16")

a.axesTicks2D = 1 # Botton
SetAnnotationAttributes(a)
Test("annot2d_17")

a.axesTicks2D = 2 # Left
SetAnnotationAttributes(a)
Test("annot2d_18")

a.axesTicks2D = 4 # All
SetAnnotationAttributes(a)
Test("annot2d_19")

a.axesTicks2D = 3 # BottomLeft

# Test how the 2D flags play together
# Especially with gridlines on.  Changing
# databases so can also test that gridline
# length is correct in each direction.

DeleteAllPlots()
OpenDatabase("../data/curv2d.silo")
AddPlot("Pseudocolor", "u")
DrawPlots()
ResetView()

# Turn on y axis labels, x grid lines
# and turn off ticks.
a.xAxisLabels2D = 0
a.yAxisLabels2D = 1
a.xGridLines2D = 1
a.yGridLines2D = 0
a.axesTicks2D = 0 # Off
SetAnnotationAttributes(a)
Test("annot2d_20")

# Turn on y axis labels, bottom ticks marks,
# and x and y gridlines 
a.xGridLines2D = 1
a.yGridLines2D = 1
a.axesTicks2D = 1 # Bottom
a.axesTickLocation2D = 1 # Outside
a.xAxisLabels2D = 0
a.yAxisLabels2D = 1
SetAnnotationAttributes(a)
Test("annot2d_21")

# Test how the gridlines work by themselves 
# Turn off labels and ticks, 
a.axesTicks2D = 0 # Off
a.xAxisLabels2D = 0
a.yAxisLabels2D = 0
SetAnnotationAttributes(a)
Test("annot2d_22")

# Test the 2d axes line width.
a.axesTicks2D = 3 # Bottom-Left
a.axesTickLocation2D = 1 # Outside
a.axesLineWidth2D = 2
SetAnnotationAttributes(a)
Test("annot2d_23")

# Test axis labeling with a narrow window.
a.xAxisLabels2D = 1
a.yAxisLabels2D = 1
a.xGridLines2D = 0
a.yGridLines2D = 0
a.axesLineWidth2D = 0
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
## Test 3D options
##

DeleteAllPlots()
OpenDatabase("../data/globe.silo")
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
a.triadFlag = 1
a.bboxFlag = 0
SetAnnotationAttributes(a)
Test("annot3d_02")

# Turn some axes off
a.bboxFlag = 1
a.xAxisLabels = 0
a.yAxisLabels = 1
a.zAxisLabels = 0
a.xAxisTicks = 0
a.yAxisTicks = 1
a.zAxisTicks = 0
SetAnnotationAttributes(a)
Test("annot3d_03")

# Turn on all gridlines
a.xAxisLabels = 1
a.yAxisLabels = 1
a.zAxisLabels = 1
a.xAxisTicks = 1
a.yAxisTicks = 1
a.zAxisTicks = 1
a.xGridLines = 1
a.yGridLines = 1
a.zGridLines = 1
SetAnnotationAttributes(a)
Test("annot3d_04")

# Turn off labels and ticks
# want to ensure gridlines work alone
a.xAxisLabels = 0
a.yAxisLabels = 0
a.zAxisLabels = 0
a.xAxisTicks = 0
a.yAxisTicks = 0
a.zAxisTicks = 0
SetAnnotationAttributes(a)
Test("annot3d_05")

# Mix up which ticks, labels, 
# or gridlines are turned on
a.xAxisLabels = 1
a.yAxisLabels = 0
a.zAxisLabels = 0
a.xAxisTicks = 0
a.yAxisTicks = 1
a.zAxisTicks = 0
a.xGridLines = 0
a.yGridLines = 0
a.zGridLines = 1
SetAnnotationAttributes(a)
Test("annot3d_06")

a.xAxisLabels = 1
a.yAxisLabels = 1
a.zAxisLabels = 1
a.xAxisTicks = 1
a.yAxisTicks = 1
a.zAxisTicks = 1
a.xGridLines = 0
a.yGridLines = 0
a.zGridLines = 0

# Change the 3D axis type
a.axesType = 4 # StaticEdges
SetAnnotationAttributes(a)
Test("annot3d_07")

# Change the 3D axis type
a.axesType = 1 # FurthestTriad
SetAnnotationAttributes(a)
Test("annot3d_08")

# Change the 3D axis type
a.axesType = 2 # OutsideEdges
SetAnnotationAttributes(a)
Test("annot3d_09")

# Change the 3D axis type
a.axesType = 3 # StaticTriad
SetAnnotationAttributes(a)
Test("annot3d_10")

Exit()
