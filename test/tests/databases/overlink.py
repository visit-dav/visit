# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  overlink.py 
#
#  Programmer: Mark C. Miller 
#  Date:       November 11, 2004 
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

OpenDatabase("../data/overlink_test_data/regrovl_qh_1000_10001_4/OvlTop.silo")

#
# This is an interesting mesh 
#
AddPlot("Mesh","MMESH")
DrawPlots()
v=GetView3D()
v.viewNormal = (0.530656, 0.558421, 0.637629)
v.viewUp = (-0.401835, 0.82812, -0.390828)
SetView3D(v)
Test("overlink_01")

AddPlot("Pseudocolor","nvar1")
DrawPlots()
Test("overlink_02")

DeleteAllPlots()

AddPlot("Subset","domains(MMESH)")
DrawPlots()
Test("overlink_03")

silr = SILRestriction()
for i in range(silr.NumSets()):
    if silr.SetName(i) == "domain1":
        silr.TurnOffSet(i)
    elif silr.SetName(i) == "domain4":
        silr.TurnOffSet(i)
SetPlotSILRestriction(silr)
Test("overlink_04")

Exit()
