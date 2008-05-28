# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  xform_precision.py 
#
#  Tests:      Transform manager's conversion to float 
#
#  Programmer: Mark C. Miller
#  Date:       September 24, 2006 
#
# ----------------------------------------------------------------------------


OpenDatabase("../data/quad_disk.silo")

#
# Turn off force single precision for this test
#
readOptions=GetDefaultFileOpenOptions("Silo")
readOptions["Force Single"] = 0
SetDefaultFileOpenOptions("Silo", readOptions)

#
# Test ordinary float data (no conversion) first
#
AddPlot("Mesh","mesh")
DrawPlots()
Test("float_xform_01")
DeleteAllPlots()

#
# Ok, now read a mesh with double coords
#
AddPlot("Mesh","meshD")
DrawPlots()
Test("float_xform_02")
DeleteAllPlots()

CloseDatabase("../data/quad_disk.silo")
OpenDatabase("../data/quad_disk.silo")

#
# test float data on a float mesh
#
AddPlot("Pseudocolor","sphElev_on_mesh")
DrawPlots()
Test("float_xform_03")
DeleteAllPlots()

#
# test float data on a double mesh
#
AddPlot("Pseudocolor","sphElev_on_meshD")
DrawPlots()
Test("float_xform_04")
DeleteAllPlots()

#
# test double data on a float mesh
#
AddPlot("Pseudocolor","sphElevD_on_mesh")
DrawPlots()
Test("float_xform_05")
DeleteAllPlots()

CloseDatabase("../data/quad_disk.silo")
OpenDatabase("../data/quad_disk.silo")

#
# test double data on a double mesh
#
AddPlot("Pseudocolor","sphElevD_on_meshD")
DrawPlots()
Test("float_xform_06")
DeleteAllPlots()

Exit()
