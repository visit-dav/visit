# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  cylindrical_radius.py
#
#  Tests:      mesh      - 3D structured, multi domain
#              plots     - pc
#
#  Ticket Id:  '7915
#
#  Notes:      Created to test axis selection for the cylindrical_radius 
#              expression
#
#  Programmer: Cyrus Harrison
#  Date:       March 31, 2008
#
# ----------------------------------------------------------------------------

# Turn off all annotations
TurnOffAllAnnotations() # defines global object 'a'

OpenDatabase("../data/multi_ucd3d.silo")

    
# Default use case
DefineScalarExpression("cylin_rad_default",   "cylindrical_radius(mesh1)")
# Cyl axis = {1,0,0}
DefineScalarExpression("cylin_rad_x",         'cylindrical_radius(mesh1,"x")')
# Cyl axis = {0,1,0}
DefineScalarExpression("cylin_rad_y",         'cylindrical_radius(mesh1,"y")')
# Cyl axis = {0,0,1} (same as default)
DefineScalarExpression("cylin_rad_z",         'cylindrical_radius(mesh1,"z")')
# Cyl axis = {1,1,1}
DefineScalarExpression("cylin_rad_diag_a",    'cylindrical_radius(mesh1,{1,1,1})')
# Cyl axis = {1,1,-1}
DefineScalarExpression("cylin_rad_diag_b",    'cylindrical_radius(mesh1,{1,1,-1})')

# use rotate and defer expression to test proper axis selection

def test_cyl(var_name):
    DeleteAllPlots();
    ResetView()
    AddPlot("Pseudocolor", var_name)
    DrawPlots()
    Test(var_name + "_%d" % 1)
    AddOperator("Transform") # rotate to another view
    TransformAtts = TransformAttributes()
    TransformAtts.doRotate = 1
    TransformAtts.rotateOrigin = (0, 0, 0)
    TransformAtts.rotateAxis = (0, 1, 0)
    TransformAtts.rotateAmount = 45
    TransformAtts.rotateType = TransformAtts.Deg
    TransformAtts.doScale = 0
    TransformAtts.doTranslate = 0
    TransformAtts.transformType = TransformAtts.Similarity  
    TransformAtts.inputCoordSys = TransformAtts.Cartesian  
    TransformAtts.outputCoordSys = TransformAtts.Cartesian
    SetOperatorOptions(TransformAtts)
    Test(var_name + "_%d" % 2)
    
def test_cyl_x(var_name):
    DeleteAllPlots();
    ResetView()
    AddPlot("Pseudocolor", var_name)
    AddOperator("Transform")
    TransformAtts = TransformAttributes()
    TransformAtts.doRotate = 1
    TransformAtts.rotateOrigin = (0, 0, 0)
    TransformAtts.rotateAxis = (0, 1, 0)
    TransformAtts.rotateAmount = 90
    TransformAtts.rotateType = TransformAtts.Deg
    TransformAtts.doScale = 0
    TransformAtts.doTranslate = 0
    TransformAtts.transformType = TransformAtts.Similarity  
    TransformAtts.inputCoordSys = TransformAtts.Cartesian  
    TransformAtts.outputCoordSys = TransformAtts.Cartesian
    SetOperatorOptions(TransformAtts)
    AddOperator("DeferExpression")
    DeferExpressionAtts = DeferExpressionAttributes()
    DeferExpressionAtts.exprs = (var_name)
    SetOperatorOptions(DeferExpressionAtts)
    DrawPlots()
    Test(var_name + "_%d" % 1)
    AddOperator("Transform") # rotate to better view
    TransformAtts = TransformAttributes()
    TransformAtts.doRotate = 1
    TransformAtts.rotateOrigin = (0, 0, 0)
    TransformAtts.rotateAxis = (0, 1, 0)
    TransformAtts.rotateAmount = -45
    TransformAtts.rotateType = TransformAtts.Deg 
    TransformAtts.doScale = 0
    TransformAtts.doTranslate = 0
    TransformAtts.transformType = TransformAtts.Similarity  
    TransformAtts.inputCoordSys = TransformAtts.Cartesian  
    TransformAtts.outputCoordSys = TransformAtts.Cartesian
    SetOperatorOptions(TransformAtts)
    Test(var_name + "_%d" % 2)
    
    
