# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  val4mat.py
#
#  Tests:      mesh      - 2d structured
#              plots     - pc
#
#  Notes
#
#  Programmer: Cyrus Harrison
#  Date:       Tuesday 12, 2008
#
#  Modifiations:
#
# ----------------------------------------------------------------------------


OpenDatabase(silo_data_path("specmix_quad.silo"))

atts = PseudocolorAttributes()
atts.minFlag = 1
atts.min = 0.0
atts.maxFlag = 1
atts.max = 1.0
SetDefaultPlotOptions(atts)

# view the per material values for each of the 3 materials

DefineScalarExpression("spec_mix", "specmf(Species,1,1)")
AddPlot("Pseudocolor", "spec_mix")
DrawPlots()
Test("specmf_0")

OpenDatabase(silo_data_path("specmix_double_quad.silo"))

AddPlot("Pseudocolor", "spec_mix")
DrawPlots()
Test("specmf_1")

Exit()
