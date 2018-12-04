# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  avg_value.py
#  Tests:      queries     - average value 
#
#  Programmer: Hank Childs
#  Date:       May 11, 2011
#
# ----------------------------------------------------------------------------

TurnOffAllAnnotations()

OpenDatabase(silo_data_path("globe.silo"))

AddPlot("Pseudocolor", "t")
DrawPlots()
Query("Average Value")
t = GetQueryOutputString()
TestText("avg_value_01", t)

DeleteAllPlots()
AddPlot("Vector", "vel")
DrawPlots()
Query("Average Value")
t = GetQueryOutputString()
TestText("avg_value_02", t)

DefineVectorExpression("vel2", "{1, t, 0}")
ChangeActivePlotsVar("vel2")
Query("Average Value")
t = GetQueryOutputString()
TestText("avg_value_03", t)

OpenDatabase(silo_data_path("wave.visit"))

AddPlot("Pseudocolor", "pressure")
DrawPlots()
Query("Average Value")
t = GetQueryOutputString()
TestText("avg_value_04", t)

QueryOverTime("Average Value")
SetActiveWindow(2)
TurnOffAllAnnotations()
Test("avg_value_05")

Exit()
