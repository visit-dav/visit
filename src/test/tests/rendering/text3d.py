# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  text3d.py
#
#  Tests:      mesh        - 2D curvilinear, single domain
#              plots       - Pseudocolor
#              annotations - Text2D
#
#  Defect ID:  None
#
#  Programmer: Kathleen Biagas 
#  Date:       August 31, 2015
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Set up the annotation colors, etc.
a = GetAnnotationAttributes()
a.backgroundColor = (80, 0, 100, 255)
a.foregroundColor = (255, 255, 255, 255)
a.backgroundMode = a.Solid
SetAnnotationAttributes(a)

# Open up the database and do a plot.
OpenDatabase(silo_data_path("globe.silo"))

AddPlot("Pseudocolor", "u")
DrawPlots()

# Create a Text2D object
text = CreateAnnotationObject("Text3D")
text.text = "Pseudocolor plot!"
text.position = (-10,10,10)
Test("text3d00")

# Set the foreground color to see if it changes
a.foregroundColor = (51, 204, 204, 255)
SetAnnotationAttributes(a)
Test("text3d01")

# Test solid text color
a.foregroundColor = (255, 255, 255, 255)
SetAnnotationAttributes(a)
text.useForegroundForTextColor = 0
text.textColor = (255, 102, 0, 255)
Test("text3d02")

# Test text opacity
text.position = (-5,0,9)
text.textColor = (255,102,0,150)
text.relativeHeight = 0.05
Test("text3d03")

# Test Height mode
text.textColor = (255,102,0,255)
text.heightMode = text.Fixed
text.fixedHeight = 2
Test("text3d04")

# Test preserveOrientation
text.position = (-10, 0, 12)
v = GetView3D()
v.viewNormal = (0, 0.707107, 0.707107)
v.viewUp = (0, 0.707107, -0.707107)
SetView3D(v)
Test("text3d05")

text.preserveOrientation = 1
Test("text3d06")


# Test rotations

# Create more text objects
AddOperator("Reflect")
reflect = ReflectAttributes()
reflect.reflections=(1, 1, 0, 0, 0, 0, 0, 0)
SetOperatorOptions(reflect)
text.text="Left"
text.position=(-20, -5, 10)
text.rotations=(0,0,90)
text.heightMode = text.Relative
DrawPlots()
ResetView()
Test("text3d07")

right = CreateAnnotationObject("Text3D")
right.position = (-5, 0, 10)
right.text = "Right"
right.relativeHeight = 0.05
right.rotations=(0,0, -45)
Test("text3d08")

text.Delete()
Test("text3d09")
right.Delete()
Test("text3d10")


Exit()
