# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  mir_cache.py
#
#  Tests:      mesh      - 3D curvilinear,multi-domain,ghost zones replicated.
#              plots     - volume, PC
#
#  Defect ID:  '3542
#
#  Programmer: Hank Childs
#  Date:       July 22, 2003
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

view = View3DAttributes()
view.viewNormal = (0.557976, 0.651128, 0.514485)
view.focus = (0.5, 0.5, 0.5)
view.viewUp = (-0.0955897, 0.666272, -0.739557)
view.viewAngle = 30
view.parallelScale = 0.866025
view.nearPlane = -1.73205
view.farPlane = 1.73205
view.perspective = 1
SetView3D(view)

#
# The volume plot does not need ghost zones, so this will create one
# set of MIRs.
#
OpenDatabase("../data/bigsil.silo")
AddPlot("Volume", "dist")
sil = SILRestriction()
sets = sil.SetsInCategory("mat")
sil.TurnOffAll()
sil.TurnOnSet(sets[0])
SetPlotSILRestriction(sil)
DrawPlots()

DeleteAllPlots()

#
# The PC plot will need ghost zones.  If it does not create a new set of MIRs,
# we will get garbled data.
#
AddPlot("Pseudocolor", "dist")
DrawPlots()

Test("mir_cache1")

#
# Make extra sure that there aren't garbled zones in the interior.
#
view.nearPlane = -0.3
SetView3D(view)
Test("mir_cache2")

Exit()


