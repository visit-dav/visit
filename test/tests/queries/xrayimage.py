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
#    Kathleen Bonnell, Thu Jul 14 10:44:55 PDT 2011
#    Change most of code to use python dictionary to pass query parameters.
#    First call to 'Query' still tests old-style argument passing.
#    Second call to 'Query' creates a Python dictionary from scratch and
#    uses that.  Prior to third call to Query, retrieve default dictionary via
#    GetQueryParameters.  All subsequent calls to Query modify that dictionary 
#    object as necessary and pass it.
#
# ----------------------------------------------------------------------------

import os

#
# Test a single block structured grid with scalars.
#
OpenDatabase(data_path("silo_%s_test_data/curv3d.silo")%SILO_MODE)

AddPlot("Pseudocolor", "d")
DrawPlots()

# old style argument passing
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

OpenDatabase(data_path("silo_%s_test_data/multi_curv3d.silo")%SILO_MODE)

AddPlot("Pseudocolor", "d")
DrawPlots()

#create our own dictionary
params = dict(output_type="png", divide_emis_by_absorb=1, origin=(0.0, 2.5, 10.0), theta=0, phi=0, width = 10., height=10., image_size=(300, 300), vars=("da", "pa"))
Query("XRay Image", params)

os.rename("output00.png", "current/queries/xrayimage/xrayimage02.png")
os.rename("output01.png", "current/queries/xrayimage/xrayimage03.png")
Test("xrayimage02", 0, 1)
Test("xrayimage03", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage04", s)

#
# Test a 2d structured grid with scalars.
#
DeleteAllPlots()

OpenDatabase(data_path("silo_%s_test_data/curv2d.silo")%SILO_MODE)

AddPlot("Pseudocolor", "d")
DrawPlots()

#retreive default query parameters
params = GetQueryParameters("XRay Image")
#modify as necessary
params['image_size'] = (300, 300)
params['divide_emis_by_absorb'] = 1
params['width'] = 10.
params['height'] = 10.
params['vars'] = ("d", "p")
Query("XRay Image", params)

os.rename("output00.png", "current/queries/xrayimage/xrayimage05.png")
Test("xrayimage05", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage06", s)

params['theta'] = 90
params['phi'] =  0
Query("XRay Image", params)

os.rename("output00.png", "current/queries/xrayimage/xrayimage07.png")
Test("xrayimage07", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage08", s)

#
# Test an unstructured grid with scalars.
#
DefineScalarExpression("u1", 'recenter(((u+10.)*0.01), "zonal")')
DefineScalarExpression("v1", 'recenter(((v+10.)*0.01*matvf(mat1,1)), "zonal")')
DefineScalarExpression("v2", 'recenter(((v+10.)*0.01*matvf(mat1,2)), "zonal")')
DefineScalarExpression("v3", 'recenter(((v+10.)*0.01*matvf(mat1,3)), "zonal")')
DefineScalarExpression("v4", 'recenter(((v+10.)*0.01*matvf(mat1,4)), "zonal")')
DefineScalarExpression("w1", 'recenter(((w+10.)*0.01), "zonal")')

DeleteAllPlots()

OpenDatabase(data_path("silo_%s_test_data/globe.silo")%SILO_MODE)

AddPlot("Pseudocolor", "u")
DrawPlots()

# Do tets.
params['theta'] = 0
params['phi'] = 0
params['width'] = 1.
params['height'] = 1.
params['vars'] = ("w1", "v1")
Query("XRay Image", params)

os.rename("output00.png", "current/queries/xrayimage/xrayimage09.png")
Test("xrayimage09", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage10", s)

params['theta'] = 90
params['width'] = 4.
params['height'] = 4.

Query("XRay Image", params)

os.rename("output00.png", "current/queries/xrayimage/xrayimage11.png")
Test("xrayimage11", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage12", s)

# Do pyramids.
params['theta'] = 0
params['vars'] = ("w1", "v2")
Query("XRay Image", params) 

os.rename("output00.png", "current/queries/xrayimage/xrayimage13.png")
Test("xrayimage13", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage14", s)

params['theta'] = 90
Query("XRay Image", params)

os.rename("output00.png", "current/queries/xrayimage/xrayimage15.png")
Test("xrayimage15", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage16", s)

# Do wedges.
params['theta'] = 0
params['width'] = 8.
params['height'] = 8.
params['vars'] = ("w1", "v3")
Query("XRay Image", params)

os.rename("output00.png", "current/queries/xrayimage/xrayimage17.png")
Test("xrayimage17", 0, 1)


s = GetQueryOutputString()
TestText("xrayimage18", s)

params['theta'] = 90
params['width'] = 20.
params['height'] = 20.
Query("XRay Image", params)

os.rename("output00.png", "current/queries/xrayimage/xrayimage19.png")
Test("xrayimage19", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage20", s)

# Do hexes.
params['theta'] = 0
params['vars'] = ("w1", "v4")
Query("XRay Image", params)

os.rename("output00.png", "current/queries/xrayimage/xrayimage21.png")
Test("xrayimage21", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage22", s)

params['theta'] = 90
Query("XRay Image", params)

os.rename("output00.png", "current/queries/xrayimage/xrayimage23.png")
Test("xrayimage23", 0, 1)

s = GetQueryOutputString()
TestText("xrayimage24", s)

Exit()
