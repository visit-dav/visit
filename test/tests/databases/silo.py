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
#
#    Jeremy Meredith, Tue Jul 15 10:43:58 EDT 2008
#    Changed number of vectors in vector plot to match the old behavior.
#    (We now account for how many domains there are.)
#
#    Mark C. Miller, Thu Jan 22 16:27:54 PST 2009
#    Modified tests of defvars using mag and vec to make them less sensitive
#    to differences in platform. The mag test was computing a vector magnitude
#    whose range was very, very tiny. Switching that to sum fixes that
#    problem. The vector plot was simply generating way to many vectors that
#    were being drawn on top of each other. I changed it to use a moderately
#    large prime as the stride.
#
#    Mark C. Miller, Mon Sep 28 20:58:24 PDT 2009
#    Added tests for AMR data from Silo file using MRG Trees.
#
#    Tom Fogal, Wed Dec 16 13:11:19 MST 2009
#    Fixed a path to a data file.
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Mark C. Miller, Fri Jan 22 17:30:29 PST 2010
#    I replaced OpenDatabase with FindAnd... variant for largefile.silo
#    to deal with data we don't keep in repo.
#
#    Cyrus Harrison, Thu Mar 25 09:57:34 PDT 2010
#    Added call(s) to DrawPlots() b/c of changes to the default plot state
#    behavior when an operator is added.
#
#    Mark C. Miller, Tue Feb 28 00:36:09 PST 2012
#    Added a slew of tests for hybrid zoo and arbitrary polygonal/polyhedral
#    meshes and variables defined upon them.
# 
#    Kathleen Biagas, Thurs May 23 14:09:15 MST 2013 
#    Don't run certain tests on Windows that cause a crash.
#
#    Kathleen Biagas, Thurs Jun 6 11:04:13 PDT 2013 
#    Re-enable tests 42,44, and 45 on Windows, now that crash has been fixed.
#
# ----------------------------------------------------------------------------
TurnOffAllAnnotations() # defines global object 'a'

# Turn off sets by name
def TurnOffSetsByName(silr, cat, names):
    sets = silr.SetsInCategory(cat)
    for s in sets:
        setname = silr.SetName(s)
        try:
            if setname in names:
                silr.TurnOffSet(s)
        except:
            if setname == names:
                silr.TurnOffSet(s)

OpenDatabase(silo_data_path("multipart_multi_ucd3d.silo"))


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

#TODO BROKEN:
# Cyrus: I tried to change this, from prev code
# b/c symlinks won't work on Windows.
#
# also silo paths in multipart_multi_ucd3d, don't point to 
# gorfo files,they won't work out of the data dir
# (and we shouldn't be modifying the data dir !)

# remove any gorfos 
for f in glob.glob("gorfo_*"):
    os.remove(f)

i = 0
for filename in glob.glob(silo_data_path("multipart_multi_ucd3d*.silo")):
    if filename.endswith("multipart_multi_ucd3d.silo"):
        shutil.copy(silo_data_path(filename),"gorfo_1000")
        shutil.copy(silo_data_path(filename),"gorfo_2000")
        shutil.copy(silo_data_path(filename),"gorfo_3000")
    else:
        shutil.copy(silo_data_path(filename),"gorfo_1000.%d" %i)
        shutil.copy(silo_data_path(filename),"gorfo_2000.%d" %i)
        shutil.copy(silo_data_path(filename),"gorfo_2000.%d" %i)
    i = i + 1

#
# Test opening a 'virtual' database of multi-part silo files
# at something other than its first timestep
#
OpenDatabase("gorfo_* database",1)

AddPlot("Pseudocolor","d")
AddPlot("Mesh", "mesh1")
DrawPlots()
Test("silo_05")

# go to the next frame
TimeSliderNextState()
Test("silo_06")


DeleteAllPlots()

OpenDatabase(silo_data_path("ucd3d.silo"))

AddPlot("Mesh", "exterior_faces")
DrawPlots()
Test("silo_07")

AddOperator("Slice")
DrawPlots()
Test("silo_08")

#
# Test something akin to Ale3d's history variables
# Note: time state 0 is purposely corrupted with
# all empty domains. So we go from 1 to end and
# back around to 1 purposely avoiding 0
DeleteAllPlots()
OpenDatabase(silo_data_path("hist_ucd3d_* database"),1)

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
OpenDatabase(silo_data_path("multi_rect3d.silo"))

AddPlot("Pseudocolor","sum")
DrawPlots()
Test("silo_20")

