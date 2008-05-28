# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  keyframe.py
#
#  Tests:      mesh      - 3D structured, single domain
#              plots     - pc, mesh
#              operators - none
#              selection - none
#
#  Defect ID:  none
#
#  Programmer: Eric Brugger
#  Date:       Thu Dec 19 16:02:41 PST 2002
#
#  Modifications:
#    Brad Whitlock, Wed Apr 7 17:24:41 PST 2004
#    I added TestSection to break up the test images a little. I also updated
#    the code so it uses the new time slider functions instead of the
#    deprecated animation functions.
#
# ----------------------------------------------------------------------------


# Open a database.
OpenDatabase("../data/wave.visit")
 
# Enable keyframe mode.
k = GetKeyframeAttributes()
k.enabled = 1
SetKeyframeAttributes(k)
 
# Set the number of frames to be 6.
AnimationSetNFrames(6)

# Create a pseudocolor plot that exists over the first
# 6 frames of the animation.
AddPlot("Pseudocolor", "pressure")
SetPlotFrameRange(0, 0, 5)

# Set the attributes at the first and last frame.
SetTimeSliderState(0)
pc = PseudocolorAttributes()
pc.min = 0
pc.max = 0.2
pc.minFlag = 1
pc.maxFlag = 1
SetPlotOptions(pc)
 
SetTimeSliderState(5)
pc.max = 0.5
SetPlotOptions(pc)

# Create a mesh plot that exists over frames 2
# through 4.
AddPlot("Mesh", "quadmesh")

SetPlotFrameRange(1, 2, 4)
SetPlotDatabaseState(1, 2, 28)
SetPlotDatabaseState(1, 4, 56)

# Cycle through all the frames, checking that all
# the images are correct.
TestSection("Simple 6 frame animation")
SetTimeSliderState(0)
DrawPlots()
Test("keyframe_01")
TimeSliderNextState()
Test("keyframe_02")
TimeSliderNextState()
Test("keyframe_03")
TimeSliderNextState()
Test("keyframe_04")
TimeSliderNextState()
Test("keyframe_05")
TimeSliderNextState()
Test("keyframe_06")

# Change to frame 4, change the frame range and
# check that all the frames are ok.
TestSection("Setting plot frame range")
SetTimeSliderState(4)
SetPlotFrameRange(1, 0, 2)
SetPlotDatabaseState(1, 0, 0)
SetPlotDatabaseState(1, 2, 28)

SetTimeSliderState(0)
Test("keyframe_07")
TimeSliderNextState()
Test("keyframe_08")
TimeSliderNextState()
Test("keyframe_09")
TimeSliderNextState()
Test("keyframe_10")
TimeSliderNextState()
Test("keyframe_11")
TimeSliderNextState()
Test("keyframe_12")

SetActivePlots(1)
DeleteActivePlots()

# Set the pseudocolor attributes for frame 2 and
# check that all the frames are ok.
TestSection("Setting a plot attributes keyframe")
SetActivePlots(0)
SetTimeSliderState(2)
pc.max = 0.7
SetPlotOptions(pc)

SetTimeSliderState(0)
Test("keyframe_13")
TimeSliderNextState()
Test("keyframe_14")
TimeSliderNextState()
Test("keyframe_15")
TimeSliderNextState()
Test("keyframe_16")
TimeSliderNextState()
Test("keyframe_17")
TimeSliderNextState()
Test("keyframe_18")

# Delete the keyframe at frame 2 and check that
# all the frames are ok.
TestSection("Removing a plot attributes keyframe")
SetTimeSliderState(2)
DeletePlotKeyframe(0, 2)

SetTimeSliderState(0)
Test("keyframe_19")
TimeSliderNextState()
Test("keyframe_20")
TimeSliderNextState()
Test("keyframe_21")
TimeSliderNextState()
Test("keyframe_22")
TimeSliderNextState()
Test("keyframe_23")
TimeSliderNextState()
Test("keyframe_24")

