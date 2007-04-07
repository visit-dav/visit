# ----------------------------------------------------------------------------
#  MODES: serial
#  CLASSES: nightly
#
#  Test Case:  scatter.py 
#
#  Tests:      Tests the scatter plot
#
#  Programmer: Brad Whitlock
#  Date:       Tue Dec 14 16:58:04 PST 2004
#
#  Modifications:
#    Brad Whitlock, Mon Jul 18 17:19:04 PST 2005
#    Added tests for multiblock data and "scale to cube" that make sure that
#    the extents are gotten correctly beforehand. If the extents are not
#    gathered beforehand for a multi-block dataset when "scale to cube" is
#    on then each domain will use its local extents to map its values to
#    a cube and none of the cubes from the different domains will match,
#    causing an incorrect picture.
#
# ----------------------------------------------------------------------------

# Set up some annotation settings. Turn off tick marks but keep axes
# and legends
a = GetAnnotationAttributes()
a.xAxisTicks = 0
a.yAxisTicks = 0
a.zAxisTicks = 0
a.axesTicks2D = 0
a.xAxisLabels2D = 0
a.yAxisLabels2D = 0
a.xAxisLabels = 0
a.yAxisLabels = 0
a.zAxisLabels = 0
a.databaseInfoFlag = 0
a.userInfoFlag = 0
a.backgroundColor = (0,0,0,255)
a.foregroundColor = (255,255,255,255)
SetAnnotationAttributes(a)

TestSection("Test plot options")

#
# Set up a simple 2 variable scatter plot.
#
OpenDatabase("../data/noise.silo")
AddPlot("Scatter", "hardyglobal")
s = ScatterAttributes()
s.var2 = "shepardglobal"
s.scaleCube = 0
SetPlotOptions(s)
DrawPlots()
Test("scatter_00")

v0 = GetView2D()


#
# Try setting x min and max
#
s.var1MinFlag = 1
s.var1MaxFlag = 1
s.var1Min = 2
s.var1Max = 4
SetPlotOptions(s)
SetView2D(v0)
Test("scatter_01")

#
# Try setting some scaling options for x
#
s.var1MinFlag = 0
s.var1MaxFlag = 0
s.var1Scaling = s.Log
s.scaleCube = 1
SetPlotOptions(s)
ResetView()
v1 = GetView2D()
Test("scatter_02")

s.var1Scaling = s.Skew
s.var1SkewFactor = 10
SetPlotOptions(s)
Test("scatter_03")
s.var1SkewFactor = 0.01
SetPlotOptions(s)
Test("scatter_04")

#
# Add a 3rd dimension
#
s.var1Scaling = s.Linear
s.var3 = "radial"
s.var3Role = s.Coordinate2
SetPlotOptions(s)
v2 = View3DAttributes()
v2.viewNormal = (-0.70917, 0.324761, 0.625787)
v2.focus = (0.5, 0.5, 0.5)
v2.viewUp = (0.218647, 0.945137, -0.242712)
v2.viewAngle = 30
v2.parallelScale = 0.866025
v2.nearPlane = -1.73205
v2.farPlane = 1.73205
v2.imagePan = (0.00504899, 0.0411036)
v2.imageZoom = 1.44477
v2.perspective = 1
v2.eyeAngle = 2
SetView3D(v2)
Test("scatter_05")

#
# Add a color dimension
#
s.var4 = "default"
s.var4Role = s.Color
SetPlotOptions(s)
Test("scatter_06")


#
# Try the min, max for the color
#
s.var4MinFlag = 1
s.var4Min = 2.5
SetPlotOptions(s)
Test("scatter_07")

s.var4MinFlag = 0
s.var4MaxFlag = 1
s.var4Max = 3.4
SetPlotOptions(s)
Test("scatter_08")

#
# Try scaling the color
#
s.var4MinFlag = 0
s.var4MaxFlag = 0
s.var4Scaling = s.Log
SetPlotOptions(s)
Test("scatter_09")

s.var4Scaling = s.Skew
s.var4SkewFactor = 10
SetPlotOptions(s)
Test("scatter_10")

s.var4SkewFactor = 0.01
SetPlotOptions(s)
Test("scatter_11")


#
# Try changing color variables
#
s.var4Scaling = s.Linear
s.var4 = "chromeVf"
SetPlotOptions(s)
Test("scatter_12")


#
# Try changing roles of color and Z.
#
s.var4Role = s.Coordinate2
s.var3Role = s.Color
SetPlotOptions(s)
Test("scatter_13")

#
# Change the roles back and change the color table.
#
s.var4Role = s.Color
s.var3Role = s.Coordinate2
s.var4 = "hardyglobal"
s.colorTableName = "gray"
SetPlotOptions(s)
Test("scatter_14")

