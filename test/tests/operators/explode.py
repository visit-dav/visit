# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  explode.py
#
#  Tests:      mesh      - 3D rectilinear, single domain, 
#                           multi domain
#                          3D unstructured, single domain
#                          3D curvilinear, single domain
#                          2D unstructured, single domain
#              plots     - filled boundary, pseudocolor
#              operators - explode
#
#  Programmer: Alister Maguire
#  Date: Tue Jan  2 15:07:06 PST 2018
#
#  Modifications:
#
#      Alister Maguire, Wed Jan 17 15:28:46 PST 2018
#      Added tests of multi-domain data.    
#
#      Alister Maguire, Mon Jan 22 11:00:51 PST 2018
#      Changed explosion factors to account of scale update. 
#
#      Alister Maguire, Wed Feb 14 16:07:51 PST 2018
#      Updated tests with multiple explosions to be perfomed
#      by a single operator.   
#
#      Alister Maguire, Tue Feb 20 16:22:05 PST 2018
#      Added a 2d test with no materials. 
#
#      Alister Maguire, Wed May  2 12:54:43 PDT 2018
#      Added tests for recentering. 
#
#      Alister Maguire, Tue Sep 18 14:57:03 PDT 2018
#      Added tests for exploding domains. 
#
# ----------------------------------------------------------------------------


def unstructured_explosions():
    OpenDatabase(silo_data_path("globe.silo"))
    AddPlot("FilledBoundary", "mat1", 1, 1)
    DrawPlots()
    
    ResetView()
    View3DAtts = View3DAttributes()
    View3DAtts.viewNormal = (0.0758172, -0.984828, -0.156097)
    View3DAtts.focus = (0, 0, 0)
    View3DAtts.viewUp = (-0.01047, 0.155753, -0.987741)
    View3DAtts.viewAngle = 30
    View3DAtts.parallelScale = 17.3205
    View3DAtts.nearPlane = -34.641
    View3DAtts.farPlane = 34.641
    View3DAtts.imagePan = (0, 0)
    View3DAtts.imageZoom = 0.826446
    View3DAtts.perspective = 1
    View3DAtts.eyeAngle = 2
    View3DAtts.centerOfRotationSet = 0
    View3DAtts.centerOfRotation = (0, 0, 0)
    View3DAtts.axis3DScaleFlag = 0
    View3DAtts.axis3DScales = (1, 1, 1)
    View3DAtts.shear = (0, 0, 1)
    View3DAtts.windowValid = 1
    SetView3D(View3DAtts)

    AddOperator("Explode")
    mainAtts = ExplodeAttributes()
    
    # Explode one material by a plane
    atts1 = ExplodeAttributes()
    atts1.explosionType = atts1.Plane 
    atts1.planePoint = (0, 0, 0)
    atts1.planeNorm = (1, 0, 0)
    atts1.materialExplosionFactor = 1
    atts1.material = "4"
    atts1.explodeMaterialCells = 1
    atts1.cellExplosionFactor = .8
    atts1.explosionPattern = atts1.Impact 
    mainAtts.AddExplosions(atts1)
    SetOperatorOptions(mainAtts, 1)
    DrawPlots()
    
    Test("explode_globe_00")
    
    # Explode second material by plane
    atts2 = ExplodeAttributes()
    atts2.explosionType = atts2.Plane  
    atts2.planePoint = (0, 0, 0)
    atts2.planeNorm = (0, 0, 1)
    atts2.materialExplosionFactor = 1
    atts2.material = "3"
    atts2.explodeMaterialCells = 1
    atts2.cellExplosionFactor = .8
    atts2.explosionPattern = atts2.Impact  
    mainAtts.AddExplosions(atts2)
    SetOperatorOptions(mainAtts, 1)
    DrawPlots()

    Test("explode_globe_01")
    
    # Explode third material by plane
    atts3 = ExplodeAttributes()
    atts3.explosionType = atts3.Plane  
    atts3.planePoint = (0, 0, 0)
    atts3.planeNorm = (0, 0, 1)
    atts3.materialExplosionFactor = 1
    atts3.material = "1"
    atts3.explodeMaterialCells = 1
    atts3.cellExplosionFactor = .3
    atts3.explosionPattern = atts3.Impact  
    mainAtts.AddExplosions(atts3)
    SetOperatorOptions(mainAtts, 1)
    DrawPlots()
    
    Test("explode_globe_02")
    
    # Explode material by point
    atts4 = ExplodeAttributes()
    atts4.explosionType = atts4.Point  
    atts4.explosionPoint = (0, 0, 0)
    atts4.materialExplosionFactor = 1
    atts4.material = "2"
    atts4.explodeMaterialCells = 1
    atts4.cellExplosionFactor = .1
    atts4.explosionPattern = atts4.Impact  # Impact, Scatter
    mainAtts.AddExplosions(atts4)
    SetOperatorOptions(mainAtts, 1)
    DrawPlots()
    Test("explode_globe_03")
    
    ResetView()
    DeleteAllPlots()
    
