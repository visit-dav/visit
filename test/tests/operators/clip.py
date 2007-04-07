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
# ----------------------------------------------------------------------------

# Turn off all annotation
a = AnnotationAttributes()
a.axesFlag2D = 0
a.axesFlag = 0
a.triadFlag = 0
a.bboxFlag = 0
a.userInfoFlag = 0
a.databaseInfoFlag = 0
a.legendInfoFlag = 0
a.backgroundMode = 0
a.foregroundColor = (0, 0, 0, 255)
a.backgroundColor = (255, 255, 255, 255)
SetAnnotationAttributes(a)

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

TestOne("../data/rect2d.silo", 4*0, "d", "u", 0.5)
TestOne("../data/rect3d.silo", 4*1, "d", "u", 0.5)
TestOne("../data/curv2d.silo", 4*2, "d", "u", 3)
TestOne("../data/curv3d.silo", 4*3, "d", "u", 4)
TestOne("../data/globe.silo", 4*4, "t", "u", 5)

# Test doing a clip where there are no planes selected.  This has caused
# a crash ['7195].
OpenDatabase("../data/rect2d.silo")
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


Exit()
