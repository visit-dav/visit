# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case: pseudocolor.py 
#
#  Tests:      meshes    - 2D rectilinear, 3D curvilinear. 
#              plots     - pseudocolor 
#
#  Defect ID:  '1016, '987
#
#  Programmer: Kevin Griffin
#  Date:       March 19, 2015
#
#  Modifications:
#
#    Kevin Griffin, Thu Mar 19 12:00:23 PDT 2015
#    Add test for drawing pseudocolor plot using a log scaling
#    and very small min value.
#
#    Alister Maguire, Wed Jul 17 08:24:37 PDT 2019
#    Wrapped the first test in a function and added 
#    TestDomainBoundaries. 
#
# ----------------------------------------------------------------------------


def TestScale():
    OpenDatabase(silo_data_path("rect2d.silo"))
    
    AddPlot("Pseudocolor", "d")
    
    pc = PseudocolorAttributes()
    pc.minFlag = 1
    pc.min = 1e-5
    pc.maxFlag = 1
    pc.max = 1
    pc.centering = pc.Nodal
    pc.scaling = pc.Log
    SetPlotOptions(pc)
    DrawPlots()
    
    Test("pseudocolor_01")

    DeleteAllPlots()

def TestDomainBoundaries():
    #
    # First, let's remove some domains and make sure that the 
    # faces are rendered. 
    #
    OpenDatabase(silo_data_path("multi_ucd3d.silo"))
    
    AddPlot("Pseudocolor", "d")
    DrawPlots()

    silr = SILRestriction()
    silr.SuspendCorrectnessChecking()
    silr.TurnOnAll()
    for silSet in (41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,60,61,
                   62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,
                   82,83,84,85,86,87,88,89,90,91,92,93,94,95,97,98,99,100,101,
                   102,103,104,105,106,107,108,109,110,111,112,113,114,116,117,
                   118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,
                   133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,
                   148,149,150,151,152,153,154,236,237,238,239,240,241,242,243,
                   244):
        silr.TurnOffSet(silSet)
    silr.EnableCorrectnessChecking()
    SetPlotSILRestriction(silr ,1)
    
    Test("pseudocolor_domain_bounds_01")
    
    #
    # Next, we need to make sure that the processer boundaries are not
    # rendered when we enable transparency. 
    #
    View3DAtts = View3DAttributes()
    View3DAtts.viewNormal = (0.883086, 0.11282, 0.455446)
    View3DAtts.focus = (0, 2.5, 10)
    View3DAtts.viewUp = (-0.0918142, 0.993447, -0.068068)
    View3DAtts.viewAngle = 30
    View3DAtts.parallelScale = 11.4564
    View3DAtts.nearPlane = -22.9129
    View3DAtts.farPlane = 22.9129
    View3DAtts.imagePan = (0, 0)
    View3DAtts.imageZoom = 1
    View3DAtts.perspective = 1
    View3DAtts.eyeAngle = 2
    View3DAtts.centerOfRotationSet = 0
    View3DAtts.centerOfRotation = (0, 2.5, 10)
    View3DAtts.axis3DScaleFlag = 0
    View3DAtts.axis3DScales = (1, 1, 1)
    View3DAtts.shear = (0, 0, 1)
    View3DAtts.windowValid = 1
    SetView3D(View3DAtts)
    
    silr = SILRestriction()
    silr.SuspendCorrectnessChecking()
    silr.TurnOnAll()
    for silSet in (41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,60,61,
                   62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,
                   82,83,84,85,86,87,88,89,90,91,92,93,94,95,97,98,99,100,101,
                   102,103,104,105,106,107,108,109,110,111,112,113,114,116,117,
                   118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,
                   133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,
                   148,149,150,151,152,153,154):
        silr.TurnOffSet(silSet)
    silr.EnableCorrectnessChecking()
    SetPlotSILRestriction(silr ,1)
    
    PseudocolorAtts = PseudocolorAttributes()
    PseudocolorAtts.opacityType = PseudocolorAtts.Constant  
    PseudocolorAtts.opacity = 0.462745
    SetPlotOptions(PseudocolorAtts)
    
    Test("pseudocolor_domain_bounds_02")
    
    #
    # Now let's make sure that the ghost/face removal has been 
    # reversed back to its original order when we turn transparency
    # back off. 
    #
    PseudocolorAtts = PseudocolorAttributes()
    PseudocolorAtts.opacityType = PseudocolorAtts.FullyOpaque
    SetPlotOptions(PseudocolorAtts)
    
    silr = SILRestriction()
    silr.SuspendCorrectnessChecking()
    silr.TurnOnAll()
    for silSet in (41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,60,61,
                   62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,
                   82,83,84,85,86,87,88,89,90,91,92,93,94,95,97,98,99,100,101,
                   102,103,104,105,106,107,108,109,110,111,112,113,114,116,117,
                   118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,
                   133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,
                   148,149,150,151,152,153,154,236,237,238,239,240,241,242,243,
                   244):
        silr.TurnOffSet(silSet)
    silr.EnableCorrectnessChecking()
    SetPlotSILRestriction(silr ,1)
    
    Test("pseudocolor_domain_bounds_03")


def Main():
    TestScale()
    TestDomainBoundaries()


Main()
Exit()
