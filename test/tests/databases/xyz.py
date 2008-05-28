# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  chgcar.py
#
#  Tests:      mesh      - 3D Points
#              plots     - Molecule
#
#  Defect ID:  none
#
#  Programmer: Jeremy Meredith
#  Date:       June 14, 2007
#
# ----------------------------------------------------------------------------


OpenDatabase("../data/nanowireTB23K298.xyz", 0, "XYZ_1.0")
AddPlot("Molecule", "element")
DrawPlots()

View3DAtts = GetView3D()
View3DAtts.viewNormal = (-0.882253, 0.0562832, 0.467398)
View3DAtts.focus = (26.3509, 22.1853, 31.5426)
View3DAtts.viewUp = (-0.0613441, 0.970619, -0.232672)
View3DAtts.viewAngle = 30
SetView3D(View3DAtts)

Test("xyz_01")
TimeSliderNextState()
Test("xyz_02")
TimeSliderNextState()
Test("xyz_03")
TimeSliderNextState()
Test("xyz_04")
TimeSliderNextState()
Test("xyz_05")
TimeSliderNextState()
Test("xyz_06")

Exit()
