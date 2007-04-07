# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  truecolor.py
#
#  Programmer: Mark C. Miller 
#  Date:       June 29, 2005 
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

OpenDatabase("../data/Image_test_data/manhattan.jpg")

AddPlot("Truecolor","color");
DrawPlots()

Test("truecolor_01")

Exit()
