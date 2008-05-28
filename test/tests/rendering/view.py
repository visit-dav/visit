# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  view.py
#
#  Tests:      mesh      - 2D structured, single domain 
#                        - 3D unstructured, multiple domains
#                        - 3D rectilinear, single domain
#                        - Curve
#              plots     - pseudocolor, curve, mesh
#              operators - none
#              selection - none
#
#  Defect ID:  VisIt00003915, VisIt00004247, VisIt00003398, VisIt00004896,
#              VisIt00005337
#
#  Programmer: Eric Brugger
#  Date:       November 18, 2003 
#
#  Modifications:
#    Eric Brugger, Fri Apr 23 08:54:22 PDT 2004
#    Add a test to make sure that we can set the 3d view before any plots
#    are created.  Add tests to make sure that we can reset and recenter
#    the view when no plots are present.
#
#    Eric Brugger, Thu May 13 10:26:02 PDT 2004
#    Add a test to make sure switching view extents type to actual results
#    in new plots being centered after deleting all the existing plots.
#
#    Eric Brugger, Mon May 24 11:21:04 PDT 2004
#    Add a test with small extents.
#
#    Eric Brugger, Tue May 25 15:17:18 PDT 2004
#    Modify the test for small extents, and add more small and large extents
#    tests.
#
#    Eric Brugger, Mon Sep 20 13:37:52 PDT 2004
#    Add several tests for combining 3d plots with degenerate extents.
#
#    Brad Whitlock, Thu Mar 15 11:31:26 PDT 2007
#    Test that the bindings for 2D,3D views can still interpolate.
#
# ----------------------------------------------------------------------------

# Turn off all annotation except the legend.
TurnOffAllAnnotations() # defines global object 'a'

# Test setting the 3d view before creating the first plot.
v = View3DAttributes()
v.imagePan = (0.2, 0.2)
v.imageZoom = 1.2
v.viewNormal = (-0.5, 0.707107, 0.5)
v.viewUp = (0.5, 0.707107, -0.5)
SetView3D(v)

OpenDatabase("../data/multi_ucd3d.silo")

AddPlot("Pseudocolor", "d")
DrawPlots()
Test("view_01")

DeleteAllPlots()

# Create a psuedocolor plot and test various degenerate 2d views.
TestSection("Test degenerate 2D views")
OpenDatabase("../data/curv2d.silo")

AddPlot("Pseudocolor", "d")
DrawPlots()

v = View2DAttributes()
v.viewportCoords = (0.2, 0.95, 0.15, 0.95)

v.windowCoords = (-4.68395, 4.89802, 2.5, 2.5)
SetView2D(v)
Test("view_02")

v.windowCoords = (0.5, 0.5, 0.216897, 4.925)
SetView2D(v)
Test("view_03")

v.windowCoords = (0.5, 0.5, 2.5, 2.5)
SetView2D(v)
Test("view_04")

v.windowCoords = (0, 0, 0, 0)
SetView2D(v)
Test("view_05")

DeleteAllPlots()

# Create several curve plots and test various degenerate curve views.
TestSection("Test degenerate curve views")
OpenDatabase("../data/c051.curve")

AddPlot("Curve", "flat")
AddPlot("Curve", "going_up")
AddPlot("Curve", "going_down")
AddPlot("Curve", "parabolic")
DrawPlots()

v = ViewCurveAttributes()
v.viewportCoords = (0.2, 0.95, 0.15, 0.95)

v.domainCoords = (0., 1.)
v.rangeCoords = (0.3, 0.3)
SetViewCurve(v)
Test("view_06")

v.domainCoords = (0.6, 0.6)
v.rangeCoords = (0., 1.)
SetViewCurve(v)
Test("view_07")

v.domainCoords = (0.6, 0.6)
v.rangeCoords = (0.3, 0.3)
SetViewCurve(v)
Test("view_08")

v.domainCoords = (0., 0.)
v.rangeCoords = (0., 0.)
SetViewCurve(v)
Test("view_09")

DeleteAllPlots()

# Create a pseudocolor and mesh plot and zoom in on a sharp edge to
# verify that the mesh lines are not bleeding through the surface.
TestSection("Test zoom in on mesh lines")
OpenDatabase("../data/multi_ucd3d.silo")

AddPlot("Pseudocolor", "u")
AddPlot("Mesh", "mesh1")
DrawPlots()

v = GetView3D()
v.viewNormal = (-0.335187, -0.557088, 0.759804)
v.focus = (0, 2.5, 10)
v.viewUp = (-0.327416, 0.825068, 0.460501)
v.viewAngle = 30
v.parallelScale = 11.4564
v.nearPlane = -22.9129
v.farPlane = 22.9129
v.imagePan = (-0.136785, -0.172112)
v.imageZoom = 20.125
v.perspective = 1
SetView3D(v)
Test("view_10")

# Test resetting and recentering the view when no plot is present.
DeleteAllPlots()

ResetView()

AddPlot("Pseudocolor", "u")
AddPlot("Mesh", "mesh1")
DrawPlots()
Test("view_11")

v.viewNormal = (0.393726, -0.416942, 0.819231)
v.focus = (0, 5., 10)
v.viewUp = (-0.603958, 0.554522, 0.572486)
v.viewAngle = 30
v.parallelScale = 11.4564
v.nearPlane = -22.9129
v.farPlane = 22.9129
v.imagePan = (0.05, 0.)
v.imageZoom = 2.
v.perspective = 1
SetView3D(v)
Test("view_12")

DeleteAllPlots()

RecenterView()

