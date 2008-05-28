# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  indexselect.py
#
#  Tests:      mesh      - 3D curvilinear, single domain,
#                          3D rectilinear, single domain.
#                          2D rectilinear, single domain.
#                          3D rectilinear, single domain.
#              plots     - pc, mesh, subset, vector, surface
#              operators - index select (with threshold and sphere slice)
#
#  Defect ID:  none
#
#  Programmer: Hank Childs
#  Date:       October 2, 2002
#
#  Modifications:
#    Kathleen Bonnell, Thu Aug 28 14:34:57 PDT 2003
#    Remove compound var name from subset plots.
#
#    Kathleen Bonnell, Wed Sep  3 09:31:25 PDT 2003
#    Opaque mode for MeshPlot is ON by default now, so turn it off
#    for this test.
#
#    Kathleen Bonnell, Thu Sep  4 11:10:11 PDT 2003
#    Change the way MeshPlot's opaque mode is set (now an enum, not a bool).
#
#    Kathleen Bonnell, Wed Sep  8 11:29:10 PDT 2004 
#    Add two new tests, testing 'modulo' problems. ('3308).
#
#    Kathleen Bonnell, Wed Jul 20 13:32:44 PDT 2005 
#    Add two new tests, testing 'group' problems. ('6386).
#
#    Kathleen Bonnell, Wed Aug 10 10:13:56 PDT 2005 
#    Add tests 27-30 which tests extracting faces along maximum and minimum
#    boundaries of 3d data. ('6106).
#
#    Kathleen Bonnell, Fri Aug 19 15:45:46 PDT 2005 
#    Add tests 31-32 which tests domain boundaries w/i a group that coincide
#    with external boundaries of the index-selection. ('6428)  
#
#    Kathleen Bonnell, Mon Jan 30 16:40:30 PST 2006 
#    Add tests 33-38 which tests point meshes.
#
#    Kathleen Bonnell, Thu Jun  7 14:43:32 PDT 2007 
#    IndexSelect atts have changed, make corrections. 
#
# ----------------------------------------------------------------------------


OpenDatabase("../data/rect3d.silo")

atts = IndexSelectAttributes()
# Dim is an enumerated type.  0 -> 1D, 1 -> 2D, 2 -> 3D.
atts.dim = 2
atts.xMin = 5
atts.xMax = 23
atts.xIncr = 2
atts.yMin = 0
atts.yMax = 20
atts.yIncr = 1
atts.zMin = 10
atts.zMax = -1
atts.zIncr = 2

# 3D, rectilinear, zonal var.
AddPlot("Pseudocolor", "d")
AddOperator("IndexSelect")
SetOperatorOptions(atts)
DrawPlots()
view = GetView3D()
view.viewNormal = (-0.5, 0.5, 0.7172)
SetView3D(view)
Test("ops_indexselect01")
DeleteAllPlots()

# 3D, rectilinear, nodal var.
AddPlot("Pseudocolor", "u")
AddOperator("IndexSelect")
SetOperatorOptions(atts)
DrawPlots()
Test("ops_indexselect02")
DeleteAllPlots()

# 3D, rectilinear, mesh.
m = MeshAttributes()
m.opaqueMode = m.Off
SetDefaultPlotOptions(m)
AddPlot("Mesh", "quadmesh3d")
AddOperator("IndexSelect")
SetOperatorOptions(atts)
DrawPlots()
Test("ops_indexselect03")
DeleteAllPlots()

# 3D, rectilinear, subset.
AddPlot("Subset", "mat1")
AddOperator("IndexSelect")
atts.xIncr = 1
atts.yIncr = 1
atts.zIncr = 1
SetOperatorOptions(atts)
DrawPlots()
Test("ops_indexselect04")
DeleteAllPlots()

# 3D, rectilinear, after a threshold
AddPlot("Pseudocolor", "p")
a2 = ThresholdAttributes()
a2.listedVarNames = ("u")
a2.lowerBounds = (0.25)
a2.upperBounds = (1.0)
AddOperator("Threshold")
SetOperatorOptions(a2)
atts.zMin = 5
atts.zMax = 20
AddOperator("IndexSelect")
SetOperatorOptions(atts)
DrawPlots()
Test("ops_indexselect05")
DeleteAllPlots()

