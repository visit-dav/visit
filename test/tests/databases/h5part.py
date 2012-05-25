# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  h5part.py
#
#  Programmer: Gunther Weber 
#  Date:       January, 2009 
#
#  Modifications:
#    Mark C. Miller, Wed Jan 21 09:36:13 PST 2009
#    Took Gunther's original code and integrated it with test suite.
#
# ----------------------------------------------------------------------------
TurnOffAllAnnotations()

OpenDatabase(data_path("h5part_test_data/sample.h5part"), 0)

AddPlot("Pseudocolor", "GaussianField", 1, 0)
DrawPlots()
Test("h5part_01")

ChangeActivePlotsVar("LinearField")
View3DAtts = GetView3D() 
View3DAtts.viewNormal = (1.000000, 0.000000, 0.0000000)
View3DAtts.focus = (31.5, 31.5, 31.5)
View3DAtts.viewUp = (0.000000, 1.000000, 0.0000000)
View3DAtts.viewAngle = 30
View3DAtts.parallelScale = 54.5596
View3DAtts.nearPlane = -109.119
View3DAtts.farPlane = 109.119
View3DAtts.imagePan = (0, 0)
View3DAtts.imageZoom = 1
View3DAtts.perspective = 1
View3DAtts.eyeAngle = 2
View3DAtts.centerOfRotationSet = 0
View3DAtts.centerOfRotation = (31.5, 31.5, 31.5)
SetView3D(View3DAtts)
Test("h5part_02")

DeleteActivePlots()
AddPlot("Pseudocolor", "px", 1, 0)
PseudocolorAtts = PseudocolorAttributes()
PseudocolorAtts.pointType = PseudocolorAtts.Sphere
PseudocolorAtts.pointSizePixels = 5
SetPlotOptions(PseudocolorAtts)
DrawPlots()
Test("h5part_03")

AddPlot("Pseudocolor", "LinearField", 1, 0)
AddOperator("Slice", 0)
SliceAtts = SliceAttributes()
SliceAtts.originType = SliceAtts.Intercept
SliceAtts.originIntercept = 30
SliceAtts.axisType = SliceAtts.XAxis
SliceAtts.project2d = 0
SliceAtts.meshName = "particles"
SetOperatorOptions(SliceAtts)
DrawPlots()
Test("h5part_04")

Exit()
