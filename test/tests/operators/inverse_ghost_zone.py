# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  inverse_ghost_zone.py
#
#  Defect ID:  '6724
#
#  Programmer: Hank Childs
#  Date:       March 3, 2006
#
#  Modifications:
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
# ----------------------------------------------------------------------------


OpenDatabase("../data/silo_%s_test_data/bigsil.silo"%SILO_MODE)
AddPlot("Pseudocolor", "dist")
# By default, we won't create ghost zones, because we don't need them
# for this case.  Do something that makes us need them.
pc = PseudocolorAttributes()
pc.centering = pc.Zonal
SetPlotOptions(pc)
AddOperator("InverseGhostZone")
DrawPlots()
Test("ops_inverse_ghost_zone01")

sil = SILRestriction()
sil.TurnOffSet(1)
SetPlotSILRestriction(sil)
Test("ops_inverse_ghost_zone02")

DeleteAllPlots()
OpenDatabase("../data/silo_%s_test_data/multi_ucd3d.silo"%SILO_MODE)
AddPlot("Pseudocolor", "d")
AddOperator("InverseGhostZone")
DrawPlots()
Test("ops_inverse_ghost_zone03")

sil = SILRestriction()
sil.TurnOffSet(1)
SetPlotSILRestriction(sil)
Test("ops_inverse_ghost_zone04")

Exit()