def test_cyl_y(var_name):
    DeleteAllPlots();
    ResetView()
    AddPlot("Pseudocolor", var_name)
    AddOperator("Transform")
    TransformAtts = TransformAttributes()
    TransformAtts.doRotate = 1
    TransformAtts.rotateOrigin = (0, 0, 0)
    TransformAtts.rotateAxis = (1, 0, 0)
    TransformAtts.rotateAmount = 90
    TransformAtts.rotateType = TransformAtts.Deg 
    TransformAtts.doScale = 0
    TransformAtts.doTranslate = 0
    TransformAtts.transformType = TransformAtts.Similarity  
    TransformAtts.inputCoordSys = TransformAtts.Cartesian  
    TransformAtts.outputCoordSys = TransformAtts.Cartesian
    SetOperatorOptions(TransformAtts)
    AddOperator("DeferExpression")
    DeferExpressionAtts = DeferExpressionAttributes()
    DeferExpressionAtts.exprs = (var_name)
    SetOperatorOptions(DeferExpressionAtts)
    DrawPlots()
    Test(var_name + "_%d" % 1)
    AddOperator("Transform") # rotate to better view
    TransformAtts = TransformAttributes()
    TransformAtts.doRotate = 1
    TransformAtts.rotateOrigin = (0, 0, 0)
    TransformAtts.rotateAxis = (1, 0, 0)
    TransformAtts.rotateAmount = -45
    TransformAtts.rotateType = TransformAtts.Deg 
    TransformAtts.doScale = 0
    TransformAtts.doTranslate = 0
    TransformAtts.transformType = TransformAtts.Similarity  
    TransformAtts.inputCoordSys = TransformAtts.Cartesian  
    TransformAtts.outputCoordSys = TransformAtts.Cartesian
    SetOperatorOptions(TransformAtts)
    Test(var_name + "_%d" % 2)

def test_cyl_diag_a(var_name):
    DeleteAllPlots();
    ResetView()
    AddPlot("Pseudocolor", var_name)
    AddOperator("Transform")
    TransformAtts = TransformAttributes()
    TransformAtts.doRotate = 1
    TransformAtts.rotateOrigin = (0, 0, 0)
    TransformAtts.rotateAxis = (0, 1, 0)
    TransformAtts.rotateAmount = 45
    TransformAtts.rotateType = TransformAtts.Deg 
    TransformAtts.doScale = 0
    TransformAtts.doTranslate = 0
    TransformAtts.transformType = TransformAtts.Similarity  
    TransformAtts.inputCoordSys = TransformAtts.Cartesian  
    TransformAtts.outputCoordSys = TransformAtts.Cartesian
    SetOperatorOptions(TransformAtts)
    AddOperator("Transform") # rotate to better view
    TransformAtts = TransformAttributes()
    TransformAtts.doRotate = 1
    TransformAtts.rotateOrigin = (0, 0, 0)
    TransformAtts.rotateAxis = (-0.70710678118654746, 0, 0.70710678118654746)
    TransformAtts.rotateAmount = 35.26
    TransformAtts.rotateType = TransformAtts.Deg
    TransformAtts.doScale = 0
    TransformAtts.doTranslate = 0
    TransformAtts.transformType = TransformAtts.Similarity  
    TransformAtts.inputCoordSys = TransformAtts.Cartesian  
    TransformAtts.outputCoordSys = TransformAtts.Cartesian
    SetOperatorOptions(TransformAtts)
    AddOperator("DeferExpression")
    DeferExpressionAtts = DeferExpressionAttributes()
    DeferExpressionAtts.exprs = (var_name)
    SetOperatorOptions(DeferExpressionAtts)
    DrawPlots()
    Test(var_name + "_%d" % 1)

def test_cyl_diag_b(var_name):
    DeleteAllPlots();
    ResetView()
    AddPlot("Pseudocolor", var_name)
    AddOperator("Transform")
    TransformAtts = TransformAttributes()
    TransformAtts.doRotate = 1
    TransformAtts.rotateOrigin = (0, 0, 0)
    TransformAtts.rotateAxis = (0, 1, 0)
    TransformAtts.rotateAmount = -45
    TransformAtts.rotateType = TransformAtts.Deg  
    TransformAtts.doScale = 0
    TransformAtts.doTranslate = 0
    TransformAtts.transformType = TransformAtts.Similarity  
    TransformAtts.inputCoordSys = TransformAtts.Cartesian  
    TransformAtts.outputCoordSys = TransformAtts.Cartesian
    SetOperatorOptions(TransformAtts)
    AddOperator("Transform") # rotate to better view
    TransformAtts = TransformAttributes()
    TransformAtts.doRotate = 1
    TransformAtts.rotateOrigin = (0, 0, 0)
    TransformAtts.rotateAxis = (0.70710678118654746, 0, 0.70710678118654746)
    TransformAtts.rotateAmount = 35.26
    TransformAtts.rotateType = TransformAtts.Deg  
    TransformAtts.doScale = 0
    TransformAtts.doTranslate = 0
    TransformAtts.transformType = TransformAtts.Similarity  
    TransformAtts.inputCoordSys = TransformAtts.Cartesian  
    TransformAtts.outputCoordSys = TransformAtts.Cartesian
    SetOperatorOptions(TransformAtts)
    AddOperator("DeferExpression")
    DeferExpressionAtts = DeferExpressionAttributes()
    DeferExpressionAtts.exprs = (var_name)
    SetOperatorOptions(DeferExpressionAtts)
    DrawPlots()
    Test(var_name + "_%d" % 1)

       
test_cyl("cylin_rad_default")
test_cyl_x("cylin_rad_x")
test_cyl_y("cylin_rad_y")
test_cyl("cylin_rad_z") # same as default
test_cyl_diag_a("cylin_rad_diag_a")
test_cyl_diag_b("cylin_rad_diag_b")
    
Exit()
