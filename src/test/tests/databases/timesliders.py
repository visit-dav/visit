# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  timesliders.py
#
#  Tests:      mesh      - 2D, curvilinear, single domain
#              plots     - FilledBoundary
#              databases - PDB
#
#  Purpose:    This test case tests out using multiple time sliders.
#
#  Programmer: Brad Whitlock
#  Date:       Fri Mar 19 11:45:12 PDT 2004
#
#  Modifications:
#    Brad Whitlock, Wed Mar 31 10:34:27 PDT 2004
#    I changed the calls to SetActiveTimeSlider so they can accept the
#    unexpanded database names instead of requiring fully qualified
#    database names.
#
#    Brad Whitlock, Mon Apr 19 09:20:29 PDT 2004
#    I added a few more tests to make sure we get the desired list of time
#    sliders under more conditions.
#
#    Brad Whitlock, Wed Feb 9 11:10:29 PDT 2005
#    I added another test section to make sure that we get the right time
#    sliders (or lack of) after deleting plots.
# 
#    Brad Whitlock, Thu Dec 21 11:57:52 PDT 2006
#    Added code to make sure that the dictionary keys in one of the tests
#    are always sorted.
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
# ----------------------------------------------------------------------------

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

def SetTheView():
    v0 = View3DAttributes()
    v0.viewNormal = (-0.735926, 0.562657, 0.376604)
    v0.focus = (5, 0.753448, 2.5)
    v0.viewUp = (0.454745, 0.822858, -0.340752)
    v0.viewAngle = 30
    v0.parallelScale = 5.6398
    v0.nearPlane = -11.2796
    v0.farPlane = 11.2796
    v0.imagePan = (0.0589778, 0.0898255)
    v0.imageZoom = 1.32552
    v0.perspective = 1
    v0.eyeAngle = 2
    SetView3D(v0)

# Print the dictionary so its keys are always sorted a particular way.
def PrintDict(dict):
    keys = list(dict.keys())
    keys.sort()
    s = "{"
    i = 0
    for k in keys:
        if type(k) == type(str("")):
            kstr = "'" + str(k) + "'"
        else:
            kstr = str(k)
        s = s + kstr + ": " + str(dict[k])
        if i < len(keys)-1:
            s = s + ", "
        i = i + 1
    s = s + "}"
    return s
        
# The plotted databases.
dbs = (data_path("pdb_test_data/dbA00.pdb"),
       data_path("pdb_test_data/dbB00.pdb"),
       data_path("pdb_test_data/dbC00.pdb"))

# Create a plot from one database
TestSection("Set time using different time sliders")
OpenDatabase(dbs[0])
AddPlot("FilledBoundary", "material(mesh)")
DrawPlots()
Test("timesliders00")
TestWindowInformation("timesliders01")

# Create a plot from another database
OpenDatabase(dbs[1])
AddPlot("FilledBoundary", "material(mesh)")
DrawPlots()
Test("timesliders02")
TestWindowInformation("timesliders03")

# Change the time state for the second time slider.
SetTimeSliderState(TimeSliderGetNStates() / 2)
Test("timesliders04")
TestWindowInformation("timesliders05")

# Make sure that GetTimeSliders returned the right dictionary.
testString = "GetTimeSliders returned:\n %s" % PrintDict(GetTimeSliders())
TestText("timesliders06", testString)

# Set the time slider back to the first time slider.
SetActiveTimeSlider(dbs[0])

# Set the time state for the first time slider.
SetTimeSliderState(7)
Test("timesliders07")
TestWindowInformation("timesliders08")

# Create a database correlation for the first two databases. This correlation
# will be an IndexForIndex correlation since we passed 0 for the correlation
# method.
TestSection("Time slider behavior with a correlation")
correlation1 = "A_and_B"
CreateDatabaseCorrelation(correlation1, dbs[:-1], 0)
SetActiveTimeSlider(correlation1)
Test("timesliders09")
TestWindowInformation("timesliders10")

# Set the time state for the active time slider since it is now the A_and_B
# database correlation.
SetTimeSliderState(0)
Test("timesliders11")
TestWindowInformation("timesliders12")

SetTimeSliderState(5)
Test("timesliders13")
TestWindowInformation("timesliders14")

SetTimeSliderState(19)
Test("timesliders15")
TestWindowInformation("timesliders16")

# Set the time slider to B. Only B should change.
SetActiveTimeSlider(dbs[1])
SetTimeSliderState(5)
Test("timesliders17")
TestWindowInformation("timesliders18")

# Add a new window and make sure that the time sliders are copied to it.
TestSection("Make sure cloned window gets time sliders")
CloneWindow()
SetActiveWindow(2)
DrawPlots()
Test("timesliders19")
TestWindowInformation("timesliders20")

SetTimeSliderState(19)
Test("timesliders21")
TestWindowInformation("timesliders22")
DeleteWindow()

# Make sure switching between different databases give the right time sliders.
TestSection("Make sure opening ST database clears time slider list")
DeleteAllPlots()
OpenDatabase(dbs[0])
SetTimeSliderState(0)
TestWindowInformation("timesliders23")
OpenDatabase(silo_data_path("curv2d.silo"))

AddPlot("Pseudocolor", "u")
DrawPlots()
ResetView()
Test("timesliders24")
TestWindowInformation("timesliders25")

# Make sure doing various replace sequences give the right time sliders.
TestSection("Make sure replace sequences give right time sliders")
DeleteAllPlots()
OpenDatabase(dbs[0])
AddPlot("FilledBoundary", "material(mesh)")
DrawPlots()
Test("timesliders26")
TestWindowInformation("timesliders27")
ReplaceDatabase(dbs[1])
Test("timesliders28")
TestWindowInformation("timesliders29")
ReplaceDatabase(dbs[0])
Test("timesliders30")
# There should only be 1 time slider at this point.
TestWindowInformation("timesliders31")

# Make sure that when we replace an MT database with an ST database, we get
# the right time sliders.
DeleteAllPlots()
OpenDatabase(silo_data_path("wave*.silo database"), 30)

AddPlot("Pseudocolor", "pressure")
DrawPlots()
SetTheView()
Test("timesliders32")
TestWindowInformation("timesliders33")
# Replace with an ST database
ReplaceDatabase(silo_data_path("wave0000.silo"))
Test("timesliders34")
TestWindowInformation("timesliders35")

# Make sure that we get the right time sliders after we delete plots.
TestSection("Make sure we get the right time sliders after deleting plots.")
DeleteAllPlots()
# Close all the sources so we get fresh time sliders.
for source in GetGlobalAttributes().sources:
    CloseDatabase(source)

OpenDatabase(silo_data_path("wave.visit"))

AddPlot("Pseudocolor", "pressure")
OpenDatabase(silo_data_path("curv3d.silo"))

AddPlot("Pseudocolor", "p")
DrawPlots()

ResetView()
v = GetView3D()
v.viewNormal = (0.163324, 0.442866, 0.881586)
v.viewUp = (-0.0889191, 0.896556, -0.433913)
v.parallelScale = 16.9558
v.imagePan = (0.0834786, 0.0495278)
v.imageZoom = 1.53171
SetView3D(v)

Test("timesliders36")
TestWindowInformation("timesliders37")
SetActivePlots(0)
DeleteActivePlots()
Test("timesliders38")
TestWindowInformation("timesliders39")
DeleteActivePlots()
TestWindowInformation("timesliders40")


Exit()
