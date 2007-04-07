# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  simplekeyframe.py
#
#  Tests:      mesh      - 3D unstructured, single domain
#              plots     - Pseudocolor, Mesh
#              operators - Clip
#
#  Notes:      This test case uses a session file to ensure that keyframe
#              animations that modify the plot attributes and the view can
#              be loaded from a session file.
#
#  Programmer: Brad Whitlock
#  Date:       Wed Oct 22 15:14:20 PST 2003
#
#  Modifications:
#    Brad Whitlock, Tue Dec 30 17:09:10 PST 2003
#    Added test cases to make sure that view keyframes are copied to windows
#    created via window cloning.
#
#    Brad Whitlock, Wed Apr 7 14:19:31 PST 2004
#    Changed the test case so the number of frames is verified first thing.
#
# ----------------------------------------------------------------------------

def GetTruncatedWindowInformationString():
    # Get the window information and convert it to a string.
    s = str(GetWindowInformation())
    # Only use the first 5 or so lines from the string.
    lines = string.split(s, "\n")
    s = ""
    for i in range(5):
        if(i < len(lines)):
            s = s + lines[i]
            s = s + "\n"
    return s

def TestSetup(testName):
    s = GetTruncatedWindowInformationString()
    s = s + str(GetKeyframeAttributes())
    TestText(testName, s)

# Set up the visualization by restoring a session file.
RestoreSession("tests/session/simplekeyframe.session", 0)

TestSection("Testing keyframes from a restored session file.")

# Make sure that the session file created the right number of keyframes.
TestSetup("simplekeyframe_setup")

# Save a test frame for all of the frames in the animation.
for i in range(TimeSliderGetNStates()):
    SetTimeSliderState(i)
    Test("simplekeyframe%02d" % i)

# 
# Clone the window and make sure that the keyframes were copied.
#
TestSection("Testing keyframes from a cloned window.")
SetTimeSliderState(0)
CloneWindow()
SetActiveWindow(2)
DrawPlots()
Test("viewkeyframe00")
SetTimeSliderState(int(TimeSliderGetNStates() * 0.3333))
Test("viewkeyframe01")
SetTimeSliderState(int(TimeSliderGetNStates() * 0.6666))
Test("viewkeyframe02")
SetTimeSliderState(TimeSliderGetNStates() - 1)
Test("viewkeyframe03")

Exit()
