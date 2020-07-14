# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  volume.py
#  Tests:      queries     - volumes of various material selected regions
#                          - weighted volume of dataset with cells of different
#                               dimension.
#
#  Programmer: Eddie Rusu
#  Date:       January 14, 2020
# ----------------------------------------------------------------------------

OpenDatabase(data_path("silo_pdb_test_data/ucd3d.silo"))
AddPlot("Pseudocolor", "mesh_quality/ucdmesh3d/volume", 1, 1)
DrawPlots()

SetQueryFloatFormat("%g")
# Material 1
TurnMaterialsOff(("4", "9"))
text = Query("Variable Sum")
TestText("mat_1", text)

# Material 4
TurnMaterialsOff("1")
TurnMaterialsOn("4")
text = Query("Variable Sum")
TestText("mat_4", text)

# Material 9
TurnMaterialsOff("4")
TurnMaterialsOn("9")
text = Query("Variable Sum")
TestText("mat_9", text)

# Material 14
TurnMaterialsOff("9")
TurnMaterialsOn(("1", "4"))
text = Query("Variable Sum")
TestText("mat_14", text)

# Material 19
TurnMaterialsOff("4")
TurnMaterialsOn("9")
text = Query("Variable Sum")
TestText("mat_19", text)

# Material 49
TurnMaterialsOff("1")
TurnMaterialsOn("4")
text = Query("Variable Sum")
TestText("mat_49", text)

# Material 149
TurnMaterialsOn()
text = Query("Variable Sum")
TestText("mat_149", text)

DeleteAllPlots()
CloseDatabase(data_path("silo_pdb_test_data/ucd3d.silo"))

# Weighted volume query on cells of different dimension
OpenDatabase(data_path("silo_pdb_test_data/multi_dim_cells.silo"))
AddPlot("Pseudocolor", "coordx", 1, 1)
DrawPlots()
text = Query("Weighted Variable Sum")
TestText("vws_mult_dim", text)

Exit()
