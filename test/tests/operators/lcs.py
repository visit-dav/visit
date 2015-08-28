#c ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  LCS.py
#
#  Tests:      operator   - LCS
#              plots      - pseudocolor
#
#  Defect ID:  -
#
#  Programmer: Allen Sanderson
#  Date:       August 25, 2015
# ----------------------------------------------------------------------------

databases=["ftle_double_gyre_1_domain", "ftle_double_gyre_2_domains"]

src_type=[LCSAtts.NativeMesh, LCSAtts.RegularGrid]
src_type_str=["NativeMesh", "RegularGrid"]

aux_grid=[LCSAtts.None, LCSAtts.TwoDim]
aux_grid_str=["NoAuxGrid", "Two2DAuxGrid"]


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

for i in range(len(databases)):
  TestSection(databases[i])
  db=data_path("pics_test_data/%s.pics") %(databases[i])
  OpenDatabase(db)
  for j in range(len(src_type)):
     str="Testing source = %s" %(src_type_str[j])
     TestSection(str)
     LCSAtts.sourceType = src_type[j]  # NativeMesh, RegularGrid
     for k in range(len(aux_grid)):
        str="Testing auxiliary grid = %s" %(aux_grid_str[k])
        TestSection(str)
        DeleteAllPlots()
        AddPlot("Pseudocolor", "operators/LCS/velocity", 1, 0)
        LCSAtts.auxiliaryGrid = aux_grid[k]  # None, TwoDim
        SetOperatorOptions(LCSAtts, 0)
        DrawPlots()
        str="lcs_%s_%s_%s" %(databases[i], src_type_str[j], aux_grid_str[k])
        Test(str)

Exit()



for i in range(len(databases)):
  db="/Projects/VisIt/trunk/data/pics_test_data/%s.pics" %(databases[i])
  OpenDatabase(db)
  for j in range(len(src_type)):
     str="Testing source = %s" %(src_type_str[j])
     LCSAtts.sourceType = src_type[j]  # NativeMesh, RegularGrid
     for k in range(len(aux_grid)):
        str="Testing auxiliary grid = %s" %(aux_grid_str[k])
        DeleteAllPlots()
        AddPlot("Pseudocolor", "operators/LCS/velocity", 1, 0)
        LCSAtts.auxiliaryGrid = aux_grid[k]  # None, TwoDim
        SetOperatorOptions(LCSAtts, 0)
        DrawPlots()
        str="lcs_%s_%s_%s" %(databases[i], src_type_str[j], aux_grid_str[k])
        swatts = SaveWindowAttributes()
        swatts.family = 0
        swatts.fileName = "/Projects/tmp/lcs/ser/%s" %(str)
        SetSaveWindowAttributes(swatts)
        SaveWindow()


