# ----------------------------------------------------------------------------
#  MODES: serial
#  CLASSES: nightly
#
#  Test Case:  parcoords.py
#
#  Tests:      Tests the parallel coordinates plot
#
#  Programmer: Jeremy Meredith
#  Date:       January 31, 2008
#
#  Modifications:
#    Jeremy Meredith, Mon Feb  4 16:11:21 EST 2008
#    Removed axis extents from plot attributes since they were unused.
#
#    Jeremy Meredith, Fri Feb  8 13:49:58 EST 2008
#    Added support for array variable expressions, raw database array
#    variables, clamping plot limits, and changing viewports.
#
#    Jeremy Meredith, Mon Feb 18 16:50:02 EST 2008
#    Renamed orderedAxisNames to scalarAxisNames.
#    Added setting of visualAxisNames in one of the places we create the plot
#    from a list of scalar.  This isn't strictly necessary, since the
#    plot can do this for us, but the GUI Wizard knows to do this, and so
#    setting visualAxisNames matches the GUI behavior more closely.
#
# ----------------------------------------------------------------------------


#
# Trying out collections of scalars as a source
#
TestSection("Test scalar collection sources")

OpenDatabase("../data/noise.silo")

# Set default plot variables
p = ParallelCoordinatesAttributes()
p.drawLines = 1
p.linesColor = (128, 0, 0, 255)
p.drawContext = 1
p.contextGamma = 2
p.contextNumPartitions = 512
p.contextColor = (0, 220, 0, 255)
p.drawLinesOnlyIfExtentsOn = 1
SetDefaultPlotOptions(p)

# Set up a simple 3 variable parallel coordinates plot.
AddPlot("ParallelCoordinates", "hardyglobal")
p.scalarAxisNames = ("hardyglobal", "shepardglobal", "chromeVf")
p.visualAxisNames = p.scalarAxisNames # unnecessary, but strictly correct
p.extentMinima = (-1e+37, -1e+37, -1e+37)
p.extentMaxima = (+1e+37, +1e+37, +1e+37)
SetPlotOptions(p)
DrawPlots()
Test("parcoords_01")

# Try setting some appearance-related attributes
p.contextGamma = 2.9
p.contextNumPartitions = 128
p.contextColor = (200, 0, 255, 255)
SetPlotOptions(p)
Test("parcoords_02")

p.contextGamma = 2.9
p.contextNumPartitions = 32
p.contextColor = (200, 0, 255, 255)
p.drawLinesOnlyIfExtentsOn = 0
SetPlotOptions(p)
Test("parcoords_03")

# Add an axis and make sure it works
p.scalarAxisNames = ("hardyglobal", "shepardglobal", "chromeVf", "radial")
p.visualAxisNames = () # test to make sure it works without setting this
p.extentMinima = (-1e+37, -1e+37, -1e+37, -1e+37)
p.extentMaxima = (+1e+37, +1e+37, +1e+37, +1e+37)
p.drawLinesOnlyIfExtentsOn = 1
SetPlotOptions(p)
Test("parcoords_04")

# Make sure the extents work for limiting the lines
p.extentMinima = (5.7, -1e+37, -1e+37, -1e+37)
p.extentMaxima = (5.8, +1e+37, +1e+37, +1e+37)
SetPlotOptions(p)
Test("parcoords_05")

# Try unifying the extents across all axes
p.unifyAxisExtents = 1
SetPlotOptions(p)
Test("parcoords_06")


#
# Trying out array variables as a source
#
TestSection("Test array expression sources")

# Try an array variable
DeleteAllPlots();

OpenDatabase("../data/astronomy.okc")

DefineArrayExpression("arrayvar1",
                      "array_compose(d, b, b/2, b+l)")
DefineArrayExpression("arrayvar2",
                      "array_compose(d+200, l, l-2, b+l)")
DefineArrayExpression("arrayvar2wb",
                      "array_compose_with_bins(d+200, l, l-2, b+l, [0,1,4,15,20])")

AddPlot("ParallelCoordinates", "arrayvar1")
p = ParallelCoordinatesAttributes()
p.contextColor = (100, 100, 255, 255)
p.contextNumPartitions = 32
SetPlotOptions(p)
DrawPlots()
Test("parcoords_07")

# Try changing the array variable
ChangeActivePlotsVar("arrayvar2")
Test("parcoords_08")

# Now change to one with bin-width defined axis x positions
ChangeActivePlotsVar("arrayvar2wb")
Test("parcoords_09")

# Now clamp the axis array limits
p.unifyAxisExtents = 1
SetPlotOptions(p)
Test("parcoords_10")

#
# Trying out raw database-generated array as a source
#
TestSection("Test raw database array sources")


# Now change to a raw database-generated array variable
ChangeActivePlotsVar("all_vars")
p.unifyAxisExtents = 0;
p.contextColor = (255,150,50,255)
SetPlotOptions(p)

# Try changing the viewport so it looks a little more normal
v = GetViewAxisArray()
v.viewportCoords = (0.15,0.9, 0.3,0.7)
SetViewAxisArray(v)
Test("parcoords_11")

# Now clamp the axis array limits again
p.unifyAxisExtents = 1
SetPlotOptions(p)
Test("parcoords_12")

Exit()
