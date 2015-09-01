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
#DrawPlots()

databases=["ftle_double_gyre_1_domain", "ftle_double_gyre_2_domains"]

src_type=[LCSAtts.NativeMesh, LCSAtts.RegularGrid]
src_type_str=["NativeMesh", "RegularGrid"]

aux_grid=[LCSAtts.None, LCSAtts.TwoDim]
aux_grid_str=["NoAuxGrid", "Two2DAuxGrid"]


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

#
LCSAtts.eigenComponent = LCSAtts.Smallest  # Smallest, Intermediate, Largest, PosShearVector, NegShearVector, PosLambdaShearVector, NegLambdaShearVector
SetOperatorOptions(LCSAtts, 0)
DrawPlots()
str="lcs_%s_%s_%s_Smallest" %(databases[i], src_type_str[j], aux_grid_str[k])
Test(str)

#-vargs="-debug 5"

#
LCSAtts.cauchyGreenTensor = LCSAtts.Left  # Left, Right
LCSAtts.eigenComponent = LCSAtts.Largest  # Smallest, Intermediate, Largest, PosShearVector, NegShearVector, PosLambdaShearVector, NegLambdaShearVector
SetOperatorOptions(LCSAtts, 0)
DrawPlots()
str="lcs_%s_%s_%s_Left" %(databases[i], src_type_str[j], aux_grid_str[k])
Test(str)

#
LCSAtts.operationType = LCSAtts.EigenValue  # IntegrationTime, ArcLength, AverageDistanceFromSeed, EigenValue, EigenVector, Lyapunov
LCSAtts.cauchyGreenTensor = LCSAtts.Right  # Left, Right
LCSAtts.eigenComponent = LCSAtts.Largest  # Smallest, Intermediate, Largest, PosShearVector, NegShearVector, PosLambdaShearVector, NegLambdaShearVector
SetOperatorOptions(LCSAtts, 0)
DrawPlots()
str="lcs_%s_%s_%s_EigenValue" %(databases[i], src_type_str[j], aux_grid_str[k])
Test(str)

Exit()







# Allen's CLI testing section that never gets executed
for i in range(len(databases)):
  db="/Projects/VisIt/trunk/build/data/pics_test_data/%s.pics" %(databases[i])
  OpenDatabase(db)
  for j in range(len(src_type)):
     str="Testing sample source = %s" %(src_type_str[j])
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


#
LCSAtts.eigenComponent = LCSAtts.Smallest  # Smallest, Intermediate, Largest, PosShearVector, NegShearVector, PosLambdaShearVector, NegLambdaShearVector
SetOperatorOptions(LCSAtts, 0)
DrawPlots()
str="lcs_%s_%s_%s_Smallest" %(databases[i], src_type_str[j], aux_grid_str[k])
swatts = SaveWindowAttributes()
swatts.family = 0
swatts.fileName = "/Projects/tmp/lcs/ser/%s" %(str)
SetSaveWindowAttributes(swatts)
SaveWindow()

#
LCSAtts.cauchyGreenTensor = LCSAtts.Left  # Left, Right
LCSAtts.eigenComponent = LCSAtts.Largest  # Smallest, Intermediate, Largest, PosShearVector, NegShearVector, PosLambdaShearVector, NegLambdaShearVector
SetOperatorOptions(LCSAtts, 0)
DrawPlots()
str="lcs_%s_%s_%s_Left" %(databases[i], src_type_str[j], aux_grid_str[k])
swatts = SaveWindowAttributes()
swatts.family = 0
swatts.fileName = "/Projects/tmp/lcs/ser/%s" %(str)
SetSaveWindowAttributes(swatts)
SaveWindow()

#
LCSAtts.operationType = LCSAtts.EigenValue  # IntegrationTime, ArcLength, AverageDistanceFromSeed, EigenValue, EigenVector, Lyapunov
LCSAtts.cauchyGreenTensor = LCSAtts.Right  # Left, Right
LCSAtts.eigenComponent = LCSAtts.Largest  # Smallest, Intermediate, Largest, PosShearVector, NegShearVector, PosLambdaShearVector, NegLambdaShearVector
SetOperatorOptions(LCSAtts, 0)
DrawPlots()
str="lcs_%s_%s_%s_EigenValue" %(databases[i], src_type_str[j], aux_grid_str[k])
swatts = SaveWindowAttributes()
swatts.family = 0
swatts.fileName = "/Projects/tmp/lcs/ser/%s" %(str)
SetSaveWindowAttributes(swatts)
SaveWindow()
# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  cracksclipper.py
#
#  Tests:      operators - CracksClipper
#
#  Programmer: Kathleen Biagas
#  Date:       August 14, 2012
#
#  Modifications:
# ----------------------------------------------------------------------------

OpenDatabase(data_path("vtk_cracked_test_data/cracked_*.vtk database"))
AddPlot("Pseudocolor", "operators/CracksClipper/mesh/den")
pcAtts = PseudocolorAttributes()
pcAtts.minFlag = 1
pcAtts.maxFlag = 1
pcAtts.min = 1.9
pcAtts.max = 3.853
SetPlotOptions(pcAtts)


v = GetView3D()
v.viewNormal = (-0.507948, 0.663707, 0.549074)
v.focus = (0.5, 0.5, 0.5)
v.viewUp = (0.388198, 0.745409, -0.541911)
v.parallelScale = 0.866025
v.nearPlane = -1.73205
v.farPlane = 1.73205
SetView3D(v)

DrawPlots()

# Changing time states shows the cracks developing, and demonstrates
# the density calculation
Test("CracksClipper_00")
SetTimeSliderState(1)
Test("CracksClipper_01")
SetTimeSliderState(3)
Test("CracksClipper_02")
SetTimeSliderState(5)
Test("CracksClipper_03")
SetTimeSliderState(12)
Test("CracksClipper_04")

SetTimeSliderState(14)
# Show that normal vars can be used.
ChangeActivePlotsVar("ems")
# 
Test("CracksClipper_05")

Exit()