AddPlot("Pseudocolor", "u")
AddPlot("Mesh", "mesh1")
DrawPlots()
Test("view_13")

# Test that setting view extents type to actual works.  Test that
# switching view extents type to actual results in new plots being
# centered after deleting all the existing plots.
TestSection("Test actual extents")
DeleteAllPlots()

ResetView()

OpenDatabase("../data/curv2d.silo")
AddPlot("Pseudocolor", "u")
AddOperator("Clip")
DrawPlots()
SetViewExtentsType("actual")
Test("view_14")

DeleteAllPlots()

OpenDatabase("../data/rect2d.silo")
AddPlot("Pseudocolor", "p")
DrawPlots()
Test("view_15")

# Test setting the view with small extents.
DeleteAllPlots()

ResetView()

OpenDatabase("../data/globe.silo")
AddPlot("Pseudocolor", "u")
AddOperator("Transform")
scale = TransformAttributes()
scale.doScale = 1
scale.scaleX = 1.0e-21
scale.scaleY = 1.0e-21
scale.scaleZ = 1.0e-21
SetOperatorOptions(scale)
DrawPlots()
Test("view_16")

ChangeActivePlotsVar("t")
Test("view_17")

# Test setting the view with large extents.
DeleteAllPlots()

AddPlot("Pseudocolor", "u")
AddOperator("Transform")
scale = TransformAttributes()
scale.doScale = 1
scale.scaleX = 1.0e17
scale.scaleY = 1.0e17
scale.scaleZ = 1.0e17
SetOperatorOptions(scale)
DrawPlots()
Test("view_18")

ChangeActivePlotsVar("t")
Test("view_19")

# Test various combinations of 3d plots with degenerate extents.
TestSection("Test 3D plots with degenerate extents")
DeleteAllPlots()

ResetView()

OpenDatabase("../data/rect_flat_i.vtk")
AddPlot("Mesh", "mesh")
DrawPlots()
OpenDatabase("../data/rect_flat_j.vtk")
AddPlot("Mesh", "mesh")
DrawPlots()
Test("view_20")

DeleteAllPlots()

OpenDatabase("../data/rect_flat_j.vtk")
AddPlot("Mesh", "mesh")
DrawPlots()
OpenDatabase("../data/rect_flat_k.vtk")
AddPlot("Mesh", "mesh")
DrawPlots()
Test("view_21")

DeleteAllPlots()

OpenDatabase("../data/rect_flat_k.vtk")
AddPlot("Mesh", "mesh")
DrawPlots()
OpenDatabase("../data/rect_flat_i.vtk")
AddPlot("Mesh", "mesh")
DrawPlots()
Test("view_22")
DeleteAllPlots()


#
# Test that the View3DAttributes object's Python binding still has the
# ability to interpolate the view.
#
TestSection("Test interpolation of View3DAttributes")
OpenDatabase("../data/globe.silo")
AddPlot("Pseudocolor", "speed")
DrawPlots()

v0 = View3DAttributes()
v0.viewNormal = (0.416973, 0.447113, 0.791343)
v0.focus = (0, 0, 0)
v0.viewUp = (-0.23799, 0.893974, -0.379699)
v0.viewAngle = 30
v0.parallelScale = 17.3205
v0.nearPlane = -34.641
v0.farPlane = 34.641
v0.imagePan = (0, 0)
v0.imageZoom = 1
v0.perspective = 1
v0.eyeAngle = 2
v0.centerOfRotationSet = 0
v0.centerOfRotation = (0, 0, 0)

v1 = View3DAttributes()
v1.viewNormal = (0.416973, 0.447113, 0.791343)
v1.focus = (0, 0, 0)
v1.viewUp = (-0.23799, 0.893974, -0.379699)
v1.viewAngle = 30
v1.parallelScale = 17.3205
v1.nearPlane = -34.641
v1.farPlane = 34.641
v1.imagePan = (0.160832, 0.126504)
v1.imageZoom = 3.95662
v1.perspective = 1
v1.eyeAngle = 2
v1.centerOfRotationSet = 0
v1.centerOfRotation = (0, 0, 0)

idx = 23
nsteps = 5
for i in range(nsteps):
    t = float(i) / float(nsteps - 1)
    v = (1. - t) * v0 + t * v1
    SetView3D(v)
    Test("view_%d" % idx)
    idx = idx + 1
DeleteAllPlots()

#
# Test that the View2DAttributes object's Python binding still has the
# ability to interpolate the view.
#
TestSection("Test interpolation of View2DAttributes")
OpenDatabase("../data/shapefile_test_data/prism0p020/prism0p020.shp")
AddPlot("Pseudocolor", "RANGE")
DrawPlots()

v0 = View2DAttributes()
v0.windowCoords = (-124.756, -66.954, 24.5183, 49.3856)
v0.viewportCoords = (0.2, 0.95, 0.15, 0.95)
v0.fullFrameActivationMode = v0.Off  # On, Off, Auto
v0.fullFrameAutoThreshold = 100

v1 = View2DAttributes()
v1.windowCoords = (-77.667, -74.881, 36.6969, 39.662)
v1.viewportCoords = (0.2, 0.95, 0.15, 0.95)
v1.fullFrameActivationMode = v1.Off  # On, Off, Auto
v1.fullFrameAutoThreshold = 100

nsteps = 5
for i in range(nsteps):
    t = float(i) / float(nsteps - 1)
    v = (1. - t) * v0 + t * v1
    SetView2D(v)
    Test("view_%d" % idx)
    idx = idx + 1
DeleteAllPlots()

Exit()
