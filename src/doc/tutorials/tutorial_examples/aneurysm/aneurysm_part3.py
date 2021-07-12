#
# file: aneurysm_part3.py
# info: Corresponds to actions taken in
# https://visit-sphinx-github-user-manual.readthedocs.io/en/develop/tutorials/Aneurysm.html
#

OpenDatabase("aneurysm.visit")
AddPlot("Mesh", "Mesh")
MeshAtts = MeshAttributes()
MeshAtts.opaqueColor = (204, 255, 255, 255)
MeshAtts.opacity = 0.25
SetPlotOptions(MeshAtts)

View3DAtts = View3DAttributes()
View3DAtts.viewNormal = (-1, 0, 0)
View3DAtts.focus = (3.89585, 3.99132, 4.90529)
View3DAtts.viewUp = (0, 1, 0)
View3DAtts.viewAngle = 30
View3DAtts.parallelScale = 2.05937
View3DAtts.nearPlane = -4.11875
View3DAtts.farPlane = 4.11875
View3DAtts.imagePan = (0, 0)
View3DAtts.imageZoom = 1
View3DAtts.perspective = 1
View3DAtts.eyeAngle = 2
View3DAtts.centerOfRotationSet = 0
View3DAtts.centerOfRotation = (3.89585, 3.99132, 4.90529)
View3DAtts.axis3DScaleFlag = 0
View3DAtts.axis3DScales = (1, 1, 1)
View3DAtts.shear = (0, 0, 1)
View3DAtts.windowValid = 1
SetView3D(View3DAtts)

AddPlot("Vector", "velocity", 1, 1)
VectorAtts = VectorAttributes()
VectorAtts.useStride = 1
VectorAtts.stride = 5
VectorAtts.lineStyle = VectorAtts.SOLID  # SOLID, DASH, DOT, DOTDASH
VectorAtts.lineWidth = 0
VectorAtts.scale = 0.5
VectorAtts.scaleByMagnitude = 1
VectorAtts.autoScale = 1
VectorAtts.headSize = 0.25
VectorAtts.headOn = 1
VectorAtts.colorByMag = 1
VectorAtts.useLegend = 1
VectorAtts.vectorColor = (0, 0, 0, 255)
VectorAtts.colorTableName = "Spectral"
VectorAtts.invertColorTable = 1
VectorAtts.lineStem = VectorAtts.Cylinder  # Cylinder, Line
VectorAtts.geometryQuality = VectorAtts.High  # Fast, High
SetPlotOptions(VectorAtts)

# We slice the mesh at Y=3 to display the velocity magnitude 
AddOperator("Slice")
SliceAtts = SliceAttributes()
SliceAtts.originType = SliceAtts.Intercept
SliceAtts.originIntercept = 3
SliceAtts.normal = (0, 1, 0)
SliceAtts.axisType = SliceAtts.Arbitrary
SliceAtts.project2d = 0
SetOperatorOptions(SliceAtts, 1)

AddPlot("Pseudocolor", "velocity_magnitude")
PseudocolorAtts = PseudocolorAttributes()
PseudocolorAtts.limitsMode = PseudocolorAtts.CurrentPlot  # OriginalData, CurrentPlot
SetPlotOptions(PseudocolorAtts)
AddOperator("Slice")
# apply same slice
SetOperatorOptions(SliceAtts, 1)

# zoom on the slice
View3DAtts = View3DAttributes()
View3DAtts.viewNormal = (-0.961998, 0.241545, 0.127339)
View3DAtts.focus = (3.89585, 3.99132, 4.90529)
View3DAtts.viewUp = (0.246041, 0.969041, 0.0206035)
View3DAtts.viewAngle = 30
View3DAtts.parallelScale = 2.05937
View3DAtts.nearPlane = -4.11875
View3DAtts.farPlane = 4.11875
View3DAtts.imagePan = (-0.145383, 0.242693)
View3DAtts.imageZoom = 6.60936
View3DAtts.perspective = 1
View3DAtts.eyeAngle = 2
View3DAtts.centerOfRotationSet = 0
View3DAtts.centerOfRotation = (3.89585, 3.99132, 4.90529)
View3DAtts.axis3DScaleFlag = 0
View3DAtts.axis3DScales = (1, 1, 1)
View3DAtts.shear = (0, 0, 1)
View3DAtts.windowValid = 1
SetView3D(View3DAtts)

DrawPlots()

# Creating a vector expression and using the Defer Expression operator

DefineVectorExpression("normals", "   cell_surface_normal(Mesh)")

SetActivePlots(1)
AddOperator("DeferExpression")
DeferExpressionAtts = DeferExpressionAttributes()
DeferExpressionAtts.exprs = ("normals")
SetOperatorOptions(DeferExpressionAtts, 1)
ChangeActivePlotsVar("normals")
DrawPlots()

# hide before calculating flux 
HideActivePlots()
# Calculating the flux on the slice 
SetActivePlots(2)
DeleteActivePlots()

AddPlot("Pseudocolor", "operators/Flux/Mesh", 1, 0)
FluxAtts = FluxAttributes()
FluxAtts.flowField = "velocity"
FluxAtts.weight = 0
FluxAtts.weightField = "default"
SetOperatorOptions(FluxAtts, 0)
AddOperator("Slice", 0)
SetOperatorOptions(SliceAtts, 0)
AddOperator("DeferExpression", 0)
DeferExpressionAtts = DeferExpressionAttributes()
DeferExpressionAtts.exprs = ("normals")
SetOperatorOptions(DeferExpressionAtts, 0)

# Move the Slice operator up, above the Flux operator 
DemoteOperator(1, 0)
# Move the Defer Expression operator up, above the Flux operator, and below the Slice
DemoteOperator(2, 0)
DrawPlots()

# Get the numerical value of the flux by query-ing for the Weighted Variable Sum
SetQueryFloatFormat("%g")
Query("Weighted Variable Sum")


