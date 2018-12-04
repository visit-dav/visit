# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  tube.py
#
#  Programmer: Kathleen Biagas 
#  Date:       August 7, 2012
#
#  Modifications:
#
# ----------------------------------------------------------------------------

OpenDatabase(data_path("vtk_test_data/lines.vtk"))

AddPlot("Pseudocolor", "var1")
AddOperator("Tube")
tube = TubeAttributes()
tube.tubeRadiusType = tube.Absolute
tube.radiusAbsolute = 0.5
tube.fineness = 20
SetOperatorOptions(tube)

view = GetView3D()
view.viewNormal = (-0.429483, -0.499742, 0.752198)
view.focus = (0, 0, 0)
view.viewUp = (-0.296677, 0.864778, 0.405144)
view.viewAngle = 30
view.parallelScale = 8.66025
view.nearPlane = -17.3205
view.farPlane = 17.3205
SetView3D(view)

DrawPlots()

Test("tube_01")

tube.scaleByVarFlag = 1
SetOperatorOptions(tube)
Test("tube_02")

tube.scaleVariable = "var2"
SetOperatorOptions(tube)
Test("tube_03")

# get the default again
tube = TubeAttributes()

ChangeActivePlotsVar("var2")
tube.tubeRadiusType = tube.Absolute
tube.radiusAbsolute = 0.5
tube.fineness = 20
SetOperatorOptions(tube)
Test("tube_04")

tube.scaleByVarFlag = 1
SetOperatorOptions(tube)
Test("tube_05")

tube.scaleVariable = "var1"
SetOperatorOptions(tube)
Test("tube_06")

Exit()