def curvilinear_explosions(): 
    OpenDatabase(silo_data_path("ucd3d.silo"))
    AddPlot("Pseudocolor", "v", 1, 1)
    AddOperator("Explode", 1)
    DrawPlots()
    
    ResetView()
    View3DAtts = View3DAttributes()
    View3DAtts.viewNormal = (-0.706303, 0.44773, 0.548338)
    View3DAtts.focus = (0, 3.41092, 10)
    View3DAtts.viewUp = (0.262178, 0.884949, -0.384874)
    View3DAtts.viewAngle = 30
    View3DAtts.parallelScale = 17.0459
    View3DAtts.nearPlane = -34.641
    View3DAtts.farPlane = 34.641
    View3DAtts.imagePan = (0, 0)
    View3DAtts.imageZoom = 0.926446
    View3DAtts.perspective = 1
    View3DAtts.eyeAngle = 2
    View3DAtts.centerOfRotationSet = 0
    View3DAtts.centerOfRotation = (0, 3.41092, 10)
    View3DAtts.axis3DScaleFlag = 0
    View3DAtts.axis3DScales = (1, 1, 1)
    View3DAtts.shear = (0, 0, 1)
    View3DAtts.windowValid = 1
    SetView3D(View3DAtts)
    
    # Explode by cylinder without radius 
    ExplodeAtts = ExplodeAttributes()
    ExplodeAtts.explosionType = ExplodeAtts.Cylinder 
    ExplodeAtts.cylinderPoint1 = (-5, 0, 20)
    ExplodeAtts.cylinderPoint2 = (5, 5, 0)
    ExplodeAtts.materialExplosionFactor = 1
    ExplodeAtts.material = "1"
    ExplodeAtts.cylinderRadius = 0
    ExplodeAtts.explodeMaterialCells = 1
    ExplodeAtts.cellExplosionFactor = 1
    ExplodeAtts.explosionPattern = ExplodeAtts.Impact 
    ExplodeAtts.explodeAllCells = 0
    SetOperatorOptions(ExplodeAtts, 1)
    
    Test("explode_ucd3d_00")
    
    # Explode by plane
    ExplodeAtts = ExplodeAttributes()
    ExplodeAtts.explosionType = ExplodeAtts.Plane  
    ExplodeAtts.explodeMaterialCells = 0
    ExplodeAtts.planePoint = (0, 2, 20)
    ExplodeAtts.planeNorm = (0, 1, 0)
    ExplodeAtts.materialExplosionFactor = 1
    ExplodeAtts.material = "4"
    ExplodeAtts.explosionPattern = ExplodeAtts.Impact 
    ExplodeAtts.explodeAllCells = 0
    SetOperatorOptions(ExplodeAtts, 1)
    
    Test("explode_ucd3d_02")

    # Explode all cells by point (impact)
    ExplodeAtts = ExplodeAttributes()
    ExplodeAtts.explosionType = ExplodeAtts.Point  
    ExplodeAtts.explosionPoint = (0, 0, 0)
    ExplodeAtts.materialExplosionFactor = 1
    ExplodeAtts.material = "1"
    ExplodeAtts.cellExplosionFactor = 1
    ExplodeAtts.explosionPattern = ExplodeAtts.Impact 
    ExplodeAtts.explodeAllCells = 1
    SetOperatorOptions(ExplodeAtts, 1)
    
    Test("explode_ucd3d_03")
    
    # Explode all cells by point (impact)
    ExplodeAtts = ExplodeAttributes()
    ExplodeAtts.explosionType = ExplodeAtts.Point 
    ExplodeAtts.explosionPoint = (0, 3, 10)
    ExplodeAtts.materialExplosionFactor = 1
    ExplodeAtts.material = "1"
    ExplodeAtts.cellExplosionFactor = 1
    ExplodeAtts.explosionPattern = ExplodeAtts.Impact  
    ExplodeAtts.explodeAllCells = 1
    SetOperatorOptions(ExplodeAtts, 1)
    
    Test("explode_ucd3d_04")
    
    # Explode all cells by point (scatter)
    ExplodeAtts = ExplodeAttributes()
    ExplodeAtts.explosionType = ExplodeAtts.Point 
    ExplodeAtts.explosionPoint = (0, 3, 10)
    ExplodeAtts.materialExplosionFactor = 1
    ExplodeAtts.material = "1"
    ExplodeAtts.cellExplosionFactor = 3
    ExplodeAtts.explosionPattern = ExplodeAtts.Scatter 
    ExplodeAtts.explodeAllCells = 1
    SetOperatorOptions(ExplodeAtts, 1)
    
    Test("explode_ucd3d_05")
    
    # Explode material by point (scatter)
    ExplodeAtts = ExplodeAttributes()
    ExplodeAtts.explosionType = ExplodeAtts.Point  
    ExplodeAtts.explosionPoint = (0, 0, 0)
    ExplodeAtts.materialExplosionFactor = 1
    ExplodeAtts.material = "1"
    ExplodeAtts.explodeMaterialCells = 1
    ExplodeAtts.cellExplosionFactor = 1.5
    ExplodeAtts.explosionPattern = ExplodeAtts.Scatter 
    ExplodeAtts.explodeAllCells = 0
    SetOperatorOptions(ExplodeAtts, 1)
    
    Test("explode_ucd3d_06")
    
    # Explode material by point
    ExplodeAtts = ExplodeAttributes()
    ExplodeAtts.explosionType = ExplodeAtts.Point  
    ExplodeAtts.explosionPoint = (0, 3, 10)
    ExplodeAtts.materialExplosionFactor = 1
    ExplodeAtts.material = "1"
    ExplodeAtts.cylinderRadius = 0
    ExplodeAtts.explodeMaterialCells = 1
    ExplodeAtts.cellExplosionFactor = 1.5
    ExplodeAtts.explosionPattern = ExplodeAtts.Scatter  
    ExplodeAtts.explodeAllCells = 0
    SetOperatorOptions(ExplodeAtts, 1)
    
    Test("explode_ucd3d_07")
    
    # Explode all cells by cylinder with radius
    ExplodeAtts = ExplodeAttributes()
    ExplodeAtts.explosionType = ExplodeAtts.Cylinder
    ExplodeAtts.cylinderPoint1 = (0, 1, 10)
    ExplodeAtts.cylinderPoint2 = (0, 0, 10)
    ExplodeAtts.cylinderRadius = 4
    ExplodeAtts.cellExplosionFactor = .4
    ExplodeAtts.explosionPattern = ExplodeAtts.Impact
    ExplodeAtts.explodeAllCells = 1
    SetOperatorOptions(ExplodeAtts, 1)
    
    Test("explode_ucd3d_08")
    
    ResetView()
    DeleteAllPlots()

