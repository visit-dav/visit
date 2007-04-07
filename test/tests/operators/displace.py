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
#  Defect ID:  '5921, '7486
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

Exit()
