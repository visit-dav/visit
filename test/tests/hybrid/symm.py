# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  symm.py
#
#  Defect ID:  '6913, '7644, '7650
#
#  Programmer: Hank Childs
#  Date:       January 23, 2006
#
#  Modifications:
#
#    Hank Childs, Fri Dec 22 11:01:09 PST 2006
#    Add testing for symm_point
#
#    Hank Childs, Fri Jan  5 11:14:59 PST 2007
#    Add testing for non-rectilinear mesh types, since they use a different
#    code path (which broke).
#
#    Hank Childs, Mon Jan  8 11:04:50 PST 2007
#    Add testing for non-scalar variable types with symmetry CMFE expressions.
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


OpenDatabase("../data/rect2d.silo")


# Test errors.
DefineScalarExpression("sp1", "symm_plane(d, [0, 0, 0, 1, 0, 0])")
AddPlot("Pseudocolor", "sp1")
DrawPlots()
t = GetLastError()
TestText("symm_01", t)

# Now test it working...
DeleteAllPlots()
DefineScalarExpression("sp2", "symm_plane(d, [1, 0, 0, 0.3, 0, 0])")
AddPlot("Pseudocolor", "sp2")
DrawPlots()
Test("symm_02")

DeleteAllPlots()
DefineScalarExpression("sp3", "symm_plane(d, [1, -1, 0, 0.1, 0, 0])")
AddPlot("Pseudocolor", "sp3")
DrawPlots()
Test("symm_03")

DeleteAllPlots()
DefineScalarExpression("sp4", "symm_plane(d, [2, -1, 0, 0.1, 0, 0])")
AddPlot("Pseudocolor", "sp4")
DrawPlots()
Test("symm_04")

DeleteAllPlots()
DefineScalarExpression("st1", "symm_transform(d, [0,1,0,1,0,0,0,0,0])")
AddPlot("Pseudocolor", "st1")
DrawPlots()
Test("symm_05")

DeleteAllPlots()
OpenDatabase("../data/noise.silo")
DefineScalarExpression("sp5", "symm_plane(hardyglobal, [1, 0, 0, 0, 0, 0])")
AddPlot("Contour", "sp5")
DrawPlots()
Test("symm_06")

DeleteAllPlots()
OpenDatabase("../data/noise.silo")
DefineScalarExpression("st2", "symm_transform(hardyglobal, [0.707, 0.707, 0, -0.707, 0.707, 0, 0, 0, 1])")
AddPlot("Contour", "st2")
DrawPlots()
Test("symm_07")

ActivateDatabase("../data/rect2d.silo")
DeleteAllPlots()
DefineScalarExpression("sp4", "symm_point(d, [0.4, 0.6, 0])")
AddPlot("Pseudocolor", "sp4")
DrawPlots()
Test("symm_08")

DeleteAllPlots()
OpenDatabase("../data/curv3d.silo")
DefineScalarExpression("sp6", "symm_plane(d, [1, 0, 0, 0.2, 0, 0])")
AddPlot("Pseudocolor", "sp6")
DrawPlots()
Test("symm_09")

DeleteAllPlots()
DefineVectorExpression("sp7", "symm_plane(vel, [1, 0, 0, 0.2, 0, 0])")
AddPlot("Vector", "sp7")
DrawPlots()
Test("symm_10")

DeleteAllPlots()
DefineScalarExpression("sp8", "magnitude(symm_plane(vel, [1, 0, 0, 0.2, 0, 0]))")
AddPlot("Pseudocolor", "sp8")
DrawPlots()
Test("symm_11")

Exit()
