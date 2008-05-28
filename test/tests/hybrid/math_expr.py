# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  math_expr.py
#
#  Tests:      mesh      - 3D unstructured, multi- domain
#              plots     - Pseudocolor
#
#  Defect ID:  '6115
#
#  Programmer: Hank Childs
#  Date:       June 30, 2005
#
# ----------------------------------------------------------------------------

# Turn off all annotation
TurnOffAllAnnotations() # defines global object 'a'


OpenDatabase("../data/globe.silo")


DefineScalarExpression("mod", "mod(t, 10)")
AddPlot("Pseudocolor", "mod")
DrawPlots()
Test("math_expr_01")

DefineScalarExpression("round", "round(u/10)")
ChangeActivePlotsVar("round")
Test("math_expr_02")

DefineScalarExpression("ceil", "ceil(u/10)")
ChangeActivePlotsVar("ceil")
Test("math_expr_03")

DefineScalarExpression("floor", "floor(u/10)")
ChangeActivePlotsVar("floor")
Test("math_expr_04")

Exit()
