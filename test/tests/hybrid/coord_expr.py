# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  coord_expr.py
#
#  Tests:      mesh      - 3D unstructured, multi- domain
#              plots     - Pseudocolor
#
#  Defect ID:  '5994, '5995
#
#  Programmer: Hank Childs
#  Date:       June 30, 2005
#
# ----------------------------------------------------------------------------



OpenDatabase("../data/globe.silo")


DefineScalarExpression("polar_radius", "polar_radius(mesh1)")
AddPlot("Pseudocolor", "polar_radius")
DrawPlots()
Test("coord_expr_01")

DefineScalarExpression("zero", "polar_radius-polar(mesh1)[0]")
ChangeActivePlotsVar("zero")
Test("coord_expr_02")

DefineScalarExpression("polar_theta", "polar_theta(mesh1)")
ChangeActivePlotsVar("polar_theta")
Test("coord_expr_03")

DefineScalarExpression("polar_phi", "polar_phi(mesh1)")
ChangeActivePlotsVar("polar_phi")
Test("coord_expr_04")

DefineScalarExpression("cylindrical_radius", "cylindrical_radius(mesh1)")
ChangeActivePlotsVar("cylindrical_radius")
Test("coord_expr_05")

DefineScalarExpression("cylindrical_theta", "cylindrical_theta(mesh1)")
ChangeActivePlotsVar("cylindrical_theta")
Test("coord_expr_06")

DeleteExpression("zero")
# Not really zero -- this is the diagonal
DefineScalarExpression("zero", "polar_radius-cylindrical_radius")
ChangeActivePlotsVar("zero")
Test("coord_expr_07")

DeleteActivePlots()

DefineVectorExpression("c", "cylindrical(mesh1)")
AddPlot("Vector", "c")
DrawPlots()
Test("coord_expr_08")

Exit()
