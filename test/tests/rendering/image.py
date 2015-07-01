# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  image.py
#
#  Tests:      mesh        - 2D curvilinear, single domain
#              plots       - Pseudocolor
#              annotations - Image
#
#  Defect ID:  None 
#
#  Programmer: Kathleen Biagas 
#  Date:       July 1, 2015
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Set up the annotation colors, etc.
a = GetAnnotationAttributes()
a.backgroundColor = (255, 255, 255, 255)
a.foregroundColor = (0, 0, 0, 255)
a.backgroundMode = a.Solid
SetAnnotationAttributes(a)

# Open up the database and do a plot.
OpenDatabase(silo_data_path("curv2d.silo"))

AddPlot("Pseudocolor", "u")
DrawPlots()

# Create an Image object
img = CreateAnnotationObject("Image")
img.image = data_path("Image_test_data/manhattan.jpg")
img.position = (0.34, 0.12)
Test("image00")

img.width = 50
Test("image01")

img.width = 10
img.position = (0.5, 0.6)
Test("image02")

Exit()
