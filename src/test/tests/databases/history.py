# ----------------------------------------------------------------------------
#  MODES: serial
#  CLASSES: nightly
#
#  Test Case:  history.py 
#
#  Tests:      variables that only have values for real zones (not ghost zones)
#              Some history variables have this property.
#
#  Programmer: Hank Childs
#  Date:       January 9, 2004
#
#  Modifications:
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
# ----------------------------------------------------------------------------

OpenDatabase(silo_data_path("multi_ucd3d.silo"))


#
# Test that we can detect that there is a problem and add 0's.
#
AddPlot("Pseudocolor","hist")
DrawPlots()

v=GetView3D()
v.viewNormal=(-0.35, -0.47, -0.8)
SetView3D(v)
Test("history_01")

#
# Doing material selection flexes the code in a different way (it confirms
# that we can detect the problem immediately upon reading it from the disk).
# Test that that works correctly as well.
#

sil = SILRestriction()
sets = sil.SetsInCategory("mat1")
sil.TurnOffSet(sets[0])
sil.TurnOffSet(sets[2])
SetPlotSILRestriction(sil)
Test("history_02")

Exit()
