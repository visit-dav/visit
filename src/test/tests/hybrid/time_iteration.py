# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  time_iteration.py
#
#  Defect ID:  None
#
#  Programmer: Hank Childs
#  Date:       August 27, 2010
# ----------------------------------------------------------------------------



OpenDatabase(silo_data_path("wave.visit"))



DefineScalarExpression("ave1", "average_over_time(pressure, 1, 10, 1)")
AddPlot("Pseudocolor", "ave1")
DrawPlots()
v = GetView3D()
v.viewNormal = (0.607716, 0.507406, 0.610918)
v.viewUp = (-0.342711, 0.861513, -0.374627)
SetView3D(v)
Test("time_iteration_01")

DeleteAllPlots()
DefineScalarExpression("ave2", "average_over_time(pressure, 10, 20, 1)")
AddPlot("Pseudocolor", "ave2")
DrawPlots()
Test("time_iteration_02")

DeleteAllPlots()
DefineScalarExpression("pressure_big", "ge(pressure, 0.8)")
DefineScalarExpression("first_time", "first_time_when_condition_is_true(pressure_big, 100, 1, 71, 1)")
AddPlot("Pseudocolor", "first_time")
DrawPlots()
Test("time_iteration_03")

Exit()
