# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  pics.py
#
#  Tests:      streamline - 3D rectilinear
#              plots      - streamline
#
#  Defect ID:  -
#
#  Programmer: Hank Childs
#  Date:       March 11, 2012
# ----------------------------------------------------------------------------

s = StreamlineAttributes()
databases=["MB_2D_rect", "MB_2D_ugrid", "MB_3D_rect", "MB_3D_ugrid"]
is2D=[1,1,0,0]
isUgrid=[0,1,0,1]
algs=[s.LoadOnDemand, s.ParallelStaticDomains]
algs_strings=["POS", "POD", "MS", "BestAlg"]
time_strings=["path_forward", "path_backward", "stream_forward", "stream_backward", "stream_both"]
intg=[s.Euler, s.Leapfrog, s.DormandPrince, s.AdamsBashforth, s.RK4]
intg_strings=["Euler", "Leapfrog", "DormandPrince", "AdamsBashforth", "RK4"]

View3DAtts = View3DAttributes()
View3DAtts.viewNormal = (0.270729, 0.624198, 0.732859)
View3DAtts.focus = (0.496062, 0.99603, 0.496062)
View3DAtts.viewUp = (-0.0922782, 0.774611, -0.62567)
View3DAtts.viewAngle = 30
View3DAtts.parallelScale = 12.1829
View3DAtts.nearPlane = -24.3658
View3DAtts.farPlane = 24.3658
View3DAtts.imagePan = (0, 0)
View3DAtts.imageZoom = 1
View3DAtts.perspective = 1
View3DAtts.eyeAngle = 2
View3DAtts.axis3DScaleFlag = 0
View3DAtts.axis3DScales = (1, 1, 1)
SetView3D(View3DAtts)

for i in range(len(databases)):
  TestSection(databases[i])
  db=data_path("%s.pics") %(databases[i])
  OpenDatabase(db)
  for j in range(len(algs)):
     str="Varying time type with parallelization algorithm = %s" %(algs_strings[j])
     TestSection(str)

     for integrator_i in range(len(intg)) :
       str="Varying integrator = %s (%d)" %(intg_strings[integrator_i], integrator_i)
       TestSection(str)
       
       for k in range(len(time_strings)):
         DeleteAllPlots()
         AddPlot("Streamline", "velocity", 1, 0)
         s = StreamlineAttributes()
         s.streamlineAlgorithmType = algs[j]
         s.sourceType = s.SpecifiedPoint
         if (is2D[i]):
           if (intg[integrator_i] in [s.RK4, s.DormandPrince, s.AdamsBashforth]) :
             s.pointSource = (0.51, 0.1, 0)
           else:
             s.pointSource = (0.5, 0.1, 0)
         else:
           if (isUgrid[i]):
             s.pointSource = (0.2, 0.6, 0.6)
           else:
             if (intg[integrator_i] in [s.RK4, s.DormandPrince, s.AdamsBashforth]) :
               s.pointSource = (0.51, 0.1, 0.1)
             else:
               s.pointSource = (0.5, 0.1, 0.1)
         if (time_strings[k] == "path_forward"):
           TimeSliderSetState(0)
           s.pathlines = 1
           s.streamlineDirection = s.Forward
         if (time_strings[k] == "path_backward"):
           TimeSliderSetState(TimeSliderGetNStates()-1)
           s.pathlines = 1
           s.streamlineDirection = s.Backward
         if (time_strings[k] == "stream_forward"):
           TimeSliderSetState(0)
           s.pathlines = 0
           s.streamlineDirection = s.Forward
         if (time_strings[k] == "stream_backward"):
           TimeSliderSetState(0)
           s.pathlines = 0
           s.streamlineDirection = s.Backward
         if (time_strings[k] == "stream_both"):
           TimeSliderSetState(0)
           s.pathlines = 0
           s.streamlineDirection = s.Both
         s.maxStepLength = 0.01
         s.legendMinFlag = 1
         s.legendMaxFlag = 1
         if (s.pathlines):
           s.legendMin = 1.5
           s.legendMax = 5.7
         else:
           s.legendMin = -0.5
           s.legendMax = 0.5
           
         s.pathlinesCMFE = s.POS_CMFE
         s.integrationType = intg[integrator_i]
         SetPlotOptions(s)
         DrawPlots()
         str="pics_%s_%s_%s_%s" %(databases[i], algs_strings[j], intg_strings[integrator_i], time_strings[k])
         Test(str)

Exit()
