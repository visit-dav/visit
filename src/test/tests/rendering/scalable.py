# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  scalable.py
#
#  Tests:      mesh      - 3D unstructured, multiple domains
#              plots     - pc, mesh 
#              operators - slice 
#              selection - some of the domains 
#
#  Defect ID:  '3359
#
#  Programmer: Mark C. Miller 
#  Date:       01Jul03
#
#  Modifications:
#
#    Mark C. Miller, Tue May 11 20:21:24 PDT 2004
#    Changed scalable rendering controls to use activation mode
#
#    Mark C. Miller, Tue May 25 14:29:40 PDT 2004
#    Added code to test automatic tranitions into and out of SR mode
#
#    Mark C. Miller, Thu Jan  6 14:23:01 PST 2005
#    Added code to test multiple windows and auto SR transitions
#
#    Kathleen Bonnell, Mon Jan 23 21:02:56 PST 2006 
#    Added tests '08-10, testing auto-opaque mesh and sr mode.
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to switch between Silo's HDF5 and PDB data.
#
#    Eric Brugger, Fri Aug 15 10:19:33 PDT 2014
#    Modified the script to use srun to launch the parallel engine on edge.
#
#    Eric Brugger, Wed Oct 29 12:08:54 PDT 2014
#    Changed the parallel job launching logic to use srun on surface instead
#    of edge.
#
#    Brad Whitlock, Mon Dec 15 17:22:38 PST 2014
#    Only close the compute engine if it is not parallel.
#
#    Kathleen Biagas, Fri Jul  7 13:41:36 PDT 2017
#    Don't run this test if the parallel engine doesn't exist.
#
#    Kathleen Biagas, Tue Feb 8 2022
#    Use run_dir for location of saving windows, it is cleaned up on exit.
#
#    Eric Brugger, Wed Nov  1 13:46:08 PDT 2023
#    Updated the list of hosts where srun was used to launch a parallel
#    compute engine.
#
# ----------------------------------------------------------------------------

if not sys.platform.startswith("win"):
    if not os.path.exists(visit_bin_path("..", "exe", "engine_par")):
        Exit(121)
else:
    if not os.path.exists(visit_bin_path("engine_par.exe")):
        Exit(121)

# sleep is needed to allow viewer to complete update of window information
# before it is queried here
import time
sleepTime=1

# Turn on scalable rendering
ra = GetRenderingAttributes()
ra.scalableActivationMode = ra.Always
if "useCompression" in dir():
    ra.compressionActivationMode = ra.Always
SetRenderingAttributes(ra)

# Close the compute engine, if it is not already parallel.
engines = GetEngineList()
if len(engines) > 0:
    props = GetEngineProperties(engines[0])
    if props.numProcessors < 2: 
        CloseComputeEngine()
        # explicitly open a parallel engine, if possible
        # if it fails, the OpenDatabase will start a serial engine
        import socket
        if "quartz" in socket.gethostname() or \
           "pascal"  in socket.gethostname() or \
           "poodle"  in socket.gethostname() or \
           "syrah"   in socket.gethostname():
            haveParallelEngine = OpenComputeEngine("localhost", ("-l", "srun", "-np", "2"))
        else:
            haveParallelEngine = OpenComputeEngine("localhost", ("-np", "2"))

OpenDatabase(silo_data_path("multi_ucd3d.silo"))

AddPlot("Mesh", "mesh1")
silr=SILRestriction()
silr.TurnOffAll()
silr.TurnOnSet(1)
silr.TurnOnSet(3)
silr.TurnOnSet(5)
silr.TurnOnSet(10)
SetPlotSILRestriction(silr)
AddPlot("Pseudocolor", "d")
SetPlotSILRestriction(silr)

DrawPlots()

view=GetView3D()
view.viewNormal=(0,1,0)
view.viewUp=(0,0,-1) 
SetView3D(view)

Test("scalable_01")

# rotate and draw them again
view.viewNormal=(0,1,0)
view.viewUp=(1,0,0)
SetView3D(view)

# Make sure mesh plot doesn't dissappear after a rotate
# ('3359)
Test("scalable_02")

