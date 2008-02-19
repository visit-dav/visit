# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  silo.py 
#
#  Tests:      multi-part silo files
#              operators - slice
#              operators - onion peel
#              selection - by domains 
#
#  Programmer: Mark C. Miller 
#  Date:       March 8, 2004 
#
#  Defects:    4335/4337.
#
#  Modifications:
#    Kathleen Bonnell, Tue Mar  9 08:48:14 PST 2004
#    Turned off databaseInfo annotation, Used TurnOffDomains instead of
#    SIL sets to get correct domain turned off, reordered DrawPlots for
#    test 3 so that we get same results in parallel.  For OnionPeel, use 
#    SetDefaultOperatorOptions so that options are applied.
#
#    Hank Childs, Thu Jun 24 09:59:12 PDT 2004
#    Add tests for quads that are stored as hexes in a ucd mesh. ['4335/'4337]
#
#    Brad Whitlock, Wed Mar 9 09:15:30 PDT 2005
#    Removed deprecated functions.
#
#    Mark C. Miller, Tue Jun 28 17:28:56 PDT 2005
#    Added tests mimicing Ale3d's history variables
#
#    Mark C. Miller, Wed Dec 13 18:32:20 PST 2006
#    Added time invariant mesh tests
#
#    Mark C. Miller, Wed Feb  7 20:23:22 PST 2007
#    Modified code to set SIL Restriction for mesh1_dup to be independent 
#    of the file structure. Added test for multivar that spans multiple
#    multimeshes; it should fail.
# ----------------------------------------------------------------------------
TurnOffAllAnnotations()

OpenDatabase("../data/multipart_multi_ucd3d.silo")

#
# Test simple read and display of a variable 
#
AddPlot("Pseudocolor","d")
DrawPlots()

v=GetView3D()
v.viewNormal=(-0.5, 0.296198, 0.813798)
SetView3D(v)
Test("silo_01")

#
# Test an intercept slice (that can use
# spatial extents tree if we have it)
#
sliceAtts=SliceAttributes()
sliceAtts.originType=sliceAtts.Intercept
sliceAtts.originIntercept=5
sliceAtts.axisType=sliceAtts.ZAxis
sliceAtts.project2d=0
SetDefaultOperatorOptions(sliceAtts)
AddOperator("Slice",1)
DrawPlots()
Test("silo_02")
DeleteAllPlots()

#
# test selection down to just 1 domain
#
AddPlot("Pseudocolor","u")
TurnDomainsOff()
TurnDomainsOn(("domain11"))
DrawPlots()
Test("silo_03")
DeleteAllPlots()

#
# Test an onion peel
#
AddPlot("Pseudocolor","p")
op = OnionPeelAttributes()
op.categoryName = "domains"
op.subsetName = "domain11"
op.index = (5) 
op.logical = 0
op.adjacencyType = op.Face
op.requestedLayer = 3
SetDefaultOperatorOptions(op)
AddOperator("OnionPeel")
DrawPlots()
Test("silo_04")

# we just hide the plots to keep camera
HideActivePlots()

#
# Do some os work to create what VisIt will see as a 
# 'virtual' database of multi-part silo files by
# creating appropriately named links
#
os.system("rm -f ../data/gorfo*")
cwd = os.getcwd()
os.chdir("../data")
i = 1
for filename in os.listdir("."):
    if filename == "multipart_multi_ucd3d.silo":
        os.system("ln -s %s gorfo_1000"%filename)
        os.system("ln -s %s gorfo_2000"%filename)
        os.system("ln -s %s gorfo_3000"%filename)
    elif string.find(filename, "multipart_multi_ucd3d") == 0:
	name1="gorfo_1000.%d"%i
	name2="gorfo_2000.%d"%i
	name3="gorfo_3000.%d"%i
        os.system("ln -s %s %s"%(filename,name1))
        os.system("ln -s %s %s"%(filename,name2))
        os.system("ln -s %s %s"%(filename,name3))
	i = i + 1
os.chdir(cwd)

#
# Test opening a 'virtual' database of multi-part silo files
# at something other than its first timestep
#
OpenDatabase("../data/gorfo_* database",1)
AddPlot("Pseudocolor","d")
AddPlot("Mesh", "mesh1")
DrawPlots()
Test("silo_05")

# go to the next frame
TimeSliderNextState()
Test("silo_06")

#
# remove all the gorfo files we created above
#
os.system("rm -f ../data/gorfo*")

DeleteAllPlots()

OpenDatabase("../data/ucd3d.silo")
AddPlot("Mesh", "exterior_faces")
DrawPlots()
Test("silo_07")

AddOperator("Slice")
Test("silo_08")

#
# Test something akin to Ale3d's history variables
# Note: time state 0 is purposely corrupted with
# all empty domains. So we go from 1 to end and
# back around to 1 purposely avoiding 0
DeleteAllPlots()
OpenDatabase("../data/hist_ucd3d_* database",1)
AddPlot("Pseudocolor","d_dup")
DrawPlots()