def rectilinear_explosions():
    OpenDatabase(silo_data_path("rect3d.silo"))
    AddPlot("FilledBoundary", "mat1")

    ResetView()
    RecenterView()

    AddOperator("Explode")
    mainAtts = ExplodeAttributes()
    DrawPlots()

    atts1 = ExplodeAttributes()
    atts1.explosionPoint = (1, 0, 0)
    atts1.materialExplosionFactor = 2
    atts1.material = "7"
    mainAtts.AddExplosions(atts1)
    SetOperatorOptions(mainAtts, 1)

    atts2 = ExplodeAttributes()
    atts2.explosionPoint = (0, 0, 1)
    atts2.materialExplosionFactor = 2
    atts2.material = "5"
    mainAtts.AddExplosions(atts2)
    SetOperatorOptions(mainAtts, 1)

    Test("explode_rect3d_00")

    ResetView()
    DeleteAllPlots()

def multi_rectilinear_explosions():
    OpenDatabase(silo_data_path("multi_rect3d.silo"))
    AddPlot("FilledBoundary", "mat1")

    ResetView()
    RecenterView()

    AddOperator("Explode")
    DrawPlots()
    mainAtts = ExplodeAttributes()

    atts1 = ExplodeAttributes()
    atts1.explosionPoint = (1, 0, 0)
    atts1.materialExplosionFactor = 2
    atts1.material = "2"
    mainAtts.AddExplosions(atts1)
    SetOperatorOptions(mainAtts, 1)

    atts2 = ExplodeAttributes()
    atts2.explosionPoint = (0, 0, 1)
    atts2.materialExplosionFactor = 2
    atts2.material = "3"
    mainAtts.AddExplosions(atts2)
    SetOperatorOptions(mainAtts, 1)

    Test("explode_multi_rect3d_00")

    ResetView()
    DeleteAllPlots()

