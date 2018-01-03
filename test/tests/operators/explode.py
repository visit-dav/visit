# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  explode.py
#
#  Tests:      mesh      - 3D rectilinear, single domain
#                          3D unstructured, single domain
#                          3D curvilinear, single domain
#              plots     - filled boundary, pseudocolor
#              operators - explode
#
#  Programmer: Alister Maguire
#  Date: Tue Jan  2 15:07:06 PST 2018
#
#  Modifications:
# ----------------------------------------------------------------------------


def unstructured_explosions():
    OpenDatabase(silo_data_path("globe.silo"))
    AddPlot("FilledBoundary", "mat1", 1, 1)
    DrawPlots()
    
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
    
    # Explode one material by a plane
    AddOperator("Explode")
    ExplodeAtts = ExplodeAttributes()
    ExplodeAtts.explosionType = ExplodeAtts.Plane 
    ExplodeAtts.planePoint = (0, 0, 0)
    ExplodeAtts.planeNorm = (1, 0, 0)
    ExplodeAtts.materialExplosionFactor = 1
    ExplodeAtts.material = "4"
    ExplodeAtts.explodeMaterialCells = 1
    ExplodeAtts.cellExplosionFactor = 10
    ExplodeAtts.explosionPattern = ExplodeAtts.Impact 
    SetOperatorOptions(ExplodeAtts, 1)
    DrawPlots()
    
    Test("explode_globe_00")
    
    # Explode second material by plane
    AddOperator("Explode")
    ExplodeAtts = ExplodeAttributes()
    ExplodeAtts.explosionType = ExplodeAtts.Plane  
    ExplodeAtts.planePoint = (0, 0, 0)
    ExplodeAtts.planeNorm = (0, 0, 1)
    ExplodeAtts.materialExplosionFactor = 1
    ExplodeAtts.material = "3"
    ExplodeAtts.explodeMaterialCells = 1
    ExplodeAtts.cellExplosionFactor = 10
    ExplodeAtts.explosionPattern = ExplodeAtts.Impact  
    SetOperatorOptions(ExplodeAtts, 1)
    DrawPlots()

    Test("explode_globe_01")
    
    # Explode third material by plane
    AddOperator("Explode")
    ExplodeAtts = ExplodeAttributes()
    ExplodeAtts.explosionType = ExplodeAtts.Plane  
    ExplodeAtts.planePoint = (0, 0, 0)
    ExplodeAtts.planeNorm = (0, 0, 1)
    ExplodeAtts.materialExplosionFactor = 1
    ExplodeAtts.material = "1"
    ExplodeAtts.explodeMaterialCells = 1
    ExplodeAtts.cellExplosionFactor = 5
    ExplodeAtts.explosionPattern = ExplodeAtts.Impact  
    SetOperatorOptions(ExplodeAtts, 2)
    DrawPlots()
    
    Test("explode_globe_02")
    
    # Explode material by point
    AddOperator("Explode")
    ExplodeAtts = ExplodeAttributes()
    ExplodeAtts.explosionType = ExplodeAtts.Point  
    ExplodeAtts.explosionPoint = (0, 0, 0)
    ExplodeAtts.materialExplosionFactor = 1
    ExplodeAtts.material = "2"
    ExplodeAtts.explodeMaterialCells = 1
    ExplodeAtts.cellExplosionFactor = 3
    ExplodeAtts.explosionPattern = ExplodeAtts.Impact  # Impact, Scatter
    SetOperatorOptions(ExplodeAtts, 3)
    DrawPlots()
    Test("explode_globe_03")
    
    DeleteAllPlots()
    
