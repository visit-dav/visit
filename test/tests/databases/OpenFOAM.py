# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  OpenFOAM.py
#
#  Programmer: Hank Childs
#  Date:       September 8, 2006
#
# ----------------------------------------------------------------------------

TurnOffAllAnnotations() # defines global object 'a'

OpenDatabase("../data/OpenFOAM_test_data/bubbleColumn/system/controlDict")

AddPlot("Vector", "Ua")
DrawPlots()
Test("OpenFOAM_01")

TimeSliderNextState()
Test("OpenFOAM_02")
TimeSliderNextState()
Test("OpenFOAM_03")

AddPlot("Mesh", "Mesh")
DrawPlots()
Test("OpenFOAM_04")

DeleteAllPlots()

AddPlot("Pseudocolor", "epsilon")
DrawPlots()
Test("OpenFOAM_05")

DeleteAllPlots()

OpenDatabase("../data/OpenFOAM_test_data/mixer2D_ascii/system/controlDict")
AddPlot("Vector", "U")
DrawPlots()
Test("OpenFOAM_06")

TimeSliderNextState()
Test("OpenFOAM_07")
TimeSliderNextState()
Test("OpenFOAM_08")

AddPlot("Mesh", "Mesh")
DrawPlots()
Test("OpenFOAM_09")

DeleteAllPlots()

AddPlot("Pseudocolor", "p")
DrawPlots()
Test("OpenFOAM_10")

Exit()