# Delete the keyframe at frame 5 and check that
# all the frames are ok.
TestSection("Deleting another plot attributes keyframe")
DeletePlotKeyframe(0, 5)

SetTimeSliderState(0)
Test("keyframe_25")
TimeSliderNextState()
Test("keyframe_26")
TimeSliderNextState()
Test("keyframe_27")
TimeSliderNextState()
Test("keyframe_28")
TimeSliderNextState()
Test("keyframe_29")
TimeSliderNextState()
Test("keyframe_30")
 
# Change the number of frames to 4.  Create a
# pseudocolor plot that changes its range but
# keeps the database state constant at 0.  Check
# that all the frames are ok.
TestSection("Decreasing number of animation frames")
DeleteActivePlots()
AnimationSetNFrames(4)
AddPlot("Pseudocolor", "pressure")
SetPlotFrameRange(0, 0, 3)
SetTimeSliderState(0)
pc.max = 0.2
SetPlotOptions(pc)
SetTimeSliderState(3)
pc.max = 0.5
SetPlotOptions(pc)
DeletePlotDatabaseKeyframe(0, 3)
DrawPlots()

SetTimeSliderState(0)
Test("keyframe_31")
TimeSliderNextState()
Test("keyframe_32")
TimeSliderNextState()
Test("keyframe_33")
TimeSliderNextState()
Test("keyframe_34")

# Test copying plots to a new window.
TestSection("Copying keyframed plot to a new window")
SetTimeSliderState(2)
AddWindow()
SetActiveWindow(2)
CopyPlotsToWindow(1, 2)
CopyAnnotationsToWindow(1, 2)
CopyViewToWindow(1, 2)
DrawPlots()

Test("keyframe_35")
TimeSliderNextState()
Test("keyframe_36")
TimeSliderNextState()
Test("keyframe_37")
TimeSliderNextState()
Test("keyframe_38")

# Take the window out of keyframe mode and
# check that setting the plot attributes sets
# them for all the frames.
TestSection("Turning off keyframe mode")
k.enabled = 0
SetKeyframeAttributes(k)

SetPlotDatabaseState(0, 3, 3)
SetTimeSliderState(3)
pc.minFlag = 0
pc.maxFlag = 0
SetPlotOptions(pc)

SetTimeSliderState(0)
Test("keyframe_39")
TimeSliderNextState()
Test("keyframe_40")
TimeSliderNextState()
Test("keyframe_41")
TimeSliderNextState()
Test("keyframe_42")

# Clear the window, reset the view and enable
# keyframe mode in preparation to test view
# keyframing.
TestSection("Testing view keyframing")
DeleteAllPlots()
ResetView()
k.enabled = 1
SetKeyframeAttributes(k)

# Open a database.
OpenDatabase("../data/globe.silo")
 
# Set the number of frames to be 9.
AnimationSetNFrames(9)
 
# Create a pseudocolor plot that exists over the first
# 9 frames of the animation.
AddPlot("Pseudocolor", "u")
SetPlotFrameRange(0, 0, 8)
 
# Create 5 view keyframes.
SetTimeSliderState(0)
v = GetView3D()
v.SetViewNormal(1, 0, 0)
v.SetFocus(0, 0, 0)
v.SetViewUp(0, 0, 1)
v.SetViewAngle(30)
v.SetParallelScale(17.3205)
v.SetNearPlane(-34.641)
v.SetFarPlane(34.641)
v.SetPerspective(1)
SetView3D(v)
SetViewKeyframe()

SetTimeSliderState(2)
v.SetViewNormal(0, 1, 0)
SetView3D(v)
SetViewKeyframe()
 
SetTimeSliderState(4)
v.SetViewNormal(-1, 0, 0)
SetView3D(v)
SetViewKeyframe()
 
SetTimeSliderState(6)
v.SetViewNormal(0, -1, 0)
SetView3D(v)
SetViewKeyframe()
 
SetTimeSliderState(8)
v.SetViewNormal(1, 0, 0)
SetView3D(v)
SetViewKeyframe()
 
# Render the first frame.
SetTimeSliderState(0)
DrawPlots()

