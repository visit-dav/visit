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
#   Hank Childs, Wed Dec 31 10:21:31 PST 2008
#   Add a test for multi-block rectilinear grids.
#
#   Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#   Added ability to swtich between Silo's HDF5 and PDB data.
#
#   Kathleen Biagas, Mon Dec 19 15:45:38 PST 2016
#   Use FilledBoundary plot for materials instead of Subset.
#
#   Eric Brugger, Fri Aug  9 13:45:49 PDT 2024
#   Added a test of combining opaque and transparent plots from
#   multi_ucd3d.silo.
#
# ----------------------------------------------------------------------------




OpenDatabase(silo_data_path("globe.silo"))

AddPlot("FilledBoundary", "mat1")
DrawPlots()

fbAtts = FilledBoundaryAttributes()
fbAtts.opacity = 0.9
fbAtts.colorType = fbAtts.ColorByMultipleColors
SetPlotOptions(fbAtts)

# Test that the normals stick around when we slightly drop the transparency
# ('2784)
Test("transparency_01")


# Test that it still looks good when we crank the transparency down.
fbAtts.opacity = 0.4
SetPlotOptions(fbAtts)

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
pcAtts.SetOpacityType(pcAtts.Constant)
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
OpenDatabase(silo_data_path("rect3d.silo"))

AddPlot("Pseudocolor", "u")
pcAtts = PseudocolorAttributes()
pcAtts.SetOpacityType(pcAtts.Constant)
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
pcAtts.SetOpacityType(pcAtts.Constant)
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
pcAtts.SetOpacityType(pcAtts.Constant)
pcAtts.opacity = 0.75
SetPlotOptions(pcAtts)
DrawPlots()
Test("transparency_11")

DeleteAllPlots()
OpenDatabase(silo_data_path("multi_rect2d.silo"))

AddPlot("Pseudocolor", "d")
DrawPlots()
ResetView()
Test("transparency_12")
pcAtts.SetOpacityType(pcAtts.Constant)
pcAtts.opacity = 0.25
SetPlotOptions(pcAtts)
Test("transparency_13")

# Test transparency with 3d multi block data
DeleteAllPlots()
OpenDatabase(silo_data_path("multi_curv3d.silo"))

AddPlot("Pseudocolor", "d")
pcAtts.SetOpacityType(pcAtts.Constant)
pcAtts.opacity = 0.25
SetPlotOptions(pcAtts)
DrawPlots()
v =GetView3D()
v.viewNormal = (0.432843, 0.303466, 0.848855)
v.focus = (0, 2.5, 15)
v.viewUp = (-0.0787945, 0.950767, -0.299721)
v.viewAngle = 30
v.parallelScale = 16.0078
v.nearPlane = -32.0156
v.farPlane = 32.0156
v.imagePan = (0, 0)
v.imageZoom = 1
v.perspective = 1
v.eyeAngle = 2
v.centerOfRotationSet = 0
v.centerOfRotation = (0, 2.5, 15)
v.axis3DScaleFlag = 0
v.axis3DScales = (1, 1, 1)
v.shear = (0, 0, 1)
v.windowValid = 1
SetView3D(v)
Test("transparency_14")

# Test both opaque and transparent plots with 3d multi block data
DeleteAllPlots()
OpenDatabase("localhost:/usr/gapps/visit/data/multi_ucd3d.silo", 0)

AddPlot("Subset", "mesh1", 1, 1)
s = SubsetAttributes()
s.colorType = s.ColorBySingleColor
s.singleColor = (153, 153, 153, 255)
SetPlotOptions(s)
AddOperator("Slice", 1)
SliceAtts = SliceAttributes()
SliceAtts.originType = SliceAtts.Intercept
SliceAtts.originIntercept = 2.5
SliceAtts.project2d = 0
SetOperatorOptions(SliceAtts, 0, 1)

AddPlot("Subset", "domains(mesh1)", 1, 0)
s = SubsetAttributes()
s.opacity = 0.403922
SetPlotOptions(s)

DrawPlots()
v = GetView3D()
v.viewNormal = (0.281187, 0.666153, 0.690778)
v.focus = (0, 2.5, 10)
v.viewUp = (-0.285935, 0.745284, -0.602323)
v.viewAngle = 30
v.parallelScale = 11.4564
v.nearPlane = -22.9129
v.farPlane = 22.9129
v.imagePan = (0, 0)
v.imageZoom = 1
v.perspective = 1
v.eyeAngle = 2
v.centerOfRotationSet = 0
v.centerOfRotation = (0, 2.5, 10)
v.axis3DScaleFlag = 0
v.axis3DScales = (1, 1, 1)
v.shear = (0, 0, 1)
v.windowValid = 1
SetView3D(v)

Test("transparency_15")

Exit()
