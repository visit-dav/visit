# ---------------------------------------------------------------------------- 
#  CLASSES: nightly
#
#  Test Case:  pick.py #
#  Tests:      queries     - Pick
#
#  Defect ID:  none
#
#  Programmer: Kathleen Bonnell 
#  Date:       May 13, 2003 
#
#  Modifications:
#    Kathleen Bonnell, Fri Jun 27 16:03:57 PDT 2003
#    Modified PickFilledBoundary (removed MeshPlot) so that test is preformed
#    on FilledBoundaryPlot and not MeshPlot.
#
#    Kathleen Bonnell, Tue Jul  8 21:17:17 PDT 2003 
#    Modified PickFilledBoundary to use flip of slice so that it still works
#    with recent slice changes. 
#
#    Kathleen Bonnell, Thu Aug 28 14:34:57 PDT 2003
#    Remove compound var name from Subset and FilledBoundary plots.
#
#    Kathleen Bonnell, Wed Sep 10 11:36:32 PDT 2003 
#    Added test for multi-block meshes. 
#
#    Kathleen Bonnell, Fri Oct 10 17:43:46 PDT 2003 
#    Added test for surface plots.
#
#    Kathleen Bonnell, Fri Oct 24 16:09:33 PDT 2003 
#    Added test for PC of BigSil, and pickvar = material. 
#
#    Kathleen Bonnell, Wed Nov  5 14:30:35 PST 2003
#    Per VisIt000033891, add another onion peel test, where there is only
#    1 cell, and window is set to use Actual extents. 
#
#    Kathleen Bonnell, Thu Nov 13 12:07:20 PST 2003 
#    Use SetOperatorOptions instead of SetDefaultOperatorOption in OnionPeel, so 
#    that OnionPeel2 will work correctly.  Added vars to OnionPeel2. 
#
#    Kathleen Bonnell, Tue Nov 18 17:23:43 PST 2003 
#    Added use of PickAtts' useNodeCoords, logicalCoords and logicalZone for
#    a few tests. 
#    
#    Kathleen Bonnell, Thu Nov 20 16:29:45 PST 2003
#    Added PicSpecMix tests, for species vars, (VisIt '3864).
#    Added a 'vel' only test to PickMatFracsThreshold (VisIt '4103).
#
#    Kathleen Bonnell, Wed Nov 26 11:26:04 PST 2003
#    Added calls to ResetPickLetter after each test, so that each test's
#    pick letter starts out with 'A'.  This will prevent the necessity of
#    updating all pick baselines when modifications are made to only one test.
#    Added tests for new pick methods: PickByZone, PickByNode. ('4123)
#   
#    Kathleen Bonnell, Wed Dec  3 07:51:52 PST 2003 
#    Added PickCurve tests, for curve plots. (VisIt '3832).
#
#    Kathleen Bonnell, Mon Mar  8 15:47:52 PST 2004 
#    Added more tests  to TestSpecMix, to verify that we get same results
#    regardless of whether 'species' is the active variable, or some other
#    scalar is the active variale.
#
#    Kathleen Bonnell, Tue Mar 16 16:26:17 PST 2004 
#    Added bad variables and expression variable tests to PickMultiBlock, 
#    from bug '4571.
#
#    Kathleen Bonnell, Thu Apr  1 09:21:22 PST 2004
#    Added  a "Variable by Zone" and "Variable by Node" Query to PickSpecMix
#    in order to test that these queries return the same values as their
#    Pick counterparts. 
#
#    Kathleen Bonnell, Tue Apr 20 09:42:30 PDT 2004 
#    Added PickMultiWindow. 
#
#    Kathleen Bonnell, Thu Apr 29 07:40:58 PDT 2004 
#    Slice defaults atts have changed, update accordingly. 
#
#    Brad Whitlock, Mon May 3 15:31:40 PST 2004
#    I added a couple of tests that test how pick works if an engine is closed
#    or if one crashes.
#
#    Kathleen Bonnell, Tue May  4 14:35:08 PDT 2004 
#    Changed a couple of screen coordinates for PickBigSilMat that were 
#    invalid.  
#    
#    Kathleen Bonnell, Wed Jun  2 10:14:58 PDT 2004 
#    Added tests to Pick2D to tests PickByNode and PickByZone on a dataset
#    with ghost zones.  Added test to PickSurface, tests various flavors
#    of pick to see if they are returning the same info for the same input.
#
#    Kathleen Bonnell, Thu Jul  8 16:30:07 PDT 2004 
#    Added  PickedBoundary, to test picking on 2D Boundary plots and
#    3D Sliced-to-2D Boundary plots with unstructured, rectilinear and
#    curvilinear data.
#
#    Jeremy Meredith, Wed Jul 21 08:59:24 PDT 2004
#    Making print output go to stderr.
#
#    Kathleen Bonnell, Thu Jul 22 08:35:45 PDT 2004 
#    Added test of data designated as 'ascii' to Pick2D (using rect2d). 
#
#    Kathleen Bonnell, Wed Aug 11 09:04:56 PDT 2004 
#    Added to Pick3DTo2D, a test of data sliced along boundary between zones,
#    to verify that picking with all materials ON returns same results as
#    with some materials OFF. ('5279) 
#
#    Brad Whitlock, Wed Aug 25 12:30:45 PDT 2004
#    Added TurnMaterialsOn to PickMultiBlock since it was remembering the
#    material selection for mulit_rect3d.silo from an earlier pick test.
#
#    Kathleen Bonnell, Wed Sep  8 09:36:30 PDT 2004 
#    Renamed 'WorldPick' to 'Pick' and 'WorldNodePick' to 'NodePick'. 
# 
#    Kathleen Bonnell, Fri Oct 22 16:11:41 PDT 2004 
#    Added PickVectorPlots.
# 
#    Kathleen Bonnell, Tue Nov  2 10:16:36 PST 2004 
#    Added PickPointMeshes.
# 
#    Kathleen Bonnell, Wed Dec 15 15:38:41 PST 2004 
#    Added PickGlobalIds.
# 
#    Kathleen Bonnell, Thu Dec 16 17:16:33 PST 2004 
#    Added PickByNode and PickByZone to PickGlobalIds. 
# 
#    Kathleen Bonnell, Mon Dec 20 10:49:59 PST 2004 
#    Removed  "Variable by Zone" and "Variable by Node" Query from PickSpecMix
#    as they are no longer supported. 
#
#    Kathleen Bonnell, Thu Feb  3 09:27:22 PST 2005 
#    Added PickExpressions, testing matvf and mirvf per '5799. 
#
#    Kathleen Bonnell, Tue Mar  1 13:11:31 PST 2005 
#    Added couple more tests to PickExpressions, testing Expression that
#    has not been plotted, per '5418. 
#
#    Brad Whitlock, Wed Mar 9 09:15:30 PDT 2005
#    Removed deprecated functions.
#
#    Kathleen Bonnell, Wed May 11 18:04:12 PDT 2005 
#    Per ticket '6097, added tests to 'PickGlobalIds' to ensure global ids
#    returned correctly for global zone/node picks. 
#
#    Kathleen Bonnell, Mon May 23 13:41:50 PDT 2005 
#    Per ticket '6217, added tests to 'PickFilledBoundary' to ensure node
#    pick of sliced FB plot returns correct coords. 
#
#    Kathleen Bonnell, Fri Jun 10 14:38:27 PDT 2005
#    Per ticket '6296 -- added tests to PickVectorPlots to ensure that
#    pick of vector expression won't causes crash
#
#    Kathleen Bonnell, Thu Jun 30 15:39:03 PDT 2005
#    Added tests for AMR data. 
# 
#    Kathleen Bonnell, Fri Jul  8 14:12:03 PDT 2005
#    Added more tests for AMR data: PickSamrai. 
# 
#    Kathleen Bonnell, Wed Aug 10 17:12:50 PDT 2005 
#    Added PickIndexSelect. 
# 
#    Kathleen Bonnell, Tue Aug 30 15:19:34 PDT 2005 
#    Added PickTensors.  '6410
# 
#    Jeremy Meredith, Wed Sep  7 12:06:04 PDT 2005
#    Allowed spaces in variable names.
#
#    Kathleen Bonnell, Tue May  2 08:38:28 PDT 2006 
#    Added PickMili ('7144, '7145).
#
#    Kathleen Bonnell, Tue Aug 29 09:34:17 PDT 2006 
#    Added Pickcontour ('7220).
#
#    Kathleen Bonnell, Thu Oct 26 14:24:45 PDT 2006 
#    Added PickBox ('7566).
#
#    Jeremy Meredith, Mon Jul 14 11:17:29 EDT 2008
#    Allow the sliced vector plot to show all vectors (even ones not from
#    from original cells/nodes) to match the old behavior.
#
#    Kathleen Bonnell, Wed Jul 15 10:22:54 PDT 2009
#    Changed some zone picks to Node picks for PickPointMeshes, should yield
#    same results.  Added Subset plot test for PickPointMeshes.
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Cyrus Harrison, Thu Mar 25 09:57:34 PDT 2010
#    Added call(s) to DrawPlots() b/c of changes to the default plot state 
#    behavior when an operator is added.
#
#    Kathleen Biagas, Wed Mar  7 19:33:48 PST 2012
#    'displayXXX' attributes have been changed to 'showXXX'.
#
#    Kathleen Biagas, Thu Mar  8 13:52:37 PST 2012
#    Added test for picking scatter plots.
#
#    Matt Larsen, Mon Sep 19 10:20:11 PDT 2016
#    Added a test for zone pick highlighting
#
#    Kathleen Biagas, Mon Dec 19 15:45:38 PST 2016
#    For PickSubset, change plot var to 'domains' as Subset plots shouldn't 
#    work for materials.  Change pick var to 'mat1' to preserve baselines.
#
#    Matt Larsen, Wed Sep 6 16:23:12 PST 2017
#    Adding tests for pick by label for mili files.
#
#    Alister Maguire, Tue Sep 26 14:23:09 PDT 2017
#    Added test for pick highlight color. 
#
#    Alister Maguire, Mon Oct 23 10:34:28 PDT 2017
#    Added tests for removing a list of picks. 
#
#    Alister Maguire, Mon Oct 30 15:54:30 PDT 2017
#    Added test for removing a list of labeled picks. 
#
#    Matt Larsen, Wed Feb 28 08:24:31 PDT 2018
#    Turning off all anotations for pick highlight tests 
#
#    Alister Maguire, Thu Aug 16 10:18:56 PDT 2018
#    Added a test for swivel highlight. 
#
#    Alister Maguire, Mon Aug 20 12:45:25 PDT 2018
#    Added a test for node highlight. 
#
#    Alister Maguire, Tue May 21 13:10:05 PDT 2019
#    Updated tests that use mili to adhere to the plugin changes. 
#
#    Alister Maguire, Thu Sep 12 15:54:36 PDT 2019
#    Add test for highlighting a zone picked by global id. 
#
# ----------------------------------------------------------------------------
from __future__ import print_function


RequiredDatabasePlugin(("Boxlib2D","SAMRAI","Mili"))
defaultAtts = GetPickAttributes()

