# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  clip.py
#
#  Tests:      mesh      - 2D rectilinear, 2D curvilinear,
#                          3D rectilinear, 3D curvilinear,
#                          3D unstructured
#
#  Defect ID:  7470, 7195
#
#  Programmer: Hank Childs
#  Date:       September 1, 2006
#
#  Modifications:
#
#    Hank Childs, Wed Sep  6 16:56:27 PDT 2006
#    Test clip operator with no plane and no sphere (this was crashing the
#    engine due to a memory problem) ['7195]
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Alister Maguire, Fri Nov 13 14:07:54 PST 2020
#    Added a test for the crinkle clip.
#
# ----------------------------------------------------------------------------

def TestOne(filename, index, zonal_var, nodal_var, radius):
   OpenDatabase(filename)
   c = ClipAttributes()
   c.funcType = c.Sphere
   c.radius = radius
   c.sphereInverse = 1
   AddPlot("Pseudocolor", zonal_var)
   AddOperator("Clip")
   SetOperatorOptions(c)
   DrawPlots()
   name = "clip%02d" %(index+0)
   Test(name)
   ChangeActivePlotsVar(nodal_var)
   name = "clip%02d" %(index+1)
   Test(name)
   c.funcType = c.Plane
   c.plane1Origin = (0.5, 0.5, 0)
   c.plane1Normal = (1, 1, 0)
   SetOperatorOptions(c)
   name = "clip%02d" %(index+2)
   Test(name)
   ChangeActivePlotsVar(zonal_var)
   name = "clip%02d" %(index+3)
   Test(name)
   DeleteAllPlots()

TestOne(silo_data_path("rect2d.silo"), 4*0, "d", "u", 0.5)
TestOne(silo_data_path("rect3d.silo"), 4*1, "d", "u", 0.5)
TestOne(silo_data_path("curv2d.silo"), 4*2, "d", "u", 3)
TestOne(silo_data_path("curv3d.silo"), 4*3, "d", "u", 4)
TestOne(silo_data_path("globe.silo"), 4*4, "t", "u", 5)

# Test doing a clip where there are no planes selected.  This has caused
# a crash ['7195].
OpenDatabase(silo_data_path("rect2d.silo"))

AddPlot("Pseudocolor" , "d")
AddOperator("Clip")
c = ClipAttributes()
c.funcType = c.Plane
c.plane1Status = 0
c.plane2Status = 0
c.plane3Status = 0
SetOperatorOptions(c)
DrawPlots()

test_idx = 20
name = "clip%02d" %(test_idx)
Test(name)
test_idx += 1

# Test accurate vs. fast.
DeleteAllPlots()
OpenDatabase(silo_data_path("globe.silo"))

AddPlot("Mesh", "mesh1")
c = ClipAttributes()
c.funcType = c.Plane  # Plane, Sphere
c.plane1Status = 1
c.plane2Status = 1
c.plane3Status = 0
c.plane1Origin = (0, 2.37883, 0)
c.plane2Origin = (0, 2.37883, 0)
c.plane3Origin = (0, 2.37883, 0)
c.plane1Normal = (1, 0, 0)
c.plane2Normal = (0, 1, 0)
c.plane3Normal = (0, 0, 1)
c.planeInverse = 0
c.center = (0, 0, 0)
c.radius = 1
c.sphereInverse = 0
AddOperator("Clip")
SetOperatorOptions(c)

v = View3DAttributes()
v.viewNormal = (0.0548192, 0.0422395, 0.997602)
v.focus = (0.427242, 3.08232, -0.155563)
v.viewUp = (0.195041, 0.979406, -0.0521867)
v.viewAngle = 30
v.parallelScale = 4.21585
v.nearPlane = -34.5981
v.farPlane = 34.5981
v.imagePan = (0, 0)
v.imageZoom = 1
v.perspective = 1
v.eyeAngle = 2
v.centerOfRotationSet = 0
v.centerOfRotation = (0, -0.0372553, 0)

DrawPlots()
SetView3D(v)

Test("clip_globe_fast")

c.quality = c.Accurate
SetOperatorOptions(c)
Test("clip_globe_accurate")

DeleteAllPlots()
ResetView()

#
# Test the crinkle clip.
#
AddPlot("Pseudocolor", "u")
AddOperator("Clip")
c = ClipAttributes()
c.crinkleClip = 1
SetOperatorOptions(c)
DrawPlots()
Test("crinkle_clip_globe")
DeleteAllPlots()

Exit()
