# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  ic_pics.py
#
#  Tests:      operator      - integralcurve
#
#  Defect ID:  -
#
#  Programmer: Allen Sanderson
#  Date:       March 10, 2016
#
#  Modifications:
#    Kathleen Biagas, Thu Sep 17 11:35:47 PDT 2020
#    Turn off IC warnings. Set larger linewidth so for better images.
# ----------------------------------------------------------------------------
RequiredDatabasePlugin("PICS_Tester")

IntegralCurveAtts = IntegralCurveAttributes()
databases=["MB_2D_rect", "MB_2D_ugrid", "MB_3D_rect", "MB_3D_ugrid"]
is2D=[1,1,0,0]
isUgrid=[0,1,0,1]
algs=[IntegralCurveAtts.LoadOnDemand, IntegralCurveAtts.ParallelStaticDomains]
algs_strings=["POS", "POD", "MS", "BestAlg"]
time_strings=["path_forward", "path_backward", "stream_forward", "stream_backward", "stream_both"]
intg=[IntegralCurveAtts.Euler, IntegralCurveAtts.Leapfrog, IntegralCurveAtts.DormandPrince, IntegralCurveAtts.AdamsBashforth, IntegralCurveAtts.RK4]
intg_strings=["Euler", "Leapfrog", "DormandPrince", "AdamsBashforth", "RK4"]

View3DAtts = View3DAttributes()
View3DAttIntegralCurveAttsviewNormal = (0.270729, 0.624198, 0.732859)
View3DAttIntegralCurveAttsfocus = (0.496062, 0.99603, 0.496062)
View3DAttIntegralCurveAttsviewUp = (-0.0922782, 0.774611, -0.62567)
View3DAttIntegralCurveAttsviewAngle = 30
View3DAttIntegralCurveAttsparallelScale = 12.1829
View3DAttIntegralCurveAttsnearPlane = -24.3658
View3DAttIntegralCurveAttsfarPlane = 24.3658
View3DAttIntegralCurveAttsimagePan = (0, 0)
View3DAttIntegralCurveAttsimageZoom = 1
View3DAttIntegralCurveAttsperspective = 1
View3DAttIntegralCurveAttseyeAngle = 2
View3DAttIntegralCurveAttsaxis3DScaleFlag = 0
View3DAttIntegralCurveAttsaxis3DScales = (1, 1, 1)
SetView3D(View3DAtts)

for i in range(len(databases)):
  TestSection(databases[i])
  db=data_path("pics_test_data/%s.pics") %(databases[i])
  for j in range(len(algs)):
     tmpstr="Varying time type with parallelization algorithm = %s" %(algs_strings[j])
     TestSection(tmpstr)

     for integrator_i in range(len(intg)) :
       tmpstr="Varying integrator = %s (%d)" %(intg_strings[integrator_i], integrator_i)
       TestSection(tmpstr)
       
       for k in range(len(time_strings)):
         DeleteAllPlots()
         OpenDatabase(db)
         AddPlot("Pseudocolor", "operators/IntegralCurve/velocity", 1, 0)
         pcAtts = PseudocolorAttributes();
         pcAtts.lineWidth = 3
         SetPlotOptions(pcAtts)
         IntegralCurveAtts = IntegralCurveAttributes()
         IntegralCurveAtts.parallelizationAlgorithmType = algs[j]
         IntegralCurveAtts.sourceType = IntegralCurveAtts.SpecifiedPoint
         IntegralCurveAtts.issueAdvectionWarnings = 0
         IntegralCurveAtts.issueBoundaryWarnings = 0
         IntegralCurveAtts.issueTerminationWarnings = 0
         IntegralCurveAtts.issueStepsizeWarnings = 0
         IntegralCurveAtts.issueStiffnessWarnings = 0
         IntegralCurveAtts.issueCriticalPointsWarnings = 0
         if (is2D[i]):
           if (intg[integrator_i] in [IntegralCurveAtts.RK4, IntegralCurveAtts.DormandPrince, IntegralCurveAtts.AdamsBashforth]) :
             IntegralCurveAtts.pointSource = (0.51, 0.1, 0)
           else:
             IntegralCurveAtts.pointSource = (0.5, 0.1, 0)
         else:
           if (isUgrid[i]):
             IntegralCurveAtts.pointSource = (0.2, 0.6, 0.6)
           else:
             if (intg[integrator_i] in [IntegralCurveAtts.RK4, IntegralCurveAtts.DormandPrince, IntegralCurveAtts.AdamsBashforth]) :
               IntegralCurveAtts.pointSource = (0.51, 0.1, 0.1)
             else:
               IntegralCurveAtts.pointSource = (0.5, 0.1, 0.1)
         if (time_strings[k] == "path_forward"):
           TimeSliderSetState(0)
           IntegralCurveAtts.pathlines = 1
           IntegralCurveAtts.integrationDirection = IntegralCurveAtts.Forward
         if (time_strings[k] == "path_backward"):
           TimeSliderSetState(TimeSliderGetNStates()-1)
           IntegralCurveAtts.pathlines = 1
           IntegralCurveAtts.integrationDirection = IntegralCurveAtts.Backward
         if (time_strings[k] == "stream_forward"):
           TimeSliderSetState(0)
           IntegralCurveAtts.pathlines = 0
           IntegralCurveAtts.integrationDirection = IntegralCurveAtts.Forward
         if (time_strings[k] == "stream_backward"):
           TimeSliderSetState(0)
           IntegralCurveAtts.pathlines = 0
           IntegralCurveAtts.integrationDirection = IntegralCurveAtts.Backward
         if (time_strings[k] == "stream_both"):
           TimeSliderSetState(0)
           IntegralCurveAtts.pathlines = 0
           IntegralCurveAtts.integrationDirection = IntegralCurveAtts.Both
         IntegralCurveAtts.maxStepLength = 0.01           
         IntegralCurveAtts.pathlinesCMFE = IntegralCurveAtts.POS_CMFE
         IntegralCurveAtts.integrationType = intg[integrator_i]
         SetOperatorOptions(IntegralCurveAtts)
         DrawPlots()
         tmpstr="ic_pics_%s_%s_%s_%s" %(databases[i], algs_strings[j], intg_strings[integrator_i], time_strings[k])
         Test(tmpstr)

Exit()
