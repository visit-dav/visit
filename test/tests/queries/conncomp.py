# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  conncomp.py
#  Tests:      queries     - connected components related 
#
#  Programmer: Cyrus Harrison
#  Date:       February 22, 2006
#
#  Modifications:
#    Cyrus Harrison, Thu Aug 23 09:42:43 PDT 2007
#    Turned off ghost neighbors optimization for exodus balls dataset.
#
# ----------------------------------------------------------------------------

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

OpenDatabase("../data/multi_rect2d.silo", 0)
DefineScalarExpression("_rand_test_2d", "rand(mesh1)")
DefineScalarExpression("_ccl_test_2d", "conn_components(mesh1)")

AddPlot("Pseudocolor", "_ccl_test_2d")
# Add isovolume op to create components
AddOperator("Isovolume")
IsovolumeAtts = IsovolumeAttributes()
IsovolumeAtts.lbound = -1e+37
IsovolumeAtts.ubound = 0.3
IsovolumeAtts.variable = "_rand_test_2d"
SetOperatorOptions(IsovolumeAtts)

#Add Defer Exp Op
AddOperator("DeferExpression")
DeferExpressionAtts = DeferExpressionAttributes()
DeferExpressionAtts.exprs = ("_ccl_test_2d")
SetOperatorOptions(DeferExpressionAtts)

DrawPlots()

Test("conncomp_2d_lbl")

# test the connected components related queries
Query("Number of Connected Components")
res = GetQueryOutputString()
TestText("conncomp_2d_count",res)

Query("Connected Component Centroids")
res = GetQueryOutputString()
TestText("conncomp_2d_centroid",res)

Query("Connected Component Area")
res = GetQueryOutputString()
TestText("conncomp_2d_area",res)

Query("Connected Component Variable Sum")
res = GetQueryOutputString()
TestText("conncomp_2d_var_sum",res)

Query("Connected Component Weighted Variable Sum")
res = GetQueryOutputString()
TestText("conncomp_2d_weighted_var_sum",res)



DeleteAllPlots()

DefineScalarExpression("_ccl_test_3d", "conn_components(Mesh,0)")
# exodus test
OpenDatabase("../data/exodus_test_data/balls.exodus")

AddPlot("Pseudocolor", "_ccl_test_3d")
silr = SILRestriction()
silr.TurnOnAll()
for silSet in (24,25,26,43,44,45,62,63,64,81,82,83):
    silr.TurnOffSet(silSet)
SetPlotSILRestriction(silr)

AddOperator("Displace")
DisplaceAtts = DisplaceAttributes()
DisplaceAtts.factor = 1
DisplaceAtts.variable = "DISPL"
SetOperatorOptions(DisplaceAtts)
DisplaceAtts = DisplaceAttributes()
DisplaceAtts.factor = 1
DisplaceAtts.variable = "DISPL"
SetOperatorOptions(DisplaceAtts)

AddOperator("Transform")
t = TransformAttributes()
t.doScale = 1
t.scaleX = 100
t.scaleY = 100
t.scaleZ = 100
SetOperatorOptions(t)

d = DeferExpressionAttributes()
d.exprs = ("_ccl_test_3d")
SetDefaultOperatorOptions(d)
AddOperator("DeferExpression")


v=GetView3D()
v.viewNormal=(.1,.1, 0)
v.viewUp=(0, 0, 1 )
SetView3D(v)

DrawPlots()

# test as displacement evolves
Test("conncomp_3d_t1");

# test the connected components related queries
Query("Number of Connected Components")
res = GetQueryOutputString()
TestText("conncomp_3d_count_t1",res)

Query("Connected Component Centroids")
res = GetQueryOutputString()
TestText("conncomp_3d_centroid_t1",res)


Query("Connected Component Volume")
res = GetQueryOutputString()
TestText("conncomp_3d_volume_t1",res)

SetTimeSliderState(11);
# test as displacement evolves
Test("conncomp_3d_t2");

Query("Number of Connected Components")
res = GetQueryOutputString()
TestText("conncomp_3d_count_t2",res)

Query("Connected Component Centroids")
res = GetQueryOutputString()
TestText("conncomp_3d_centroid_t2",res)


Query("Connected Component Volume")
res = GetQueryOutputString()
TestText("conncomp_3d_volume_t2",res)


SetTimeSliderState(20);
Test("conncomp_3d_t3");

Query("Number of Connected Components")
res = GetQueryOutputString()
TestText("conncomp_3d_count_t3",res)

Query("Connected Component Centroids")
res = GetQueryOutputString()
TestText("conncomp_3d_centroid_t3",res)


Query("Connected Component Volume")
res = GetQueryOutputString()
TestText("conncomp_3d_volume_t3",res)



Exit()
