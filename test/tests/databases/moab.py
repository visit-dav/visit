# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  moab.py
#
#  Tests:      mesh      - 3D unstructured,multi-domain, 
#              plots     - Pseudocolor, mesh
#
#  Programmer: Mark C Miller
#  Date:       August 10, 2016
# ----------------------------------------------------------------------------

OpenDatabase(data_path("moab_test_data/64bricks_12ktet.h5m"))

AddPlot("Mesh", "mesh")
AddPlot("Pseudocolor", "NODE_vertex_field")
DrawPlots()

v = GetView3D()
v.RotateAxis(0,45)
v.RotateAxis(1,135)
SetView3D(v)
Test("moab_01")

DeleteAllPlots()
AddPlot("Mesh", "mesh")
AddPlot("Subset", "Materials")
DrawPlots()
Test("moab_02")

DeleteAllPlots()
CloseDatabase(data_path("moab_test_data/64bricks_12ktet.h5m"))

OpenDatabase(data_path("moab_test_data/64bricks_512hex_256part.h5m"))
AddPlot("Mesh", "mesh")
DrawPlots()

v = GetView3D()
v.RotateAxis(0,45)
v.RotateAxis(1,135)
SetView3D(v)
Test("moab_03")

silr = SILRestriction()
for i in range(silr.NumSets()):
    if silr.SetName(i)[0:8] == "GeomSet_":
        silr.TurnOffSet(i)
    if   silr.SetName(i) == "GeomSet_427_dim_2":
        silr.TurnOnSet(i)
    elif silr.SetName(i) == "GeomSet_433_dim_2":
        silr.TurnOnSet(i)
    elif silr.SetName(i) == "GeomSet_438_dim_2":
        silr.TurnOnSet(i)
    elif silr.SetName(i) == "GeomSet_443_dim_2":
        silr.TurnOnSet(i)
SetPlotSILRestriction(silr)
Test("moab_04")

Exit()
