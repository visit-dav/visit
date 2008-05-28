# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  transparency.py
#
#  Tests:      mesh      - 3D unstructured, single domain
#              plots     - pc, subset
#              operators - none
#              selection - none
#
#  Defect ID:  '2784 + general transparency testing + '7772 + '7773 + '7879
#
#  Programmer: Hank Childs
#  Date:       November 20, 2002
#
# Modifications:
#   Brad Whitlock, Thu Dec 12 09:56:02 PDT 2002
#   I made the test use the new SubsetAttributes.
#
#   Brad Whitlock, Mon Feb 24 10:36:21 PDT 2003
#   I made it use the multiple colors coloring mode for the Subset attributes
#   instead of the single color coloring mode.
#
#   Kathleen Bonnell, Thu Aug 28 14:34:57 PDT 2003
#   Remove compound var name from Subset plots.
#
#   Hank Childs, Thu May  6 09:34:27 PDT 2004
#   Remove out of date comments regarding test that was recently fixed ('4878).
#
#   Hank Childs, Mon Feb 12 14:06:10 PST 2007
#   Add tests for rectilinear transparency ('7772 & '7773).
#
#   Hank Childs, Thu Mar  8 11:13:49 PST 2007
#   Test transparency after scaling ('7879) ... that the scale factor in the
#   transparency actor gets set back to 1.
#
# ----------------------------------------------------------------------------




OpenDatabase("../data/globe.silo")
AddPlot("Subset", "mat1")
DrawPlots()

subsetAtts = SubsetAttributes()
subsetAtts.opacity = 0.9
subsetAtts.colorType = subsetAtts.ColorByMultipleColors
SetPlotOptions(subsetAtts)

# Test that the normals stick around when we slightly drop the transparency
# ('2784)
Test("transparency_01")


# Test that it still looks good when we crank the transparency down.
subsetAtts.opacity = 0.4
SetPlotOptions(subsetAtts)

Test("transparency_02")


AddPlot("Pseudocolor", "u")
AddOperator("Box")
box = BoxAttributes()
box.minx = -4
box.maxx = +2
box.miny = -6
box.maxy = 0
box.minz = -4
box.maxz = +4
SetOperatorOptions(box)
DrawPlots()

# Make sure that it still looks good when we have solid geometry in there.
Test("transparency_03")

pcAtts = PseudocolorAttributes()
pcAtts.opacity = 0.75
SetPlotOptions(pcAtts)

# Test having two transparent plots
Test("transparency_04")


AddPlot("Pseudocolor", "t")
AddOperator("Box")
box.minx = 2
box.maxx = 10
SetOperatorOptions(box)
DrawPlots()

# Test having two transparent plots + plus one solid geometry plot.
Test("transparency_05")


SetPlotOptions(pcAtts)

# Test having three transparent plots, one with zonal centering.  Note:
# this is testing whether or not we can encode zonal-normals as nodal-normals
# and have it play well with other plots that have legitimate nodal-normals.
Test("transparency_06")

# Test that it can do transparency correctly for rectilinear grids ('7772)
DeleteAllPlots()
OpenDatabase("../data/rect3d.silo")
AddPlot("Pseudocolor", "u")
pcAtts = PseudocolorAttributes()
pcAtts.opacity = 0.75
SetPlotOptions(pcAtts)
DrawPlots()
Test("transparency_07")
DeleteAllPlots()

# Test that updates transparency correctly for rectilinear grids ('7773)
# To test this, we must draw a non-transparent grid first.
AddPlot("Pseudocolor", "u")
DrawPlots()
Test("transparency_08")
pcAtts.opacity = 0.75
SetPlotOptions(pcAtts)
DrawPlots()
Test("transparency_09")

DeleteAllPlots()
AddPlot("Histogram", "d")
DrawPlots()
Test("transparency_10")
DeleteAllPlots()
AddPlot("Pseudocolor", "u")
pcAtts.opacity = 0.75
SetPlotOptions(pcAtts)
DrawPlots()
Test("transparency_11")

Exit()