def Pick3DTo2D():
    # Testing pick on 3d plots projected to 2d via Slice and Cone operators
    OpenDatabase(silo_data_path("globe.silo"))

    AddPlot("Pseudocolor", "u")
    AddOperator("Slice")
    slice = SliceAttributes()
    slice.SetOriginPoint(0, 0, 0)
    slice.SetNormal(1, 0, 0)
    slice.SetUpAxis(0, 1, 0)
    slice.project2d = 1
    SetOperatorOptions(slice)
    DrawPlots()

    # Create the variable list.
    vars = ("default")

    # Do some picks.
    Pick(128, 107, vars)
    s = GetPickOutput()
    Pick(200, 207, vars)
    s = s + GetPickOutput()

    # Test new pick methods
    PickByZone(554, vars)
    s = s + GetPickOutput()
    PickByZone(945, vars)
    s = s + GetPickOutput()

    # Turn off some materials, and verify we get 
    #  same results
    TurnMaterialsOff("1")
    Pick(128, 107, vars)
    s = s + GetPickOutput()
    Pick(200, 207, vars)
    s = s + GetPickOutput()
    PickByZone(554, vars)
    s = s + GetPickOutput()
    PickByZone(945, vars)
    s = s + GetPickOutput()
 
    TurnMaterialsOn("1")
    RemoveLastOperator()

    AddOperator("Cone")
    DrawPlots()
    ResetView()
    Pick(165, 170, vars)
    s  = s + GetPickOutput()
    Pick(137, 100, vars)
    s = s + GetPickOutput()

    # Test new Pick methods.
    PickByZone(503, vars)
    s  = s + GetPickOutput()
    PickByZone(596, vars)
    s = s + GetPickOutput()

    DeleteAllPlots()
    
    # VisIt00005279
    #  When sliced along boundary between zones,
    #  pick with all materials returns different output than
    #  pick with some materials turned off.
    OpenDatabase(silo_data_path("multi_rect3d.silo"))

    AddPlot("Pseudocolor", "d")
    AddOperator("Slice")
    slice = SliceAttributes()
    slice.axisType = slice.XAxis
    #set the slice plane on a boundary between zones
    slice.originIntercept = 0.2
    slice.project2d = 1
    SetOperatorOptions(slice)
    DrawPlots()
   
    s = s + "\nPick with all materials turned on:\n"
    Pick(80, 60)
    s = s + GetPickOutput()
    PickByZone(226, 1)
    s = s + GetPickOutput()
    Pick(185, 160)
    s = s + GetPickOutput()
    PickByZone(606, 19)
    s = s + GetPickOutput()
    NodePick(80, 60)
    s = s + GetPickOutput()
    PickByNode(402, 1)
    s = s + GetPickOutput()
    NodePick(185, 160)
    s = s + GetPickOutput()
    PickByNode(853, 19)
    s = s + GetPickOutput()
    TurnMaterialsOff("2")
    s = s + "\nPick with material 2 turned off:\n"
    Pick(80, 60)
    s = s + GetPickOutput()
    PickByZone(226, 1)
    s = s + GetPickOutput()
    Pick(185, 160)
    s = s + GetPickOutput()
    PickByZone(606, 19)
    s = s + GetPickOutput()
    NodePick(80, 60)
    s = s + GetPickOutput()
    PickByNode(402, 1)
    s = s + GetPickOutput()
    NodePick(185, 160)
    s = s + GetPickOutput()
    PickByNode(853, 19)
    s = s + GetPickOutput()
    TurnMaterialsOn("2")
    TestText("Pick3DTo2D", s)
    DeleteAllPlots()
    ResetPickLetter()

def Pick2D():
    OpenDatabase(silo_data_path("ucd2d.silo"))

    AddPlot("Pseudocolor", "u")
    DrawPlots()
    ResetView()
    vars = ("d", "p", "u")
    Pick(170, 65, vars)
    s = GetPickOutput()
    Pick(140, 161, vars)
    s = s + GetPickOutput()

    # Test new pick method
    PickByZone(5, vars)
    s = s + GetPickOutput()
    PickByZone(4, vars)
    s = s + GetPickOutput()

    DeleteAllPlots()

    # Test PickByZone and PickByNode on data with ghost zones.
    OpenDatabase(silo_data_path("curv2d.silo"))

    AddPlot("Pseudocolor", "d")
    DrawPlots()

    ResetView()

    v = GetView2D()
    v.windowCoords = (-0.70073, 0.9927, 1.75365, 2.63085)
    v.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    SetView2D(v)

    vars = ("default")
    Pick(177, 90, vars)
    s = s + GetPickOutput()
    PickByZone(13)
    s = s + GetPickOutput()
    NodePick(177, 90, vars)
    s = s + GetPickOutput()
    PickByNode(13)
    s = s + GetPickOutput()

    ResetView()

    Pick(164, 136)
    s = s + GetPickOutput()
    PickByZone(716)
    s = s + GetPickOutput()

    NodePick(164, 136)
    s = s + GetPickOutput()
    PickByNode(744)
    s = s + GetPickOutput()

    Pick(113, 84)
    s = s + GetPickOutput()
    PickByZone(360)
    s = s + GetPickOutput()

    NodePick(113, 84)
    s = s + GetPickOutput()
    PickByNode(374)
    s = s + GetPickOutput()

    Pick(218, 65)
    s = s + GetPickOutput()
    PickByZone(81)
    s = s + GetPickOutput()

    NodePick(218, 65)
    s = s + GetPickOutput()
    PickByNode(85)
    s = s + GetPickOutput()

    DeleteAllPlots()

    # bug '5152, ascii data not reported as such.
    OpenDatabase(silo_data_path("rect2d.silo"))

    AddPlot("Pseudocolor", "ascii")
    DrawPlots()

    ResetView()

    Pick(104, 224) 
    s = s + GetPickOutput()
    Pick(155, 140)
    s = s + GetPickOutput()
    Pick(234, 278)
    s = s + GetPickOutput()
    Pick(151, 266)
    s = s + GetPickOutput()
    Pick(187, 106)
    s = s + GetPickOutput()

    NodePick(104, 224) 
    s = s + GetPickOutput()
    NodePick(155, 140)
    s = s + GetPickOutput()
    NodePick(234, 278)
    s = s + GetPickOutput()
    NodePick(151, 266)
    s = s + GetPickOutput()
    NodePick(187, 106)
    s = s + GetPickOutput()

    TestText("Pick2D", s)
    DeleteAllPlots()
    ResetPickLetter()

def PickOnionPeel():
    # Testing pick on 3d plots with OnionPeel operator
    OpenDatabase(silo_data_path("multi_ucd3d.silo"))

    AddPlot("Pseudocolor", "u")
    AddOperator("OnionPeel")
    op = OnionPeelAttributes()
    op.adjacencyType = 0
    op.categoryName = "domains"
    op.subsetName = "domain4"
    op.index = 15
    op.requestedLayer = 0
    SetOperatorOptions(op)

    DrawPlots()

    v = GetView3D()
    v.viewNormal = (0.0199972, -0.0176736, 0.999644)
    v.focus = (1.05702, 2.19945, 3.66667)
    v.viewUp = (-0.0156863, 0.999715, 0.0179886)
    v.viewAngle = 30
    v.parallelScale = 0.794807
    v.nearPlane = -7.9865
    v.farPlane = 7.9865
    v.perspective = 0
    SetView3D(v)

    #From defect VisIt00002762, pick shouldn't report repeated vars
    vars = ("default", "d", "p", "u", "p", "v")
    pa = GetPickAttributes()
    pa.showNodePhysicalCoords = 1
    SetPickAttributes(pa)
    Pick(271, 64, vars)
    s = GetPickOutput()
    Pick(282, 53, vars)
    s = s + GetPickOutput()

    PickByZone(15, 4, vars)
    s = s + GetPickOutput()
 
    TestText("PickOnionPeel", s)
    pa.showNodePhysicalCoords = 0
    SetPickAttributes(pa)
    DeleteAllPlots()
    ResetPickLetter()

def PickSubset():
    #Testing Pick's accuracy on a sil-restricted plot.
    #All the picks in this test should return zone 30.
    #From defect VisIt00003348
    OpenDatabase(silo_data_path("globe.silo"))

    AddPlot("Subset", "domains")
    TurnMaterialsOff()
    TurnMaterialsOn("1")
 
    DrawPlots()

    v = GetView3D()
    v.viewNormal = (-0.997376, 0.0429928, 0.058241)
    v.focus = (-0.0906961, 0.0532744, -1.5925)
    v.viewUp = (0.0448435, 0.998518, 0.0308499)
    v.viewAngle = 30
    v.parallelScale = 0.598088
    v.nearPlane = -34.641
    v.farPlane = 34.641
    v.perspective = 0
    SetView3D(v)

    vars = ("mat1")
    Pick(173, 111, vars)
    s = GetPickOutput()
    Pick(191, 116, vars)
    s = s + GetPickOutput()
    Pick(294, 125, vars)
    s = s + GetPickOutput()
    Pick(156, 128, vars)
    s = s + GetPickOutput()
    Pick(147, 112, vars)
    s = s + GetPickOutput()
    TestText("PickSubset", s)
    TurnMaterialsOn()
    DeleteAllPlots()
    ResetPickLetter()
 
 
def PickMatFracsThreshold():
    # Testing pick on 3d plot with Threshold operator, returning
    # material var (mat fracs) and vector var. 
    OpenDatabase(silo_data_path("rect3d.silo"))

    AddPlot("Pseudocolor", "p")
    thresh = ThresholdAttributes()
    thresh.upperBounds = (37.0)
    thresh.lowerBounds = (10.0)
    SetDefaultOperatorOptions(thresh)
    AddOperator("Threshold")
    DrawPlots()
 
    v = GetView3D()
    v.viewNormal = (0, 0, 1)
    v.focus = (0.5, 0.5, 0.5)
    v.viewUp = (0, 1, 0)
    v.viewAngle = 30
    v.parallelScale = 0.181485
    v.nearPlane = -1.73205
    v.farPlane = 1.73205
    v.perspective = 0
    SetView3D(v) 

    pa = GetPickAttributes()
    pa.showNodeDomainLogicalCoords = 1
    pa.showZoneDomainLogicalCoords = 1
    SetPickAttributes(pa)
    vars = ("mat1", "vel")
    Pick(151, 172, vars)
    s = GetPickOutput()
    Pick(211, 185, vars)
    s = s + GetPickOutput()
    Pick(78, 153, vars)
    s = s + GetPickOutput()
    Pick(150, 124, vars)
    s = s + GetPickOutput()

    #Test new pick method
    PickByZone(21045, vars)
    s = s + GetPickOutput()
    PickByZone(18647, vars)
    s = s + GetPickOutput()
    PickByZone(19812, vars)
    s = s + GetPickOutput()
    PickByZone(20954, vars)
    s = s + GetPickOutput()

    pa.showNodeDomainLogicalCoords = 0
    pa.showZoneDomainLogicalCoords = 0
    SetPickAttributes(pa)

    # Add a few with only vel as the var 
    # From bug: VisIt00004103
    RemoveLastOperator()
    vars = ("vel")
    Pick(78, 153, vars)
    s = s + GetPickOutput()
    Pick(150, 124, vars)
    s = s + GetPickOutput()

    #Test new pick method
    PickByZone(35412, vars)
    s = s + GetPickOutput()
    PickByZone(35354, vars)
    s = s + GetPickOutput()
 
    TestText("PickMatFracsThreshold", s)
    DeleteAllPlots()
    ResetPickLetter()

def PickReflect():
    # Testing pick on plot with Reflect operator.
    OpenDatabase(silo_data_path("curv2d.silo"))

    AddPlot("Pseudocolor", "u")
    AddOperator("Reflect")
    DrawPlots()
    ResetView()
    vars = ("default")
    Pick(149, 206, vars)
    s = GetPickOutput()
    Pick(149, 94, vars)
    s = s + GetPickOutput()
    NodePick(coord=(0, 3, 0))
    s = s + GetPickOutput()
    NodePick(coord=(0, -3, 0))
    s = s + GetPickOutput()

    DeleteAllPlots()

    # Try a rectilinear grid
    OpenDatabase(silo_data_path("noise.silo"))
    AddPlot("Subset", "Mesh")
    AddOperator("Reflect")
    DrawPlots()
    ResetView()

    Pick(coord=(-5, 5, 10))
    s = s + GetPickOutput()
    NodePick(coord=(-5, 5, 10))
    s = s + GetPickOutput()
    Pick(coord=(-5, -25, 10))
    s = s + GetPickOutput()
    NodePick(coord=(-5, -25, 10))
    s = s + GetPickOutput()


    TestText("PickReflect", s)
    DeleteAllPlots()
    ResetPickLetter()


