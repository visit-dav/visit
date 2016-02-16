# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  LCS.py
#
#  Tests:      operator   - LCS
#
#  Programmer: Allen Sanderson
#  Date:       August 25, 2015
# ----------------------------------------------------------------------------

#-vargs="-debug 5"

# For testing using the CLI

#def data_path( db_name ):
#  db="/Projects/VisIt/trunk/build/data/%s" %(db_name)
#  return db

#def Test(case_name):
#   swatts = SaveWindowAttributes()
#   swatts.family = 0
#   swatts.fileName = "/Projects/tmp/lcs/ser/%s" %(case_name)
#   SetSaveWindowAttributes(swatts)
#   SaveWindow()
#   return

#def TestSection(tmpstr):
#   tmpstr
#   return

#def Exit():
#   return   


# Open the database here and add a plot as for some reason it fails
# within a loop. It only happens with all-in-one plots with an operator
# such as with "Pseudocolor" and "operators/LCS/velocity"
db=data_path("pics_test_data/ftle_double_gyre_1_domain.pics")
OpenDatabase(db)
AddPlot("Pseudocolor", "operators/LCS/velocity")
AddOperator("IntegralCurve")

LCSAtts = LCSAttributes()
LCSAtts.Resolution = (101, 51, 1)
LCSAtts.integrationDirection = LCSAtts.Forward  # Forward, Backward, Both
LCSAtts.auxiliaryGridSpacing = 0.005
LCSAtts.maxSteps = 1000000
LCSAtts.operationType = LCSAtts.EigenVector  # IntegrationTime, ArcLength, AverageDistanceFromSeed, EigenValue, EigenVector, Lyapunov
LCSAtts.cauchyGreenTensor = LCSAtts.Right  # Left, Right
LCSAtts.eigenComponent = LCSAtts.PosLambdaShearVector  # Smallest, Intermediate, Largest, PosShearVector, NegShearVector, PosLambdaShearVector, NegLambdaShearVector
LCSAtts.eigenWeight = 0.98
LCSAtts.operatorType = LCSAtts.BaseValue  # BaseValue, Gradient
LCSAtts.terminationType = LCSAtts.Time  # Time, Distance, Size
LCSAtts.terminateByTime = 1
LCSAtts.termTime = 10
LCSAtts.maxStepLength = 0.001
LCSAtts.integrationType = LCSAtts.AdamsBashforth  # Euler, Leapfrog, DormandPrince, AdamsBashforth, RK4, M3DC12DIntegrator
LCSAtts.parallelizationAlgorithmType = LCSAtts.ParallelStaticDomains  # LoadOnDemand, ParallelStaticDomains, MasterSlave, VisItSelects
LCSAtts.pathlines = 1
LCSAtts.pathlinesCMFE = LCSAtts.CONN_CMFE  # CONN_CMFE, POS_CMFE
SetOperatorOptions(LCSAtts, 0)


IntegralCurveAtts = IntegralCurveAttributes()
IntegralCurveAtts.sourceType = IntegralCurveAtts.Point  # Point, PointList, Line_, Circle, Plane, Sphere, Box, Selection, FieldData
IntegralCurveAtts.pointSource = (1.23053, 0.624189, 0)
IntegralCurveAtts.dataValue = IntegralCurveAtts.SeedPointID  # Solid, SeedPointID, Speed, Vorticity, ArcLength, TimeAbsolute, TimeRelative, AverageDistanceFromSeed, CorrelationDistance, Difference, Variable
IntegralCurveAtts.integrationDirection = IntegralCurveAtts.ForwardDirectionless  # Forward, Backward, Both, ForwardDirectionless, BackwardDirectionless, BothDirectionless
IntegralCurveAtts.maxSteps = 2000
IntegralCurveAtts.maxStepLength = 0.001
IntegralCurveAtts.integrationType = IntegralCurveAtts.AdamsBashforth  # Euler, Leapfrog, DormandPrince, AdamsBashforth, RK4, M3DC12DIntegrator
IntegralCurveAtts.parallelizationAlgorithmType = IntegralCurveAtts.ParallelStaticDomains  # LoadOnDemand, ParallelStaticDomains, MasterSlave, VisItSelects
SetOperatorOptions(IntegralCurveAtts, 0)


databases=["ftle_double_gyre_1_domain", "ftle_double_gyre_2_domains"]

src_type=[LCSAtts.RegularGrid, LCSAtts.NativeMesh]
src_type_str=["RegularGrid", "NativeMesh"]

aux_grid=[LCSAtts.TwoDim]
aux_grid_str=["2DAuxGrid"]

# FTLE with integral curve
TestSection("Basic FTLE function with integral curve operator")

for i in range(len(databases)):
  db=data_path("pics_test_data/%s.pics") %(databases[i])
  tmpstr="Testing database = %s" %(db)
  TestSection(tmpstr)
  OpenDatabase(db)
  # Replace the database from before with this one as a new plot can
  # not be opened within the loop when using runtest. This issue is a
  # bug.
  ReplaceDatabase(db)
  #  DeleteAllPlots()
  #  AddPlot("Pseudocolor", "operators/LCS/velocity")
  for j in range(len(src_type)):
     tmpstr="Testing sample source = %s" %(src_type_str[j])
     TestSection(tmpstr)
     LCSAtts.sourceType = src_type[j]  # NativeMesh, RegularGrid
     for k in range(len(aux_grid)):
        tmpstr="Testing auxiliary grid = %s" %(aux_grid_str[k])
        TestSection(tmpstr)
        tmpstr="lcs_%s_%s_%s_IntegralCurve" %(databases[i], src_type_str[j], aux_grid_str[k])
        LCSAtts.auxiliaryGrid = aux_grid[k]  # TwoDim
        SetOperatorOptions(LCSAtts, 0)
        DrawPlots()
        Test(tmpstr)

Exit()


# LCS->IC single   domain - native mesh      - serial   - okay
# LCS->IC single   domain - rectilinear grid - serial   - okay
# LCS->IC multiple domain - native mesh      - serial   - okay
# LCS->IC multiple domain - rectilinear grid - serial   - failed zero velocity

# LCS->IC single   domain - native mesh      - parallel - okay
# LCS->IC single   domain - rectilinear grid - parallel - okay
# LCS->IC multiple domain - native mesh      - parallel - okay
# LCS->IC multiple domain - rectilinear grid - parallel - failed in avtPICSFilter::InitializeLocators (fixed but failed like serial)
