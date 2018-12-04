# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  Fluent.py
#
#  Programmer: Hank Childs
#  Date:       September 8, 2006
#
#  Modifications:
#    Jeremy Meredith, Tue Jul 15 10:43:58 EDT 2008
#    Changed number of vectors in vector plot to match the old behavior.
#    (We now account for how many domains there are.)
#
# ----------------------------------------------------------------------------


OpenDatabase(data_path("Fluent_test_data/Gas2OxyEntSlurry3d30m_MSmods_Ar_Wen_Moist_CO2_WGS_HCl_COdevol.cas"))

AddPlot("Pseudocolor", "TEMPERATURE")
DrawPlots()
Test("Fluent_01")

AddPlot("Mesh", "Mesh")
DrawPlots()
Test("Fluent_02")

DeleteAllPlots()

AddPlot("Vector", "DPMS_MOM")
v = VectorAttributes()
v.nVectors = 400*5
SetPlotOptions(v)
DrawPlots()
Test("Fluent_03")

DeleteAllPlots()

AddPlot("Subset", "blocks")
DrawPlots()
Test("Fluent_04")

Exit()