def PickFilledBoundary():
    OpenDatabase(silo_data_path("multi_ucd3d.silo"))

    AddPlot("FilledBoundary", "mat1")

    AddOperator("Slice")
    s= SliceAttributes() 
    s.originIntercept = 2.89833
    s.originType = s.Intercept
    s.normal = (0, 1, 0)
    s.upAxis = (1, 0, 0)
    s.axisType = s.YAxis
    s.flip = 1
    s.project2d = 1
    SetOperatorOptions(s)

    v = GetView2D()
    v.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v.windowCoords = (7.07307, 8.30598, 2.26402, 2.88047)
    SetView2D(v)
    DrawPlots()

    vars = ("default")
    Pick(153, 69, vars)
    p = GetPickOutput()
    Pick(222, 56, vars)
    p += GetPickOutput()
    Pick(118, 77, vars)
    p += GetPickOutput()
    Pick(141, 74, vars)
    p += GetPickOutput()
    Pick(180, 68, vars)
    p += GetPickOutput()
    Pick(224, 75, vars)
    p += GetPickOutput()
    Pick(198, 56, vars)
    p += GetPickOutput()
    Pick(113, 70, vars)
    p += GetPickOutput()
    Pick(119, 57, vars)
    p += GetPickOutput()
    Pick(157, 66, vars)
    p += GetPickOutput()
    Pick(199, 66, vars)
    p += GetPickOutput()
    Pick(227, 79, vars)
    p += GetPickOutput()

    # Test new pick method
    PickByZone(320, 17, vars)
    p += GetPickOutput()

    DeleteAllPlots()

    # bug 6217 -- NodePick on Sliced FB returning wrong coords.
    # will test with Mesh, OnionPeeled Mesh and FB to verify
    # they all return the same nodes/zones and coords.
    OpenDatabase(silo_data_path("globe.silo"))

    AddPlot("Mesh", "mesh1")
    AddPlot("Mesh", "mesh1")
    m = MeshAttributes()
    m.meshColor = (255, 255, 0, 255)
    m.foregroundFlag = 0
    m.lineWidth = 2
    SetPlotOptions(m)
    AddOperator("OnionPeel", 0)
    op = OnionPeelAttributes()
    op.index = 679
    SetOperatorOptions(op)

    AddPlot("FilledBoundary","mat1")
    AddOperator("Slice", 1)
    DrawPlots()
    ResetView()

    SetActivePlots(0)
    Pick(247, 188, vars)
    p = p + GetPickOutput()
    NodePick(247, 188, vars)
    p = p + GetPickOutput()

    SetActivePlots(1)
    Pick(247, 188, vars)
    p = p + GetPickOutput()
    NodePick(247, 188, vars)
    p = p + GetPickOutput()
    SetActivePlots(2)
    Pick(247, 188, vars)
    p = p + GetPickOutput()
    NodePick(247, 188, vars)
    p = p + GetPickOutput()
    TestText("PickFilledBoundary", p)
    DeleteAllPlots()
    ResetPickLetter()

def PickMultiBlock():
    # Testing pick on 3d multi-domain plots 
    OpenDatabase(silo_data_path("multi_ucd3d.silo"))

    AddPlot("Pseudocolor", "u")

    DrawPlots()
    ResetView()
    v = GetView3D()
    v.perspective = 1
    SetView3D(v)
    
    vars = ("default", "d", "p") 
    Pick(109, 113, vars)
    s = GetPickOutput()
    Pick(74, 114, vars)
    s = s + GetPickOutput()
    Pick(192, 114, vars)
    s = s + GetPickOutput()
    Pick(230, 114, vars)
    s = s + GetPickOutput()
    Pick(155, 148, vars)
    s = s + GetPickOutput()
    Pick(153, 185, vars)
    s = s + GetPickOutput()
    Pick(106, 149, vars)
    s = s + GetPickOutput()
    Pick(196, 144, vars)
    s = s + GetPickOutput()
    Pick(119, 157, vars)
    s = s + GetPickOutput()
    Pick(180, 160, vars)
    s = s + GetPickOutput()
    Pick(97, 130, vars)
    s = s + GetPickOutput()
    Pick(212, 129, vars)
    s = s + GetPickOutput()


    # Test new pick method
    PickByZone(15, 34, vars)
    s = s + GetPickOutput()
    PickByZone(15, 36, vars)
    s = s + GetPickOutput()
    PickByZone(75, 25, vars)
    s = s + GetPickOutput()
    PickByZone(83, 27, vars)
    s = s + GetPickOutput()
    PickByZone(15, 28, vars)
    s = s + GetPickOutput()
    PickByZone(4, 30, vars)
    s = s + GetPickOutput()
    PickByZone(94, 35, vars)
    s = s + GetPickOutput()
    PickByZone(15, 26, vars)
    s = s + GetPickOutput()
    PickByZone(25, 32, vars)
    s = s + GetPickOutput()
    PickByZone(64, 29, vars)
    s = s + GetPickOutput()
    PickByZone(56, 35, vars)
    s = s + GetPickOutput()
    PickByZone(51, 26, vars)
    s = s + GetPickOutput()

    DeleteAllPlots()

    OpenDatabase(silo_data_path("multi_rect3d.silo"))

    AddPlot("Mesh", "mesh1")
    TurnMaterialsOn()
    DrawPlots()
    ResetView()
    vars = ("default", "u", "d", "p") 
    Pick(61, 61, vars)
    s = s + GetPickOutput()
    Pick(61, 188, vars)
    s = s + GetPickOutput()
    Pick(79, 233, vars)
    s = s + GetPickOutput()
    Pick(80, 64, vars)
    s = s + GetPickOutput()
    Pick(99, 120, vars)
    s = s + GetPickOutput()
    Pick(115, 236, vars)
    s = s + GetPickOutput()
    Pick(154, 194, vars)
    s = s + GetPickOutput()
    Pick(155, 150, vars)
    s = s + GetPickOutput()
    Pick(174, 72, vars)
    s = s + GetPickOutput()
    Pick(192, 115, vars)
    s = s + GetPickOutput()
    Pick(229, 235, vars)
    s = s + GetPickOutput()
    Pick(236, 59, vars)
    s = s + GetPickOutput()
    Pick(246, 157, vars)
    s = s + GetPickOutput()

    # Test new pick method
    PickByZone(921, 25, vars)
    s = s + GetPickOutput()
    PickByZone(971, 31, vars)
    s = s + GetPickOutput()
    PickByZone(964, 34, vars)
    s = s + GetPickOutput()
    PickByZone(934, 25, vars)
    s = s + GetPickOutput()
    PickByZone(947, 28, vars)
    s = s + GetPickOutput()
    PickByZone(969, 34, vars)
    s = s + GetPickOutput()
    PickByZone(985, 32, vars)
    s = s + GetPickOutput()
    PickByZone(995, 29, vars)
    s = s + GetPickOutput()
    PickByZone(948, 26, vars)
    s = s + GetPickOutput()
    PickByZone(931, 30, vars)
    s = s + GetPickOutput()
    PickByZone(966, 36, vars)
    s = s + GetPickOutput()
    PickByZone(927, 27, vars)
    s = s + GetPickOutput()
    PickByZone(919, 33, vars)
    s = s + GetPickOutput()

    DeleteAllPlots()

    OpenDatabase(silo_data_path("bigsil.silo"))

    AddPlot("Pseudocolor", "dist")
    DrawPlots()
    ResetView()
    v = GetView3D()
    v.viewNormal= (1, 0, 0) 
    SetView3D(v)

    vars = ("default")
    Pick(250, 64, vars)
    s = s + GetPickOutput()
    Pick(149, 135, vars)
    s = s + GetPickOutput()
    Pick(227, 72, vars)
    s = s + GetPickOutput()
    Pick(65, 65, vars)
    s = s + GetPickOutput()
    Pick(67, 228, vars)
    s = s + GetPickOutput()

    # Test new pick method
    PickByZone(13, 3, vars)
    s = s + GetPickOutput()
    PickByZone(1, 15, vars)
    s = s + GetPickOutput()
    PickByZone(118, 3, vars)
    s = s + GetPickOutput()
    PickByZone(258, 21, vars)
    s = s + GetPickOutput()
    PickByZone(279, 27, vars)
    s = s + GetPickOutput()

    #Bug '4571, pick crashing when bad vars specified
    vars = ("aspect")
    Pick(67, 228, vars)
    s = s + GetPickOutput()
    vars = ("mesh_quality/aspect", "spurious")
    Pick(67, 228, vars)
    s = s + GetPickOutput()

    TestText("PickMultiBlock", s)
    DeleteAllPlots()
    ResetPickLetter()

def PickSurface():
    OpenDatabase(silo_data_path("rect3d.silo"))

    AddPlot("Surface", "d")
    AddOperator("Slice")
    slice = SliceAttributes()
    slice.originIntercept = 0.5
    slice.project2d = 1
    slice.axisType = slice.YAxis
    SetOperatorOptions(slice)
    DrawPlots()
    ResetView()
    v = GetView3D()
    v.viewNormal = (0, 1, 0)
    v.viewUp = (0, 0, -1)
    SetView3D(v)

    pa = GetPickAttributes()
    pa.variables = ("default")
    SetPickAttributes(pa)

    Pick(141, 178)
    s = GetPickOutput()
    NodePick(141, 178)
    s = s + GetPickOutput()
    Pick(194, 144)
    s = s + GetPickOutput()
    NodePick(194, 144)
    s = s + GetPickOutput()
    Pick(149, 197)
    s = s + GetPickOutput()
    NodePick(149, 197)
    s = s + GetPickOutput()
    Pick(64, 102)
    s = s + GetPickOutput()
    NodePick(64, 102)
    s = s + GetPickOutput()
    Pick(138, 126)
    s = s + GetPickOutput()
    NodePick(138, 126)
    s = s + GetPickOutput()
  
    DeleteAllPlots()

    OpenDatabase(silo_data_path("curv2d.silo"))

    AddPlot("Surface", "p")
    DrawPlots()
    ResetView()
    v = GetView3D()
    v.viewNormal = (0, 1, 0)
    v.viewUp = (0, 0, -1)
    SetView3D(v)

    Pick(103, 227)
    s = s + GetPickOutput()
    NodePick(103, 227)
    s = s + GetPickOutput()
    Pick(171, 86)
    s = s + GetPickOutput()
    NodePick(171, 86)
    s = s + GetPickOutput()
    Pick(129, 63)
    s = s + GetPickOutput()
    NodePick(129, 63)
    s = s + GetPickOutput()
    Pick(153, 177)
    s = s + GetPickOutput()
    NodePick(153, 177)
    s = s + GetPickOutput()
    Pick(188, 167)
    s = s + GetPickOutput()
    NodePick(188, 167)
    s = s + GetPickOutput()

    DeleteAllPlots() 

    OpenDatabase(silo_data_path("ucd2d.silo"))

    AddPlot("Surface", "d")
    DrawPlots()
    ResetView()
    v = GetView3D()
    v.viewNormal = (0.329152, -0.014518, -0.944165)
    v.focus = (2, 2, 2)
    v.viewUp = (-0.944025, -0.0281555, -0.32867)
    SetView3D(v)

    s = s + "\nThe next three results should be the same.\n"
    Pick(118, 112)
    s = s + GetPickOutput()
    PickByZone(3)
    s = s + GetPickOutput()
    Pick((2.74917, 2.79753, 2.82784))

    s = s + GetPickOutput()

    s = s + "\nThe next three results should be the same.\n"
    NodePick(118, 112)
    s = s + GetPickOutput()
    PickByNode(11)
    s = s + GetPickOutput()
    NodePick((2.5, 2.5, 1.41421))
    s = s + GetPickOutput()

    s = s + "\nThe next three results should be the same.\n"
    NodePick(139, 73)
    s = s + GetPickOutput()
    PickByNode(4)
    s = s + GetPickOutput()
    NodePick((4.0, 2.0, 4.0))
    s = s + GetPickOutput()

    TestText("PickSurface", s)
    DeleteAllPlots()
    ResetPickLetter()

