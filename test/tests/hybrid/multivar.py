# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  multivar.py
#
#  Tests:      mesh      - 3D unstructured, single domain
#              plots     - pc, vector, mat subset
#              operators - isosurface
#
#  Defect ID:  '7843
#
#  Programmer: Hank Childs
#  Date:       April 24, 2002
#
#  Modifications:
#    Kathleen Bonnell, Thu Sep  5 10:55:47 PDT 2002
#    Changed the variable format for SubsetPlots, to reflect new interface
#    which allows subsets of groups.
#
#    Kathleen Bonnell, Thu Aug 28 14:34:57 PDT 2003
#    Remove compound var name from subset plots.
#
#    Jeremy Meredith, Thu Jun 24 12:58:06 PDT 2004
#    Set the vector origin explicitly for some cases because I changed the
#    default to Tail.
#
#    Hank Childs, Tue Mar 13 09:42:24 PDT 2007
#    Add new test for mixed centering multiple variables and expressions.
#
# ----------------------------------------------------------------------------



OpenDatabase("../data/globe.silo")


# Set up the isosurface operators attributes

atts = IsosurfaceAttributes()
atts.contourMethod = atts.Value
atts.contourValue = 500
atts.variable = "t"
SetDefaultOperatorOptions(atts)


# Test the PC plot with the isosurface operator.

AddPlot("Pseudocolor", "u")
AddOperator("Isosurface")
DrawPlots()

v = GetView3D()
v.SetViewNormal(-0.528889, 0.367702, 0.7649)
v.SetViewUp(0.176641, 0.929226, -0.324558)
v.SetParallelScale(17.3205)
v.SetPerspective(1)
SetView3D(v)

Test("multivar_01")
DeleteAllPlots()


# Test the subset plot with the isosurface operator.

AddPlot("Subset", "mat1")
AddOperator("Isosurface")
DrawPlots()
Test("multivar_02")
DeleteAllPlots()


# Test the subset plot with the isosurface operator.

AddPlot("Vector", "vel")
vec=VectorAttributes()
vec.useStride = 1
vec.stride = 1
vec.vectorOrigin = vec.Middle
SetPlotOptions(vec)
AddOperator("Isosurface")
DrawPlots()
Test("multivar_03")
DeleteAllPlots()

# Test multiple variables with different centerings with expressions ('7843)
OpenDatabase("../data/curv3d.silo")
DefineScalarExpression("x", "coord(curvmesh3d)[0]")
DefineScalarExpression("y", "coord(curvmesh3d)[1]")
DefineScalarExpression("z", "coord(curvmesh3d)[2]")
DefineScalarExpression("myvar", "if(and(lt(x,0),gt(z,0)), abs(x*z), -abs(x*z))")
AddPlot("Pseudocolor", "myvar")
AddOperator("Isosurface")
atts.variable = "d"
atts.contourValue = 3
SetOperatorOptions(atts)
DrawPlots()
Test("multivar_04")

Exit()