#
# Make it use a single color
#
s.var4Role = s.None
s.foregroundFlag = 0
s.singleColor = (0,255,0,255)
SetPlotOptions(s)
Test("scatter_15")

#
# Turn off the legend
#
s.legendFlag = 0
SetPlotOptions(s)
Test("scatter_16")
DeleteActivePlots()

#
# Try a Scatter plot with multi-block data
#
TestSection("Multiblock and scale to cube")
OpenDatabase("../data/multi_rect3d.silo")
AddPlot("Scatter", "d")
s = ScatterAttributes()
s.var2 = "u"
s.var2Role = s.Coordinate1
s.var3 = "v"
s.var3Role = s.Coordinate2
s.var4 = "u"
s.var4Role = s.Color
s.scaleCube = 1
s.pointSize = 0.01
s.pointType = s.Box
SetPlotOptions(s)
DrawPlots()

v = View3DAttributes()
v.viewNormal = (0.482571, 0.41728, 0.770066)
v.focus = (0.5, 0.5, 0.5)
v.viewUp = (-0.269481, 0.907303, -0.322772)
v.viewAngle = 30
v.parallelScale = 0.864825
v.nearPlane = -1.72965
v.farPlane = 1.72965
v.imagePan = (0.0547107, 0.0441637)
v.imageZoom = 1.08726
v.perspective = 1
v.eyeAngle = 2
v.centerOfRotationSet = 0
v.centerOfRotation = (0.5, 0.5, 0.5)
SetView3D(v)
Test("scatter_17")

# Try different scalings.
s.var1Scaling = s.Log
SetPlotOptions(s)
Test("scatter_18")

s.var1Scaling = s.Skew
s.var1SkewFactor = 50.
SetPlotOptions(s)
Test("scatter_19")
DeleteActivePlots()

#
# Do different variables.
#
DefineScalarExpression("xc", "coord(mesh1)[0]")
DefineScalarExpression("yc", "coord(mesh1)[1]")
DefineScalarExpression("zc", "coord(mesh1)[2]")
DefineScalarExpression("nid", "nodeid(mesh1)")
AddPlot("Scatter", "xc")
s = ScatterAttributes()
s.var2 = "yc"
s.var2Role = s.Coordinate1
s.var3 = "zc"
s.var3Role = s.Coordinate2
s.var4 = "nid"
s.var4Role = s.Color
s.scaleCube = 1
s.pointSize = 0.01
s.pointType = s.Box
SetPlotOptions(s)
DrawPlots()

v.viewNormal = (0.799291, 0.383343, 0.462798)
v.focus = (0.5, 0.5, 0.5)
v.viewUp = (-0.325714, 0.92354, -0.202447)
v.viewAngle = 30
v.parallelScale = 0.866025
v.nearPlane = -1.73205
v.farPlane = 1.73205
v.imagePan = (0.000204097, 0.0398577)
v.imageZoom = 1.07422
v.perspective = 1
v.eyeAngle = 2
v.centerOfRotationSet = 0
v.centerOfRotation = (0.5, 0.5, 0.5)
SetView3D(v)
Test("scatter_20")
DeleteActivePlots()

#
# Set min/max values in X,Y,Z and make sure that the points are the right
# color and the extents are right.
#
TestSection("Set min and max values, make sure colors and extents are right")
OpenDatabase("../data/curv3d.silo")
AddPlot("Scatter", "d")
s = ScatterAttributes()
s.var2 = "p"
s.var2Role = s.Coordinate1
s.var3 = "default"
s.var3Role = s.Color
s.pointType = s.Axis
SetPlotOptions(s)
DrawPlots()
ResetView()

Test("scatter_21")

# Set a min in X and make sure the points are still the right color.
s.var1MinFlag = 1
s.var1Min = 3.5
SetPlotOptions(s)
Test("scatter_22")

# Set min max in X and Y
s.var1MinFlag = 1
s.var1MaxFlag = 1
s.var1Min = 2.
s.var1Max = 4.
s.var2MinFlag = 1
s.var2MaxFlag = 1
s.var2Min = 0.3
s.var2Max = 0.4
SetPlotOptions(s)
Test("scatter_23")

DeleteActivePlots()
OpenDatabase("../data/noise.silo")
AddPlot("Scatter", "hardyglobal")
s = ScatterAttributes()
s.var2 = "shepardglobal"
s.var2Role = s.Coordinate1
s.var3 = "radial"
s.var3Role = s.Coordinate2
s.var4 = "radial"
s.var4Role = s.Color
s.scaleCube = 1
SetPlotOptions(s)
DrawPlots()
SetView3D(v2)
Test("scatter_24")

# Set a min in Z.
s.var3MinFlag = 1
s.var3Min = 15.
SetPlotOptions(s)
Test("scatter_25")

# Set a max in Z.
s.var3MaxFlag = 1
s.var3Max = 22.
SetPlotOptions(s)
Test("scatter_26")


Exit()
