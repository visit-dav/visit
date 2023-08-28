# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Tests: weather or not SaveWindow can result in a python exception
#
#  Programmer: Mark C. Miller
#  Date:       March 7, 2006 
#
#  Modifications:
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Kathleen Biagas, Tue Feb 8, 2022
#    Use run_dir as outputDirectory instead of current. It is in the
#    testing output directory and is cleaned up on exit.
#    Added taskkill for Windows to kill the engine.
#
# ----------------------------------------------------------------------------

import os

TurnOnAllAnnotations()

swa = SaveWindowAttributes()
swa.outputToCurrentDirectory = 0
swa.outputDirectory = TestEnv.params["run_dir"]
SetSaveWindowAttributes(swa)

OpenDatabase(silo_data_path("wave.visit"))

AddPlot("Pseudocolor","pressure")
DrawPlots()

pa = GetProcessAttributes("engine")
enginePid = int(pa.pids[0])

s = ""

for i in range(6):
    TimeSliderSetState(i)
    if i == 3:
        if sys.platform.startswith("win"):
            os.system("taskkill.exe /F /PID %d /T"%enginePid)
        else:
            os.system("kill -9 %d"%enginePid)
    try:
        SaveWindow()
    except Exception as inst:
        s = s + "save %d had exception \"%s\"\n"%(i,inst);
    else:
        s = s + "save %d succeeded\n"%i;

TestText("SaveWindowEngineCrash", s)

Exit()
