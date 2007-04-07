# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  tensor.py
#
#  Tests:      tensor    - 3D rectilinear, single domain
#              plots     - tensor + some more exhibiting tensor functionality.
#
#  Defect ID:  '4165
#
#  Programmer: Hank Childs
#  Date:       December 1, 2003
#
#  Modifications:
#
#    Hank Childs, Fri May  7 08:12:41 PDT 2004
#    Re-enable test for decomposing tensors that now works.
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

OpenDatabase("../data/noise.silo")

AddPlot("Tensor", "grad_tensor")
t = TensorAttributes()
t.autoScale = 0
t.nTensors = 55
t.scale = 5
SetPlotOptions(t)
DrawPlots()

v = GetView3D()
v.viewNormal = (-0.312459, 0.334641, 0.889036)
v.focus = (0, 0, 0)
v.viewUp = (0.198376, 0.938247, -0.283443)
v.viewAngle = 30
v.parallelScale = 17.3205
v.nearPlane = -34.641
v.farPlane = 34.641
v.imagePan = (0, 0)
v.imageZoom = 1.18209
v.perspective = 1
v.eyeAngle = 2
SetView3D(v)

# Test the general tensor plot.
Test("tensor_01")

AddOperator("Slice")
a = SliceAttributes()
a.project2d = 0
a.normal = (0.667, 0.333, -0.667)
SetOperatorOptions(a)

# Test the interaction of the tensor plot with the slice operator.
Test("tensor_02")

RemoveLastOperator()

b = BoxAttributes()
b.minx = -2
b.maxx = 5
b.miny = -3
b.maxy = 0
b.minz = -8
b.maxz = -4
SetDefaultOperatorOptions(b)
AddOperator("Box")

t.nTensors = 20
SetPlotOptions(t)

# Test the interaction of the tensor plot with the box operator.
Test("tensor_03")

DeleteAllPlots()


# Test decomposing tensors and creating vectors.  The decomposition of
# the XX, YY, and ZZ components of the tensor is the standard gradient --
# so if we subtract the gradient from it, we should get all zeros.
DefineVectorExpression("major_comps", "{ grad_tensor[0][0], grad_tensor[1][1], grad_tensor[2][2] }")

DefineVectorExpression("diff", "major_comps-grad")
DefineScalarExpression("mag_diff", "magnitude(diff)")

AddPlot("Pseudocolor", "mag_diff")
DrawPlots()
Test("tensor_04")

DeleteAllPlots()


DefineScalarExpression("det", "determinant(grad_tensor)")
AddPlot("Pseudocolor", "det")
p = PseudocolorAttributes()
p.maxFlag = 1
p.max = 0.4
SetPlotOptions(p)
t = ThresholdAttributes()
t.listedVarNames = ("det")
t.lowerBounds = (0.0)
SetDefaultOperatorOptions(t)
AddOperator("Threshold")
DrawPlots()

v.viewNormal = (-0.568609, -0.669889, 0.477424)
v.focus = (0, 0, 0)
v.viewUp = (0.792201, -0.289591, 0.537172)
v.viewAngle = 30
v.parallelScale = 17.3205
v.nearPlane = -34.641
v.farPlane = 34.641
v.imagePan = (0, 0)
v.imageZoom = 1
v.perspective = 1
v.eyeAngle = 2

Test("tensor_05")

DeleteAllPlots()
DefineScalarExpression("ident", "determinant(grad_tensor*inverse(grad_tensor))")
AddPlot("Pseudocolor", "ident")
DrawPlots()

Test("tensor_06")

DeleteAllPlots()
DefineScalarExpression("trace", "trace(grad_tensor)")
AddPlot("Contour", "trace")
DrawPlots()

Test("tensor_07")

DeleteAllPlots()
DefineVectorExpression("ev", "eigenvalue(grad_tensor)")
AddPlot("Vector", "ev")
v = VectorAttributes()
v.autoScale = 0
v.scale = 6
v.nVectors = 55
SetPlotOptions(v)
DrawPlots()

Test("tensor_08")




Exit()
