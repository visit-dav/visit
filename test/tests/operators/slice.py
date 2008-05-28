# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  slice.py
#
#  Tests:      mesh      - 3D rectilinear, single domain,
#                          3D unstructured single domain
#                          3D curvilinear multi-domain
#                          3D unstructured multi-domain
#              plots     - pc, filled boundary, mesh
#              operators - slice, onion peel, threshold, index select
#
#  Defect ID:  '3950 (for tests 16,17,18).  '4662 (test 19).
#              '4706 (tests 20-27).  '4913 (test 28), '4975 (test 29).
#              '5157 (test 31-2). '5794 (test 33), '7630 (test 35)
#
#  Known problems:   (1) Slicing of plots that have had material interface
#                    reconstruction can often have holes.  This is a problem
#                    with the interface reconstruction routines.  When those
#                    are fixed, the holes should go away.
#                    Tests affected: 4, 5, 6, 15
#                    (2) The mesh lines are being shifted towards the camera.
#                    For one test, the line actually gets shifted too far and
#                    slightly intersects a cell.
#                    Tests affected: 7, 8, 9
#                    (3) The zonal center for the slice is not consistently
#                    determined (see defect '3437).  This means that test 2
#                    and 5 do not have the same origin for their slice.
#                    This is a known problem.
#
#  Programmer: Hank Childs
#  Date:       June 19, 2003
#
#  Modifications:
#    Kathleen Bonnell, Thu Aug 28 14:34:57 PDT 2003
#    Remove compound var name from subset and boundary plots.
#
#    Hank Childs, Fri Oct 31 14:35:14 PST 2003
#    Added test for getting extents right when projecting to 2D and setting
#    the origin based on a percent.  From '3950.
#
#    Hank Childs, Tue Mar 30 08:42:02 PST 2004
#    Added test for slicing point meshes. ['4662]
#
#    Hank Childs, Tue Apr 13 09:53:21 PDT 2004
#    Test slicing extremes with percent (0%, 100%) ['4706]
#
#    Hank Childs, Mon May 24 16:20:49 PDT 2004
#    Test slicing point meshes in conjunction with the displace operator
#    when the points are all on a plane. ['4913]
#
#    Hank Childs, Fri Jun 18 08:45:11 PDT 2004
#    Test that we can slice boundary plots. ['4975]
#
#    Hank Childs, Tue Jul 20 16:53:16 PDT 2004
#    Test the projection to 2D when the normal and up-axis are not orthogonal
#    ['5157].
#
#    Eric Brugger, Wed Dec 29 15:04:04 PST 2004
#    Added a test for changing the time state for a plot that has been
#    sliced through a zone.  This tests visit00005794.
#
#    Kathleen Bonnell, Tue Jan 25 17:27:16 PST 2005 
#    Added a test for specifying the meshname with domain/zone type of slice. 
#
#    Hank Childs, Sat Jan 27 12:45:03 PST 2007
#    Added a test for slicing 1xJxK and Ix1xK meshes.
#
# ----------------------------------------------------------------------------

# Turn off all annotation
TurnOffAllAnnotations() # defines global object 'a'

OpenDatabase("../data/rect3d.silo")

# Test 1 -- a slice through a point
AddPlot("Pseudocolor", "d")
AddOperator("Slice")
atts = SliceAttributes()
atts.project2d = 0
atts.normal = (-0.1, 0.8, -0.6)
atts.originType = atts.Point
atts.originPoint = (0.3333, 0.094, 0.081)
SetOperatorOptions(atts)
DrawPlots()

view = GetView3D()
view.viewNormal = (-0.53, -0.07, -0.84)
view.focus = (0.5, 0.5, 0.5)
view.viewUp = (0, 1, 0)
view.viewAngle = 30
view.parallelScale = 0.866
view.nearPlane = -1.73
view.farPlane = 1.73
view.imagePan = (0.058, 0.26)
view.imageZoom = 12.6
view.perspective = 1
SetView3D(view)

