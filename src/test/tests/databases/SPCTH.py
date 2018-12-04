# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  SPCTH.py 
#
#  Programmer: Kevin Griffin 
#  Date:       September 15, 2017 
#
#  Modifications:
#
# ----------------------------------------------------------------------------

#efp2d.spcth
OpenDatabase(data_path("SPCTH_test_data/efp2d.spcth"))
AddPlot("FilledBoundary", "materials")
DrawPlots()
Test("spcth_01")
DeleteAllPlots()
CloseDatabase(data_path("SPCTH_test_data/efp2d.spcth"))

#efp3d.spcth
OpenDatabase(data_path("SPCTH_test_data/efp3d.spcth"))
AddPlot("Pseudocolor", "M")

silr = SILRestriction()
silr.TurnOnAll()
silr.TurnOffSet(silr.SetsInCategory('materials')[3])
SetPlotSILRestriction(silr)

DrawPlots()
Test("spcth_02")
DeleteAllPlots()
CloseDatabase(data_path("SPCTH_test_data/efp3d.spcth"))

Exit()