def multi_tire():
    OpenDatabase(silo_data_path("tire.silo"))
    AddPlot("FilledBoundary", "Materials")
    
    ResetView()
    RecenterView()

    View3DAtts = View3DAttributes()
    View3DAtts.viewNormal = (1.0, 0.0, 0.0)
    View3DAtts.focus = (0, 0, 0)
    View3DAtts.viewUp = (0, 1, 0)
    View3DAtts.viewAngle = 30
    View3DAtts.parallelScale = 8.0
    View3DAtts.nearPlane = -0.5
    View3DAtts.farPlane = 0.5
    View3DAtts.imagePan = (0, 0)
    View3DAtts.imageZoom = 0.826446
    View3DAtts.perspective = 1
    View3DAtts.eyeAngle = 2
    View3DAtts.centerOfRotationSet = 0
    View3DAtts.centerOfRotation = (0, 0, 0)
    View3DAtts.axis3DScaleFlag = 0
    View3DAtts.axis3DScales = (1, 1, 1)
    View3DAtts.shear = (0, 0, 1)
    View3DAtts.windowValid = 1
    SetView3D(View3DAtts)

    
    AddOperator("Explode")
    DrawPlots()
    mainAtts = ExplodeAttributes()

    # If domains are being treated correctly, this
    # should not produce an explosion. 
    atts1 = ExplodeAttributes()
    atts1.materialExplosionFactor = 500
    atts1.material = "1 Rubber"
    mainAtts.AddExplosions(atts1)
    SetOperatorOptions(mainAtts, 1)

    Test("explode_tire_00")

    atts2 = ExplodeAttributes()
    atts2.materialExplosionFactor = 0
    atts2.explodeMaterialCells = 1
    atts2.cellExplosionFactor = 1
    atts2.explosionType = atts2.Plane 
    atts2.planePoint = (0, 0, 0)
    atts2.planeNorm = (0, 1, 0)
    atts2.material = "1 Rubber"
    mainAtts.AddExplosions(atts2)
    SetOperatorOptions(mainAtts, 1)

    Test("explode_tire_01")

    atts3 = ExplodeAttributes()
    atts3.materialExplosionFactor = 1
    atts3.explodeMaterialCells = 0
    atts3.cellExplosionFactor = 0
    atts3.explosionType = atts3.Point 
    atts3.explosionPoint = (0, 0, 60)
    atts3.material = "2 Steel"
    mainAtts.AddExplosions(atts3)
    SetOperatorOptions(mainAtts, 1)

    Test("explode_tire_02")
    ResetView()
    DeleteAllPlots()

