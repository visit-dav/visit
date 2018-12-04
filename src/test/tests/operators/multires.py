# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  multires.py
#
#  Programmer: Tom Fogal
#  Date:       August 6, 2010
#
# ----------------------------------------------------------------------------

ds = data_path("Chombo_test_data/chombo.visit")
OpenDatabase(ds)

AddPlot("Pseudocolor", "Scalar_1")
AddOperator("MultiresControl")

att = MultiresControlAttributes()
att.resolution = 0
SetOperatorOptions(att)

DrawPlots()
Test("multires_0")

att.resolution = 1
SetOperatorOptions(att)
DrawPlots()
Test("multires_1")

DeleteAllPlots()

CloseDatabase(ds)

Exit()
