# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  image_proc.py
#
#  Tests:      mesh      - Image
#              plots     - Pseudocolor
#
#  Defect ID:  '6492
#
#  Programmer: Hank Childs
#  Date:       August 19, 2005
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


OpenDatabase("../data/Image_test_data/manhattan.jpg")
DefineScalarExpression("cm", "conservative_smoothing(intensity)")
AddPlot("Pseudocolor", "cm")
DrawPlots()
Test("image_proc_01")

DefineScalarExpression("median", "median_filter(intensity)")
ChangeActivePlotsVar("median")
Test("image_proc_02")

DefineScalarExpression("mean", "mean_filter(intensity)")
ChangeActivePlotsVar("mean")
Test("image_proc_03")

Exit()
