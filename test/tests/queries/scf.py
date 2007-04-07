# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  scf.py
#  Tests:      queries     - spherical compactness factor
#
#  Defect ID:  VisIt00006385
#
#  Programmer: Hank Childs
#  Date:       July 14, 2005
#
#  Modifications:
#    Mark C. Miller, Fri Nov 17 22:03:34 PST 2006
#    Accounted for fact that threshold operator no longer has a default var
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

OpenDatabase("../data/rect2d.silo")
AddPlot("Pseudocolor", "d")
AddOperator("Isovolume")
iso_atts = IsovolumeAttributes()
iso_atts.ubound = 0.7
SetOperatorOptions(iso_atts)
DrawPlots()

Query("Spherical Compactness Factor")
text = GetQueryOutputString()
TestText("scf_01", text)

AddOperator("Revolve")
Query("Spherical Compactness Factor")
text = GetQueryOutputString()
TestText("scf_02", text)

DeleteAllPlots()

OpenDatabase("../data/wave.visit")
AddPlot("Pseudocolor", "pressure")
DrawPlots()
qota = GetQueryOverTimeAttributes()
qota.stride = 10
SetQueryOverTimeAttributes(qota)
QueryOverTime("Spherical Compactness Factor")

SetActiveWindow(2)
SetAnnotationAttributes(a)
Test("scf_03")

DeleteAllPlots()
OpenDatabase("../data/rect2d.silo")
AddPlot("Pseudocolor", "d")
i = ThresholdAttributes()
i.lowerBounds = (0.0)
i.upperBounds = (0.5)
i.listedVarNames = ("d")
SetDefaultOperatorOptions(i)
AddOperator("Threshold")
DrawPlots()
# Don't test localized compactness factor until radii is sorted out.
#Query("Localized Compactness Factor")
#t = GetQueryOutputString()
#TestText("scf_04", t)
Query("Elliptical Compactness Factor")
t = GetQueryOutputString()
TestText("scf_05", t)

AddOperator("Revolve")
#Query("Localized Compactness Factor")
#t = GetQueryOutputString()
#TestText("scf_06", t)
Query("Elliptical Compactness Factor")
t = GetQueryOutputString()
TestText("scf_07", t)

Exit()
