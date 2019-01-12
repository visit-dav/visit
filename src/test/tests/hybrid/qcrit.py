# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  qcrit.py
#
#  Tests:#     plots   - pseudocolor, contour
#  Defect ID:  1829
#
#  Programmer: Kevin Griffin
#  Date:       Thu Jul 31 14:21:02 PDT 2014
#
#  Modifications:
#
# ----------------------------------------------------------------------------
ds = data_path("miranda_test_data/TG_vortex/plot.raw")
#ds = "/g/g14/kgriffin/trunk/build_debug/data/miranda_test_data/TG_vortex/plot.raw"
OpenDatabase(ds)
DefineScalarExpression("qcrit", "q_criterion(gradient(velocity[0]), gradient(velocity[1]), gradient(velocity[2]))")
TimeSliderNextState()
TimeSliderNextState()

#
# Test 1
#
AddPlot("Pseudocolor", "qcrit")
AddOperator("Isovolume")
IsovolumeAtts = IsovolumeAttributes()
IsovolumeAtts.lbound = 0.2
IsovolumeAtts.ubound = 5.0
IsovolumeAtts.variable = "default"
SetOperatorOptions(IsovolumeAtts)

DrawPlots()
Test("iso_qcrit")
DeleteAllPlots()

#
# Test 2
#
AddPlot("Contour", "qcrit")
DrawPlots()
Test("contour_qcrit")
DeleteAllPlots()

#
# Test 3
#
AddPlot("Pseudocolor", "qcrit")
DrawPlots()
Test("pseudo_qcrit")
DeleteAllPlots()

Exit()
