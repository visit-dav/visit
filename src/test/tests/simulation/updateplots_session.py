# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  updateplots_session.py
#
#  Tests:      libsim - batch simulations that make images via a session file.
#
#  Programmer: Brad Whitlock
#  Date:       Fri Oct  2 15:18:56 PDT 2015
#
#  Modifications:
#
# ----------------------------------------------------------------------------
import string, time

def WaitForFile(filename, starttime, timelimit):
    fileSize = 0
    lastFileSize = 0
    samecount = 0
    for iteration in range(1000):
        delay = 2
        try:
            # Get the size of the file.
            fileSize = os.stat(filename).st_size

            # If the file grew, wait a little bit and check again.
            if fileSize > lastFileSize:
                samecount = 0
            elif fileSize == lastFileSize and fileSize > 0:
                delay = 1
                samecount = samecount + 1
                if samecount == 3:
                    break

            lastFileSize = fileSize
        except:
            print("Problem stat'ing ", filename)
            pass

        time.sleep(delay)
        if (time.time() - starttime) > timelimit:
            raise "Out of time"

    return fileSize

TestSection("Testing batch simulation with session file")

# The name of the session file.
session = string.replace(TestEnv.params["script"], "updateplots_session.py", "updateplots_session.session")

# Create our simulation object.
sim = Simulation(SimVisItDir(), SimProgram("updateplots"), SimFile("dummy"))
sim.addargument("-batch")
sim.addargument("-sessionfile")
sim.addargument(session)
sim.addargument("-maxcycles")
sim.addargument("11")
sim.addargument("-width")
sim.addargument("%d" % TestEnv.params["width"])
sim.addargument("-height")
sim.addargument("%d" % TestEnv.params["height"])


# Start the sim.
sim.startsim()

# Iterate through the images that we hoped the simulation would create.
timelimit = 300
starttime = time.time()
for testindex in range(10):
    filename = "updateplots%04d.png" % testindex
    if WaitForFile(filename, starttime, timelimit):
        OpenDatabase(filename)
        AddPlot("Truecolor", "color")
        DrawPlots()
        ResetView()
        v = GetView2D()
        v.viewportCoords = (0,1,0,1)
        SetView2D(v)
        testname = "updateplots_session_%02d" % testindex
        Test(testname)
        DeleteAllPlots()
        CloseDatabase(filename)

sim.endsim()

Exit()
