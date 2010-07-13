# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  xrayimage.py
#  Tests:      queries     - xray imagel
#
#  Programmer: Eric Brugger
#  Date:       July 13, 2010
#
#  Modifications:
# 
# ----------------------------------------------------------------------------

import os

#
# Test a single block structured grid with scalars.
#
OpenDatabase("/usr/gapps/visit/data/curv3d.silo")
AddPlot("Pseudocolor", "d")
DrawPlots()

Query("XRay Image", "png", 1, 0.0, 2.5, 10.0, 0, 0, 10., 10., 300, 300, ("d", "p"))

if not os.path.isdir("current/queries"):
    os.mkdir("current/queries")
if not os.path.isdir("current/queries/xrayimage"):
    os.mkdir("current/queries/xrayimage")
os.rename("output00.png", "current/queries/xrayimage/xrayimage00.png")
Test("xrayimage00", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage01", s)

#
# Test a multi block structured grid with an array variable.
#
DefineScalarExpression("d1", 'recenter(d, "zonal")')
DefineScalarExpression("p1", 'recenter(p, "zonal")')

DefineArrayExpression("da", "array_compose(d1,d1)")
DefineArrayExpression("pa", "array_compose(p1,p1)")

DeleteAllPlots()

OpenDatabase("/usr/gapps/visit/data/multi_curv3d.silo")
AddPlot("Pseudocolor", "d")
DrawPlots()

Query("XRay Image", "png", 1, 0.0, 2.5, 10.0, 0, 0, 10., 10., 300, 300, ("da", "pa"))

os.rename("output00.png", "current/queries/xrayimage/xrayimage02.png")
os.rename("output01.png", "current/queries/xrayimage/xrayimage03.png")
Test("xrayimage02", 0, 1)
Test("xrayimage03", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage04", s)

Exit()