def PickBigSilMat():
    OpenDatabase(silo_data_path("bigsil.silo"))

    AddPlot("Pseudocolor", "dist")
    DrawPlots()
    ResetView()
    pa = GetPickAttributes()
    pa.showNodePhysicalCoords = 1
    SetPickAttributes(pa)
    vars = "mat"
    Pick(87, 215, vars)
    s = GetPickOutput()
    NodePick(87, 215, vars)
    s = s + GetPickOutput()
    Pick(149, 146, vars)
    s = s + GetPickOutput()
    NodePick(149, 146, vars)
    s = s + GetPickOutput()
    Pick(89, 54, vars)
    s = s + GetPickOutput()
    NodePick(89, 54, vars)
    s = s + GetPickOutput()
    Pick(217, 74, vars)
    s = s + GetPickOutput()
    NodePick(217, 74, vars)
    s = s + GetPickOutput()
    Pick(215, 233, vars)
    s = s + GetPickOutput()
    NodePick(215, 233, vars)
    s = s + GetPickOutput()
    Pick(186, 193, vars)
    s = s + GetPickOutput()
    NodePick(186, 193, vars)
    s = s + GetPickOutput()
    Pick(190, 106, vars)
    s = s + GetPickOutput()
    NodePick(190, 106, vars)
    s = s + GetPickOutput()
    Pick(117, 173, vars)
    s = s + GetPickOutput()
    NodePick(117, 173, vars)
    s = s + GetPickOutput()
    Pick(98, 86, vars)
    s = s + GetPickOutput()
    NodePick(98, 86, vars)
    s = s + GetPickOutput()
    Pick(185, 116, vars)
    s = s + GetPickOutput()
    NodePick(185, 116, vars)
    s = s + GetPickOutput()

    # Test new pick methods
    PickByZone(319, 25, vars)
    s = s + GetPickOutput()
    PickByNode(476, 25, vars)
    s = s + GetPickOutput()
    PickByZone(318, 23, vars)
    s = s + GetPickOutput()
    PickByNode(475, 23, vars)
    s = s + GetPickOutput()
    PickByZone(311, 21, vars)
    s = s + GetPickOutput()
    PickByNode(475, 21, vars)
    s = s + GetPickOutput()
    PickByZone(332, 27, vars)
    s = s + GetPickOutput()
    PickByNode(491, 27, vars)
    s = s + GetPickOutput()
    PickByZone(294, 27, vars)
    s = s + GetPickOutput()
    PickByNode(456, 27, vars)
    s = s + GetPickOutput()
    PickByZone(329, 21, vars)
    s = s + GetPickOutput()
    PickByNode(497, 21, vars)
    s = s + GetPickOutput()
    PickByZone(329, 23, vars)
    s = s + GetPickOutput()
    PickByNode(496, 23, vars)
    s = s + GetPickOutput()
    PickByZone(329, 19, vars)
    s = s + GetPickOutput()
    PickByNode(485, 19, vars)
    s = s + GetPickOutput()
    PickByZone(294, 24, vars)
    s = s + GetPickOutput()
    PickByNode(448, 24, vars)
    s = s + GetPickOutput()

    TestText("PickBigSilMat", s)
    DeleteAllPlots() 
    pa.showNodePhysicalCoords = 0
    SetPickAttributes(pa)
    ResetPickLetter()

def PickOnionPeel2():
    #From defect VisIt00003981, onionpeel and window ACTUAL_EXTENTS 
    #pick not working
    print(tests_path("queries","pickonionpeel.session"), 0,silo_data_path("curv3d.silo"))
    RestoreSessionWithDifferentSources(tests_path("queries","pickonionpeel.session"), 0,
                                       silo_data_path("curv3d.silo"))
    vars = "default"
    Pick(196, 194, vars)
    s = GetPickOutput()
    Pick(59, 73, vars)
    s = s + GetPickOutput()
    Pick(128, 111, vars)
    s = s + GetPickOutput()
    Pick(50, 90, vars)
    s = s + GetPickOutput()
    Pick(275, 203, vars)
    s = s + GetPickOutput()
    Pick(277, 225, vars)
    s = s + GetPickOutput()
    Pick(52, 109, vars)
    s = s + GetPickOutput()

    op = OnionPeelAttributes()
    op.adjacencyType = op.Face
    op.requestedLayer = 1
    op.index = 1
    SetOperatorOptions(op)

    Pick(52, 105, vars)
    s = s + GetPickOutput()
    Pick(67, 65, vars)
    s = s + GetPickOutput()
    Pick(51, 86, vars)
    s = s + GetPickOutput()
    Pick(265, 210, vars)
    s = s + GetPickOutput()
    Pick(172, 165, vars)
    s = s + GetPickOutput()

    op.index = 317
    op.requestedLayer = 0
    SetOperatorOptions(op)

    Pick(76, 70, vars)
    s = s + GetPickOutput()
    Pick(37, 120, vars)
    s = s + GetPickOutput()
    Pick(119, 174, vars)
    s = s + GetPickOutput()
    Pick(273, 188, vars)
    s = s + GetPickOutput()

    op.adjacencyType = op.Node
    op.requestedLayer = 3
    SetOperatorOptions(op)

    Pick(96, 72, vars)
    s = s + GetPickOutput()
    Pick(27, 144, vars)
    s = s + GetPickOutput()
    Pick(115, 102, vars)
    s = s + GetPickOutput()
    Pick(134, 140, vars)
    s = s + GetPickOutput()
    Pick(288, 185, vars)
    s = s + GetPickOutput()

    TestText("PickOnionPeel2", s)
    DeleteAllPlots()
    ResetPickLetter()

def PickSpecMix():
    #VisIt '3864
    OpenDatabase(silo_data_path("specmix_quad.silo"))

    AddPlot("Pseudocolor", "Species")
    DrawPlots()

    v = GetView2D()
    v.windowCoords = (0, 1, 0, 1)
    SetView2D(v)

    vars = ("default")
    Pick(69, 137, vars)
    s = GetPickOutput()
    NodePick(90, 119, vars)
    s = s + GetPickOutput()
    Pick(139, 142, vars)
    s = s + GetPickOutput()
    NodePick(170, 126, vars)
    s = s + GetPickOutput()
    Pick(201, 95, vars)
    s = s + GetPickOutput()
    Pick(78, 75, vars)
    s = s + GetPickOutput()
    Pick(256, 169, vars)
    s = s + GetPickOutput()

    # Test new pick method
    PickByZone(233, vars)
    s = s +GetPickOutput()
    PickByNode(266, vars)
    s = s +GetPickOutput()
 
    silr = SILRestriction()
    onoff = 0
    for sets in silr.SetsInCategory('Species'):
        if(onoff == 0):
            silr.TurnOffSet(sets)
            onoff = 1
        else:
            onoff = 0
    SetPlotSILRestriction(silr)

    NodePick(140, 197, vars)
    s = s + GetPickOutput()
    Pick(249, 112, vars)
    s = s + GetPickOutput()
    NodePick(125, 137, vars)
    s = s + GetPickOutput()
    Pick(191, 136, vars)
    s = s + GetPickOutput()
    vars = ("d", "Species")
    NodePick(204, 135, vars)
    s = s + GetPickOutput()
    Pick(138, 118, vars)
    s = s + GetPickOutput()
    NodePick(104, 139, vars)
    s = s + GetPickOutput()
    Pick(94, 151, vars)
    s = s + GetPickOutput()
    NodePick(182, 99, vars)
    s = s + GetPickOutput()
    # now test with 'd' as the active plot variable, should
    # get the same results.
    ChangeActivePlotsVar("d")
    NodePick(204, 135, vars)
    s = s + GetPickOutput()
    Pick(138, 118, vars)
    s = s + GetPickOutput()
    NodePick(104, 139, vars)
    s = s + GetPickOutput()
    Pick(94, 151, vars)
    s = s + GetPickOutput()

    NodePick(182, 99, vars)
    s = s + GetPickOutput()

    TestText("PickSpecMix", s)
    DeleteAllPlots()
    ResetPickLetter()

def PickCurve():
    OpenDatabase(data_path("curve_test_data/curve.visit"))

    AddPlot("Curve", "parabolic")
    SetTimeSliderState(72)
    DrawPlots()

    v = GetViewCurve()
    v.rangeCoords = (0, 1)
    SetViewCurve(v)

    pa = GetPickAttributes()
    pa.variables = ("default")
    SetPickAttributes(pa)
 
    Pick(72, 53)
    p = GetPickOutput()
    Pick(156, 22)
    p = p + GetPickOutput()
    Pick(15, 128)
    p = p + GetPickOutput()
    Pick(212, 200)
    p = p + GetPickOutput()
    Pick((0.2, 0.7))
    p = p + GetPickOutput()
    Pick((0.8, 0.35))
    p = p + GetPickOutput()
    Pick((0.6, 0.6))
    p = p + GetPickOutput()
    Pick((0.3, 0.5))
    p = p + GetPickOutput()

    TestText("PickCurve", p)
    ResetPickLetter()
    DeleteAllPlots()

def PickMultiWindow():
    OpenDatabase(silo_data_path("wave*.silo database"))

    AddPlot("FilledBoundary", "Material")
    SetTimeSliderState(31)
    DrawPlots()

    v = GetView3D()
    v.viewNormal = (0, -1, 0)
    v.viewUp = (0, 0, 1)
    SetView3D(v)

    CloneWindow()
    SetTimeSliderState(64)
    DrawPlots()

    pt1a = (7.20585, 0.099664, 2.5)
    pt1b = (5.867554, 0.264801,  3.566963)

    pt2a = (7.20585, 0.099664, 2.378386)
    pt2b = (5.867554, 0.039652,  3.566963)


    # Same DB, same plot, same var, different ts
    SetActiveWindow(1)
    Pick(pt1a)
    s = GetPickOutput()

    SetActiveWindow(2)
    Pick(pt2a)
    s = s + GetPickOutput()

    SetActiveWindow(1)
    Pick(pt1b)
    s = s + GetPickOutput()

    SetActiveWindow(2)
    Pick(pt2b)
    s = s + GetPickOutput()

    # Same DB, same plot, different var, different ts
    SetActiveWindow(1)
    DeleteAllPlots()
    AddPlot("Pseudocolor", "v")
    DrawPlots()

    Pick(pt1a)
    s = s + GetPickOutput()
    Pick(pt1b)
    s = s + GetPickOutput()

    SetActiveWindow(2)
    DeleteAllPlots()
    AddPlot("Pseudocolor", "pressure")
    DrawPlots()

    Pick(pt2a)
    s = s + GetPickOutput()
    Pick(pt2b)
    s = s + GetPickOutput()

    TestText("PickMultiWindow", s)
    ResetPickLetter()
 
    # delete window 2
    DeleteWindow()
    # remove plots from window 1
    DeleteAllPlots()

#
# Sets up a couple test cases that test how pick interacts with the engine
# being gone.
#
def PickBadEngineSetup(testName, GetRidOfEngineCallback):
    OpenDatabase(silo_data_path("curv3d.silo"))

    AddPlot("Pseudocolor", "u")
    DrawPlots()

    v = View3DAttributes()
    v.viewNormal = (-0.423782, 0.331476, 0.842931)
    v.focus = (0, 2.5, 15)
    v.viewUp = (0.111716, 0.942652, -0.314525)
    v.viewAngle = 30
    v.parallelScale = 16.0078
    v.nearPlane = -32.0156
    v.farPlane = 32.0156
    v.imagePan = (-0.0549008, 0.0570848)
    v.imageZoom = 1.35304
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 0, 0)
    SetView3D(v)

    ptA = (0.115986, 4.290420, 29.9)
    Pick(ptA)
    s = ""
    s = s + GetPickOutput()
    ptB = (0.040697, 2.250070, 29.9)
    Pick(ptB)
    s = s + GetPickOutput()

    # Get rid of the compute engine running on localhost.
    s = s + GetRidOfEngineCallback()

    # Create plots in a new window before returning to window 1 to
    # pick again. This will force the network ids in window 1 to be
    # invalid in case they do not get reset. (But they do get reset now)
    AddWindow()
    SetActiveWindow(2)
    DeleteAllPlots()
    OpenDatabase(silo_data_path("globe.silo"))

    AddPlot("Pseudocolor", "u")
    DrawPlots()
    SetActiveWindow(1)

    # Resume picking in window 1 now that there are networks in window 2.
    ptC = (0.15, 3.250070, 26.9)
    Pick(ptC)
    s = s + GetPickOutput()
    ptD = (0.19, 3.75, 26.9)
    Pick(ptD)
    s = s + GetPickOutput()

    TestText(testName, s)
    ResetPickLetter()
    DeleteAllPlots()
    # Delete window 2.
    SetActiveWindow(2)
    DeleteWindow()

#
# Tests that pick works after an engine has been closed by the user.
#
def PickAfterEngineClosed():
    def CloseCB():
        CloseComputeEngine("localhost")
        return "***\n*** Closed engine on localhost.\n***\n"
    PickBadEngineSetup("PickAfterEngineClose", CloseCB)

