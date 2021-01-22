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
Query("Variable Sum")
TestValueEQ("mat_1", GetQueryOutputValue(), 255.762, rndprec=3)

# Material 4
TurnMaterialsOff("1")
TurnMaterialsOn("4")
Query("Variable Sum")
TestValueEQ("mat_4", GetQueryOutputValue(), 224.805, rndprec=3)

# Material 9
TurnMaterialsOff("4")
TurnMaterialsOn("9")
Query("Variable Sum")
TestValueEQ("mat_9", GetQueryOutputValue(), 176.458, rndprec=3)

# Material 14
TurnMaterialsOff("9")
TurnMaterialsOn(("1", "4"))
Query("Variable Sum")
TestValueEQ("mat_14", GetQueryOutputValue(), 480.567, rndprec=3)

# Material 19
TurnMaterialsOff("4")
TurnMaterialsOn("9")
Query("Variable Sum")
TestValueEQ("mat_19", GetQueryOutputValue(), 432.22, rndprec=3)

# Material 49
TurnMaterialsOff("1")
TurnMaterialsOn("4")
Query("Variable Sum")
TestValueEQ("mat_49", GetQueryOutputValue(), 401.263, rndprec=3)

# Material 149
TurnMaterialsOn()
Query("Variable Sum")
TestValueEQ("mat_149", GetQueryOutputValue(), 657.025, rndprec=3)

DeleteAllPlots()
CloseDatabase(data_path("silo_pdb_test_data/ucd3d.silo"))

# Weighted volume query on cells of different dimension
OpenDatabase(data_path("silo_pdb_test_data/multi_dim_cells.silo"))
AddPlot("Pseudocolor", "coordx", 1, 1)
DrawPlots()
Query("Weighted Variable Sum")
TestValueEQ("vws_mult_dim", GetQueryOutputValue(), 4.5, rndprec=3)

Exit()
