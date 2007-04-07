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
# ----------------------------------------------------------------------------

# Turn off all annotation
a = AnnotationAttributes()
a.axesFlag2D = 0
a.axesFlag = 0
a.triadFlag = 0
a.bboxFlag = 0
a.userInfoFlag = 0
a.databaseInfoFlag = 0
a.legendInfoFlag = 0
a.backgroundMode = 0
a.foregroundColor = (0, 0, 0, 255)
a.backgroundColor = (255, 255, 255, 255)
SetAnnotationAttributes(a)

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

Exit()
