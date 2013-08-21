# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  OpenFOAM.py
#
#  Programmer: Hank Childs
#  Date:       September 8, 2006
#
#  Modifications:
#    Jeremy Meredith, Tue Jul 15 10:43:58 EDT 2008
#    Changed number of vectors in vector plot to match the old behavior.
#    (We now account for how many domains there are.)
#
#    Kathleen Biagas, Tue Aug 20 14:06:46 PDT 2013
#    Reader has been updated, default mesh is now 'internalMesh', and
#    variables need mesh name.
# ----------------------------------------------------------------------------

OpenDatabase(data_path("OpenFOAM_test_data/bubbleColumn/system/controlDict"))

AddPlot("Vector", "internalMesh/Ua")
v = VectorAttributes()
v.nVectors = 400*5
SetPlotOptions(v)
DrawPlots()
Test("OpenFOAM_01")

TimeSliderNextState()
Test("OpenFOAM_02")
TimeSliderNextState()
Test("OpenFOAM_03")

AddPlot("Mesh", "internalMesh")
DrawPlots()
Test("OpenFOAM_04")

DeleteAllPlots()

AddPlot("Pseudocolor", "internalMesh/epsilon")
DrawPlots()
Test("OpenFOAM_05")

DeleteAllPlots()

OpenDatabase(data_path("OpenFOAM_test_data/mixer2D_ascii/system/controlDict"))

AddPlot("Vector", "internalMesh/U")
v = VectorAttributes()
v.nVectors = 400*6
SetPlotOptions(v)
DrawPlots()
Test("OpenFOAM_06")

TimeSliderNextState()
Test("OpenFOAM_07")
TimeSliderNextState()
Test("OpenFOAM_08")

AddPlot("Mesh", "internalMesh")
DrawPlots()
Test("OpenFOAM_09")

DeleteAllPlots()

AddPlot("Pseudocolor", "internalMesh/p")
DrawPlots()
Test("OpenFOAM_10")

Exit()
