# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  pixie.py 
#
#  Programmer: Mark C. Miller 
#  Date:       April 4, 2005 
#
#  Modifications:
#    Jeremy Meredith, Wed Sep  7 12:06:04 PDT 2005
#    Allowed spaces in variable names.
#
# ----------------------------------------------------------------------------


# test a 2D pixie database
OpenDatabase(data_path("pixie_test_data/pixie3d3.h5"))


#
# Test simple read and display of a variable 
#
AddPlot("Pseudocolor","Car variables/Bx")
DrawPlots()
Test("pixie_01")

AddPlot("Mesh","curvemesh_33x33x2")
DrawPlots()
Test("pixie_02")

# really zoom in on the center of this mesh
v=GetView2D()
v.windowCoords = (-5.84435e-05, 5.84435e-05, -5.12076e-05, 5.12076e-05)
SetView2D(v)
Test("pixie_03")

DeleteAllPlots()

# test a 3D pixie database
OpenDatabase(data_path("pixie_test_data/pixie3d4.h5"))

AddPlot("Mesh","curvemesh_33x33x33")
AddPlot("Contour","Car variables/Vx")
v=GetView3D()
v.viewNormal=(-0.5, 0, 0.866025)
SetView3D(v)
DrawPlots()
Test("pixie_04")
DeleteAllPlots()
ResetView()

# test a pixie expression (plot vector magnitude)
OpenDatabase(data_path("pixie_test_data/pixie_expressions.h5"))

# Test that vector data can be used. We use a Contour plot instead of a Vector
# plot now because the glyph indices for the Vector plot can change depending
# on how the mesh was automatically decomposed by the reader.
AddPlot("Contour","V_magnitude")
AddOperator("Transform")
t = TransformAttributes()
t.doScale = 1
t.scaleZ = 0.25
SetOperatorOptions(t)
DrawPlots()
v=GetView3D()
v.viewNormal = (0.557477, 0.525516, 0.642692)
v.focus = (0.484375, 0.5, 0.984375)
v.viewUp = (-0.252277, 0.844773, -0.471926)
v.viewAngle = 30
v.parallelScale = 1.22474
v.nearPlane = -2.44949
v.farPlane = 2.44949
v.imagePan = (0.0454993, 0.0670485)
v.imageZoom = 1
v.perspective = 1
v.eyeAngle = 2
v.centerOfRotationSet = 0
v.centerOfRotation = (0.484375, 0.5, 0.984375)
SetView3D(v)
Test("pixie_05")
DeleteAllPlots()

# Test a transparent plot to see how well ghosting works when the
# mesh gets divided up in parallel.
DefineScalarExpression("one", "point_constant(curvemesh_33x33x65, 1.)")
AddPlot("Pseudocolor", "one")
pc = PseudocolorAttributes()
pc.SetOpacityType(pc.Constant)
pc.opacity = 0.31
SetPlotOptions(pc)
AddOperator("Transform")
t = TransformAttributes()
t.doScale = 1
t.scaleZ = 0.25
SetOperatorOptions(t)
DrawPlots()
Test("pixie_06")
DeleteAllPlots()

# test a contour plot on double precision data
AddPlot("Contour","Car_variables/Vx")
v = GetView3D()
v.viewNormal = (0, 0, 1)
v.focus = (0.484375, 0.5, 3.9375)
v.viewUp = (0, 1, 0)
v.viewAngle = 30
v.parallelScale = 4.06202
v.nearPlane = -8.12404
v.farPlane = 8.12404
v.imagePan = (0, 0)
v.imageZoom = 20
v.perspective = 1
v.eyeAngle = 2
v.centerOfRotationSet = 0
v.centerOfRotation = (0.484375, 0.5, 3.9375)
SetView3D(v)
DrawPlots()
Test("pixie_07")
DeleteAllPlots()

# test a mesh plot of a non-square, 2D, curvilinear mesh 
OpenDatabase(data_path("pixie_test_data/pixie3d5.h5"))

AddPlot("Mesh","curvemesh_65x33x2")
AddPlot("Pseudocolor","Diagnostics/q factor")
DrawPlots()
v=GetView2D()
v.windowCoords=(-0.241119, 0.241119, -0.162714, 0.162714)
SetView2D(v)
Test("pixie_08")

Exit()
