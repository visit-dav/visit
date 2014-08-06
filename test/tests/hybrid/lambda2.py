# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  lambda2.py
#
#  Tests:#     plots   - pseudocolor, contour
#  Defect ID:  1829
#
#  Programmer: Kevin Griffin
#  Date:       Tue Aug 5 15:01:27 PDT 2014
#
#  Modifications:
#
# ----------------------------------------------------------------------------
ds = data_path("miranda_test_data/TG_vortex/plot.raw")
OpenDatabase(ds)
DefineScalarExpression("lambda2", "lambda2(gradient(velocity[0]), gradient(velocity[1]), gradient(velocity[2]))")
TimeSliderNextState()
TimeSliderNextState()

#
# Test 1
#
AddPlot("Pseudocolor", "lambda2")
AddOperator("Isovolume")
IsovolumeAtts = IsovolumeAttributes()
IsovolumeAtts.lbound = -999.99 
IsovolumeAtts.ubound = 0.0
IsovolumeAtts.variable = "default"
SetOperatorOptions(IsovolumeAtts)

DrawPlots()
Test("iso_lambda2")
DeleteAllPlots()

#
# Test 2
#
AddPlot("Contour", "lambda2")
DrawPlots()
Test("contour_lambda2")
DeleteAllPlots()

#
# Test 3
#
AddPlot("Pseudocolor", "lambda2")
DrawPlots()
Test("pseudo_lambda2")
DeleteAllPlots()

Exit()
