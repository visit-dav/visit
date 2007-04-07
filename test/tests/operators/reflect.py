# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  reflect.py
#
#  Tests:      mesh      - 3D curvilinear, single domain,
#                          3D rectilinear, single domain.
#                          3D unstructured, single domain.
#              plots     - pc, vector, subset
#
#  Defect ID:  '5079, '5872, '6321
#
#  Programmer: Hank Childs
#  Date:       June 24, 2004
#
#  Modifications:
#
#    Hank Childs, Fri Aug 13 09:52:28 PDT 2004
#    Test ghost nodes along the reflection plane.
#
#    Hank Childs, Thu Aug 19 16:37:35 PDT 2004
#    Do not test ops_refl12 since that is done in reflect2.py (and will be
#    until SR mode plays well with transparency).
#
#    Jeremy Meredith, Thu Oct 21 11:56:05 PDT 2004
#    Reintegrated ops_refl12 since SR/transparency works now.
#
#    Hank Childs, Thu Jan 20 11:31:43 PST 2005
#    Test projecting then reflecting ['5872].
#
#    Hank Childs, Tue Jul  5 09:29:40 PDT 2005
#    Test that reflected plots are not degenerate by using the volume plot
#    ['6321].
#
# ----------------------------------------------------------------------------

# Turn off all annotation
a = AnnotationAttributes()
a.axesFlag2D = 0
a.axesFlag = 0
a.triadFlag = 0
a.bboxFlag = 0
a.userInfoFlag = 0
a.databaseInfoFlag = 0
a.legendInfoFlag = 0
a.backgroundMode = 0
a.foregroundColor = (0, 0, 0, 255)
a.backgroundColor = (255, 255, 255, 255)
SetAnnotationAttributes(a)

# 3D, rectilinear.  Multi-block  Tests ghost zones as well.
OpenDatabase("../data/rect2d.silo")

atts = ReflectAttributes()

AddPlot("Pseudocolor", "d")
AddOperator("Reflect")
atts.reflections = (1, 0, 1, 0, 0, 0, 0, 0)
SetOperatorOptions(atts)
DrawPlots()
Test("ops_refl01")

DeleteAllPlots()

AddPlot("Mesh", "quadmesh2d")
AddPlot("Subset", "mat1")
SetActivePlots((0,1))
AddOperator("Reflect")
DrawPlots()
atts.reflections = (0, 1, 1, 1, 0, 0, 0, 0)
atts.useXBoundary = 0
atts.specifiedX = -0.1
atts.useYBoundary = 0
atts.specifiedY = -0.05
SetOperatorOptions(atts)
Test("ops_refl02")

DeleteAllPlots()

OpenDatabase("../data/curv2d.silo")
AddPlot("Pseudocolor", "d")
AddOperator("Reflect")
DrawPlots()
Test("ops_refl03")

AddOperator("Isosurface")
Test("ops_refl04")

# Move the isosurface operator before the reflect.  This tests whether
# or not we can reflect polydata.
PromoteOperator(0)
DrawPlots()
Test("ops_refl05")
DeleteAllPlots()

# Test unstructured mesh -- plus test vectors.
OpenDatabase("../data/globe.silo")
AddPlot("Vector", "vel")
v = VectorAttributes()
v.vectorOrigin = v.Head
SetPlotOptions(v)

AddOperator("Reflect")
atts.reflections = (1, 0, 1, 0, 1, 0, 0, 1)
atts.useXBoundary = 1
atts.useYBoundary = 1
SetOperatorOptions(atts)
DrawPlots()

v = GetView3D()
v.viewNormal = (-0.324974, 0.839345, 0.435765)
v.focus = (-10, -10, -10)
v.viewUp = (-0.252067, 0.367233, -0.895322)
v.viewAngle = 30
v.parallelScale = 34.641
v.nearPlane = -69.282
v.farPlane = 69.282
v.imagePan = (0, 0)
v.imageZoom = 1.56244
v.perspective = 1
v.eyeAngle = 2
v.centerOfRotationSet = 0
v.centerOfRotation = (0, 0, 0)
SetView3D(v)