#
# Tests that pick works after an engine has crashed.
#
def PickAfterEngineCrashed():
    #TODO: THIS WONT WORK ON WINDOWs
    def CrashCB():
        try:
            # Read the output of ps
            f = os.popen("ps")
            lines = f.readlines()
            f.close()

            # Get a list of engine pids from the ps output.
            pids = []
            for line in lines:
                if string.find(line, "engine_") == -1:
                    continue
                tokens = string.split(line, " ")
                lv = []
                for t in tokens:
                    if t != "":
                        lv = lv + [t]
                if len(lv) > 0:
                   pids = pids + [lv[0]]

            # Kill all of the engines in the pid list.
            if len(pids) < 1:
                print("Killing all engines because we could not get a list of pids", file=sys.stderr)
                # TODO_WINDOWS THIS WONT WORK ON WINDOWS
                os.system("killall engine_ser")
                os.system("killall engine_par")
            else:
                for pid in pids:
                    print("Killing engine %s" % pid, file=sys.stderr)
                    # TODO_WINDOWS THIS WONT WORK ON WINDOWS
                    os.system("kill -9 %s" % pid)
        except:
            # There was an error. Just close the engine instead of killing it.
            CloseComputeEngine("localhost")
        return "***\n*** Killed compute engine(s).\n***\n"
    PickBadEngineSetup("PickAfterEngineCrash", CrashCB)
    
def PickBoundary():
    # VisIt00004807
    # Test Pick on a 2D Boundary plot.

    ResetPickAttributes()

    OpenDatabase(silo_data_path("curv2d.silo"))

    AddPlot("Boundary", "mat1")
    DrawPlots()

    ResetView()
    SetViewExtentsType("original")

    Pick(164, 136)
    s = GetPickOutput()
    PickByZone(482)
    s = s + GetPickOutput()

    NodePick(164, 136)
    s = s + GetPickOutput()
    PickByNode(528)
    s = s + GetPickOutput()

    Pick(113, 84)
    s = s + GetPickOutput()
    PickByZone(230)
    s = s + GetPickOutput()

    NodePick(113, 84)
    s = s + GetPickOutput()
    PickByNode(239)
    s = s + GetPickOutput()

    Pick(218, 65)
    s = s + GetPickOutput()
    PickByZone(211)
    s = s + GetPickOutput()

    NodePick(218, 65)
    s = s + GetPickOutput()
    PickByNode(220)
    s = s + GetPickOutput()

    DeleteAllPlots()
 

    OpenDatabase(silo_data_path("specmix_quad.silo"))

    AddPlot("Boundary", "Material")
    DrawPlots()
    ResetView()

    Pick(107, 131)
    s = s + GetPickOutput()
    PickByZone(86)
    s = s + GetPickOutput()

    NodePick(107, 131)
    s = s + GetPickOutput()
    PickByNode(89)
    s = s + GetPickOutput()

    Pick(207, 145)
    s = s + GetPickOutput()
    PickByZone(297)
    s = s + GetPickOutput()

    NodePick(207, 145)
    s = s + GetPickOutput()
    PickByNode(333)
    s = s + GetPickOutput()

    DeleteAllPlots()

    OpenDatabase(silo_data_path("specmix_ucd.silo"))

    AddPlot("Boundary", "Material")
    DrawPlots()
    ResetView()

    vars = "p"
    Pick(107, 131, vars)
    s = s + GetPickOutput()
    PickByZone(86, vars)
    s = s + GetPickOutput()

    NodePick(107, 131, vars)
    s = s + GetPickOutput()
    PickByNode(89, vars)
    s = s + GetPickOutput()

    Pick(207, 145, vars)
    s = s + GetPickOutput()
    PickByZone(297, vars)
    s = s + GetPickOutput()

    NodePick(207, 145, vars)
    s = s + GetPickOutput()
    PickByNode(333, vars)
    s = s + GetPickOutput()

    DeleteAllPlots()

    #Test Sliced Boundary plots
    ResetOperatorOptions("Slice")

    OpenDatabase(silo_data_path("curv3d.silo"))

    AddPlot("Boundary", "mat1")
    AddOperator("Slice")

    slice = SliceAttributes()
    slice.normal = (0, -1, 0)
    slice.upAxis = (0, 0, 1)
    SetOperatorOptions(slice)

    DrawPlots()
    ResetView()

    Pick(83, 223, vars)
    s = s + GetPickOutput()
    PickByZone(26670, vars)
    s = s + GetPickOutput()

    NodePick(83, 223, vars)
    s = s + GetPickOutput()
    PickByNode(28272, vars)
    s = s + GetPickOutput()

    Pick(127, 84, vars)
    s = s + GetPickOutput()
    PickByZone(5399, vars)
    s = s + GetPickOutput()

    NodePick(127, 84, vars)
    s = s + GetPickOutput()
    PickByNode(7005, vars)
    s = s + GetPickOutput()

    DeleteAllPlots()

    OpenDatabase(silo_data_path("sid97.silo"))

    AddPlot("Boundary", "mat1")

    AddOperator("Slice")

    slice = SliceAttributes()
    slice.originType = slice.Point
    slice.originPoint = (7400, -1300, 1000)
    slice.normal  = (0, 1, 0)
    slice.project2d = 1
    SetOperatorOptions(slice)

    DrawPlots()
    ResetView()
    SetViewExtentsType("actual")

    pick = GetPickAttributes()
    pick.variables = ("default")
    SetPickAttributes(pick)

    Pick(128, 140)
    s = s + GetPickOutput()
    PickByZone(1269)
    s = s + GetPickOutput()

    NodePick(128, 140)
    s = s + GetPickOutput()
    PickByNode(1356)
    s = s + GetPickOutput()

    Pick(120, 89)
    s = s + GetPickOutput()
    PickByZone(10438)
    s = s + GetPickOutput()

    NodePick(120, 89)
    s = s + GetPickOutput()
    PickByNode(12836)
    s = s + GetPickOutput()

    Pick(154, 50)
    s = s + GetPickOutput()
    PickByZone(10455)
    s = s + GetPickOutput()

    NodePick(154, 50)
    s = s + GetPickOutput()
    PickByNode(12842)
    s = s + GetPickOutput()

    DeleteAllPlots()

    OpenDatabase(silo_data_path("rect3d.silo"))

    AddPlot("Boundary", "mat1")

    AddOperator("Slice")

    slice.originType = slice.Intercept
    slice.originIntercept = 0.5
    slice.normal  = (0, 1, 1)
    slice.upAxis  = (0, 1, 0)
    slice.project2d = 1
    SetOperatorOptions(slice)

    DrawPlots()
    ResetView()
    SetViewExtentsType("original")

    Pick(75, 258) 
    s = s + GetPickOutput()
    PickByZone(810)
    s = s + GetPickOutput()

    NodePick(75, 258) 
    s = s + GetPickOutput()
    PickByNode(869)
    s = s + GetPickOutput()

    Pick(121, 108)
    s = s + GetPickOutput()
    PickByZone(18217)
    s = s + GetPickOutput()

    NodePick(121, 108)
    s = s + GetPickOutput()
    PickByNode(20561)
    s = s + GetPickOutput()

    TestText("PickBoundary", s)
    DeleteAllPlots()
    ResetPickLetter()

def PickVectorPlots():
    OpenDatabase(silo_data_path("globe.silo"))

    AddPlot("Vector", "vel")
    vector = VectorAttributes()
    vector.autoScale = 0
    SetPlotOptions(vector)
    DrawPlots()

    v = GetView3D()
    v.viewNormal = (0, 0, 1)
    v.focus = (0, 0, 0)
    v.viewUp = (0, 1, 0)
    v.viewAngle = 30
    v.parallelScale = 17.3205
    v.nearPlane = -34.641
    v.farPlane = 34.641
    v.imagePan = (0, 0)
    v.imageZoom = 2.02836
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 0, 0)
    SetView3D(v)
   
    p = GetPickAttributes()
    p.variables = ("default") 
    SetPickAttributes(p)
    Pick(274, 259)
    s = GetPickOutput()
    Pick(269, 258)
    s = s + GetPickOutput()
    Pick(86, 37)
    s = s + GetPickOutput()
    Pick(83, 28) 
    s = s + GetPickOutput()

    AddOperator("Slice")
    slice = SliceAttributes()
    slice.axisType = slice.ZAxis
    SetOperatorOptions(slice)
    DrawPlots()
    
    Pick(96, 244)
    s = s + GetPickOutput()
    Pick(204, 88)
    s = s + GetPickOutput()

    RemoveLastOperator()

    ChangeActivePlotsVar("disp")
    vector = VectorAttributes()
    vector.autoScale = 0
    vector.scale = 1.
    vector.origOnly = 0
    SetPlotOptions(vector)

    v.viewNormal = (-0.8775, 0.112599, 0.46617)
    v.focus = (0, 0, 0)
    v.viewUp = (0.120127, 0.992665, -0.0136456)
    v.viewAngle = 30
    v.parallelScale = 17.3205
    v.nearPlane = -34.641
    v.farPlane = 34.641
    v.imagePan = (0, 0)
    v.imageZoom = 2.3515
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 0, 0)
    SetView3D(v)
    
    Pick(119, 87)
    s = s + GetPickOutput(); 
    Pick(148, 230)
    s = s + GetPickOutput(); 
    Pick(22, 97)
    s = s + GetPickOutput(); 

    AddOperator("Slice")
    DrawPlots()
    
    Pick(76, 113)
    s = s + GetPickOutput(); 
    Pick(244, 67)
    s = s + GetPickOutput(); 
    Pick(127, 217)
    s = s + GetPickOutput(); 

    DeleteAllPlots()

    # VisIt00006296 -- pick of vector expression causes crash
    DefineVectorExpression("polarVector", "polar(mesh1)")
    DefineVectorExpression("xyzVector", "coord(mesh1)")
    AddPlot("Vector", "polarVector")
    DrawPlots()
    ResetView()

    v = GetView3D()
    v.viewNormal = (0, 0, 1)
    v.focus = (0, 0, 0)
    v.viewUp = (0, 1, 0)
    v.viewAngle = 30
    v.parallelScale = 17.3205
    v.nearPlane = -34.641
    v.farPlane = 34.641
    v.imagePan = (-0.0369757, 0.0171359)
    v.imageZoom = 4.66857
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 0, 0)
    SetView3D(v)

    Pick(233, 241)
    s = s + GetPickOutput()
    Pick(226, 30)
    s = s + GetPickOutput()

    ChangeActivePlotsVar("xyzVector")

    v. imagePan = (0.0220781, -0.0509109)
    v. imageZoom = 9.89113
    SetView3D(v)
    Pick(149, 209)
    s = s + GetPickOutput()
    Pick(214, 6) 
    s = s + GetPickOutput()


    TestText("PickVectorPlot", s)
    DeleteAllPlots()
    ResetPickLetter()

def PickPointMeshes():
    OpenDatabase(silo_data_path("noise.silo"))

    AddPlot("Pseudocolor", "PointVar")
    pc = PseudocolorAttributes()
    pc.pointSize = 1
    SetPlotOptions(pc)
    DrawPlots()
    ResetView()

    NodePick(100, 240)
    s = GetPickOutput()
    Pick(223, 197)
    s = s + GetPickOutput()
    NodePick(138, 66)
    s = s + GetPickOutput()

    DeleteAllPlots()
    AddPlot("Mesh", "PointMesh")
    mesh = MeshAttributes()
    mesh.pointSize = 1
    mesh.pointType = mesh.Box
    SetPlotOptions(mesh)
    DrawPlots()

    NodePick(100, 240)
    s = s + GetPickOutput()
    Pick(223, 197)
    s = s + GetPickOutput()
    NodePick(138, 66)
    s = s + GetPickOutput()

    mesh.pointType = mesh.Point
    SetPlotOptions(mesh)

    NodePick(100, 240)
    s = s + GetPickOutput()
    Pick(223, 197)
    s = s + GetPickOutput()
    NodePick(138, 66)
    s = s + GetPickOutput()

    DeleteAllPlots()

    OpenDatabase(silo_data_path("multi_point2d.silo"))

    AddPlot("Mesh", "mesh1")
    m = MeshAttributes()
    m.meshColor = (255, 0, 255, 255)
    m.foregroundFlag = 0
    m.pointSize = 0.05
    m.pointType = m.Box
    SetPlotOptions(m)
    DrawPlots()

    v = GetView2D()
    v.fullFrameActivationMode = v.On 
    v.windowCoords = (-2.05391, -0.442661, 1.02233, 2.43681)
    SetView2D(v)

    vars = ("d", "p", "u", "v")

    NodePick(122, 72, vars)
    s = s + GetPickOutput()
    Pick(134, 229, vars)
    s = s + GetPickOutput()
    NodePick(278, 266, vars)
    s = s + GetPickOutput()

    m.pointType = mesh.Point
    SetPlotOptions(m)

    NodePick(122, 72, vars)
    s = s + GetPickOutput()
    Pick(134, 229, vars)
    s = s + GetPickOutput()
    NodePick(278, 266, vars)
    s = s + GetPickOutput()

    DeleteAllPlots()
    OpenDatabase(silo_data_path("noise.silo"))

    AddPlot("Subset", "PointMesh")
    DrawPlots()
    ResetView()

    NodePick(100, 240)
    s = s+ GetPickOutput()
    Pick(223, 197)
    s = s + GetPickOutput()
    NodePick(138, 66)
    s = s + GetPickOutput()

    TestText("PickPointMeshes", s)
    DeleteAllPlots()
    ResetPickLetter()

