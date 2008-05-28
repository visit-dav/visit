# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  global_node_ids.py
#
#  Tests:      mesh      - 3D unstructured,multi-domain, global node ids, but
#                          no ghost zones.
#              plots     - mat subset, domain subset
#
#  Defect ID:  '5448
#
#  Programmer: Hank Childs
#  Date:       October 5, 2004
#
# ----------------------------------------------------------------------------

# Turn off all annotation
TurnOffAllAnnotations() # defines global object 'a'

view = View3DAttributes()
view.viewNormal = (0.557976, 0.651128, 0.514485)
view.focus = (0.5, 0.5, 0.5)
view.viewUp = (-0.0955897, 0.666272, -0.739557)
view.viewAngle = 30
view.parallelScale = 0.866025
view.nearPlane = -1.73205
view.farPlane = 1.73205
view.perspective = 1
SetView3D(view)

OpenDatabase("../data/global_node.silo")
AddPlot("Subset", "mat")
DrawPlots()

# Test the normal material plot.
Test("global_node_ids01")

# Make sure that the ghost zones were generated correctly.
view.nearPlane = -0.3
SetView3D(view)
Test("global_node_ids02")

view.nearPlane = -1.73205
SetView3D(view)

TurnMaterialsOff("1")
Test("global_node_ids03")

TurnMaterialsOff()
TurnMaterialsOn("1")
Test("global_node_ids04")

DeleteAllPlots()

# Test that the SIL from the previous plot is preserved.
AddPlot("Subset", "domains")
DrawPlots()
Test("global_node_ids05")

DeleteAllPlots()
AddPlot("Pseudocolor", "dist")
DrawPlots()
Test("global_node_ids06")

AddPlot("Mesh", "mesh")
DrawPlots()
Test("global_node_ids07")

DeleteAllPlots()
AddPlot("Contour", "dist")
DrawPlots()
Test("global_node_ids08")

DefineScalarExpression("dist2", "recenter(dist)")
ChangeActivePlotsVar("dist2")
Test("global_node_ids09")

DeleteAllPlots()
AddPlot("Pseudocolor", "dist2")
DrawPlots()
Test("global_node_ids10")

Exit()