# Pan and zoom
view.imageZoom = 0.75
view.imagePan = (0.25, 0.25)
SetView3D(view)
Test("scalable_03")

# Make sure hide behavior still works
SetActivePlots(1)
HideActivePlots()
Test("scalable_04")

HideActivePlots()
SetActivePlots(0)
DeleteActivePlots()
Test("scalable_05")

# reset the SIL restriction for the succeeding SR auto mode tests
# tests to make sure the ERI actor doesn't leave stale image data around
ResetView()
silr.TurnOnAll()
SetPlotSILRestriction(silr)
DeleteAllPlots()
Test("scalable_06")

# accumulate results of SR mode changes here
srModeHistory=""

# function to save temporary windows used to force renders
def MySaveWindow():
    swa = GetSaveWindowAttributes()
    swa.fileName = pjoin(TestEnv.params["run_dir"], "scalable_tmp.png")
    swa.format = swa.PNG
    swa.family = 0
    SetSaveWindowAttributes(swa)
    SaveWindow() # required to force render

# function to examine current SR mode of window
def CheckSRMode(winID):
   global srModeHistory
   DrawPlots()
   MySaveWindow() # required to force render
   time.sleep(sleepTime)
   wi=GetWindowInformation()
   if wi.usingScalableRendering == 1:
       srModeHistory+="window %d:SR is ON\n"%winID
   else:
       srModeHistory+="window %d:SR is off\n"%winID

# function to force window into specified SR mode and confirm it 
def ForceSRMode(mode): 
   global srModeHistory
   if mode:
       ra.scalableActivationMode = ra.Always
   else:
       ra.scalableActivationMode = ra.Never
   SetRenderingAttributes(ra)
   DrawPlots()
   time.sleep(sleepTime)
   wi=GetWindowInformation()
   srModeHistory+="force SR mode %d\n"%wi.usingScalableRendering

ForceSRMode(0)

# test an individual plot taking poly count above threshold
# with proc 0's data alone
ra.scalableActivationMode = ra.Auto
ra.scalableAutoThreshold = 2000
SetRenderingAttributes(ra)
DrawPlots()
AddPlot("Pseudocolor", "d")
DrawPlots()
time.sleep(sleepTime)
wi=GetWindowInformation()
srModeHistory+="single plot, proc 0 >threshold %d\n"%wi.usingScalableRendering
DeleteAllPlots()

ForceSRMode(0)

# test an individual plot taking poly count above threshold with
# data from proc 2 (or above)
ra.scalableActivationMode = ra.Auto
ra.scalableAutoThreshold = 5000
SetRenderingAttributes(ra)
DrawPlots()
AddPlot("Pseudocolor", "d")
DrawPlots()
time.sleep(sleepTime)
wi=GetWindowInformation()
srModeHistory+="single plot, proc >0 >threshold %d\n"%wi.usingScalableRendering
DeleteAllPlots()

ForceSRMode(0)

# test a addition and deletion causing to go above threshold and then
# back below
ra.scalableActivationMode = ra.Auto
ra.scalableAutoThreshold = 15000
SetRenderingAttributes(ra)
DrawPlots()
AddPlot("Pseudocolor", "d")
DrawPlots()
time.sleep(sleepTime)
wi=GetWindowInformation()
srModeHistory+="two plots, first <threshold %d\n"%wi.usingScalableRendering
AddPlot("Mesh", "mesh1")
DrawPlots()
time.sleep(sleepTime)
wi=GetWindowInformation()
srModeHistory+="two plots, second >threshold %d\n"%wi.usingScalableRendering
DeleteActivePlots()
DrawPlots()
MySaveWindow() # force engine to render
time.sleep(sleepTime)
wi=GetWindowInformation()
srModeHistory+="delete 2nd, <threshold %d\n"%wi.usingScalableRendering

ForceSRMode(0)

# test setting auto threshold below current count
ra.scalableActivationMode = ra.Auto
ra.scalableAutoThreshold = 3000 
SetRenderingAttributes(ra)
DrawPlots()
time.sleep(sleepTime)
wi=GetWindowInformation()
srModeHistory+="auto threshold set < plot %d\n"%wi.usingScalableRendering

