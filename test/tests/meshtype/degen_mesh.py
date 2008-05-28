# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  degenerate.py
#
#  Tests:      mesh      - 3D curvilinear, rectilinear
#              plots     - pc, contour, mesh
#              operators - none
#              selection - none
#
#  Defect ID:  '5333 & '5335
#
#  Programmer: Hank Childs
#  Date:       August 26, 2004
#
# ----------------------------------------------------------------------------

TurnOffAllAnnotations() # defines global object 'a'

# Define some expressions that will be handy for later.
DefineVectorExpression("coords_plus1", "coords(mesh)")
DefineVectorExpression("coords", "{coords_plus1[0]-1., coords_plus1[1]-1., coords_plus1[2]-1. }")
DefineVectorExpression("i_disp", "{ -sqrt(coords[1]*coords[1] + coords[2]*coords[2]), 0, 0 }")
DefineVectorExpression("j_disp", "{ 0, -sqrt(coords[0]*coords[0] + coords[2]*coords[2]), 0 }")
DefineVectorExpression("k_disp", "{ 0, 0, -sqrt(coords[0]*coords[0] + coords[1]*coords[1]) }")
DefineScalarExpression("pos", "sqrt(coords[0]*coords[0]+coords[1]*coords[1]+coords[2]*coords[2])")

#
# Start off by testing curvilinear meshes.
#

# Test that we can make the meshes with no modification.
OpenDatabase("../data/curv_flat_i.vtk")
AddPlot("Mesh", "mesh")
# If you do a DrawPlots now, you will exhibit '5337
OpenDatabase("../data/curv_flat_j.vtk")
AddPlot("Mesh", "mesh")
OpenDatabase("../data/curv_flat_k.vtk")
AddPlot("Mesh", "mesh")
DrawPlots()
Test("degen_mesh_01")

# Test that we can handle displacing them and operating on them with
# expressions.
disp = DisplaceAttributes()
disp.variable = "i_disp"
SetDefaultOperatorOptions(disp)
SetActivePlots(0)
AddOperator("Displace")
disp.variable = "j_disp"
SetDefaultOperatorOptions(disp)
SetActivePlots(1)
AddOperator("Displace")
disp.variable = "k_disp"
SetDefaultOperatorOptions(disp)
SetActivePlots(2)
AddOperator("Displace")

v= GetView3D()
v.viewNormal = (0.449864, 0.604932, 0.657023)
v.focus = (0.801178, 0.801178, 0.801178)
v.viewUp = (0.377875, 0.537649, -0.753753)
v.viewAngle = 30
v.parallelScale = 2.00714
v.nearPlane = -4.01428
v.farPlane = 4.01428
v.imagePan = (0, 0)
v.imageZoom = 0.873008
v.perspective = 1
v.eyeAngle = 2
v.centerOfRotationSet = 0
v.centerOfRotation = (0, 0, 0)
SetView3D(v)
Test("degen_mesh_02")

# Test that we can do PC plots ('5335)
ActivateDatabase("../data/curv_flat_i.vtk")
AddPlot("Pseudocolor", "pos")
disp = DisplaceAttributes()
disp.variable = "i_disp"
SetDefaultOperatorOptions(disp)
AddOperator("Displace")

ActivateDatabase("../data/curv_flat_j.vtk")
AddPlot("Pseudocolor", "pos")
disp.variable = "j_disp"
SetDefaultOperatorOptions(disp)
AddOperator("Displace")

ActivateDatabase("../data/curv_flat_k.vtk")
AddPlot("Pseudocolor", "pos")
disp.variable = "k_disp"
SetDefaultOperatorOptions(disp)
AddOperator("Displace")

DrawPlots()
Test("degen_mesh_03")

SetActivePlots(5)
DeleteActivePlots()
SetActivePlots(4)
DeleteActivePlots()
SetActivePlots(3)
DeleteActivePlots()


