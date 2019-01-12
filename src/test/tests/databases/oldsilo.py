# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  oldsilo.py 
#
#  Tests:      old-style silo files (netcdf driver)
#
#  Programmer: Mark C. Miller 
#  Date:       February 11, 2004 
#
#  Modifications:
#    Jeremy Meredith, Thu Jan 14 12:38:24 EST 2010
#    Changed path to point to new location.
#
# ----------------------------------------------------------------------------



OpenDatabase(data_path("old_silo_test_data/pion0244.silo"))

AddPlot("Pseudocolor","d")
AddPlot("Mesh","hydro_mesh")
DrawPlots()
Test("oldsilo_01")
DeleteAllPlots()

OpenDatabase(data_path("old_silo_test_data/fwl.17217"))

AddPlot("Pseudocolor","d")
AddPlot("Mesh","mesh1")
DrawPlots()
Test("oldsilo_02")
DeleteAllPlots()



Exit()
