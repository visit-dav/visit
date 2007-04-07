# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  singlemulti.py
#
#  Tests:      mesh      - 3D, curvilinear, single domain
#              plots     - Pseudocolor
#              databases - Silo
#
#  Purpose:    This test case tests the ability of VisIt to have single
#              time state files in the same window as multi time state
#              windows and do the right thing when changing time states.
#
#  Bugs:       '4011
#
#  Programmer: Brad Whitlock
#  Date:       Thu Mar 18 13:45:29 PST 2004
#
#  Modifications:
#
# ----------------------------------------------------------------------------

import string

#
# Look at the first few lines of the string representation of the
# WindowInformation to see the list of time sliders, etc.
#
def TestWindowInformation(testname):
    # Get the window information and convert it to a string.
    s = str(GetWindowInformation())
    # Only use the first 5 or so lines from the string.
    lines = string.split(s, "\n")
    s = ""
    for i in range(5):
        if(i < len(lines)):
            s = s + lines[i]
            s = s + "\n"
    TestText(testname, s)

# Create a Pseudocolor plot of wave by opening it up at a late time state.
OpenDatabase("../data/wave*.silo database", 20)
AddPlot("Pseudocolor", "pressure")
DrawPlots()

# Set the view.
v0 = View3DAttributes()
v0.viewNormal = (-0.661743, 0.517608, 0.542382)
v0.focus = (5, 0.757692, 2.5)
v0.viewUp = (0.370798, 0.854716, -0.363277)
v0.viewAngle = 30
v0.parallelScale = 5.63924
v0.nearPlane = -11.2785
v0.farPlane = 11.2785
v0.imagePan = (0.00100868, 0.0454815)
v0.imageZoom = 1.17935
v0.perspective = 1
SetView3D(v0)
Test("singlemulti00")

# Convert the WindowInformation to a string and use that as a test case.
# The WindowInformation contains the list of time sliders, the active time
# state, and the states for each time slider.
TestWindowInformation("singlemulti01")

# Create a Pseudocolor plot of curv3d, a single time state database.
OpenDatabase("../data/curv3d.silo")
AddPlot("Pseudocolor", "u")
AddOperator("Transform")
t = TransformAttributes()
t.doScale = 1
t.scaleX = 0.5
t.scaleY = 0.5
t.scaleZ = 0.1667
t.doTranslate = 1
t.translateX = 5
t.translateY = 0
t.translateZ = -5
SetOperatorOptions(t)
DrawPlots()

# Set the view again
v1 = View3DAttributes()
v1.viewNormal = (-0.661743, 0.517608, 0.542382)
v1.focus = (5, 1.25, 0)
v1.viewUp = (0.370798, 0.854716, -0.363277)
v1.viewAngle = 30
v1.parallelScale = 7.1807
v1.nearPlane = -14.3614
v1.farPlane = 14.3614
v1.imagePan = (0.00100868, 0.0454815)
v1.imageZoom = 1.17935
v1.perspective = 1
SetView3D(v1)
Test("singlemulti02")

# Make sure there is still just one time slider.
TestWindowInformation("singlemulti03")

# Go to the middle time slider state.
SetTimeSliderState(TimeSliderGetNStates() / 2)
Test("singlemulti04")
# Check the time states
TestWindowInformation("singlemulti05")

# Go to the last time slider state.
SetTimeSliderState(TimeSliderGetNStates() - 1)
Test("singlemulti06")
# Check the time states
TestWindowInformation("singlemulti07")

Exit()
