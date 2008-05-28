# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  textanot.py
#
#  Tests:      mesh      - 2D curvilinear, single domain
#              plots     - Pseudocolor
#
#  Notes:      This test case uses a session file to set up a plot and create
#              several text annotations and a time slider annotation.
#
#  Programmer: Brad Whitlock
#  Date:       Thu Dec 4 14:17:46 PST 2003
#
#  Modifications:
#    Brad Whitlock, Wed Mar 9 09:15:30 PDT 2005
#    Removed deprecated functions.
#
# ----------------------------------------------------------------------------

TurnOnAllAnnotations()

# Set up the visualization by restoring a session file.
RestoreSession("tests/session/textannot.session", 0)

# Save a test frame for some of the frames in the animation so we can see
# if the time slider is working.
testindex = 0
ntests = 5
for i in range(ntests):
    frame = int(float(i) / float(ntests - 1) * (TimeSliderGetNStates() - 1))
    SetTimeSliderState(frame)
    Test("textannot%02d" % testindex)
    testindex = testindex + 1

Exit()
