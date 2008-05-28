# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  simplify_mixed.py
#
#  Tests:      plots     - filled boundary
#
#  Defect ID:  '4363, '6464, '6504, '8082
#
#  Programmer: Hank Childs
#  Date:       August 19, 2005
#
#  Modifications:
# 
#    Hank Childs, Fri Sep 28 12:48:54 PDT 2007
#    Add testing for mixvars with simplify heavily mixed ['8082].
#
# ----------------------------------------------------------------------------



OpenDatabase("../data/boxlib_test_data/2D/plt0822/Header")

AddPlot("FilledBoundary", "materials")
DrawPlots()

v = GetView2D()
v.windowCoords = (0.0084, 0.0215, 0.0920, 0.1034)
v.viewportCoords = (0.2, 0.95, 0.15, 0.95)
SetView2D(v)

Test("simplify_mixed01")

m = MaterialAttributes()
m.simplifyHeavilyMixedZones = 1
m.maxMaterialsPerZone = 2
SetMaterialAttributes(m)

AddPlot("Boundary", "materials")
b = BoundaryAttributes()
b.colorType = b.ColorBySingleColor
SetPlotOptions(b)

DrawPlots()

Test("simplify_mixed02")

DeleteAllPlots()
m.maxMaterialsPerZone = 1
SetMaterialAttributes(m)

AddPlot("FilledBoundary", "materials")
DrawPlots()

Test("simplify_mixed03")

DeleteAllPlots()

OpenDatabase("../data/thinplane.silo")

m.forceMIR = 1
SetMaterialAttributes(m)
AddPlot("Pseudocolor", "den")
p = PseudocolorAttributes()
p.limitsMode = p.CurrentPlot
SetPlotOptions(p)
DrawPlots()
Test("simplify_mixed04")

m.maxMaterialsPerZone = 2
SetMaterialAttributes(m)
ReOpenDatabase("../data/thinplane.silo")
Test("simplify_mixed05")

m.maxMaterialsPerZone = 3
SetMaterialAttributes(m)
ReOpenDatabase("../data/thinplane.silo")
Test("simplify_mixed06")

Exit()
