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
# ----------------------------------------------------------------------------

# Turn off all annotation
TurnOffAllAnnotations() # defines global object 'a'


OpenDatabase("../data/old_silo_data/pion0244.silo")
AddPlot("Pseudocolor","d")
AddPlot("Mesh","hydro_mesh")
DrawPlots()
Test("oldsilo_01")
DeleteAllPlots()

OpenDatabase("../data/old_silo_data/fwl.17217")
AddPlot("Pseudocolor","d")
AddPlot("Mesh","mesh1")
DrawPlots()
Test("oldsilo_02")
DeleteAllPlots()



Exit()
