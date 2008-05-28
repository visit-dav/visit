# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  image.py 
#
#  Defect ID:  '6277
#
#  Tests:      image reader and data selections 
#
#  Programmer: Mark C. Miller 
#  Date:       November 4, 2004 
#
#  Modifications:
# 
#    Hank Childs, Fri May 20 15:08:37 PDT 2005
#    Added tests for image volumes.
#
# ----------------------------------------------------------------------------


#
# we'll make all the pc plots gray scale
#
pcatts=PseudocolorAttributes()
pcatts.colorTableName="gray"
SetDefaultPlotOptions(pcatts)

#
# test ability to read an image as usual 
#
OpenDatabase("../data/Image_test_data/manhattan.jpg")

AddPlot("Pseudocolor","intensity")
DrawPlots()
Test("image_01")

DeleteAllPlots()

#
# Test a data selection on a format that cannot
# handle it during read. The selection will
# occur after WHOLE image is read
#
AddPlot("Pseudocolor","intensity")
box=BoxAttributes()
box.minx = 0
box.maxx = 100
box.miny = 0
box.maxy = 100
AddOperator("Box")
SetOperatorOptions(box)
DrawPlots()
Test("image_02")

DeleteAllPlots()

#
# Now test a data selection on a format that can
# handle selection during read 
#
OpenDatabase("../data/Image_test_data/manhattan.pnm")
AddPlot("Pseudocolor","intensity")
AddOperator("Box")
SetOperatorOptions(box)
DrawPlots()
Test("image_03")

DeleteAllPlots()
f = open("../data/manhattan.imgvol", "wt")
f.write("Z_STEP:60\n")
for i in range(3):
   f.write("Image_test_data/manhattan.jpg\n")
f.close()
OpenDatabase("../data/manhattan.imgvol")
AddPlot("Contour", "green")
c = ContourAttributes()
c.contourMethod = c.Value
c.contourValue = (128)
SetPlotOptions(c)
DrawPlots()
Test("image_04")
   
Exit()
