# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  silo.py 
#
#  Tests:      The wave_tv database which varies domains and materials and
#              number of meshes over time.
#
#  Defects:    '4760
#
#  Programmer: Hank Childs
#  Date:       April 9, 2004 
#
#  Modifications:
#
#    Mark C. Miller, June 12, 2007
#    Replaced explicit annotation manipulation with call to
#    TurnOffAllAnnotations. Added tests of TreatAllDBsAsTimeVarying
#    functionality
#
# ----------------------------------------------------------------------------

TurnOffAllAnnotations()

OpenDatabase("../data/wave_tv*.silo database", 9)

#
# This brings up a subset plot with 5 materials.
#
AddPlot("Subset", "Material")
DrawPlots()

Test("wave_tv_01")

#
# There was a bug ('4760), where animating over time would crash the engine
# when some materials went out of scope.
#
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()

Test("wave_tv_02")

DeleteAllPlots()
CloseDatabase("../data/wave_tv*.silo database")

#
# Open wave_tv one state BEFORE where variable 'transient' is
# defined and then march forward. With 'TreatAllDBsAsTimeVarying'
# turned off, the add should always fail. With it turned on,
# the add should succeed on state 17.
#
OpenDatabase("../data/wave_tv*.silo database", 16)
addOk16 = AddPlot("Pseudocolor","transient")
DeleteAllPlots()
TimeSliderNextState()
addOk17 = AddPlot("Pseudocolor","transient")
msg = "With TreatAllDBsAsTimeVarying set to its default value,\n"
msg = msg + "AddPlot() returned %d for state 16 and %d for state 17"%(addOk16,addOk17)
TestText("wave_tv_03", msg)
DeleteAllPlots()
CloseDatabase("../data/wave_tv*.silo database")

SetTreatAllDBsAsTimeVarying(1)
OpenDatabase("../data/wave_tv*.silo database", 16)
addOk16 = AddPlot("Pseudocolor","transient")
DeleteAllPlots()
TimeSliderNextState()
addOk17 = AddPlot("Pseudocolor","transient")
msg = "With TreatAllDBsAsTimeVarying set to 1 (true),\n"
msg = msg + "AddPlot() returned %d for state 16 and %d for state 17"%(addOk16,addOk17)
TestText("wave_tv_04", msg)
DrawPlots()
Test("wave_tv_05")

Exit()