# Test that we can do contours ('5333)
ActivateDatabase("../data/curv_flat_i.vtk")
AddPlot("Contour", "pos")
disp = DisplaceAttributes()
disp.variable = "i_disp"
SetDefaultOperatorOptions(disp)
AddOperator("Displace")

ActivateDatabase("../data/curv_flat_j.vtk")
AddPlot("Contour", "pos")
disp.variable = "j_disp"
SetDefaultOperatorOptions(disp)
AddOperator("Displace")

ActivateDatabase("../data/curv_flat_k.vtk")
AddPlot("Contour", "pos")
disp.variable = "k_disp"
SetDefaultOperatorOptions(disp)
AddOperator("Displace")

DrawPlots()
Test("degen_mesh_04")


#
# Now do the same thing over again with rectilinear meshes.
#
DeleteAllPlots()

# Test that we can make the meshes with no modification.
OpenDatabase("../data/rect_flat_i.vtk")
AddPlot("Mesh", "mesh")
# If you do a DrawPlots now, you will exhibit '5337
OpenDatabase("../data/rect_flat_j.vtk")
AddPlot("Mesh", "mesh")
OpenDatabase("../data/rect_flat_k.vtk")
AddPlot("Mesh", "mesh")
DrawPlots()
Test("degen_mesh_05")

# Test that we can handle displacing them and operating on them with
# expressions.
disp = DisplaceAttributes()
disp.variable = "i_disp"
SetDefaultOperatorOptions(disp)
SetActivePlots(0)
AddOperator("Displace")
disp.variable = "j_disp"
SetDefaultOperatorOptions(disp)
SetActivePlots(1)
AddOperator("Displace")
disp.variable = "k_disp"
SetDefaultOperatorOptions(disp)
SetActivePlots(2)
AddOperator("Displace")

v= GetView3D()
v.viewNormal = (0.449864, 0.604932, 0.657023)
v.focus = (0.801178, 0.801178, 0.801178)
v.viewUp = (0.377875, 0.537649, -0.753753)
v.viewAngle = 30
v.parallelScale = 2.00714
v.nearPlane = -4.01428
v.farPlane = 4.01428
v.imagePan = (0, 0)
v.imageZoom = 0.873008
v.perspective = 1
v.eyeAngle = 2
v.centerOfRotationSet = 0
v.centerOfRotation = (0, 0, 0)
SetView3D(v)
Test("degen_mesh_06")

# Test that we can do PC plots ('5335)
ActivateDatabase("../data/rect_flat_i.vtk")
AddPlot("Pseudocolor", "pos")
disp = DisplaceAttributes()
disp.variable = "i_disp"
SetDefaultOperatorOptions(disp)
AddOperator("Displace")

ActivateDatabase("../data/rect_flat_j.vtk")
AddPlot("Pseudocolor", "pos")
disp.variable = "j_disp"
SetDefaultOperatorOptions(disp)
AddOperator("Displace")

ActivateDatabase("../data/rect_flat_k.vtk")
AddPlot("Pseudocolor", "pos")
disp.variable = "k_disp"
SetDefaultOperatorOptions(disp)
AddOperator("Displace")

DrawPlots()
Test("degen_mesh_07")

SetActivePlots(5)
DeleteActivePlots()
SetActivePlots(4)
DeleteActivePlots()
SetActivePlots(3)
DeleteActivePlots()


# Test that we can do contours ('5333)
ActivateDatabase("../data/curv_flat_i.vtk")
AddPlot("Contour", "pos")
disp = DisplaceAttributes()
disp.variable = "i_disp"
SetDefaultOperatorOptions(disp)
AddOperator("Displace")

ActivateDatabase("../data/curv_flat_j.vtk")
AddPlot("Contour", "pos")
disp.variable = "j_disp"
SetDefaultOperatorOptions(disp)
AddOperator("Displace")

ActivateDatabase("../data/curv_flat_k.vtk")
AddPlot("Contour", "pos")
disp.variable = "k_disp"
SetDefaultOperatorOptions(disp)
AddOperator("Displace")

DrawPlots()
Test("degen_mesh_08")


Exit()
