# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  rect3d.py
#
#  Tests:      mesh      - 3D regular, single domain
#              plots     - pc, contour, mat subset, mesh
#              operators - none
#              selection - material
#
#  Defect ID:  none
#
#  Programmer: Jeremy Meredith
#  Date:       April 17, 2002
#
#  Modifications:
#
#    Hank Childs, Wed Apr 24 10:04:35 PDT 2002
#    Turn off opaque mesh.
#
#    Kathleen Bonnell, Thu Sep  5 10:55:47 PDT 2002
#    Changed the variable format for SubsetPlots, to reflect new interface
#    which allows subsets of groups.
#
#    Kathleen Bonnell, Thu Aug 28 14:34:57 PDT 2003
#    Remove compound var name from subset plots.
#
#    Kathleen Bonnell, Thu Sep  4 11:10:11 PDT 2003
#    Change the way MeshPlot's opaque mode is set (now an enum, not a bool).
#
# ----------------------------------------------------------------------------


OpenDatabase("../data/rect3d.silo")

AddPlot("Pseudocolor", "d")
pc=PseudocolorAttributes()
pc.opacity=0.2
SetPlotOptions(pc)

AddPlot("Mesh", "quadmesh3d")
m1=MeshAttributes()
m1.opaqueMode=m1.Off
SetPlotOptions(m1)

AddPlot("Contour", "u")

# Turn off alternate materials.
silr=SILRestriction()
onoff = 0
for sets in silr.SetsInCategory('mat1'):
    if(onoff == 0):
        silr.TurnOffSet(sets)
        onoff = 1
    else:
        onoff = 0

AddPlot("Subset", "mat1")
SetPlotSILRestriction(silr)

DrawPlots()

Test("rect3d_01")

v = GetView3D()
v.SetViewNormal(.7,.6,.5)
SetView3D(v)

Test("rect3d_02")

Exit()