OpenDatabase("../data/curv3d.silo")

# 3D, curvilinear, mesh plot
AddPlot("Mesh", "curvmesh3d")
AddOperator("IndexSelect")
SetOperatorOptions(atts)
DrawPlots()
ResetView()
view = GetView3D()
view.viewNormal = (-0.5, 0.5, 0.7172)
SetView3D(view)
Test("ops_indexselect06")
DeleteAllPlots()

# 3D, curvilinear, PC plot
AddPlot("Pseudocolor", "d")
AddOperator("IndexSelect")
SetOperatorOptions(atts)
DrawPlots()
Test("ops_indexselect07")
DeleteAllPlots()

# 3D, curvilinear, with sphere slice
AddPlot("Pseudocolor", "d")
AddOperator("SphereSlice")
a3 = SphereSliceAttributes()
a3.origin = (2.5, -0.5, 13)
a3.radius = 7
SetOperatorOptions(a3)
AddOperator("IndexSelect")
atts.xMin = 0
atts.xMax = -1
atts.yMin = 0
atts.yMax = -1
atts.zMin = 5
atts.zMax = 18
SetOperatorOptions(atts)
DrawPlots()
Test("ops_indexselect08")
DeleteAllPlots()

OpenDatabase("../data/rect2d.silo")

# 2D, rectilinear, mesh plot
AddPlot("Mesh", "quadmesh2d")
AddOperator("IndexSelect")
# Dim is an enumerated type.  0 -> 1D, 1 -> 2D, 2 -> 3D.
atts.dim = 1
atts.xMin = 5
atts.xMax = 25
atts.yMin = 5
atts.yMax = 30
SetOperatorOptions(atts)
DrawPlots()
ResetView()
Test("ops_indexselect09")
DeleteAllPlots()

# 2D, rectilinear, PC plot
AddPlot("Pseudocolor", "d")
AddOperator("IndexSelect")
SetOperatorOptions(atts)
DrawPlots()
Test("ops_indexselect10")
DeleteAllPlots()

OpenDatabase("../data/curv2d.silo")

# 2D, curvilinear, mesh plot
AddPlot("Mesh", "curvmesh2d")
AddOperator("IndexSelect")
atts.yMax = 25
SetOperatorOptions(atts)
DrawPlots()
ResetView()
Test("ops_indexselect11")
DeleteAllPlots()

# 2D, curv, PC plot
AddPlot("Pseudocolor", "d")
AddOperator("IndexSelect")
SetOperatorOptions(atts)
DrawPlots()
Test("ops_indexselect12")
DeleteAllPlots()

# 2D, curv, with threshold.
AddPlot("Pseudocolor", "d")
AddOperator("Threshold")
a2.listedVarNames = ("p")
a2.lowerBounds = (0.34)
a2.upperBounds = (0.5)
SetOperatorOptions(a2)
AddOperator("IndexSelect")
atts.xMin = 10
atts.xMax = -1
atts.yMin = 0
atts.yMin = -1
SetOperatorOptions(atts)
DrawPlots()
Test("ops_indexselect13")
DeleteAllPlots()

# 2D, curv of vector
AddPlot("Vector", "vel")
AddOperator("IndexSelect")
SetOperatorOptions(atts)
DrawPlots()
Test("ops_indexselect14")
DeleteAllPlots()

# 2D surface of curvilinear
AddPlot("Surface", "d")
AddOperator("IndexSelect")
SetOperatorOptions(atts)
DrawPlots()
ResetView()
Test("ops_indexselect15")
DeleteAllPlots()

# 3D multi-rect 
# test 'modulo' problems, bug: VisIt00003308
OpenDatabase("../data/multi_rect3d.silo")
AddPlot("Pseudocolor", "d")
AddOperator("IndexSelect")
atts.dim = 2
atts.xMin = 0
atts.xMax = 1000
atts.yMin = 0
atts.yMax = 1000
atts.zMin = 0
atts.zMax = 1000
atts.xIncr = 3
atts.yIncr = 3
atts.zIncr = 3
SetOperatorOptions(atts)
DrawPlots()
ResetView()
Test("ops_indexselect16")

