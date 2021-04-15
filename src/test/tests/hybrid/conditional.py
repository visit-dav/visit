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
#  Modifications:
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Cyrus Harrison, Thu Mar 25 09:57:34 PDT 2010
#    Added call(s) to DrawPlots() b/c of changes to the default plot state
#    behavior when an operator is added.
#
#    Eddie Rusu, Tue Apr 13 12:08:59 PDT 2021
#    Changed Pseudocolor CurrentPlot to ActualData.
#
# ----------------------------------------------------------------------------



OpenDatabase(silo_data_path("rect3d.silo"))



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
pc_atts.limitsMode = pc_atts.ActualData
SetPlotOptions(pc_atts)
DrawPlots()
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
