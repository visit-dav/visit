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


ca = CurveAttributes()
ca.showPoints = 1
SetDefaultPlotOptions(ca)

OpenDatabase(data_path("curve_test_data/zonecent.ultra"))
DefineCurveExpression("curve2Expr","curve2+10")

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

DeleteActivePlots()

AddPlot("Curve","curve2")
AddPlot("Curve","curve2Expr")
ResetView()
DrawPlots()
Test("curve2Expr")

DeleteActivePlots()

Exit()
