# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  error_quries.py
#  Tests:      queries     - zone center, node coords
#
#  Programmer: Eric Brugger
#  Date:       August 3, 2023
#
#  Modifications:
#
# ----------------------------------------------------------------------------

#
# Test with a single domain 2d curvilinear mesh
#
OpenDatabase(silo_data_path("curv2d.silo"))

AddPlot("Pseudocolor", "d")
DrawPlots()

Query("Zone Center", domain=0, element=10)
s = GetQueryOutputString()
v = GetQueryOutputValue()

TestValueEQ("Zone_Center_Curv2d_Valid_Str", s, "The center of  zone 10 is (0.923738, 1.81294).")
TestValueEQ("Zone_Center_Curv2d_Valid_Val", v, (0.9237379878759384, 1.8129377663135529))

Query("Zone Center", domain=0, element=1000000)
s = GetQueryOutputString()
v = GetQueryOutputValue()

TestValueEQ("Zone_Center_Curv2d_Invalid_Str", s, "The center of zone 1000000 could not be determined.")
TestValueEQ("Zone_Center_Curv2d_Invalid_Val", v, None)

Query("Node Coords", domain=0, element=10)
s = GetQueryOutputString()
v = GetQueryOutputValue()

TestValueEQ("Node_Coords_Curv2d_Valid_Str", s, "The coords of  node 10 are (1, 1.73205).")
TestValueEQ("None_Coords_Curv2d_Valid_Val", v, (1.0, 1.7320507764816284))

Query("Node Coords", domain=0, element=1000000)
s = GetQueryOutputString()
v = GetQueryOutputValue()

TestValueEQ("Node_Coords_Curv2d_Invalid_Str", s, "The coords of node 1000000 could not be determined.")
TestValueEQ("Node_Coords_Curv2d_Invalid_Val", v, None)

DeleteAllPlots()

#
# Test with a multi domain 3d unstructured mesh
#
OpenDatabase(silo_data_path("multi_ucd3d.silo"))

AddPlot("Pseudocolor", "d")
DrawPlots()

Query("Zone Center", domain=5, element=10)
s = GetQueryOutputString()
v = GetQueryOutputValue()

TestValueEQ("Zone_Center_Multi_Ucd3d_Valid_Str", s, "The center of zone 10 (domain 5) is (0.463913, 3.91959, 6.33333).")
TestValueEQ("Zone_Center_Multi_Ucd3d_Valid_Val", v, (0.4639131799340248, 3.9195945262908936, 6.333332777023315))

Query("Zone Center", domain=5, element=1000000)
s = GetQueryOutputString()
v = GetQueryOutputValue()

TestValueEQ("Zone_Center_Multi_Ucd3d_Invalid_Str", s, "The center of zone 1000000 (domain 5) could not be determined.")
TestValueEQ("Zone_Center_Multi_Ucd3d_Invalid_Val", v, None)

Query("Node Coords", domain=5, element=10)
s = GetQueryOutputString()
v = GetQueryOutputValue()

TestValueEQ("Node_Coords_Multi_Ucd3d_Valid_Str", s, "The coords of node 10 (domain 5) are (2.96558, 2.53285, 0).")
TestValueEQ("None_Coords_Multi_Ucd3d_Valid_Val", v, (2.9655816555023193, 2.532846450805664, 0.0))

Query("Node Coords", domain=5, element=1000000)
s = GetQueryOutputString()
v = GetQueryOutputValue()

TestValueEQ("Node_Coords_Multi_Ucd3d_Invalid_Str", s, "The coords of node 1000000 (domain 5) could not be determined.")
TestValueEQ("Node_Coords_Multi_Ucd3d_Invalid_Val", v, None)

Exit()