def PickGlobalIds():
    OpenDatabase(silo_data_path("global_node.silo"))

    AddPlot("Pseudocolor", "dist")
    DrawPlots()
    ResetView()
    p = GetPickAttributes()
    p.showGlobalIds = 0
    p.variables = ("default")
    SetPickAttributes(p)

    Pick(82, 225)
    s = GetPickOutput()

    p.showGlobalIds = 1
    SetPickAttributes(p)

    Pick(82, 225)
    s = s + GetPickOutput()

    p.showGlobalIds = 0
    SetPickAttributes(p)

    NodePick(195, 101)
    s = s + GetPickOutput()

    p.showGlobalIds = 1
    SetPickAttributes(p)

    NodePick(195, 101)
    s = s + GetPickOutput()

    p.showGlobalIds = 0
    SetPickAttributes(p)

    Pick(175, 162)
    s = s + GetPickOutput()
    NodePick(175, 162)
    s = s + GetPickOutput()

    p.showGlobalIds = 1
    SetPickAttributes(p)

    Pick(175, 162)
    s = s + GetPickOutput()
    NodePick(175, 162)
    s = s + GetPickOutput()

    PickByGlobalZone(3250)
    s = s + GetPickOutput()
    PickByGlobalZone(237394)
    s = s + GetPickOutput()
    PickByGlobalNode(3869)
    s = s + GetPickOutput()
    PickByGlobalNode(246827)
    s = s + GetPickOutput()

    # bug '6097 -- global ids not displayed correctly with global picks. 
    p.showGlobalIds = 1
    SetPickAttributes(p)

    PickByGlobalZone(236919)
    s = s + GetPickOutput()
    PickByGlobalNode(244455)
    s = s + GetPickOutput()

    ChangeActivePlotsVar("p")
    PickByGlobalZone(236919)
    s = s + GetPickOutput()
    PickByGlobalNode(244455)
    s = s + GetPickOutput()

    TestText("PickGlobalIds", s)
    p.showGlobalIds = 0
    SetPickAttributes(p)
    DeleteAllPlots()
    ResetPickLetter()

def PickExpressions():
    OpenDatabase(silo_data_path("rect3d.silo"))

    DefineScalarExpression("m1", 'matvf(mat1, ["1", "3"])')
    DefineScalarExpression("mir1", 'mirvf(mat1, zoneid(quadmesh3d), volume(quadmesh3d), 1)')

    AddPlot("Pseudocolor", "m1")
    DrawPlots()
    ResetView()

    Pick(151, 172)
    s = GetPickOutput()
    Pick(68, 239)
    s = s + GetPickOutput()
    Pick(237, 86)
    s = s + GetPickOutput()
    NodePick(151, 172)
    s = s + GetPickOutput()
    NodePick(68, 239)
    s = s + GetPickOutput()
    NodePick(237, 86)
    s = s + GetPickOutput()

    ChangeActivePlotsVar("mir1")
    Pick(151, 172)
    s = s + GetPickOutput()
    Pick(68, 239)
    s = s + GetPickOutput()
    Pick(237, 86)
    s = s + GetPickOutput()
    NodePick(151, 172)
    s = s + GetPickOutput()
    NodePick(68, 239)
    s = s + GetPickOutput()
    NodePick(237, 86)
    s = s + GetPickOutput()

    DeleteAllPlots()
    OpenDatabase(silo_data_path("globe.silo"))

    AddPlot("Pseudocolor", "t")
    DrawPlots()
    ResetView()
    DefineScalarExpression("polar", "polar(mesh1)")

    # pick with default var -- 't'
    Pick(128, 107)
    s = s + GetPickOutput()
    # pick again, adding new expression var -- 'polar'
    vars = ("default", "polar")
    Pick(128, 107, vars)
    s = s + GetPickOutput()

    TestText("PickExpressions", s)
    DeleteAllPlots()
    ResetPickLetter()

def PickAMR():
    OpenDatabase(data_path("boxlib_test_data/2D/plt0822/Header"))

    AddPlot("Mesh", "Mesh")
    AddPlot("Pseudocolor", "density")
    DrawPlots()
    ResetView()
    v = GetView2D()
    v.fullFrameActivationMode = v.Off 
    v.windowCoords = (0.0232322, 0.0302984, 0.11825, 0.123807) 
    SetView2D(v)

    s = "Testing boxlib 2D, pseudocolor\n"
    vars = "default"
    NodePick(226, 179, vars)
    s = s + GetPickOutput()
    ZonePick(226, 179, vars)
    s = s + GetPickOutput()
    NodePick(215, 168, vars)
    s = s + GetPickOutput()
    ZonePick(215, 168, vars)
    s = s + GetPickOutput()
    NodePick(114, 94, vars)
    s = s + GetPickOutput()
    ZonePick(114, 94, vars)
    s = s + GetPickOutput()
    NodePick(189, 197, vars)
    s = s + GetPickOutput()
    ZonePick(189, 197, vars)
    s = s + GetPickOutput()
    NodePick(242, 72, vars)
    s = s + GetPickOutput()
    ZonePick(242, 72, vars)
    s = s + GetPickOutput()

    DeleteAllPlots()
    s = s + "\nTesting boxlib 2D, contour\n"
    AddPlot("Contour", "density")
    DrawPlots()
    v.windowCoords = (0.00408394, 0.0401985, 0.109754, 0.138135)
    SetView2D(v)
    
    Pick(161, 137)
    s = s + GetPickOutput()
    NodePick(161, 137)
    s = s + GetPickOutput()
    Pick(176, 139)
    s = s + GetPickOutput()
    NodePick(176, 139)
    s = s + GetPickOutput()
    Pick(228, 71)
    s = s + GetPickOutput()
    NodePick(228, 71)
    s = s + GetPickOutput()
    Pick(255, 196)
    s = s + GetPickOutput()
    NodePick(255, 196)
    s = s + GetPickOutput()

    DeleteAllPlots()
    s = s + "\nTesting boxlib 2D, Boundary\n"
    AddPlot("Boundary", "materials")
    DrawPlots()
    v.windowCoords = (0.00226909, 0.0378978, 0.0908832, 0.121254)
    SetView2D(v)
    Pick(161, 137)
    s = s + GetPickOutput()
    NodePick(161, 137)
    s = s + GetPickOutput()
    Pick(228, 71)
    s = s + GetPickOutput()
    NodePick(228, 71)
    s = s + GetPickOutput()
    Pick(139, 196)
    s = s + GetPickOutput()
    NodePick(139, 196)
    s = s + GetPickOutput()

    TestText("PickAMR", s)
    DeleteAllPlots()
    ResetPickLetter()

def PickSamrai():

    def doPicks1():
        def pickList(index):
            if index == 0 :
                ZonePick(172, 178)
                return GetPickOutput()
            elif index == 1 :
                NodePick(191, 188)
                return GetPickOutput()
            elif index == 2:
                NodePick(145, 157)
                return GetPickOutput()
            elif index == 3:
                PickByZone(1863, 2) 
                return GetPickOutput()
            elif index == 4:
                PickByNode(2442, 2) 
                return GetPickOutput()
            elif index == 5:
                PickByNode(2422, 2) 
                return GetPickOutput()
            else :
                return ""

        s = ""
        for j in range(6):
            for i in range(4):
                SetActivePlots(i)
                s = s + pickList(j)
        return s

    def doPicks2():
        def pickList2(index):
            if index == 0 :
                ZonePick(239, 141)
                return GetPickOutput()
            elif index == 1 :
                NodePick(246, 108)
                return GetPickOutput()
            else :
                return ""

        s = ""
        for j in range(2):
            for i in range(4):
                SetActivePlots(i)
                s = s + pickList2(j)
        return s

    def doPicks3():
        def pickList3(index):
            if index == 0 :
                ZonePick(210, 148)
                return GetPickOutput()
            elif index == 1 :
                NodePick(200, 148)
                return GetPickOutput()
            elif index == 2 :
                ZonePick(210, 168)
                return GetPickOutput()
            elif index == 3 :
                NodePick(210, 168)
                return GetPickOutput()
            else :
                return ""

        s = ""
        for j in range(4):
            for i in range(4):
                SetActivePlots(i)
                s = s + pickList3(j)
        return s


    # Open SAMRAI (add default subset plot)
    # Add PC, Mesh, and Contour Plots
    OpenDatabase(data_path("samrai_test_data/sil_changes/dumps.visit"))

    slice = SliceAttributes()
    slice.axisType = slice.ZAxis
    slice.originType = slice.Percent
    slice.originPercent = 20
    slice.project2d = 1

    AddPlot("Pseudocolor", "Primitive Var _number_0")
    AddPlot("Mesh", "amr_mesh")
    AddPlot("Contour", "Primitive Var _number_0")

    SetActivePlots((0, 1, 2, 3))
    AddOperator("Slice")
    SetOperatorOptions(slice)
    DrawPlots()

    v = GetView2D()
    v.windowCoords = (4.89115, 6.43144, 6.80336, 8.63983)
    SetView2D(v)
    s = "\nTesting sliced SAMRAI:\n"
    s = s + doPicks1()  
    TestText("PickSamrai_01", s)

    SetActivePlots((0, 1, 2, 3))
    clip = ClipAttributes()
    clip.plane1Origin = (5.1, 0, 0)
    clip.plane1Normal = (-1, 0, 0)

    AddOperator("Clip")
    SetOperatorOptions(clip)
    DrawPlots()
    ResetPickLetter()
    s = "\nTesting clipped sliced SAMRAI:\n"
    s = s + doPicks1()  
    TestText("PickSamrai_02", s)

    SetActivePlots((0, 1, 2, 3))
    DemoteOperator(1)
    DrawPlots()
    ResetPickLetter()
    s = "\nTesting sliced clipped SAMRAI:\n"
    s = s + doPicks1()  
    TestText("PickSamrai_03", s)

    SetActivePlots((0, 1, 2, 3))
    RemoveLastOperator()
    clip.plane1Origin = (4.3, 0, 0)
    SetOperatorOptions(clip)

    DrawPlots()
    v = GetView3D()
    v.viewNormal = (0, 0, -1)
    v.focus = (15, 10, 10)
    v.viewUp = (0, 1, 0)
    v.parallelScale = 20.6155
    v.nearPlane = -41.2311
    v.farPlane = 41.2311
    v.perspective = 0
    SetView3D(v)
    v.imageZoom = 8 
    SetView3D(v)
    v.imagePan = (-0.2, 0.1)
    SetView3D(v)
    ResetPickLetter()
    s = "\nTesting clipped SAMRAI (3D):\n"
    s = s + doPicks2()  
    TestText("PickSamrai_04", s)

    DeleteAllPlots()
    ResetPickLetter()


    OpenDatabase(data_path("samrai_test_data/front/dumps.visit"))

    AddPlot("Mesh", "amr_mesh")
    AddPlot("Pseudocolor", "Distance to front")
    AddPlot("Contour", "Distance to front")

    DrawPlots()
    v = GetView2D()
    v.windowCoords = (0.678228, 0.865969, 0.467317, 0.650137)

    SetView2D(v)
    s = "\nTesting front:\n"
    s = s + doPicks3()  
    TestText("PickSamrai_05", s)

    DeleteAllPlots()
    ResetPickLetter()