Test("ops_refl06")


# Now test a zonal vector variable (different code in the reflect operator).
ChangeActivePlotsVar("disp")
Test("ops_refl07")

disp_atts = DisplaceAttributes()
disp_atts.variable = "vel"
SetDefaultOperatorOptions(disp_atts)
AddOperator("Displace")
Test("ops_refl08")

PromoteOperator(0)
disp_atts.variable = "disp"
disp_atts.factor = 4
SetOperatorOptions(disp_atts)
ChangeActivePlotsVar("vel")
DrawPlots()
Test("ops_refl09")
DeleteAllPlots()


# Test that we properly put ghost nodes along the reflection plane.

# There is a lot of code dedicated to creating the ghost nodes for a 
# rectilinear grid.  Use the PXPYPZ and NXNYNZ octants to flex all of that 
# code.

OpenDatabase("../data/noise.silo")
AddPlot("Pseudocolor", "hardyglobal")
pc = PseudocolorAttributes()
pc.opacity = 0.3
SetPlotOptions(pc)

AddOperator("Reflect")
r = ReflectAttributes()
r.reflections = (1,1,1,1,1,1,1,1)
SetOperatorOptions(r)
DrawPlots()

v = GetView3D()
v.viewNormal = (-0.390471, 0.546441, 0.740901)
v.focus = (-10, -10, -10)
v.viewUp = (0.00925777, 0.807079, -0.590371)
v.viewAngle = 30
v.parallelScale = 34.641
v.nearPlane = -69.282
v.farPlane = 69.282
v.imagePan = (0, 0)
v.imageZoom = 1
v.perspective = 1
v.eyeAngle = 2
v.centerOfRotationSet = 0
v.centerOfRotation = (0, 0, 0)
SetView3D(v)

Test("ops_refl10")

r.octant = r.NXNYNZ
SetOperatorOptions(r)

v.viewNormal = (0.576101, 0.343493, 0.741701)
v.focus = (10, 10, 10)
v.viewUp = (-0.15522, 0.936877, -0.313318)
SetView3D(v)

Test("ops_refl11")


# Now make sure that this works well with an everyday vtkPointSet as well.
# Also test that we can handle the presence of both ghost nodes (from the
# reflect) and ghost zones (from the multi_ucd3d file).

DeleteAllPlots()
OpenDatabase("../data/multi_ucd3d.silo")
AddPlot("Pseudocolor", "d")
SetPlotOptions(pc)
AddOperator("Reflect")
r = ReflectAttributes()
r.reflections = (1,0,0,0,1,0,0,0)
SetOperatorOptions(r)
DrawPlots()

v.viewNormal = (0.328912, 0.896692, 0.296244)
v.focus = (0, 2.5, 20)
v.viewUp = (-0.710536, 0.441617, -0.547826)
v.viewAngle = 30
v.parallelScale = 20.7666
v.nearPlane = -41.5331
v.farPlane = 41.5331
v.imagePan = (-0.237006, -0.229008)
v.imageZoom = 1.25605
v.perspective = 1
v.eyeAngle = 2
v.centerOfRotationSet = 0
v.centerOfRotation = (0, 0, 0)
SetView3D(v)

Test("ops_refl12")

DeleteAllPlots()
OpenDatabase("../data/globe.silo")
AddPlot("Boundary", "mat1")
AddOperator("Project")
AddOperator("Reflect")
DrawPlots()
Test("ops_refl13")

# The "mass volume extractor" of the volume renderer depends on the
# rectilinear grid not being inverted.  Test that here ('6321).
DeleteAllPlots()
OpenDatabase("../data/rect3d.silo")
AddPlot("Volume", "d")
AddOperator("Reflect")
DrawPlots()
Test("ops_refl14")

Exit()