Test("ops_Slice01")

# Test 2 -- A slice through a zone.  Add an onion peel of that zone to show
# that the slice is in the right spot.
atts.originType = atts.Zone
atts.originZone = 2500
atts.originZoneDomain = 0
SetOperatorOptions(atts)

AddPlot("Pseudocolor", "d")
AddOperator("OnionPeel")
op_atts = OnionPeelAttributes()
op_atts.index = 2500
SetOperatorOptions(op_atts)
DrawPlots()
SetView3D(view)

Test("ops_Slice02")

# Test 3 -- A slice through a node.  The node (2645) is incident to the zone
# from before (2500).
SetActivePlots(0)
atts.originType = atts.Node
atts.originNode = 2645
atts.originNodeDomain = 0
SetOperatorOptions(atts)

Test("ops_Slice03")

# Now repeat the three tests with a filled boundary plot.

DeleteActivePlots()

AddPlot("FilledBoundary", "mat1")
AddOperator("Slice")
atts.originType = atts.Point
atts.originPoint = (0.3333, 0.094, 0.081)
SetOperatorOptions(atts)
DrawPlots()

Test("ops_Slice04")

atts.originType = atts.Zone
atts.originZone = 2500
atts.originZoneDomain = 0
SetOperatorOptions(atts)

Test("ops_Slice05")

atts.originType = atts.Node
atts.originNode = 2645
atts.originNodeDomain = 0
SetOperatorOptions(atts)

Test("ops_Slice06")

# Slice a mesh plot.
DeleteActivePlots()

AddPlot("Mesh", "quadmesh3d")
AddOperator("Slice")
atts.originType = atts.Zone
atts.originZone = 2500
atts.originZoneDomain = 0
SetOperatorOptions(atts)
DrawPlots()

Test("ops_Slice07")

t_atts = ThresholdAttributes()
t_atts.listedVarNames = ("d")
t_atts.lowerBounds = (0.6)
t_atts.upperBounds = (1.0)
SetDefaultOperatorOptions(t_atts)
AddOperator("Threshold")
DemoteOperator(1, 0)
DrawPlots()

Test("ops_Slice08")

RemoveOperator(0, 0)

insel_atts = IndexSelectAttributes()
insel_atts.dim = insel_atts.ThreeD
insel_atts.xMin = 10
insel_atts.xMax = 18
insel_atts.yMin = 3
insel_atts.yMax = 4
insel_atts.zMin = 0
insel_atts.zMax = 3
SetDefaultOperatorOptions(insel_atts)
AddOperator("IndexSelect")
DemoteOperator(1, 0)
DrawPlots()

Test("ops_Slice09")

DeleteAllPlots()

OpenDatabase("../data/sid97.silo")
AddPlot("FilledBoundary", "mat1")
AddOperator("Slice")
atts = SliceAttributes()
atts.originType = atts.Point
atts.originPoint = (7400, -1300, 1000)
atts.normal = (0, 1, 0)
atts.project2d = 1
SetOperatorOptions(atts)
DrawPlots()

Test("ops_Slice10")

DeleteAllPlots()


OpenDatabase("../data/tire.silo")
AddPlot("Subset", "domains")
AddOperator("Slice")
atts.originType = atts.Zone
atts.originZone = 100
atts.originZoneDomain = 2
atts.normal = (0.707, -0.707, 0)
SetOperatorOptions(atts)
DrawPlots()

Test("ops_Slice11")


atts.normal = (0.707, 0.707, 0)
SetOperatorOptions(atts)

Test("ops_Slice12")


atts.originType = atts.Node
atts.originNode = 55
atts.originNodeDomain = 3
SetOperatorOptions(atts)

Test("ops_Slice13")


DeleteAllPlots()

