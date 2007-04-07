# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  text2d.py
#
#  Tests:      mesh        - 2D curvilinear, single domain
#              plots       - Pseudocolor
#              annotations - Text2D
#
#  Defect ID:  None
#
#  Programmer: Brad Whitlock
#  Date:       Thu Dec 4 14:39:21 PST 2003
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Set up the annotation colors, etc.
a = AnnotationAttributes()
a.backgroundColor = (80, 0, 100, 255)
a.foregroundColor = (255, 255, 255, 255)
a.userInfoFlag = 0
a.backgroundMode = a.Solid
SetAnnotationAttributes(a)

# Open up the database and do a plot.
OpenDatabase("../data/curv2d.silo")
AddPlot("Pseudocolor", "u")
DrawPlots()

# Create a Text2D object
text = CreateAnnotationObject("Text2D")
text.text = "Pseudocolor plot!"
text.width = 0.6
text.position = (0.25, 0.45)
Test("text2d00")

# Set the foreground color to see if it changes
a.foregroundColor = (255, 255, 0, 255)
SetAnnotationAttributes(a)
Test("text2d01")

# Test solid text color
a.foregroundColor = (255, 255, 255, 255)
SetAnnotationAttributes(a)
text.useForegroundForTextColor = 0
text.textColor = (0, 255, 0, 255)
Test("text2d02")

# Test text opacity
text.width = 2
text.textColor = (255,255,255,150)
Test("text2d03")

# Set the text atts
text.textColor = (255,255,255,255)
text.useForegroundForTextColor = 1
text.width = 0.6

# Create more text objects
courier = CreateAnnotationObject("Text2D")
courier.text = "Courier"
courier.fontFamily = courier.Courier
courier.position = (0.25, 0.35)
courier.width = 0.35
times = CreateAnnotationObject("Text2D")
times.text = "Times"
times.fontFamily = courier.Times
times.position = (0.25, 0.25)
times.width = 0.25
Test("text2d04")

# Set the bold flag
text.fontBold = 1
courier.fontBold = 1
times.fontBold = 1
Test("text2d05")

# Set the italic flag
text.fontItalic = 1
courier.fontItalic = 1
times.fontItalic = 1
Test("text2d06")

# Set the shadow flag
text.fontShadow = 1
courier.fontShadow = 1
times.fontShadow = 1
Test("text2d07")

# Start Removing text
courier.Delete()
Test("text2d08")
times.Delete()
Test("text2d09")
text.Delete()
Test("text2d10")

Exit()
