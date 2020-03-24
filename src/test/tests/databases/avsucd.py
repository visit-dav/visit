# ----------------------------------------------------------------------------)
#  CLASSES: nightly
#
#  Test Case:  avsucd.py 
#
#  Programmer: Kathleen Biagas 
#  Date:       February 6, 2020
#
# ----------------------------------------------------------------------------

OpenDatabase(data_path("AVSucd_test_data/cell_data.inp"))
AddPlot("Pseudocolor", "stress sx")
DrawPlots()

v = GetView3D()
v.viewNormal = (0.804006, 0.580568, 0.128511)
viewUp = (-0.482246, 0.763086, -0.430278)
SetView3D(v)
Test("avsucd_01")

ChangeActivePlotsVar("stress sz")
Test("avsucd_02")

ChangeActivePlotsVar("temp1")
Test("avsucd_03")

DeleteAllPlots()
CloseDatabase(data_path("AVSucd_test_data/cell_data.inp"))

# file with vectors
OpenDatabase(data_path("AVSucd_test_data/vec1.1.000.inp"))
AddPlot("Vector", "vect")
DrawPlots()
ResetView()
Test("avsucd_04")

DeleteAllPlots()
CloseDatabase(data_path("AVSucd_test_data/vec1.1.000.inp"))

Exit()
