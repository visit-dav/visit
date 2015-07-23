# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  sph_resample.py
#
#  Tests:      mesh    - 2D rectilinear, single domain,
#              plots   - pseudocolor
#
#  Defect ID:  2338 
#
#  Programmer: Kevin Griffin
#  Date:       Wed Jul 22 17:36:55 PDT 2015
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# 2D, Rectilinear
ds = data_path("silo_sph_test_data/KelvinHelmholtz-2d_McNally.visit") 
OpenDatabase(ds)

AddPlot("Pseudocolor", "mass_density") 
AddOperator("SPHResample")

SPHResampleAtts = SPHResampleAttributes()
SPHResampleAtts.minX = 0
SPHResampleAtts.maxX = 1
SPHResampleAtts.xnum = 100
SPHResampleAtts.minY = 0
SPHResampleAtts.maxY = 1
SPHResampleAtts.ynum = 100
SPHResampleAtts.minZ = 0
SPHResampleAtts.maxZ = 1
SPHResampleAtts.znum = 1
SPHResampleAtts.tensorSupportVariable = "H"
SPHResampleAtts.weightVariable = "mass"
SPHResampleAtts.RK = 1
SetOperatorOptions(SPHResampleAtts, 1)
DrawPlots()

Test("sph_2d")

DeleteAllPlots()
CloseDatabase(ds)

Exit()