# Enter camera view mode and cycle through all the
# frames, checking that all the images are correct.
ToggleCameraViewMode()
Test("keyframe_43")
TimeSliderNextState()
Test("keyframe_44")
TimeSliderNextState()
Test("keyframe_45")
TimeSliderNextState()
Test("keyframe_46")
TimeSliderNextState()
Test("keyframe_47")
TimeSliderNextState()
Test("keyframe_48")
TimeSliderNextState()
Test("keyframe_49")
TimeSliderNextState()
Test("keyframe_50")
TimeSliderNextState()
Test("keyframe_51")
 
# Delete the view keyframe at frame 8 and check the image.
DeleteViewKeyframe(8)
Test("keyframe_52")
 
# Clear all the view keyframes and check the first image.
ClearViewKeyframes()
TimeSliderNextState()
Test("keyframe_53")

# Clear the window and reset the view in preparation
# to test moving keyframes.
TestSection("Moving keyframes")
DeleteAllPlots()
ResetView()

# Open a database.
OpenDatabase("../data/wave.visit")

# Set the number of frames to be 6.
AnimationSetNFrames(6)
 
# Create a pseudocolor plot that exists over the first
# 6 frames of the animation.
AddPlot("Pseudocolor", "pressure")
SetPlotFrameRange(0, 0, 5)
 
# Set 3 keyframes.
SetTimeSliderState(0)
pc = PseudocolorAttributes()
pc.min = 0
pc.max = 0.2
pc.minFlag = 1
pc.maxFlag = 1
SetPlotOptions(pc)
 
SetTimeSliderState(3)
pc.max = 0.7
SetPlotOptions(pc)
 
SetTimeSliderState(5)
pc.max = 0.5
SetPlotOptions(pc)
 
# Render all the images.
SetTimeSliderState(0)
DrawPlots()
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()
 
# Move the keyframe at frame 3 to 4, regenerate all
# the frames and check a few of them.
MovePlotKeyframe(0, 3, 4)
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()
 
SetTimeSliderState(2)
Test("keyframe_54")
SetTimeSliderState(4)
Test("keyframe_55")
SetTimeSliderState(5)
Test("keyframe_56")

# Delete the last 2 keyframes, move the database
# keyframe from frame 5 to 3, regenerate all
# the frames and check a few of them.
DeletePlotKeyframe(0, 4)
DeletePlotKeyframe(0, 5)
 
MovePlotDatabaseKeyframe(0, 5, 3)
SetTimeSliderState(0)
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()
 
SetTimeSliderState(1)
Test("keyframe_57")
SetTimeSliderState(3)
Test("keyframe_58")
SetTimeSliderState(4)
Test("keyframe_59")
 
# Delete the second database keyframe, create
# 3 view keyframes and regenerate all the images.
DeletePlotDatabaseKeyframe(0, 3)

SetTimeSliderState(0)
v = GetView3D()
v.SetViewNormal(-1, 0, 1)
v.SetFocus(5, 0.35, 2.5)
v.SetViewUp(0, 1, 0)
v.SetViewAngle(30)
v.SetParallelScale(5.6009)
v.SetNearPlane(-11.2018)
v.SetFarPlane(11.2018)
v.SetPerspective(1)
SetView3D(v)
SetViewKeyframe()
 
SetTimeSliderState(2)
v.SetViewNormal(0, 0, 1)
SetView3D(v)
SetViewKeyframe()
 
SetTimeSliderState(5)
v.SetViewNormal(1, 0, 1)
SetView3D(v)
SetViewKeyframe()
 
ToggleCameraViewMode()
SetTimeSliderState(0)
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()

# Move the view keyframe at frame 2 to frame 3,
# regenerate all the frames and check a few of them.
MoveViewKeyframe(2, 3)
SetTimeSliderState(0)
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()

SetTimeSliderState(2)
Test("keyframe_60")
SetTimeSliderState(3)
Test("keyframe_61")
SetTimeSliderState(4)
Test("keyframe_62")

Exit()
