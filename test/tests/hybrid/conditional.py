# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  expressions.py
#
#  Tests:      mesh      - 3D rectilinear, single domain
#              plots     - Pseudocolor
#              operators - Threshold
#
#  Defect ID:  '3655
#
#  Programmer: Hank Childs
#  Date:       August 21, 2003
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


OpenDatabase("../data/rect3d.silo")


DefineScalarExpression("bigun", 'if(and(gt(nmats(mat1), 1.), gt(matvf(mat1, "4"), 0.)), d, 0.)')

AddPlot("Pseudocolor", "bigun")
DrawPlots()

v = View3DAttributes()
v.viewNormal = (0.17, 0.74, 0.64)
v.focus = (0.5, 0.5, 0.5)
v.viewUp = (-0.31, 0.65, -0.68)
v.parallelScale = 0.866
v.nearPlane = -1.73
v.farPlane = 1.73
SetView3D(v)

Test("conditional_01")

t = ThresholdAttributes()
t.lowerBounds = (0.01)
t.upperBounds = (0.99)
SetDefaultOperatorOptions(t)
AddOperator("Threshold")

pc_atts = PseudocolorAttributes()
pc_atts.limitsMode = pc_atts.CurrentPlot
SetPlotOptions(pc_atts)

Test("conditional_02")

DeleteAllPlots()

DefineVectorExpression("vel2", "{ if(lt(u, 0), u, -u/2), if(gte(v, -0.333), v, -v/4), w}")
AddPlot("Vector", "vel2")
vec_atts = VectorAttributes()
vec_atts.nVectors = 100
SetPlotOptions(vec_atts)
DrawPlots()

Test("conditional_03")

DeleteAllPlots()

DefineScalarExpression("eq", "if(eq(u, v), abs(w), 0.)")
AddPlot("Pseudocolor", "eq")
AddOperator("Threshold")
DrawPlots()

Test("conditional_04")

DefineScalarExpression("close", "if(lte(abs(u-v), 0.05), abs(w), 0.)")
ChangeActivePlotsVar("close")

Test("conditional_05")


DefineScalarExpression("close_and_far", "if(or(lte(abs(u-v), 0.05), gte(abs(u-v), 1.)), abs(w), 0.)")
ChangeActivePlotsVar("close_and_far")

Test("conditional_06")


Exit()
