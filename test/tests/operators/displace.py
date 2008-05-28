# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  displace.py
#
#  Tests:      mesh      - 2D/3D rectilinear, single domain.
#                          3D unstructured, single domain.
#                          2D/3D curvilinear, single domain.
#              plots     - pc, mesh
#
#  Defect ID:  '5921, '7486, '8160, '8259, '8265
#
#  Programmer: Hank Childs
#  Date:       May 6, 2004
#
#  Modifications:
#
#    Hank Childs, Tue Jun  7 09:47:58 PDT 2005
#    Added tests for filled boundary with cell-based vectors ['5921].
#
#    Hank Childs, Tue Sep  5 16:39:40 PDT 2006
#    Added test for when users put in non-vectors ['7486].
#
#    Hank Childs, Fri Sep 28 12:28:30 PDT 2007
#    Add tests for three recent bugs ['8160, '8259, '8265]
#
# ----------------------------------------------------------------------------

# Turn off all annotation
TurnOffAllAnnotations() # defines global object 'a'

disp = DisplaceAttributes()
disp.variable = "vel"
SetDefaultOperatorOptions(disp)

# 2D, rectilinear.
OpenDatabase("../data/rect2d.silo")
AddPlot("Pseudocolor", "d")
AddOperator("Displace")
AddPlot("Mesh", "quadmesh2d")
AddOperator("Displace")
DrawPlots()

Test("ops_disp01")

SetActivePlots(1)
DeleteActivePlots()
ReplaceDatabase("../data/curv2d.silo")
ResetView()

Test("ops_disp02")

ReplaceDatabase("../data/curv3d.silo")
v = GetView3D()
v.viewNormal = (0.61, -0.61, 0.61)
SetView3D(v)

Test("ops_disp03")

ReplaceDatabase("../data/rect3d.silo")
Test("ops_disp04")

DeleteAllPlots()

OpenDatabase("../data/ucd3d.silo")

AddPlot("Subset", "mat1")
AddOperator("Displace")
disp.factor = 5
SetOperatorOptions(disp)
DrawPlots()

Test("ops_disp05")

DeleteAllPlots()
OpenDatabase("../data/globe.silo")
AddPlot("FilledBoundary", "mat1")
AddOperator("Displace")
disp.variable = "vel"
disp.factor = 1
SetOperatorOptions(disp)
DrawPlots()
Test("ops_disp06")

disp.variable = "disp"
SetOperatorOptions(disp)
Test("ops_disp07")

disp.variable = "t"
SetOperatorOptions(disp)
error = GetLastError()
TestText("ops_disp08", error)

DeleteAllPlots()

OpenDatabase("../data/boxlib_test_data/2D/plt0822/Header")

#   8265: cell-centered vector with rectilinear mesh 
# + 8259: file format the declares itself having 2D vectors + displace
AddPlot("Pseudocolor", "density")
AddOperator("Displace")
disp.variable = "mom"
disp.factor = 1e-12 # this way the quads won't overlap each other.
SetOperatorOptions(disp)
DrawPlots()
Test("ops_disp09")

DeleteAllPlots()

# '8160: subset plot displaced by a vector
AddPlot("Subset", "patches")
AddOperator("Displace")
SetOperatorOptions(disp)
DrawPlots()
Test("ops_disp10")

Exit()
