# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  line3d.py
#
#  Tests:      mesh        - 3D unstructured, single domain
#              plots       - Pseudocolor
#              annotations - Line3D
#
#  Programmer: Kathleen Biagas 
#  Date:       August 27, 2015
#
#  Modifications:
# ----------------------------------------------------------------------------

# Set up the annotation colors, etc.
a = GetAnnotationAttributes()
a.backgroundColor = (255, 255, 255, 255)
a.foregroundColor = (0, 0, 0, 255)
a.backgroundMode = a.Solid
SetAnnotationAttributes(a)

# Open up the database and do a plot.
OpenDatabase(silo_data_path("globe.silo"))

AddPlot("Pseudocolor", "u")
DrawPlots()

# Create a Line2D object
line = CreateAnnotationObject("Line3D")
line.point1 = (0, -15, 0)
line.point2 = (0, 15, 0)
Test("line3d00")

line.width = 7
Test("Line3d01")

line.style = line.DASH
Test("Line3d02")

line.style = line.DOT
Test("Line3d03")

line.style = line.DOTDASH
Test("Line3d04")


# Set the foreground color to see if it changes
a.foregroundColor = (0, 255, 255, 255)
SetAnnotationAttributes(a)
Test("line3d05")

# Test line color
a.foregroundColor = (255, 255, 255, 255)
SetAnnotationAttributes(a)
line.useForegroundForLineColor = 0
line.color = (255, 0, 255, 255)
Test("line3d06")

# Test arrow at start
line.arrow1Radius = 5
line.arrow2Radius = 5
line.arrow1 = 1
Test("line3d07")
# Test arrow at end
line.arrow1 = 0
line.arrow2 = 1
Test("line3d08")

# Test both arrows 
line.arrow1 = 1
Test("line3d09")

# Tubes
line.arrow1 = 0
line.lineType = line.TUBE
line.tubeRadius = 0.3
line.arrow2Resolution = 15
line.arrow2Radius = 1
Test("line3d10")


# Create more line objects
nextLine = CreateAnnotationObject("Line3D")
nextLine.useForegroundForLineColor = 0
nextLine.color=(0, 255, 0, 255)
nextLine.point1=(-15, 0, 0)
nextLine.point2=(15, 0, 0)
nextLine.arrow2 = 1
nextLine.lineType = line.TUBE
nextLine.tubeRadius = 0.3
nextLine.arrow2Resolution = 15
nextLine.arrow2Radius = 1
Test("line3d11")

# Start Removing text
line.Delete()
Test("line3d12")
nextLine.Delete()
Test("line3d13")


Exit()
