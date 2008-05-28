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
# ----------------------------------------------------------------------------


OpenDatabase("../data/bigsil.silo")
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
OpenDatabase("../data/multi_ucd3d.silo")
AddPlot("Pseudocolor", "d")
AddOperator("InverseGhostZone")
DrawPlots()
Test("ops_inverse_ghost_zone03")

sil = SILRestriction()
sil.TurnOffSet(1)
SetPlotSILRestriction(sil)
Test("ops_inverse_ghost_zone04")

Exit()
