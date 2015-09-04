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

#def TestSection(str):
#   return

#def Exit():
#   return   


# Open the database here and add a plot as for some reason it fails
# within a loop. It only happens with all-in-one plots with an operator
# such as with "Pseudocolor" and "operators/LCS/velocity"
db=data_path("pics_test_data/ftle_double_gyre_1_domain.pics")
OpenDatabase(db)
AddPlot("Pseudocolor", "operators/LCS/velocity")


LCSAtts = LCSAttributes()
LCSAtts.Resolution = (101, 51, 1)
LCSAtts.integrationDirection = LCSAtts.Forward  # Forward, Backward, Both
LCSAtts.auxiliaryGridSpacing = 0.005
LCSAtts.maxSteps = 1000000
LCSAtts.operationType = LCSAtts.Lyapunov  # IntegrationTime, ArcLength, AverageDistanceFromSeed, EigenValue, EigenVector, Lyapunov
LCSAtts.cauchyGreenTensor = LCSAtts.Right  # Left, Right
LCSAtts.eigenComponent = LCSAtts.Largest  # Smallest, Intermediate, Largest, PosShearVector, NegShearVector, PosLambdaShearVector, NegLambdaShearVector
LCSAtts.operatorType = LCSAtts.BaseValue  # BaseValue, Gradient
LCSAtts.terminationType = LCSAtts.Time  # Time, Distance, Size
LCSAtts.terminateByTime = 1
LCSAtts.termTime = 4
LCSAtts.maxStepLength = 0.001
LCSAtts.integrationType = LCSAtts.AdamsBashforth  # Euler, Leapfrog, DormandPrince, AdamsBashforth, RK4, M3DC12DIntegrator
LCSAtts.parallelizationAlgorithmType = LCSAtts.ParallelStaticDomains  # LoadOnDemand, ParallelStaticDomains, MasterSlave, VisItSelects
LCSAtts.pathlines = 1
LCSAtts.pathlinesCMFE = LCSAtts.CONN_CMFE  # CONN_CMFE, POS_CMFE
SetOperatorOptions(LCSAtts, 0)


databases=["ftle_double_gyre_1_domain", "ftle_double_gyre_2_domains"]

src_type=[LCSAtts.RegularGrid, LCSAtts.NativeMesh]
src_type_str=["RegularGrid", "NativeMesh"]

aux_grid=[LCSAtts.None, LCSAtts.TwoDim]
aux_grid_str=["NoAuxGrid", "2DAuxGrid"]

TestSection("Basic FTLE function")
for i in range(len(databases)):
  db=data_path("pics_test_data/%s.pics") %(databases[i])
  str="Testing database = %s" %(db)
  TestSection(str)
  OpenDatabase(db)
  # Replace the database from before with this one as a new plot can
  # not be opened within the loop when using runtest. This issue is a
  # bug.
  ReplaceDatabase(db)
  #  DeleteAllPlots()
  #  AddPlot("Pseudocolor", "operators/LCS/velocity")
  for j in range(len(src_type)):
     str="Testing sample source = %s" %(src_type_str[j])
     TestSection(str)
     LCSAtts.sourceType = src_type[j]  # NativeMesh, RegularGrid
     for k in range(len(aux_grid)):
        str="Testing auxiliary grid = %s" %(aux_grid_str[k])
        TestSection(str)
        LCSAtts.auxiliaryGrid = aux_grid[k]  # None, TwoDim
        SetOperatorOptions(LCSAtts, 0)
        DrawPlots()
        str="lcs_%s_%s_%s" %(databases[i], src_type_str[j], aux_grid_str[k])
        Test(str)



# FTLE with smallest exponent
LCSAtts.eigenComponent = LCSAtts.Smallest  # Smallest, Intermediate, Largest, PosShearVector, NegShearVector, PosLambdaShearVector, NegLambdaShearVector
SetOperatorOptions(LCSAtts, 0)
DrawPlots()
str="lcs_%s_%s_%s_Smallest" %(databases[i], src_type_str[j], aux_grid_str[k])
Test(str)



# FTLE with left Cauchy Green Tensor
LCSAtts.cauchyGreenTensor = LCSAtts.Left  # Left, Right
LCSAtts.eigenComponent = LCSAtts.Largest  # Smallest, Intermediate, Largest, PosShearVector, NegShearVector, PosLambdaShearVector, NegLambdaShearVector
SetOperatorOptions(LCSAtts, 0)
DrawPlots()
str="lcs_%s_%s_%s_Left" %(databases[i], src_type_str[j], aux_grid_str[k])
Test(str)



# FTLE with eigen value
LCSAtts.operationType = LCSAtts.EigenValue  # IntegrationTime, ArcLength, AverageDistanceFromSeed, EigenValue, EigenVector, Lyapunov
LCSAtts.cauchyGreenTensor = LCSAtts.Right  # Left, Right
LCSAtts.eigenComponent = LCSAtts.Largest  # Smallest, Intermediate, Largest, PosShearVector, NegShearVector, PosLambdaShearVector, NegLambdaShearVector
SetOperatorOptions(LCSAtts, 0)
DrawPlots()
str="lcs_%s_%s_%s_EigenValue" %(databases[i], src_type_str[j], aux_grid_str[k])
Test(str)


Exit()


# 1 processor:

#wo/aux grid

# Native 1  -0.04343 - 1.066   190 zeros # Match
# Rect   1  -0.04343 - 1.066   190 zeros #

# Native 2  -0.04343 - 1.066   193 zeros # Match
# Rect   2  -0.04343 - 1.174   190 zeros # Except along the domain boundaries

# Errors in the domain boundary gradients 


#w/aux grid

# Native 1  0.004539 - 1.396   304 exited / 680 zeros # Match
# Rect   1  0.004539 - 1.396   304 exited / 680 zeros # 

# Native 2  0.004539 - 1.396   308 exited / 690 zeros # Match
# Rect   2  0.004539 - 1.396   304 exited / 680 zeros #


# 4 processors:

#wo/aux grid

# Native 1  -0.04343 - 1.066   190 zeros # Match
# Rect   1  -0.04343 - 1.066   190 zeros #

# Native 2  -0.04343 - 1.066   193 zeros # Match
# Rect   2  -0.04343 - 1.174   190 zeros # Except along the domain boundaries

#w/aux grid

# Native 1  0.004539 - 1.396   304 exited / 680 zeros # Match
# Rect   1  0.004539 - 1.396   304 exited / 680 zeros # 

# Native 2  0.004539 - 1.396   308 exited / 690 zeros # Match
# Rect   2  0.004539 - 1.396   304 exited / 680 zeros #