#
# Build a sil restriction
#
silr=SILRestriction()
for i in silr.Wholes():
   if silr.SetName(i) == "mesh1_dup":
       silr.TurnOffSet(i+11)
       silr.TurnOffSet(i+12)
       silr.TurnOffSet(i+22)
       silr.TurnOffSet(i+23)
SetPlotSILRestriction(silr)
Test("silo_09")

for i in range(TimeSliderGetNStates()-2):
    TimeSliderNextState()
    Test("silo_%02d"%(i+10))
TimeSliderSetState(1)
Test("silo_18")

#
# Test defvar object
#
DeleteAllPlots()
OpenDatabase("../data/multi_rect3d.silo")
AddPlot("Pseudocolor","mag")
DrawPlots()
Test("silo_20")

DeleteActivePlots()
AddPlot("Vector","vec")
DrawPlots()
Test("silo_21")

DeleteActivePlots()
AddPlot("Pseudocolor","nmats")
DrawPlots()
Test("silo_22")

#
# Test curves from silo
#
DeleteAllPlots()
a=AnnotationAttributes()
a.axes2D.visible
SetAnnotationAttributes(a)

OpenDatabase("../data/multi_ucd3d.silo")
AddPlot("Curve","line")
DrawPlots()
Test("silo_23")

DeleteActivePlots()
AddPlot("Curve","wave")
DrawPlots()
Test("silo_24")

DeleteActivePlots()
AddPlot("Curve","log")
DrawPlots()
Test("silo_25")

#
# Test objects existing past 2Gig limit in a >2 Gig file
# Large File Support. Because file is large, it is NOT
# part of the repo. We create a sym-link to it from the
# data dir.
#
DeleteAllPlots()
CloseDatabase("../data/multi_ucd3d.silo")
OpenDatabase("../data/largefile.silo")
AddPlot("Curve","sincurve")
AddPlot("Curve","coscurve")
DrawPlots()
Test("silo_26")

#
# Test time invariant mesh
#
DeleteAllPlots()
CloseDatabase("/usr/gapps/visit/data/largefile.silo")
OpenDatabase("../data/multi_ucd3d_ti_* database",2)
AddPlot("Pseudocolor","d")
DrawPlots()
ResetView()
v=GetView3D()
v.viewNormal=(-0.5, 0.296198, 0.813798)
SetView3D(v)
silr=SILRestriction()
silr
for i in range(1,36,2):
    silr.TurnOffSet(i)
SetPlotSILRestriction(silr)
Test("silo_27")
TimeSliderNextState()
Test("silo_28")

#
# Test that multivars that span multimeshes are correctly
# invalidated by VisIt
#
DeleteAllPlots()
CloseDatabase("../data/multi_ucd3d_ti_* database")
OpenDatabase("../data/multi_ucd3d.silo")
AddPlot("Pseudocolor","d_split")
DrawPlots()
t = GetLastError()
TestText("silo_29", t)

#
# Test that we get correct SIL behavior for a database
# with a varying SIL and TreatAllDBsAsTimeVarying turned
# on
#
DeleteAllPlots()
CloseDatabase("../data/multi_ucd3d.silo")
OpenDatabase("../data/histne_ucd3d_* database", 2)
AddPlot("Pseudocolor", "d_dup")
DrawPlots()
Test("silo_30")
TimeSliderNextState()
Test("silo_31")

SetTreatAllDBsAsTimeVarying(1)
TimeSliderNextState()
Test("silo_32")
TimeSliderNextState()
Test("silo_33")
TimeSliderPreviousState()
TimeSliderPreviousState()
TimeSliderPreviousState()
Test("silo_34")

#
# Test a database with some odd multi-block structure
#
DeleteAllPlots()
CloseDatabase("../data/histne_ucd3d_* database")
SetTreatAllDBsAsTimeVarying(0)
OpenDatabase("../data/odd_multi.silo")
AddPlot("Pseudocolor","cyc_00000/den")
DrawPlots()
ResetView()
Test("silo_35")

#
# Test a database in which all timesteps are in one file
#
DeleteAllPlots()
CloseDatabase("../data/odd_multi.silo")
OpenDatabase("../data/wave_1file.visit")
AddPlot("Mesh","quadmesh")
AddPlot("Pseudocolor","pressure")
DrawPlots()
ResetView()
Test("silo_36")
TimeSliderSetState(23)
Test("silo_37")
TimeSliderNextState()
TimeSliderNextState()
TimeSliderNextState()
Test("silo_38")
TimeSliderPreviousState()
TimeSliderPreviousState()
TimeSliderPreviousState()
TimeSliderPreviousState()
TimeSliderPreviousState()
TimeSliderPreviousState()
Test("silo_39")

Exit()
