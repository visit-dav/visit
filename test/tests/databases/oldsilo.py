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
a = AnnotationAttributes()
a.axesFlag2D = 0
a.axesFlag = 0
a.triadFlag = 0
a.bboxFlag = 0
a.userInfoFlag = 0
a.databaseInfoFlag = 0
a.legendInfoFlag = 0
a.backgroundMode = 0
a.foregroundColor = (0, 0, 0, 255)
a.backgroundColor = (255, 255, 255, 255)
SetAnnotationAttributes(a)


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
