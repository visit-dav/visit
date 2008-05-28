# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  sami.py 
#
#  Tests:      sami files 
#
#  Programmer: Mark C. Miller 
#  Date:       October 12, 2006 
#
# ----------------------------------------------------------------------------


# test the main mesh
OpenDatabase("../data/bdw.sami")
AddPlot("Mesh","mesh")
DrawPlots()
v=GetView3D()
v.viewNormal = (0.433013, 0.5, -0.75)
v.viewUp = (-0.25, 0.866025, 0.433013)
SetView3D(v)
Test("sami_01")

# test the material object
AddPlot("FilledBoundary","mat")
DrawPlots()
Test("sami_02")
DeleteAllPlots()

# test a node set
AddPlot("Mesh","nodesets/sl5m")
DrawPlots()
Test("sami_03")
DeleteAllPlots()

# test a couple of slide plots
AddPlot("Mesh","slide_0/master")
AddPlot("Mesh","slide_1/master")
AddPlot("Mesh","slide_2/slave")
AddPlot("Mesh","slide_3/slave")
DrawPlots()
Test("sami_04")
DeleteAllPlots()

# test a multi-part sami file
OpenDatabase("../data/sample_sami.visit")
AddPlot("Pseudocolor","global_node_numbers")
DrawPlots()
ResetView()
v=GetView3D()
v.viewNormal = (-0.866025, 0, -0.5)
v.viewUp = (0, 1, 0)
SetView3D(v)
Test("sami_05")
DeleteAllPlots()

AddPlot("Mesh","mesh")
AddPlot("FilledBoundary","mat")
DrawPlots()
Test("sami_06")

# turn off material 2 and domain 0
mat2SetId = -1
dom0SetId = -1
silr = SILRestriction()
for i in range(silr.NumSets()):
    if silr.SetName(i) == "002":
        mat2SetId = i
    if silr.SetName(i) == "domain0":
        dom0SetId = i
silr.TurnOffSet(mat2SetId)
silr.TurnOffSet(dom0SetId)
SetPlotSILRestriction(silr)
DrawPlots()
Test("sami_07")

Exit()