OpenDatabase("../data/multi_ucd3d.silo")
AddPlot("Pseudocolor", "d")
AddOperator("Slice")
atts.originType = atts.Zone
atts.originZone = 1005
atts.originZoneDomain = 18
atts.normal = (0.707, 0, -0.707)
SetOperatorOptions(atts)
DrawPlots()

Test("ops_Slice14")

DeleteAllPlots()

OpenDatabase("../data/multi_ucd3d.silo")
AddPlot("FilledBoundary", "mat1")
AddOperator("Slice")
atts.originType = atts.Zone
atts.originZone = 1005
atts.originZoneDomain = 18
atts.normal = (0.707, 0, -0.707)
SetOperatorOptions(atts)
DrawPlots()

Test("ops_Slice15")


atts.originType = atts.Percent
atts.originPercent = 69
atts.axisType = atts.YAxis
SetOperatorOptions(atts)

Test("ops_Slice16")

atts.originType = atts.Percent
atts.originPercent = 50
atts.axisType = atts.ZAxis
SetOperatorOptions(atts)

Test("ops_Slice17")

atts.originType = atts.Percent
atts.originPercent = 10
atts.axisType = atts.XAxis
SetOperatorOptions(atts)

Test("ops_Slice18")

DeleteAllPlots()

# Test that we can slice point meshes.  The points must be exactly on the
# plane (or at least within some tolerance).  To guarantee the point positions,
# take a 2D plot and put it into 3D.  Then slice it.
OpenDatabase("../data/noise2d.silo")
AddPlot("Pseudocolor", "PointVar")
pc_atts = PseudocolorAttributes()
pc_atts.pointType = pc_atts.Box
pc_atts.pointSize = 0.4
SetPlotOptions(pc_atts)
AddOperator("Transform")
trans_atts = TransformAttributes()
trans_atts.doRotate = 1
trans_atts.rotateAxis = (0, 1, 0)
SetOperatorOptions(trans_atts)
AddOperator("Slice")
atts = SliceAttributes()
atts.axisType = atts.ZAxis
atts.originType = atts.Intercept
atts.originIntercept = 0
SetOperatorOptions(atts)
DrawPlots()

Test("ops_Slice19")

DeleteAllPlots()
ActivateDatabase("../data/rect3d.silo")

# Test 1 -- a slice through a point
AddPlot("Pseudocolor", "d")
AddOperator("Slice")
atts.originType = atts.Percent
atts.project2d = 0
atts.originPercent = 0
atts.axisType=atts.XAxis
SetOperatorOptions(atts)
DrawPlots()

view = GetView3D()
view.viewNormal = (0.565246, 0.332, 0.755)
view.focus = (0.5, 0.0, 0.5)
view.viewUp = (-0.08, 0.933, -0.35)
view.viewAngle = 30
view.parallelScale = 0.707
view.nearPlane = -1.41421
view.farPlane = 1.41421
view.imagePan = (0.1, -0.181)
view.imageZoom = 0.7
view.perspective = 1
SetView3D(view)

Test("ops_Slice20")

atts.originPercent = 100
SetOperatorOptions(atts)
Test("ops_Slice21")

atts.axisType=atts.YAxis
atts.originPercent = 0
SetOperatorOptions(atts)
Test("ops_Slice22")

atts.axisType=atts.YAxis
atts.originPercent = 100
SetOperatorOptions(atts)
Test("ops_Slice23")

atts.axisType=atts.ZAxis
atts.originPercent = 0
SetOperatorOptions(atts)
Test("ops_Slice24")

atts.axisType=atts.ZAxis
atts.originPercent = 100
SetOperatorOptions(atts)
Test("ops_Slice25")

# The X-axis for orthogonal slices is -1,0,0.  Try 1,0,0.
atts.axisType=atts.Arbitrary
atts.normal=(1,0,0)
atts.originPercent = 0
SetOperatorOptions(atts)
Test("ops_Slice26")

