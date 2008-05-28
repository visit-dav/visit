# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  emptydomains.py
#
#  Tests:      multi-objects with empty domains
#
#  Defect ID:  VisIt00005226
#
#  Programmer: Jeremy Meredith
#  Date:       June  7, 2005
#
#  Modificatons:
#
# ----------------------------------------------------------------------------

TurnOffAllAnnotations() # defines global object 'a'

OpenDatabase("../data/emptydomains.silo")

# Test the multi-mesh support
AddPlot("Mesh", "mesh")
DrawPlots()

v = GetView3D()
v.SetViewNormal(-0.528889, 0.367702, 0.7649)
v.SetViewUp(0.176641, 0.929226, -0.324558)
v.SetPerspective(1)
SetView3D(v)

Test("meshtype_emptydomains_01")

# Test the multi-var support
AddPlot("Pseudocolor", "density")
DrawPlots()
Test("meshtype_emptydomains_02")

# Test material selection support
TurnMaterialsOff("5")
Test("meshtype_emptydomains_03")

# Test multi-material support
TurnMaterialsOn()
DeleteAllPlots()
AddPlot("FilledBoundary", "mat")
DrawPlots()
Test("meshtype_emptydomains_04")

# Test multi-species/species selection support
DeleteAllPlots()
AddPlot("Pseudocolor","spec")
silr=SILRestriction()
silr.TurnOffSet(6)
SetPlotSILRestriction(silr)
DrawPlots()
Test("meshtype_emptydomains_05")

silr.TurnOffSet(5)
SetPlotSILRestriction(silr)
Test("meshtype_emptydomains_06")

# Test domain selection support
DeleteAllPlots()
silr.TurnOnAll()
silr.TurnOffSet(1)
SetPlotSILRestriction(silr)
AddPlot("Mesh", "mesh")
AddPlot("Pseudocolor", "density")
DrawPlots()
Test("meshtype_emptydomains_07")

# Test slice-by-zone
silr.TurnOnAll()
SetPlotSILRestriction(silr)
DeleteAllPlots()
AddPlot("Pseudocolor", "density")
AddOperator("Slice")
s1 = SliceAttributes()
s1.originType = s1.Zone
s1.project2d = 0
s1.originZone = 170
s1.originZoneDomain = 2
SetOperatorOptions(s1)
DrawPlots()
Test("meshtype_emptydomains_08")

# Test slice-by-node
silr.TurnOnAll()
SetPlotSILRestriction(silr)
DeleteAllPlots()
AddPlot("Pseudocolor", "density")
AddOperator("Slice")
s2 = SliceAttributes()
s2.originType = s2.Node
s2.project2d = 0
s2.originNode = 11
s2.originNodeDomain = 2
SetOperatorOptions(s2)
DrawPlots()
Test("meshtype_emptydomains_09")

# Test onion peel
DeleteAllPlots()
AddPlot("Pseudocolor", "density")
AddOperator("OnionPeel")
o = OnionPeelAttributes()
o.categoryName = "domains"
o.subsetName = "domain2"
o.index = (210)
o.requestedLayer = 3
DrawPlots()
SetOperatorOptions(o)
Test("meshtype_emptydomains_10")

# Test zone pick
DeleteAllPlots()
AddPlot("Pseudocolor", "density")
DrawPlots()
PickByZone(120, 2, "density")
TestText("meshtype_emptydomains_11", GetPickOutput())

# Test node pick
DeleteAllPlots()
AddPlot("Pseudocolor", "density")
DrawPlots()
PickByNode(100, 2, "density")
TestText("meshtype_emptydomains_12", GetPickOutput())

Exit()