atts.xMin = 1
atts.xMax = 8
atts.yMin = 1
atts.yMax = 8
atts.zMin = 1
atts.zMax = 8
atts.xIncr = 5
atts.yIncr = 5
atts.zIncr = 5
SetOperatorOptions(atts)

Test("ops_indexselect17")
DeleteAllPlots()

# bigsil 
# test 'domainIndex' problems, bug: VisIt00005559
OpenDatabase("../data/bigsil.silo")
AddPlot("Subset", "domains")
AddPlot("Mesh", "mesh")
AddOperator("IndexSelect")
atts.dim = atts.ThreeD
atts.xMin = 0
atts.xMax = 1000
atts.yMin = 0
atts.yMax = 1000
atts.zMin = 0
atts.zMax = 1000
atts.xIncr = 1
atts.yIncr = 1
atts.zIncr = 1
atts.useWholeCollection = 0
atts.categoryName = "domains"
atts.subsetName = "domain2"
SetOperatorOptions(atts)
DrawPlots()
ResetView()

v = GetView3D()
v.viewNormal = (0.653901, -0.39603, -0.64465)
v.focus = (0.5, 0.5, 0.5)
v.viewUp = (-0.251385, -0.917378, 0.308584)
v.viewAngle = 30
v.parallelScale = 0.866025
v.nearPlane = -1.73205
v.farPlane = 1.73205
v.imagePan = (0, 0)
v.imageZoom = 1
v.perspective = 1
v.eyeAngle = 2
v.centerOfRotationSet = 0
v.centerOfRotation = (0, 0, 0)
SetView3D(v)

Test("ops_indexselect18")

# smallest domainIndex should be 1
atts.subsetName = "domain1" 
SetOperatorOptions(atts)
Test("ops_indexselect19")

atts.subsetName = "domain27"
SetOperatorOptions(atts)
Test("ops_indexselect20")

DeleteAllPlots()

#test whichData = OneGroup
AddPlot("Subset", "blocks")
s = SubsetAttributes()
s.wireframe = 1
s.lineWidth = 3
SetPlotOptions(s)
AddPlot("Mesh", "mesh")
AddPlot("Pseudocolor", "dist")
AddOperator("IndexSelect")
atts.dim = atts.ThreeD
atts.xMin = 1
atts.xMax = 2
atts.yMin = 1
atts.yMax = 2
atts.zMin = 0
atts.zMax = -1
atts.categoryName = "blocks"
atts.subsetName = "block0"
SetOperatorOptions(atts)

DrawPlots()
ResetView()
v = GetView3D()
v.viewNormal = (0, 0, -1)
v.focus = (0.5, 0.5, 0.5)
v.parallelScale = 0.866025
v.viewUp = (0, 1, 0)
v.nearPlane = -1.73205
v.farPlane  = 1.73205
v.perspective = 0
SetView3D(v)
v.imageZoom = 5.39496
SetView3D(v)
v.imagePan = (-0.23029, 0.259159)
SetView3D(v)
Test("ops_indexselect21")

atts.categoryName = "domains"
atts.subsetName = "domain1"
SetOperatorOptions(atts)
Test("ops_indexselect22")

SetActivePlots(1)
DeleteActivePlots()

SetActivePlots(1)
atts.categoryName = "blocks"
atts.subsetName = "block2"
atts.xMin = 0
atts.xMax = -1
atts.yMin = 5
atts.yMax = 19
atts.zMin = 7
atts.zMax = 8
SetOperatorOptions(atts)
ResetView()
Test("ops_indexselect23")

TurnMaterialsOff(("1", "5", "8"))
Test("ops_indexselect24")

TurnMaterialsOn()
TurnDomainsOff(("domain23"))
Test("ops_indexselect25")

TurnMaterialsOff(("1", "5", "8"))
Test("ops_indexselect26")

TurnMaterialsOn()
TurnDomainsOn()

DeleteAllPlots()