# The Y-axis for orthogonal slices is 0,-1,0.  Try 0,1,0.
atts.axisType=atts.Arbitrary
atts.normal=(0,1,0)
atts.originPercent = 0
SetOperatorOptions(atts)
Test("ops_Slice27")

DeleteAllPlots()

OpenDatabase("../data/noise.silo")
AddPlot("Pseudocolor", "PointVar")
pc_atts.pointSize = 1.0
SetPlotOptions(pc_atts)
DefineVectorExpression("disp", "{0,0,-coord(PointMesh)[2]}")
AddOperator("Displace")
disp = DisplaceAttributes()
disp.variable = "disp"
SetOperatorOptions(disp)
AddOperator("Slice")
slice = SliceAttributes()
slice.axisType = slice.ZAxis
SetOperatorOptions(slice)
DrawPlots()
Test("ops_Slice28")

# First, test slice by a zone with a boundary plot where the zone is along
# a material boundary.
DeleteAllPlots()
OpenDatabase("../data/rect3d.silo")
AddPlot("Boundary", "mat1")
AddOperator("Slice")
slice = SliceAttributes()
slice.originType = atts.Zone
slice.originZone = 10000
slice.originZoneDomain = 0
slice.axisType = slice.ZAxis
SetOperatorOptions(slice)
DrawPlots()
Test("ops_Slice29")

# Now, test slice by a zone where the zone is not along a material boundary.
slice.originZone = 9998
SetOperatorOptions(slice)
Test("ops_Slice30")

DeleteAllPlots()

# Test what happens when the up axis is not orthogonal to the normal.
# Start off with the two being orthogonal, though.
OpenDatabase("../data/noise.silo")
AddPlot("Pseudocolor", "hardyglobal")
AddOperator("Slice")
slice = SliceAttributes()
slice.normal = (1, -1, 0)
slice.upAxis = (0, 0, 1)
slice.project2d = 1
slice.originType = slice.Point
slice.originPoint = (0,0,0)
SetOperatorOptions(slice)
DrawPlots()
Test("ops_Slice31")

# Now make them be non-orthogonal.
slice.upAxis = (0,1,0)
SetOperatorOptions(slice)
Test("ops_Slice32")

# Open a multi time state database, create a pseudocolor plot slicing
# through a zone and then change the time state.
DeleteAllPlots()
ResetView()
OpenDatabase("../data/wave*.silo database")

AddPlot("Pseudocolor", "pressure")
AddOperator("Slice")
atts = SliceAttributes()
atts.project2d = 1
atts.axisType = atts.YAxis
atts.originType = atts.Zone
atts.originZone = 100
atts.originZoneDomain = 0
SetOperatorOptions(atts)
DrawPlots()

TimeSliderNextState()
Test("ops_Slice33")


DeleteAllPlots()
ResetView()
OpenDatabase("../data/multi_ucd3d.silo")
AddPlot("Pseudocolor", "d_dup")
AddOperator("Slice")
atts = SliceAttributes()
atts.project2d = 1
atts.axisType = atts.YAxis
atts.originType = atts.Zone
atts.originZone = 27
atts.originZoneDomain = 2 
atts.meshName = "mesh1"
SetOperatorOptions(atts)
DrawPlots()

Test("ops_Slice34")

DeleteAllPlots()
OpenDatabase("../data/curv3d.silo")
AddPlot("Pseudocolor", "d")
AddOperator("IndexSelect")
iatts = IndexSelectAttributes()
iatts.dim = iatts.ThreeD
iatts.xMin = 0
iatts.xMax = 1000
iatts.yMin = 10
iatts.yMax = 10
iatts.zMin = 0
iatts.zMax = 1000
iatts.xIncr = 1
iatts.yIncr = 1
iatts.zIncr = 1
SetOperatorOptions(iatts)
AddOperator("Slice")
DrawPlots()
ResetView()
Test("ops_Slice35")

Exit()
