# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  csg.py
#
#  Tests:      mesh      - a csg mesh
#              subset plot
#              selection - subset
#
#  Programmer: Mark C. Miller 
#  Date:       August 16, 2005
#
#  Mofications:
#
#    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
#    Added tests for materials, variables and Matt's csg data
# ----------------------------------------------------------------------------


OpenDatabase("../data/csg.silo")

mma=MeshManagementAttributes()
mma.discretizationMode = mma.Uniform
mma.discretizationTolerance=(0.01, 0.25)
SetMeshManagementAttributes(mma)

#
# the mesh named "csgmesh" uses just spheres, cylinder and plane
# primitives
#
AddPlot("Mesh", "csgmesh")
DrawPlots()
v = GetView3D()
v.SetViewNormal(0.650274, -0.646958, 0.398232)
v.SetViewUp(-0.290226, 0.272881, 0.917227)
SetView3D(v)
Test("csg_01")

#
# Change the discretization tolerance
#
mma.discretizationTolerance=(0.02, 0.25)
SetMeshManagementAttributes(mma)
ClearWindow()
DrawPlots()
Test("csg_02")
DeleteAllPlots()
ResetView()

#
# Ok, lets try Greg Greenman's example mesh. It uses
# all general quadrics and planes
#
mma.discretizationTolerance=(0.005, 0.25)
mma.discretizationMode = mma.Adaptive
SetMeshManagementAttributes(mma)
AddPlot("Subset", "regions(greenman_mesh)")
DrawPlots()
v = GetView3D()
v.SetViewNormal(-0.496376, -0.386195, -0.777473)
v.SetViewUp(0.865609, -0.152286, -0.477001)
SetView3D(v)
Test("csg_03")

#
# Find set ids using their names
#
voidSetId = -1
uraniumSetId = -1
airSetId = -1
silr = SILRestriction()
for i in range(silr.NumSets()):
    if silr.SetName(i) == "void":
        voidSetId = i
    elif silr.SetName(i) == "uranium":
        uraniumSetId = i
    elif silr.SetName(i) == "air":
        airSetId = i

#
# Turn off the air
#
silr.TurnOffSet(airSetId)
SetPlotSILRestriction(silr)
Test("csg_04")

#
# Turn on air, but turn void and uranium off
#
silr.TurnOnSet(airSetId)
silr.TurnOffSet(voidSetId)
silr.TurnOffSet(uraniumSetId)
SetPlotSILRestriction(silr)
Test("csg_05")

#
# Try a material plot
#
#DeleteAllPlots()
#AddPlot("FilledBoundary","mat")
#AddPlot("Mesh","csgmesh")
#DrawPlots()
#ResetView()
#v = GetView3D()
#v.SetViewNormal(0.650274, -0.646958, 0.398232)
#v.SetViewUp(-0.290226, 0.272881, 0.917227)
#SetView3D(v)
#Test("csg_06")

#
# Try plotting a variable
#
DeleteAllPlots()
AddPlot("Pseudocolor","var1")
AddPlot("Mesh","csgmesh")
DrawPlots()
ResetView()
v = GetView3D()
v.SetViewNormal(0.650274, -0.646958, 0.398232)
v.SetViewUp(-0.290226, 0.272881, 0.917227)
SetView3D(v)
Test("csg_07")

#
# Try various other objects in this file 
#
DeleteAllPlots()

#
# Now use Matt's Fusion data
#
DeleteAllPlots()
CloseDatabase("../data/csg.silo")
mma.discretizationTolerance=(0.05, 0.25)
mma.discretizationMode = mma.Adaptive
SetMeshManagementAttributes(mma)
OpenDatabase("../data/csg_test_data/Fusion_000001_root.gra")
AddPlot("FilledBoundary","MC_Material(MC_ThreeD_Structured)")
DrawPlots()
silr = SILRestriction()
for i in range(silr.NumSets()):
    if silr.SetName(i) == "2 Concrete_B":
        silr.TurnOffSet(i)
    if silr.SetName(i) == "6 Concrete_F":
        silr.TurnOffSet(i)
    if silr.SetName(i) == "7 Concrete_G":
        silr.TurnOffSet(i)
    if silr.SetName(i) == "8 Air_A":
        silr.TurnOffSet(i)
    if silr.SetName(i) == "9 Air_B":
        silr.TurnOffSet(i)
    if silr.SetName(i) == "10 Air_C":
        silr.TurnOffSet(i)
SetPlotSILRestriction(silr)
ResetView()
v = GetView3D()
v.SetViewNormal(-0.5, -0.852869, -0.150384)
v.SetViewUp(0.0, -0.173648, 0.984808)
v.imageZoom = 1.5
SetView3D(v)
Test("csg_08")

Exit()
