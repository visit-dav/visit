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

# Turn off all annotation
a = AnnotationAttributes()
a.axesFlag2D = 0
a.axesFlag = 0
a.triadFlag = 0
a.bboxFlag = 0
a.userInfoFlag = 0
a.databaseInfoFlag = 0
a.legendInfoFlag = 0
a.backgroundMode = 0
a.foregroundColor = (0, 0, 0, 255)
a.backgroundColor = (255, 255, 255, 255)
SetAnnotationAttributes(a)

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