def TwoDimNoMat():
    OpenDatabase(silo_data_path("quad_disk.silo"))
    AddPlot("Pseudocolor", "sphElevD_on_mesh")
    
    ResetView()
    RecenterView()

    AddOperator("Explode")
    DrawPlots()

    atts = ExplodeAttributes()
    atts.explodeAllCells  = 1
    atts.explosionPattern = atts.Scatter
    SetOperatorOptions(atts)
    
    Test("explode_quad_disk_00")
    ResetView()
    DeleteAllPlots()

def TestRecenter():
    #
    # Recentering is enabled when exploding all cells,
    # and it allows a cell to be displaced even when 
    # its center lies on an explode origin. 
    #
    OpenDatabase(silo_data_path("arbpoly-zoohybrid.silo"))
    AddPlot("Pseudocolor", "3D/z1")

    ResetView()
    RecenterView()

    AddOperator("Explode")
    DrawPlots()

    #
    # Test when a cell center lies on an explode
    # cylinder with radius 0. 
    #
    atts = ExplodeAttributes()
    atts.explodeAllCells = 1
    atts.explosionType   = atts.Cylinder 
    atts.cylinderPoint1  = (0, .5, .5)
    atts.cylinderPoint2  = (1, .5, .5)
    SetOperatorOptions(atts)

    Test("recenter_00")

    #
    # Test when a cell center lies on an explode
    # point.
    #
    atts = ExplodeAttributes()
    atts.explodeAllCells = 1
    atts.explosionType   = atts.Point
    atts.explosionPoint  = (.5, .5, .5)
    SetOperatorOptions(atts)

    Test("recenter_01")

    #
    # Test when a cell center lies on an explode
    # plane.
    #
    atts = ExplodeAttributes()
    atts.explodeAllCells = 1
    atts.explosionType   = atts.Plane
    atts.planePoint      = (1.5, 0, 0)
    atts.planeNorm       = (1, 0, 0)
    SetOperatorOptions(atts)

    Test("recenter_02")

    ResetView()
    DeleteAllPlots()


def TestDomainExplode():
    OpenDatabase(silo_data_path("multi_ucd3d.silo"))
    AddPlot("Subset", "domains(mesh1)")
    
    ResetView()
    RecenterView()

    #
    # Test exploding full mesh domains. 
    #
    AddOperator("Explode")
    expAtts = ExplodeAttributes()
    expAtts.explosionType = expAtts.Point  
    expAtts.explosionPoint = (0, 0, 0)
    expAtts.materialExplosionFactor = 1.2
    expAtts.material = "15"
    expAtts.explodeMaterialCells = 0
    expAtts.explosionPattern = expAtts.Impact 
    SetOperatorOptions(expAtts, 1)
    DrawPlots()
    Test("explode_domains_00")
    
    ResetView()
    DeleteAllPlots()

    AddPlot("Subset", "domains(mesh1_front)")

    #
    # Test exploding a subset of mesh domains. 
    #
    AddOperator("Explode")
    expAtts = ExplodeAttributes()
    expAtts.explosionType = expAtts.Point  
    expAtts.explosionPoint = (0, 0, 0)
    expAtts.materialExplosionFactor = 1.2
    expAtts.material = "8"
    expAtts.explodeMaterialCells = 0
    expAtts.explosionPattern = expAtts.Impact 
    SetOperatorOptions(expAtts, 1)
    DrawPlots()
    Test("explode_domains_01")

    ResetView()
    DeleteAllPlots()


def Main():
    unstructured_explosions()
    curvilinear_explosions()
    rectilinear_explosions()
    multi_rectilinear_explosions()
    multi_tire()
    TwoDimNoMat()
    TestRecenter()
    TestDomainExplode()
    

Main()
Exit()
