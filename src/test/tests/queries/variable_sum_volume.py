# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  volume.py
#  Tests:      queries     - volumes of various material selected regions
#
#  Defect ID:  VisIt00006077
#
#  Programmer: Eddie Rusu
#  Date:       December 30, 2019
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

Exit()