DeleteActivePlots()
AddPlot("Vector","vec")
vec = VectorAttributes()
vec.useStride = 1
vec.stride = 41
vec.colorByMag = 0
SetPlotOptions(vec)
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
a=GetAnnotationAttributes()
a.axes2D.visible = 1
SetAnnotationAttributes(a)

OpenDatabase(silo_data_path("multi_ucd3d.silo"))

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
CloseDatabase(silo_data_path("multi_ucd3d.silo"))

# this crashes on windows, so don't try to run it.
if not sys.platform.startswith("win"):
    (err, dbname) = FindAndOpenDatabase("largefile.silo")
    if (err != 1):
        AddSkipCase("silo_26")
        Test("silo_26")
    else:
        AddPlot("Curve","sincurve")
        AddPlot("Curve","coscurve")
        DrawPlots()
        Test("silo_26")
        DeleteAllPlots()
        CloseDatabase(dbname)

#
# Test time invariant mesh
#
OpenDatabase(silo_data_path("multi_ucd3d_ti_* database"),2)

AddPlot("Pseudocolor","d")
DrawPlots()
ResetView()
v=GetView3D()
v.viewNormal=(-0.5, 0.296198, 0.813798)
SetView3D(v)
silr=SILRestriction()
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
CloseDatabase(silo_data_path("multi_ucd3d_ti_* database"))

OpenDatabase(silo_data_path("multi_ucd3d.silo"))

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
CloseDatabase(silo_data_path("multi_ucd3d.silo"))

OpenDatabase(silo_data_path("histne_ucd3d_* database"), 2)

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
CloseDatabase(silo_data_path("histne_ucd3d_* database"))

SetTreatAllDBsAsTimeVarying(0)
OpenDatabase(data_path("silo_pdb_test_data/odd_multi.silo"))

AddPlot("Pseudocolor","cyc_00000/den")
DrawPlots()
ResetView()
Test("silo_35")

#
# Test a database in which all timesteps are in one file
#
DeleteAllPlots()
CloseDatabase(data_path("silo_pdb_test_data/odd_multi.silo"))

OpenDatabase(silo_data_path("wave_1file.visit"))

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

DeleteAllPlots()
CloseDatabase(silo_data_path("wave_1file.visit"))

TestSection("Silo AMR w/Mrgtrees")
TurnOffAllAnnotations()
OpenDatabase(data_path("silo_amr_test_data/amr2d_wmrgtree.silo"))

AddPlot("Mesh","amr_mesh_wmrgtree")
DrawPlots()
ResetView()
v=GetView2D()
v.windowCoords = (0.368424, 0.412063, 0.265434, 0.310012)
SetView2D(v)
Test("silo_40")
AddPlot("Pseudocolor","Density_wmrgtree")
DrawPlots()
Test("silo_41")

silr=SILRestriction()
s = silr.SetsInCategory("levels")
TurnOffSetsByName(silr, "levels", "level2")
SetPlotSILRestriction(silr)
Test("silo_42")

DeleteAllPlots()
CloseDatabase(data_path("silo_amr_test_data/amr2d_wmrgtree.silo"))

OpenDatabase(data_path("silo_amr_test_data/amr3d_wmrgtree.silo"))

AddPlot("Contour","foo_wmrgtree")
ca=ContourAttributes()
ca.contourValue = (60,)
ca.contourMethod = ca.Value
SetPlotOptions(ca)
DrawPlots()
ResetView()
v=GetView3D()
v.imagePan = (0.2066, 0.104372)
v.imageZoom = 6.03355
SetView3D(v)
Test("silo_43")

silr=SILRestriction()
TurnOffSetsByName(silr, "levels", "level2")
SetPlotSILRestriction(silr)
Test("silo_44")
TurnOffSetsByName(silr, "levels", "level1")
SetPlotSILRestriction(silr)
Test("silo_45")

DeleteAllPlots()
CloseDatabase(data_path("silo_amr_test_data/amr2d_wmrgtree.silo"))

OpenDatabase(silo_data_path("multi_ucd3d.silo"))

TestSection("Variables defined on material subsets")
sa = SliceAttributes()
sa.originType = sa.Percent
sa.originPercent = 50
sa.axisType = sa.ZAxis
testNum = 46
for varname in ("p_on_mats_2","d_on_mats_1_3","m1vf_on_mats_1","m2vf_on_mats_2","m3vf_on_mats_3"):
    DeleteAllPlots()
    AddPlot("Pseudocolor", varname)
    AddOperator("Slice")
    SetOperatorOptions(sa)
    DrawPlots()
    SetViewExtentsType("actual")
    ResetView()
    Test("silo_%d"%testNum)
    testNum = testNum + 1