def PickIndexSelect():
    OpenDatabase(silo_data_path("rect2d.silo"))

    AddPlot("Mesh", "quadmesh2d")
    AddPlot("Pseudocolor", "u")
    AddOperator("IndexSelect")
    isAtts = IndexSelectAttributes()
    isAtts.dim = isAtts.TwoD
    isAtts.xMin = 1
    isAtts.xMax = 2
    isAtts.yMin = 1
    isAtts.yMax = 2
    SetOperatorOptions(isAtts)

    DrawPlots()

    v = GetView2D()
    v.windowCoords = (-0.0583437, 0.254323, -0.0576062, 0.220944)
    SetView2D(v)

    SetActivePlots(0)
    s = "Pick on Index-selected 2D Mesh Plot\n"
    Pick(132, 117)
    s = s + GetPickOutput()
    NodePick(132, 117)
    s = s + GetPickOutput()

    SetActivePlots(1)
    s = s + "\nSame Picks on Index-selected 2D Pseudocolor Plot\n"
    Pick(132, 117)
    s = s + GetPickOutput()
    NodePick(132, 117)
    s = s + GetPickOutput()

    isAtts.xIncr = 5
    isAtts.yIncr = 5
    SetOperatorOptions(isAtts)

    s = s+ "\nChanged stride of IndexSelect, 2D Mesh Plot picks\n"
    SetActivePlots(0)
    Pick(132, 117)
    s = s + GetPickOutput()
    NodePick(132, 117)
    s = s + GetPickOutput()

    s = s + "\nSame picks 2D Pseudocolor Plot \n"
    SetActivePlots(1)
    Pick(132, 117)
    s = s + GetPickOutput()
    NodePick(132, 117)
    s = s + GetPickOutput()

    DeleteAllPlots()

    OpenDatabase(silo_data_path("rect3d.silo"))

    AddPlot("Mesh", "quadmesh3d")
    AddPlot("Pseudocolor", "d")
    SetActivePlots((0, 1))
    AddOperator("IndexSelect")
    isAtts = IndexSelectAttributes()
    isAtts.dim = isAtts.ThreeD
    isAtts.xIncr = 4
    isAtts.yIncr = 4
    SetOperatorOptions(isAtts)

    DrawPlots()

    ResetView()
    v = GetView3D()
    v.viewNormal = (-1, 0, 0)
    v.focus = (0.5, 0.5, 0.5)
    v.viewUp = (0, 1, 0)
    v.parallelScale = 0.866025
    v.perspective = 1
    SetView3D(v)
    v.imagePan = (0.0552434, 0.225336)
    SetView3D(v)
    v.imageZoom = 5.01124
    SetView3D(v)

    s = s + "\nIndex selected 3D Mesh Plot\n"
    SetActivePlots(0)
    Pick(143, 144)
    s = s + GetPickOutput()
    NodePick(143, 144)
    s = s + GetPickOutput()
    s = s + "\nSame picks on 3D Pseudocolor Plot\n"
    SetActivePlots(1)
    Pick(143, 144)
    s = s + GetPickOutput()
    NodePick(143, 144)
    s = s + GetPickOutput()

    SetActivePlots((0, 1))
    AddOperator("Slice")
    slice = SliceAttributes()
    slice.normal = (0, 0, 1)
    slice.originType = slice.Intercept
    slice.originIntercept = 0.5
    slice.upAxis = (0, 1, 0)
    slice.axisType = slice.ZAxis
    slice.project2d = 1
    SetOperatorOptions(slice)
    DrawPlots()

    v = GetView2D()
    v.windowCoords = (0.226685, 0.501966, 0.147753, 0.423034)
    SetView2D(v)

    s = s + "\nSliced, Index-selected 3D Mesh Plot\n"
    SetActivePlots(0)
    Pick(184, 234)
    s = s + GetPickOutput()
    NodePick(184, 234)
    s = s + GetPickOutput()


    s = s + "\nSame pickes on Sliced, Index-selected 3D Pseudocolor Plot\n"
    SetActivePlots(1)
    Pick(184, 234)
    s = s + GetPickOutput()
    NodePick(184, 234)
    s = s + GetPickOutput()

    AddPlot("FilledBoundary", "mat1")
    AddOperator("IndexSelect")
    SetOperatorOptions(isAtts)
    AddOperator("Slice")
    SetOperatorOptions(slice)
    AddPlot("Mesh", "quadmesh3d")
    AddOperator("Slice")
    SetOperatorOptions(slice)
    DrawPlots()

    s = s + "\nSame picks on Sliced, Index-selected 3D FilledBoundary Plot\n"
    s = s + "Stride changes back to 1 due to MIR, so should match Mesh results below\n" 
    SetActivePlots(2)
    Pick(184, 234)
    s = s + GetPickOutput()
    NodePick(184, 234)
    s = s + GetPickOutput()

    s = s + "\nSame picks on Sliced, 3D Mesh Plot\n"
    SetActivePlots(3)
    Pick(184, 234)
    s = s + GetPickOutput()
    NodePick(184, 234)
    s = s + GetPickOutput()


    TestText("PickIndexSelect", s)
    DeleteAllPlots()
    ResetPickLetter()

def PickTensors():
    OpenDatabase(silo_data_path("noise.silo"))

    AddPlot("Mesh", "Mesh")
    DrawPlots()
    vars = ("grad_tensor")
    Pick(133 , 178, vars)
    s = GetPickOutput()
    NodePick(133 ,178 , vars)
    s = s + GetPickOutput()

    DeleteAllPlots()
    AddPlot("Tensor", "grad_tensor")
    DrawPlots()
    v = GetView3D()
    v.viewNormal = (0, 0, 1)
    v.focus = (0, 0, 0)
    v.viewUp = (0, 1, 0)
    v.viewAngle = 30
    v.parallelScale = 17.3205
    v.nearPlane = -34.641
    v.farPlane = 34.641
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 0, 0)
    SetView3D(v)
    v.imagePan = (-0.181384, 0.137462)
    SetView3D(v)
    v.imageZoom = 4.86765
    SetView3D(v)

    Pick(127, 59)
    s = s + GetPickOutput()
    Pick(269, 166)
    s = s + GetPickOutput()
    TestText("PickTensors", s)
    ResetView()
    DeleteAllPlots()
    ResetPickLetter()

def PickMili():
    #'7144
    OpenDatabase(data_path("mili_test_data/single_proc/m_plot.mili"))

    AddPlot("Pseudocolor", "Primal/brick/stress/sx")
    AddOperator("Threshold")
    thresh = ThresholdAttributes()
    thresh.lowerBounds = (1.0)
    thresh.upperBounds = (1.0)
    thresh.listedVarNames = ("Primal/Shared/sand")
    SetOperatorOptions(thresh)
    SetTimeSliderState(77)
    DrawPlots()
    v = GetView3D()
    v.viewNormal = (0.761946, 0.114249, -0.637483)
    v.focus = (2, -4.76837e-07, -1.43051e-06)
    v.viewUp = (-.133671, -.990866, 0.0178134)
    v.viewAngle = 30
    v.parallelScale = 15.7797
    v.nearPlane = -31.5595
    v.farPlane = 31.5595
    SetView3D(v)

    vars = "default"
    Pick(100, 150, vars)
    s = GetPickOutput()
    NodePick(100, 150, vars)
    s = s + GetPickOutput()

    RemoveLastOperator()

    #'7145
    AddPlot("Mesh", "mesh1")  
    SetActivePlots((0, 1))
    AddOperator("Clip")
    clip = ClipAttributes()
    clip.plane1Status = 1
    clip.plane1Origin = (8, 7.77778, -10)
    clip.plane1Normal = (1, 0, 0)
    SetOperatorOptions(clip)
    SetTimeSliderState(77)
    DrawPlots()
    v = GetView3D()
    v.viewNormal = (1, 0, 6.12307e-17)
    v.focus = (10.5296, -0.00138283, -0.000236511)
    v.viewUp = (0, 1, 0)
    v.viewAngle = 30
    v.parallelScale = 19.0748
    v.nearPlane = -38.1495
    v.farPlane = 38.1495
    SetView3D(v)

    Pick(204, 215, vars)
    s = s + GetPickOutput()
    NodePick(204, 215, vars)
    s = s + GetPickOutput()
    TestText("PickMili", s)
    DeleteAllPlots()
    ResetPickLetter()

def PickContour():
    #'7144
    OpenDatabase(silo_data_path("multi_rect3d.silo"))

    AddPlot("Contour", "d")
    contour = ContourAttributes()
    contour.contourNLevels = 4
    SetPlotOptions(contour)
    DrawPlots()
    v = GetView3D()
    v.viewNormal = (-0.582341, 0.658239, 0.477075)
    v.focus = (0.5, 0.5, 0.5)
    v.viewUp = (0.402453, 0.743318, -0.534331)
    v.viewAngle = 30
    v.parallelScale = 0.866025
    v.nearPlane = -1.73205
    v.farPlane = 1.73205
    SetView3D(v)

    vars = "default"
    Pick(166, 138, vars)
    s = GetPickOutput()
    NodePick(166, 138, vars)
    s = s + GetPickOutput()
    Pick(160, 175, vars)
    s = s + GetPickOutput()
    NodePick(160, 175, vars)
    s = s + GetPickOutput()
    Pick(158, 210, vars)
    s = s + GetPickOutput()
    NodePick(158, 210, vars)
    s = s + GetPickOutput()

    TestText("PickContour", s)
    DeleteAllPlots()
    ResetPickLetter()

def PickBox():
    #'7566
    OpenDatabase(silo_data_path("rect2d.silo"))

    AddPlot("Pseudocolor", "d")
    AddOperator("Box")
    box = BoxAttributes()
    box.minx = 0.2
    box.maxx = 0.8
    box.miny = 0.2
    box.maxy = 0.8
    SetOperatorOptions(box)
    AddPlot("Mesh", "quadmesh2d")
    DrawPlots()

    vars = "default"
    SetActivePlots(0)

    Pick(155, 140)
    s = GetPickOutput()
    NodePick(155, 140)
    s = s + GetPickOutput()

    SetActivePlots(1)
    Pick(155, 140)
    s = s + GetPickOutput()
    NodePick(155, 140)
    s = s + GetPickOutput()


    DeleteAllPlots()
    OpenDatabase(silo_data_path("rect3d.silo"))

    AddPlot("Pseudocolor", "d")
    AddOperator("Box")
    box.minx = 0.3
    box.maxx = 0.83
    box.miny = 0.3
    box.maxy = 0.83
    box.minz = 0
    box.maxz = 1
    SetOperatorOptions(box)
    AddOperator("Slice")
    slice = SliceAttributes()
    slice.axisType = slice.YAxis
    slice.originType = slice.Intercept
    slice.originIntercept = 0.5
    slice.project2d = 1
    SetOperatorOptions(slice)

    AddPlot("Mesh", "quadmesh3d")
    AddOperator("Slice")
    SetOperatorOptions(slice)
    DrawPlots()
    SetActivePlots(0)
    TurnMaterialsOff("8")

    Pick(221, 230)
    s = s + GetPickOutput()
    Pick(206, 67)
    s = s + GetPickOutput()
    NodePick(221, 230)
    s = s + GetPickOutput()
    NodePick(206, 67)
    s = s + GetPickOutput()

    SetActivePlots(1)
    Pick(221, 230)
    s = s + GetPickOutput()
    Pick(206, 67)
    s = s + GetPickOutput()
    NodePick(221, 230)
    s = s + GetPickOutput()
    NodePick(206, 67)
    s = s + GetPickOutput()
    
    TestText("PickBox", s)
    DeleteAllPlots()
    ResetPickLetter()

