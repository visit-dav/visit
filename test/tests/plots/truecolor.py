# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  truecolor.py
#
#  Programmer: Mark C. Miller 
#  Date:       June 29, 2005 
# ----------------------------------------------------------------------------



OpenDatabase("../data/Image_test_data/manhattan.jpg")

AddPlot("Truecolor","color");
DrawPlots()

Test("truecolor_01")

Exit()