OpenDatabase("../data/rect3d.silo")
# 3D, rectilinear, zonal var, index-selected at max/min boundaries.
AddPlot("Pseudocolor", "d")
AddOperator("IndexSelect")
atts.dim = atts.ThreeD
atts.xMin = 0
atts.xMax = -1 # maximum
atts.yMin = 40 # maximum boundary
atts.yMax = 40 # maximum boundary
atts.zMin = 0
atts.zMax = -1
atts.xIncr = 1
atts.yIncr = 1
atts.zIncr = 1
atts.useWholeCollection = 1
SetOperatorOptions(atts)
pc = PseudocolorAttributes()
pc.limitsMode = pc.CurrentPlot
SetPlotOptions(pc)
DrawPlots()
ResetView()
view = GetView3D()
view.viewNormal = (0, 1, 0)
view.viewUp = (0, 0, -1)
view.perspective = 0
SetView3D(view)
Test("ops_indexselect27")

atts.yMin = 0
atts.yMax = 0
SetOperatorOptions(atts)
Test("ops_indexselect28")

DeleteAllPlots()


OpenDatabase("../data/curv3d.silo")
# 3D, curvilinear, zonal var, index-selected at max/min boundaries.
AddPlot("Pseudocolor", "p")
AddOperator("IndexSelect")
atts.dim = atts.ThreeD
atts.xMin = 30 
atts.xMax = 30 
atts.yMin = 0 
atts.yMax = -1 
atts.zMin = 0
atts.zMax = -1
atts.xIncr = 1
atts.yIncr = 1
atts.zIncr = 1
SetOperatorOptions(atts)
pc = PseudocolorAttributes()
pc.limitsMode = pc.CurrentPlot
SetPlotOptions(pc)
DrawPlots()
ResetView()
view = GetView3D()
view.viewNormal = (0, 1, 0)
view.viewUp = (0, 0, -1)
view.perspective = 0
SetView3D(view)
Test("ops_indexselect29")

atts.xMin = 0
atts.xMax = 0
SetOperatorOptions(atts)
Test("ops_indexselect30")

DeleteAllPlots()

#'6428, boundary between domains disappear when on edge of
# index-selected plot 
OpenDatabase("../data/bigsil.silo")
AddPlot("Pseudocolor", "dist")
atts.dim = atts.ThreeD
atts.xMin = 0
atts.xMax = -1 #max
atts.yMin = 8
atts.yMax = 17 
atts.zMin = 0
atts.zMax = -1 #max
atts.xIncr = 1
atts.yIncr = 1
atts.zIncr = 1
atts.useWholeCollection = 0
atts.categoryName = "blocks"
# middle group for this dataset, has group boundaries and internal
# domain boundaries
atts.subsetName = "block1" 

AddOperator("IndexSelect")
SetOperatorOptions(atts)
DrawPlots()
ResetView()
v = GetView3D()
v.viewNormal = (0.437142, -0.719538, -0.539603)
v.viewUp = (-0.309012, -0.683599, 0.661214)
v.perspective = 0
SetView3D(v)
Test("ops_indexselect31")

DeleteAllPlots()

# Filled boundary takes different path through IndexSelect than
# does PC, so test it too. 
AddPlot("FilledBoundary", "mat")
AddOperator("IndexSelect")
SetOperatorOptions(atts)
DrawPlots()
SetView3D(v)
Test("ops_indexselect32")
DeleteAllPlots()


# Test point meshes
TestSection("IndexSelect on Point meshes.")
OpenDatabase("../data/ugrid_points.vtk")
AddPlot("Pseudocolor", "u")
p = PseudocolorAttributes()
p.pointType = p.Box
p.pointSize = 0.2
SetPlotOptions(p)
DrawPlots()

v = GetView2D()
v.windowCoords = (-0.4, 9.4, -0.4, 9.4)
SetView2D(v)

Test("ops_indexselect33")

AddOperator("IndexSelect")
isa = IndexSelectAttributes()
isa.dim = isa.OneD
isa.xMin = 60
isa.xMax = 70
isa.xIncr = 1
SetOperatorOptions(isa)

Test("ops_indexselect34")

isa.xMin = 0
isa.xMax = -1
isa.xIncr = 2
SetOperatorOptions(isa)

Test("ops_indexselect35")

isa.xIncr = 3
SetOperatorOptions(isa)
Test("ops_indexselect36")

isa.xMin = 4
isa.xMax = -1
isa.xIncr = 2
SetOperatorOptions(isa)

Test("ops_indexselect37")

isa.xMin = 10
isa.xMax = 89 
isa.xIncr = 7
SetOperatorOptions(isa)

Test("ops_indexselect38")

DeleteAllPlots()

Exit()