def PickScatter():
    OpenDatabase(silo_data_path("noise.silo"))

    AddPlot("Scatter", "hardyglobal")
    scatter = ScatterAttributes()
    scatter.var1= "hardyglobal"
    scatter.var2= "shepardglobal"
    scatter.var3= "radial"
    scatter.var3Role = scatter.Color
    scatter.scaleCube = 1
    SetPlotOptions(scatter)
    DrawPlots()
    Pick(124, 174)
    s = GetPickOutput()

    scatter.scaleCube = 0
    SetPlotOptions(scatter)
    DrawPlots()
    ResetView()
    Pick(124, 174)
    s = s + GetPickOutput()


    DeleteAllPlots()

    s = s + "Expressions used in Scatter\n";
    OpenDatabase(silo_data_path("multi_rect3d.silo"))

    DefineScalarExpression("xc", "coord(mesh1)[0]")
    DefineScalarExpression("yc", "coord(mesh1)[1]")
    DefineScalarExpression("zc", "coord(mesh1)[2]")
    DefineScalarExpression("uvw_prod", "u*v*w")
    AddPlot("Scatter", "xc")
    scatter.var1 = "xc"
    scatter.var1Role = scatter.Coordinate0
    scatter.var2 = "yc"
    scatter.var2Role = scatter.Coordinate1
    scatter.var3 = "zc"
    scatter.var3Role = scatter.Coordinate2
    scatter.var4 = "uvw_prod"
    scatter.var4Role = scatter.Color
    SetPlotOptions(scatter)
    DrawPlots()
    ResetView() 

    Pick(100, 200)
    s = s + GetPickOutput()

    s = s + "Mix up the coordinates\n";
    scatter.var1Role = scatter.Coordinate1
    scatter.var2Role = scatter.Coordinate0
    SetPlotOptions(scatter)
    DrawPlots()
    Pick(100, 200)
    s = s + GetPickOutput()

    TestText("PickScatter", s)
    DeleteAllPlots()
    ResetPickLetter()

def PickHighlight():
    OpenDatabase(silo_data_path("noise.silo"))
    TurnOffAllAnnotations()
    AddPlot("Pseudocolor", "hardyglobal")
    pickAtts = GetPickAttributes()
    pickAtts.showPickHighlight = 1
    SetPickAttributes(pickAtts)
    DrawPlots()
    PickByZone(116242)
    Test("PickHighlight_01")
    DeleteAllPlots()
    ResetPickLetter()

    OpenDatabase(silo_data_path("global_node.silo"))
    AddPlot("Pseudocolor", "p")
    DrawPlots()

    # bug '3880 -- global id highlights incorrect cell. 
    origAtts = GetPickAttributes()
    pickAtts = origAtts
    pickAtts.showPickHighlight = 1
    pickAtts.showPickLetter = 1
    SetPickAttributes(pickAtts)

    PickByGlobalZone(236919)
    Test("GlobalHighlight_00")

    SetPickAttributes(origAtts)
    DeleteAllPlots()
    ResetPickLetter()

    #restore the attributes
    annotAtts  = GetAnnotationAttributes() 
    annotAtts.userInfoFlag = 1
    annotAtts.axes3D.xAxis.title.visible = 1
    annotAtts.axes3D.yAxis.title.visible = 1
    annotAtts.axes3D.zAxis.title.visible = 1
    annotAtts.axes3D.bboxFlag = 1;
    annotAtts.axes3D.triadFlag = 1;
    SetAnnotationAttributes(annotAtts)

def PickHighlightColor():
    OpenDatabase(silo_data_path("noise.silo"))
    TurnOffAllAnnotations()
    AddPlot("Pseudocolor", "hardyglobal")
    pickAtts = GetPickAttributes()
    pickAtts.showPickHighlight  = 1
    pickAtts.pickHighlightColor = (0, 0, 255) 
    SetPickAttributes(pickAtts)
    DrawPlots()
    PickByZone(116242)
    Test("PickHighlightColor_01")
    DeleteAllPlots()
    ResetPickLetter()
    #restore the attributes
    pickAtts.pickHighlightColor = (255, 0, 0)
    SetPickAttributes(pickAtts)
    annotAtts  = GetAnnotationAttributes() 
    annotAtts.userInfoFlag = 1
    annotAtts.axes3D.xAxis.title.visible = 1
    annotAtts.axes3D.yAxis.title.visible = 1
    annotAtts.axes3D.zAxis.title.visible = 1
    annotAtts.axes3D.bboxFlag = 1;
    annotAtts.axes3D.triadFlag = 1;
    SetAnnotationAttributes(annotAtts)
    
def PickRange():
    OpenDatabase(silo_data_path("noise.silo"))
    TurnOffAllAnnotations()
    AddPlot("Pseudocolor", "hardyglobal")
    pickAtts = GetPickAttributes()
    pickAtts.showPickHighlight = 1
    SetPickAttributes(pickAtts)
    DrawPlots()
    options = {}
    options["pick_range"] = "116242-116300, 116350,1"
    PickByZone(options)
    Test("PickRange_01")
    DeleteAllPlots()
    ResetPickLetter()

def PickLines3D():
    OpenDatabase(silo_data_path("ucd_lines3d.silo"))
    AddPlot("Mesh", "ucd_linesmesh3d")
    DrawPlots()

    ZonePick(113,182)
    s = GetPickOutput();
    ZonePick(134,107)
    s = s + GetPickOutput();
    ZonePick(177,151)
    s = s + GetPickOutput();

    NodePick(126,161)
    s = s + GetPickOutput();
    NodePick(203,130)
    s = s + GetPickOutput();
    NodePick(150,193)
    s = s + GetPickOutput();

    # rotate, so lines closer to camera are reversed.
    v = GetView3D()
    v.viewNormal=(0,0,-1)
    SetView3D(v)

    ZonePick(149,168)
    s = s + GetPickOutput();
    ZonePick(139,97)
    s = s + GetPickOutput();
    ZonePick(191,206)
    s = s + GetPickOutput();

    NodePick(161,73)
    s = s + GetPickOutput();
    NodePick(173,159)
    s = s + GetPickOutput();
    NodePick(54,153)
    s = s + GetPickOutput();

    TestText("PickLines3D", s)

    ClearPickPoints()
    DeleteAllPlots()
    ResetPickLetter()

def PickZoneLabel():
    OpenDatabase(data_path("mili_test_data/single_proc/sslide14ball_l.plt.mili"))
    TurnOffAllAnnotations()
    AddPlot("Pseudocolor", "Primal/shell/bend_magnitude")
    DrawPlots()
    options = {}
    options["element_label"] = "shell 9"
    options["vars"] = "OriginalZoneLabels"
    PickByZoneLabel(options)
    s = GetPickOutput()
    TestText("PickByZoneLabel",s)
    ClearPickPoints()
    DeleteAllPlots()
    ResetPickLetter()

def PickNodeLabel():
    OpenDatabase(data_path("mili_test_data/single_proc/sslide14ball_l.plt.mili"))
    TurnOffAllAnnotations()
    AddPlot("Pseudocolor", "Primal/shell/bend_magnitude")
    DrawPlots()
    options = {}
    options["element_label"] = "node 93"
    options["vars"] = "OriginalNodeLabels"
    PickByNodeLabel(options)
    s = GetPickOutput()
    TestText("PickByNodeLabel",s)
    ClearPickPoints()
    DeleteAllPlots()
    ResetPickLetter()

def PickRangeLabel():
    OpenDatabase(data_path("mili_test_data/single_proc/d3samp6.plt.mili"))
    TurnOffAllAnnotations()
    AddPlot("Pseudocolor", "Primal/shell/bend_magnitude")
    DrawPlots()

    SetPickAttributes(defaultAtts)

    options = {}
    options["element_label"] = "shell"
    options["return_curves"] = 1
    options["start_time"] = 0
    options["end_time"] = 2
    options["pick_range"] = "4-5,9"
    options["vars"] = "OriginalZoneLabels"

    output_dict = PickByZoneLabel(options)
    s = str(output_dict)
    TestText("PickRangeByZoneLabel",s)
    ClearPickPoints()
    DeleteAllPlots()
    ResetPickLetter()

def TestRemovePicks():
    OpenDatabase(silo_data_path("noise.silo"))
    TurnOffAllAnnotations()
    AddPlot("Pseudocolor", "hardyglobal")
    pickAtts = GetPickAttributes()
    pickAtts.showPickHighlight = 1
    SetPickAttributes(pickAtts) 
    DrawPlots()

    PickByZone(0)
    PickByZone(5)
    PickByZone(35)
    PickByZone(18)
    Test("RemovePicks_01")
    to_remove = "A, C, D"
    expected  = to_remove
    removed   = RemovePicks(to_remove)
    Test("RemovePicks_02")

    #check that the returned list matches
    #what we expect
    AssertEqual("Removed expected picks", removed, expected)
    ClearPickPoints() 

    PickByZone(0)
    PickByZone(18)
    Test("RemovePicks_03")
    to_remove = "A, C, D, E"
    expected  = "E"
    removed   = RemovePicks(to_remove)
    Test("RemovePicks_04")

    AssertEqual("Removed expected picks 2", removed, expected)

    ClearPickPoints() 
    DeleteAllPlots()
    ResetPickLetter()

def TestRemoveLabeledPicks():
    OpenDatabase(data_path("mili_test_data/single_proc/sslide14ball_l.plt.mili"))
    TurnOffAllAnnotations()
    AddPlot("Pseudocolor", "Primal/shell/bend_magnitude")
    pickAtts = GetPickAttributes()
    pickAtts.showPickHighlight = 1
    SetPickAttributes(pickAtts) 
    DrawPlots()

    options = {}
    options["element_label"] = "shell 1"
    PickByZoneLabel(options)
    options["element_label"] = "shell 2"
    PickByZoneLabel(options)
    options["element_label"] = "shell 5"
    PickByZoneLabel(options)

    Test("RemoveLabeledPicks_00")
    to_remove = "shell 1, shell 2, shell 19"
    removed   = RemovePicks(to_remove)
    Test("RemoveLabeledPicks_01")

    expected = "shell 1, shell 2"
    AssertEqual("Removed expected shell", expected, removed)

    ClearPickPoints() 
    DeleteAllPlots()
    ResetPickLetter()

def TestSwivelHighlight():
    ResetPickAttributes()
    ClearPickPoints() 
    DeleteAllPlots()
    ResetPickLetter()
    ResetView()

    OpenDatabase(silo_data_path("globe.silo"))
    TurnOffAllAnnotations()
    AddPlot("Pseudocolor", "v")
    DrawPlots()
    orig_atts = GetPickAttributes()
    fh_atts   = GetPickAttributes()
    fh_atts.overridePickLabel = 1
    fh_atts.forcedPickLabel   = "fhpick"
    fh_atts.showPickHighlight = 1
    fh_atts.swivelFocusToPick = 1
    fh_atts.showPickLetter    = 0
    SetPickAttributes(fh_atts)

    PickByZone(element=580)     
    Test("SwivelHighlight_00")
    
    SetPickAttributes(orig_atts)
    ClearPickPoints() 
    DeleteAllPlots()
    ResetPickLetter()

def TestNodeHighlight():
    ResetPickAttributes()
    ClearPickPoints() 
    DeleteAllPlots()
    ResetPickLetter()
    ResetView()

    OpenDatabase(silo_data_path("globe.silo"))
    TurnOffAllAnnotations()
    AddPlot("Pseudocolor", "v")
    DrawPlots()

    pAtts = GetPickAttributes()
    pAtts.showPickHighlight = 1
    pAtts.pickHighlightColor = (230, 100, 0)
    SetPickAttributes(pAtts)
    PickByNode(800)
    pAtts.pickHighlightColor = (0, 0, 255)
    SetPickAttributes(pAtts)
    PickByNode(600)
    pAtts.pickHighlightColor = (255, 0, 0)
    SetPickAttributes(pAtts)
    PickByNode(1000)
    Test("NodeHighlight_00")

    ResetPickAttributes()
    ClearPickPoints() 
    DeleteAllPlots()
    ResetPickLetter()



def PickMain():
    Pick3DTo2D()
    Pick2D()
    PickOnionPeel()
    PickSubset()
    PickMatFracsThreshold()
    PickReflect()
    PickFilledBoundary()
    PickMultiBlock()
    PickSurface()
    PickBigSilMat()
    PickOnionPeel2()
    PickSpecMix()
    PickCurve()
    PickMultiWindow()
    PickAfterEngineClosed()
    PickAfterEngineCrashed()
    PickBoundary()
    PickVectorPlots()
    PickPointMeshes()
    PickGlobalIds()
    PickExpressions()
    PickAMR()
    PickSamrai()
    PickIndexSelect()
    PickTensors()
    PickMili()
    PickContour()
    PickBox()
    PickScatter()
    PickHighlight()
    PickHighlightColor()
    PickRange()
    TestRemovePicks()
    TestRemoveLabeledPicks()
    PickLines3D()
    PickZoneLabel()
    PickNodeLabel()
    PickRangeLabel()
    TestSwivelHighlight()
    TestNodeHighlight()

# Call the main function
TurnOnAllAnnotations()
PickMain()
Exit()

