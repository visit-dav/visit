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
# ----------------------------------------------------------------------------


# Turn off all annotation
a = AnnotationAttributes()
a.axesFlag2D = 0
a.axesFlag = 0
a.triadFlag = 0
a.bboxFlag = 0
a.userInfoFlag = 0
a.databaseInfoFlag = 1
a.backgroundMode = 0
a.foregroundColor = (0, 0, 0, 255)
a.backgroundColor = (255, 255, 255, 255)
SetAnnotationAttributes(a)

OpenDatabase("../data/multi_ucd3d.silo")

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
