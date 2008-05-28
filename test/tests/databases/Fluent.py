# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  Fluent.py
#
#  Programmer: Hank Childs
#  Date:       September 8, 2006
#
# ----------------------------------------------------------------------------

TurnOffAllAnnotations() # defines global object 'a'

OpenDatabase("../data/Fluent_test_data/Gas2OxyEntSlurry3d30m_MSmods_Ar_Wen_Moist_CO2_WGS_HCl_COdevol.cas")
AddPlot("Pseudocolor", "TEMPERATURE")
DrawPlots()
Test("Fluent_01")

AddPlot("Mesh", "Mesh")
DrawPlots()
Test("Fluent_02")

DeleteAllPlots()

AddPlot("Vector", "DPMS_MOM")
DrawPlots()
Test("Fluent_03")

DeleteAllPlots()

AddPlot("Subset", "blocks")
DrawPlots()
Test("Fluent_04")

Exit()
