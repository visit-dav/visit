# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  sil.py
#
#  Tests:      mesh      - 3D curvilinear,multi-domain,ghost zones replicated.
#              plots     - mat subset, domain subset
#
#  Defect ID:  none
#
#  Programmer: Hank Childs
#  Date:       December 5, 2002
#
#  Modifications:
#    Kathleen Bonnell, Thu Aug 28 14:34:57 PDT 2003
#    Remove compound var name from subset plots.
#
# ----------------------------------------------------------------------------


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

OpenDatabase("../data/bigsil.silo")
AddPlot("Subset", "mat")
DrawPlots()

# Test the normal material plot.
Test("sil1")

# Make sure that the ghost zones were generated correctly.
view.nearPlane = -0.3
SetView3D(view)
Test("sil2")

view.nearPlane = -1.73205
SetView3D(view)

TurnMaterialsOff("1")
Test("sil3")

TurnMaterialsOff()
TurnMaterialsOn("1")
Test("sil4")

DeleteAllPlots()

# Test that the SIL from the previous plot is preserved.
AddPlot("Subset", "domains")
DrawPlots()
Test("sil5")

Exit()