def curvilinear_explosions(): 
    OpenDatabase(silo_data_path("ucd3d.silo"))
    AddPlot("Pseudocolor", "v", 1, 1)
    AddOperator("Explode", 1)
    DrawPlots()
    
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
    ExplodeAtts.cellExplosionFactor = 10
    ExplodeAtts.explosionPattern = ExplodeAtts.Impact 
    SetOperatorOptions(ExplodeAtts, 1)
    
    Test("explode_ucd3d_00")
    ResetOperatorOptions("Explode", 1)
    
    # Explode by plane
    ExplodeAtts = ExplodeAttributes()
    ExplodeAtts.explosionType = ExplodeAtts.Plane  
    ExplodeAtts.planePoint = (0, 2, 20)
    ExplodeAtts.planeNorm = (0, 1, 0)
    ExplodeAtts.materialExplosionFactor = 100
    ExplodeAtts.material = "4"
    ExplodeAtts.cellExplosionFactor = 1
    ExplodeAtts.explosionPattern = ExplodeAtts.Impact 
    SetOperatorOptions(ExplodeAtts, 1)
    
    Test("explode_ucd3d_02")
    ResetOperatorOptions("Explode", 1)

    # Explode all cells by point (impact)
    ExplodeAtts = ExplodeAttributes()
    ExplodeAtts.explosionType = ExplodeAtts.Point  
    ExplodeAtts.explosionPoint = (0, 0, 0)
    ExplodeAtts.materialExplosionFactor = 1
    ExplodeAtts.material = "1"
    ExplodeAtts.cellExplosionFactor = 10
    ExplodeAtts.explosionPattern = ExplodeAtts.Impact 
    ExplodeAtts.explodeAllCells = 1
    SetOperatorOptions(ExplodeAtts, 1)
    
    Test("explode_ucd3d_03")
    
    # Explode all cells by point (impact)
    ResetOperatorOptions("Explode", 1)
    ExplodeAtts = ExplodeAttributes()
    ExplodeAtts.explosionType = ExplodeAtts.Point 
    ExplodeAtts.explosionPoint = (0, 3, 10)
    ExplodeAtts.materialExplosionFactor = 1
    ExplodeAtts.material = "1"
    ExplodeAtts.cellExplosionFactor = 10
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
    ExplodeAtts.cellExplosionFactor = 1
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
    ExplodeAtts.cellExplosionFactor = 0.5
    ExplodeAtts.explosionPattern = ExplodeAtts.Scatter 
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
    ExplodeAtts.cellExplosionFactor = 0.5
    ExplodeAtts.explosionPattern = ExplodeAtts.Scatter  
    SetOperatorOptions(ExplodeAtts, 1)
    
    Test("explode_ucd3d_07")
    
    # Explode all cells by cylinder with radius
    ExplodeAtts = ExplodeAttributes()
    ExplodeAtts.explosionType = ExplodeAtts.Cylinder
    ExplodeAtts.cylinderPoint1 = (0, 1, 10)
    ExplodeAtts.cylinderPoint2 = (0, 0, 10)
    ExplodeAtts.cylinderRadius = 4
    ExplodeAtts.cellExplosionFactor = 4
    ExplodeAtts.explosionPattern = ExplodeAtts.Impact
    ExplodeAtts.explodeAllCells = 1
    SetOperatorOptions(ExplodeAtts, 1)
    
    Test("explode_ucd3d_08")
    
    ResetOperatorOptions("Explode", 1)
    DeleteAllPlots()

def rectilinear_explosions():
    OpenDatabase(silo_data_path("rect3d.silo"))
    AddPlot("FilledBoundary", "mat1")

    View3DAtts = View3DAttributes()
    View3DAtts.viewNormal = (0.0758172, -0.984828, -0.156097)
    View3DAtts.focus = (0, -15, 0)
    View3DAtts.viewUp = (-0.01047, 0.155753, -0.987741)
    View3DAtts.viewAngle = 30
    View3DAtts.parallelScale = 8.3205
    View3DAtts.nearPlane = -34.641
    View3DAtts.farPlane = 34.641
    View3DAtts.imagePan = (0, 0)
    View3DAtts.imageZoom = 1.0
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
    expAtts = ExplodeAttributes()
    expAtts.explosionPoint = (1, 0, 0)
    expAtts.materialExplosionFactor = 10
    expAtts.material = "7"
    SetOperatorOptions(expAtts, 1)
    DrawPlots()

    AddOperator("Explode")
    expAtts = ExplodeAttributes()
    expAtts.explosionPoint = (0, 0, 1)
    expAtts.materialExplosionFactor = 10
    expAtts.material = "5"
    SetOperatorOptions(expAtts, 1)
    DrawPlots()

    Test("explode_rect3d_00")


def Main():
    unstructured_explosions()
    curvilinear_explosions()
    rectilinear_explosions()

Main()
Exit()