DeleteAllPlots()
CloseDatabase(silo_data_path("multi_ucd3d.silo"))

OpenDatabase(silo_data_path("arbpoly-zoohybrid.silo"))

TestSection("Various arbitrary polygonal/polyhedral meshes")

ma = MeshAttributes()
ma.showInternal = 1
for meshname in ("2D/mesh1_zl1","2D/mesh1_zl2","2D/mesh1_phzl","2D/mesh1_phzl2", "2D/mesh1g_zl2"):
    DeleteAllPlots()
    AddPlot("Mesh", meshname)
    SetPlotOptions(ma)
    DrawPlots()
    ResetView()
    v = GetView2D()
    v.windowCoords=(-1, 10, -1, 2)
    SetView2D(v)
    Test("silo_%d"%testNum)
    testNum = testNum + 1

for meshname in ("2Dz/mesh1_zl1","2Dz/mesh1_zl2","2Dz/mesh1_phzl","2Dz/mesh1_phzl2", "2Dz/mesh1g_zl2"):
    DeleteAllPlots()
    AddPlot("Mesh", meshname)
    DrawPlots()
    ResetView()
    Test("silo_%d"%testNum)
    testNum = testNum + 1

ma = MeshAttributes()
ma.showInternal = 1
ma.opaqueMode = ma.On
for meshname in ("3D/mesh1", "3D/mesh2", "3D/mesh2g", "3D/mesh3", "3D/mesh3r"):
    DeleteAllPlots()
    AddPlot("Mesh", meshname)
    SetPlotOptions(ma)
    DrawPlots()
    ResetView()
    v = GetView3D()
    v.RotateAxis(1,-30)
    v.RotateAxis(0,20)
    SetView3D(v)
    Test("silo_%d"%testNum)
    testNum = testNum + 1

for varname in ("2D/n11", "2D/n12", "2D/n12g", "2D/n1phzl", "2D/n1phzl2",
                "2D/z11", "2D/z12", "2D/z12g", "2D/z1phzl", "2D/z1phzl2"):
    DeleteAllPlots()
    AddPlot("Pseudocolor", varname)
    DrawPlots()
    ResetView()
    v = GetView2D()
    v.windowCoords=(-1, 10, -1, 2)
    SetView2D(v)
    Test("silo_%d"%testNum)
    testNum = testNum + 1

for varname in ("2Dz/n11", "2Dz/n12", "2Dz/n12g", "2Dz/n1phzl", "2Dz/n1phzl2",
                "2Dz/z11", "2Dz/z12", "2Dz/z12g", "2Dz/z1phzl", "2Dz/z1phzl2"):
    DeleteAllPlots()
    AddPlot("Pseudocolor", varname)
    DrawPlots()
    ResetView()
    Test("silo_%d"%testNum)
    testNum = testNum + 1

for varname in ("3D/n1", "3D/n2", "3D/n2g", "3D/n3", "3D/n3r",
                "3D/z1", "3D/z2", "3D/z2g", "3D/z3", "3D/z3r"):
    DeleteAllPlots()
    AddPlot("Pseudocolor", varname)
    DrawPlots()
    ResetView()
    v = GetView3D()
    v.RotateAxis(1,-30)
    v.RotateAxis(0,20)
    SetView3D(v)
    Test("silo_%d"%testNum)
    testNum = testNum + 1


DeleteAllPlots()
CloseDatabase(silo_data_path("arbpoly-zoohybrid.silo"))

TestSection("Scalar and Vector ASCII Label Plots")

OpenDatabase(silo_data_path("rect2d.silo"))
AddPlot("Label", "ascii")
DrawPlots()
ResetView()
v = GetView2D()
v.windowCoords = (0.62, 0.71, 0.23, 0.32)
SetView2D(v)
Test("silo_%d"%testNum)
testNum = testNum + 1

DeleteAllPlots()
AddPlot("Label", "asciiw")
DrawPlots()
ResetView()
v = GetView2D()
v.windowCoords = (0.62, 0.71, 0.23, 0.32)
SetView2D(v)
Test("silo_%d"%testNum)
testNum = testNum + 1

DeleteAllPlots()
CloseDatabase(silo_data_path("rect2d.silo"))

TestSection("Curve with polar coordinates")

OpenDatabase(silo_data_path("multi_rect2d.silo"))
AddPlot("Curve","circle")
DrawPlots()
ResetView()
Test("silo_%d"%testNum)
testNum = testNum + 1

Exit()
