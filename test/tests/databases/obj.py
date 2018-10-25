# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  obj.py
#
#  Tests:      Wavefront OBJ reader
#                  - relative indexing
#                  - groups w/ and w/o multiple inclusion via enum scalar
#                  - groups with coloration (via material)
#
#  Programmer: Mark C. Miller, Tue Oct 23 16:09:00 PDT 2018
#
# ----------------------------------------------------------------------------
def TurnOnSetsByName(silr, setNames):
    silr.TurnOffAll()
    silr.TurnOnSet(1) # the block0 set
    for i in range(silr.NumSets()):
        if silr.SetName(i) in setNames:
            silr.TurnOnSet(i)

OpenDatabase(data_path("obj_test_data/cube2.obj"))

AddPlot("Mesh", "OBJMesh")
ma = MeshAttributes()
ma.opaqueColorSource = ma.OpaqueCustom
ma.opaqueColor = (192, 192, 192)
DrawPlots()
SetPlotOptions(ma)

v = GetView3D()
v.viewNormal = (0.00415051, 0.90155, -0.432654)
v.viewUp = (0.959161, 0.118781, 0.256713)
SetView3D(v)

Test("obj_cube")

silr = SILRestriction()
setsToTest = ("face1","face2","face3","face4","face5","face6","tri1","tri2","box")
for set in setsToTest:
    TurnOnSetsByName(silr, (set,))
    SetPlotSILRestriction(silr)
    Test("obj_%s"%set)

silr.TurnOnAll()
SetPlotSILRestriction(silr)
DeleteAllPlots()
CloseDatabase(data_path("obj_test_data/cube2.obj"))
OpenDatabase(data_path("obj_test_data/cube1.obj"))

# Test coloration of faces from matlib
AddPlot("FilledBoundary", "GroupsAsMaterials")
DrawPlots()
v.RotateAxis(0,30)
SetView3D(v)
Test("obj_colors")

DeleteAllPlots()
CloseDatabase(data_path("obj_test_data/cube1.obj"))
OpenDatabase(data_path("obj_test_data/cube3.obj"))

# Test coloration of faces from immediate mode colors
# Should be same picture as above
AddPlot("FilledBoundary", "GroupsAsMaterials")
DrawPlots()
Test("obj_immediate_colors")

DeleteAllPlots()
CloseDatabase(data_path("obj_test_data/cube3.obj"))
OpenDatabase(data_path("obj_test_data/test.obj"))

# Test various objects from Scott's example database
AddPlot("FilledBoundary", "GroupsAsMaterials")
DrawPlots()
ResetView()
Test("obj_scott1")
silr = SILRestriction()
silr.TurnOffSet(17) # Plane
SetPlotSILRestriction(silr)
Test("obj_scott2")
silr.TurnOffSet(18) # Prism
SetPlotSILRestriction(silr)
Test("obj_scott3")

Exit()
