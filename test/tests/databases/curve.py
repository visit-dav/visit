# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  curve.py 
#
#  Tests:      normal curves, zone-centered curves, errors in curve files
#
#  Programmer: Mark C. Miller 
#  Date:       October 31, 2006 
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

ca = CurveAttributes()
ca.showPoints = 1
SetDefaultPlotOptions(ca)

OpenDatabase("../data/zonecent.ultra")
AddPlot("Curve","curve2")
DrawPlots()
v = GetViewCurve()
v.rangeCoords = (-3, 11)
SetViewCurve(v)
Test("curve1")

AddPlot("Curve","curve1")
DrawPlots()
v.domainCoords = (-4, 16)
SetViewCurve(v)
Test("curve2")

DeleteActivePlots()

AddPlot("Curve","curve4")
DrawPlots()
v.domainCoords = (-4, 25)
SetViewCurve(v)
Test("curve3")

Exit()