ForceSRMode(1)

# test setting auto threshold above current count
ra.scalableActivationMode = ra.Auto
ra.scalableAutoThreshold = 30000
SetRenderingAttributes(ra)
DrawPlots()
# save window is required to force the re-draw that causes engine to
# decide SR mode is no longer necessary
MySaveWindow()
time.sleep(sleepTime)
wi=GetWindowInformation()
srModeHistory+="auto threshold set > plot %d\n"%wi.usingScalableRendering

TestText("SRModeHistory",srModeHistory)

DeleteAllPlots()
srModeHistory=""

#
# Do some testing with multiple windows
#
TestSection("Multiple Windows")

ra.scalableActivationMode = ra.Auto
ra.scalableAutoThreshold = 14000 
SetRenderingAttributes(ra)

OpenDatabase(silo_data_path("multi_ucd3d.silo"))

AddPlot("Pseudocolor", "d")
DrawPlots()
CheckSRMode(1)

CloneWindow()
SetActiveWindow(2)
DrawPlots()
CheckSRMode(2)

SetActiveWindow(1)
AddPlot("Pseudocolor", "p")
AddPlot("Pseudocolor", "u")
AddPlot("Pseudocolor", "w")
DrawPlots()
CheckSRMode(1)
SetActiveWindow(2)
AddPlot("Pseudocolor", "p")
DrawPlots()
CheckSRMode(2)

SetActiveWindow(1)
SetActivePlots((1,2,3))
DeleteActivePlots()
DrawPlots()
CheckSRMode(1)
SetActiveWindow(2)
CheckSRMode(2)

TestText("multiWindow_SRModeHistory",srModeHistory)

#
# test unhiding mesh to make sure it doesn't go opaque ('5947)
#
DeleteWindow()
SetActiveWindow(1)
DeleteAllPlots()
OpenDatabase(silo_data_path("globe.silo"))

ra.scalableActivationMode = ra.Always
SetRenderingAttributes(ra)
AddPlot("Mesh","mesh1")
AddPlot("Pseudocolor","dx")
DrawPlots()
MySaveWindow() # force a render
SetActivePlots((0))
HideActivePlots()
DrawPlots()
MySaveWindow() # force a render
HideActivePlots()
DrawPlots()
Test("scalable_07")

DeleteAllPlots()

TestSection("Testing Mesh plot's opaque flag in SR mode")
OpenDatabase(silo_data_path("curv2d.silo"))

AddPlot("FilledBoundary", "mat1")
AddPlot("Mesh", "curvmesh2d")
m = MeshAttributes()
m.opaqueMode = m.Auto
SetPlotOptions(m)

AddPlot("Pseudocolor", "d")
DrawPlots()
Test("scalable_08")
SetActivePlots(1)
Test("scalable_09")
Pick(164, 136)
Test("scalable_10")
DeleteAllPlots()


TestSection("Glyphed Plot Causing SR Transitions")

srModeHistory=""
ForceSRMode(0)
ra.scalableActivationMode = ra.Auto
ra.scalableAutoThreshold = 30000 
SetRenderingAttributes(ra)

OpenDatabase(silo_data_path("galaxy0000.silo"))

AddPlot("Pseudocolor", "vx")
DrawPlots()
CheckSRMode(1)
TestText("glyphedPlot_SRModeHistory",srModeHistory)

DeleteAllPlots()

# this section causes a crash, comment out until fixed.
#TestSection("Stereo Modes")

#ra.scalableActivationMode = ra.Always
#ra.stereoRendering = 1
#ra.stereoType = ra.RedBlue
#SetRenderingAttributes(ra)

#OpenDatabase(silo_data_path("multi_rect3d.silo"))

#AddPlot("Mesh","mesh1")

# make the eye separation as noticable as possible
#ResetView()
#v = GetView3D()
#v.eyeAngle = 5
#v.viewNormal=(0.707, 0, 0.707)
#SetView3D(v)
#DrawPlots()

#Test("scalable_11")

Exit()
