# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  globe.py
#
#  Tests:      mesh      - 3D unstructured, single domain
#              plots     - pc, contour, mat subset, mesh, vector
#              operators - none
#              selection - none
#
#  Defect ID:  none
#
#  Programmer: Jeremy Meredith
#  Date:       April 17, 2002
#
#  Modificatons:
#    Kathleen Bonnell, Thu Sep  5 10:55:47 PDT 2002
#    Changed the variable format for SubsetPlots, to reflect new interface
#    which allows subsets of groups.
#
#    Kathleen Bonnell, Thu Aug 28 14:34:57 PDT 2003
#    Remove compound var name from subset plots.
#
#    Kathleen Bonnell, Wed Sep  3 09:31:25 PDT 2003
#    Opaque mode for MeshPlot is ON by default now, so turn it off
#    for this test.
#
#    Kathleen Bonnell, Thu Sep  4 11:10:11 PDT 2003
#    Change the way MeshPlot's opaque mode is set (now an enum, not a bool).
#
# ----------------------------------------------------------------------------

TurnOffAllAnnotations() # defines global object 'a'

OpenDatabase("../data/globe.silo")

AddPlot("Pseudocolor", "u")
DrawPlots()

v = GetView3D()
v.SetViewNormal(-0.528889, 0.367702, 0.7649)
v.SetViewUp(0.176641, 0.929226, -0.324558)
v.SetParallelScale(17.3205)
v.SetPerspective(1)
SetView3D(v)

Test("globe_01")

DeleteAllPlots()
AddPlot("Contour", "t")
DrawPlots()
Test("globe_02")

DeleteAllPlots()
AddPlot("Subset", "mat1")
DrawPlots()
Test("globe_03")

DeleteAllPlots()
AddPlot("Volume", "u")
DrawPlots()
Test("globe_04")

#Test MeshPlot with OpaqueFlag ON
DeleteAllPlots()
AddPlot("Mesh", "mesh1")
m = MeshAttributes()
m.opaqueMode = m.Off
SetPlotOptions(m)
DrawPlots()
Test("globe_05")

DeleteAllPlots()
AddPlot("Vector", "vel")
DrawPlots()
Test("globe_06")

Exit()
