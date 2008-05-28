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
TurnOffAllAnnotations() # defines global object 'a'


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
