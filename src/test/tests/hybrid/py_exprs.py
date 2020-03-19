# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  py_exprs.py
#
#  Tests:      python filters / python expressions
#              plots     - Pseudocolor
#
#
#  Programmer: Cyrus Harrison
#  Date:       Fri May 21 09:14:07 PDT 2010
#
#  Modifications:
#
#  Eddie Rusu - Tue Jul 30 13:22:17 PDT 2019
#  Added testing for python expressions with numpy mixed with simple
#  expressions.
#
# ----------------------------------------------------------------------------

import os
from os.path import join as pjoin

OpenDatabase(silo_data_path("rect2d.silo"))


script_file = pjoin(os.path.split(TestScriptPath())[0],"py_expr_script_00.vpe")

DefinePythonExpression("d_wave",("d"),file=script_file)
AddPlot("Pseudocolor", "d_wave")
AddOperator("Elevate")
v = View3DAttributes()
v.viewNormal = (0.724969, -0.544156, 0.422273)
v.focus = (0.5, 0.666667, 0.526085)
v.viewUp = (-0.377473, 0.198927, 0.904401)
v.viewAngle = 30
v.parallelScale = 0.981769
v.nearPlane = -1.96354
v.farPlane = 1.96354
v.imagePan = (0, 0)
v.imageZoom = 1
v.perspective = 1
v.eyeAngle = 2
v.centerOfRotationSet = 0
v.centerOfRotation = (0.5, 0.666667, 0.526085)
SetView3D(v)
DrawPlots()
Test("py_exprs_00")
DeleteAllPlots()


# Test simple expressions mixed with python expressions using numpy
OpenDatabase(silo_data_path("multi_rect2d.silo"))

script_file = pjoin(os.path.split(TestScriptPath())[0],"simple_numpy_expr.py")
DefinePythonExpression("python_multiply", ['d','p'], file=script_file)
AddPlot("Pseudocolor", "python_multiply", 1, 1)
DrawPlots()
Test("py_exprs_01")
DeleteAllPlots()

script_file = pjoin(os.path.split(TestScriptPath())[0],"python_simple_mix_01.py")
DefineScalarExpression("my_expr", "d*p")
DefinePythonExpression("python_with_simple", ['d','p', 'my_expr'], file=script_file)
AddPlot("Pseudocolor", "python_with_simple", 1, 1)
DrawPlots()
Test("py_exprs_02")
DeleteAllPlots()

script_file = pjoin(os.path.split(TestScriptPath())[0],"python_simple_mix_02.py")
DefinePythonExpression("python_with_python", ['d','p', 'python_multiply'], file=script_file)
AddPlot("Pseudocolor", "python_with_python", 1, 1)
DrawPlots()
Test("py_exprs_03")
DeleteAllPlots()

DefineScalarExpression("simple_with_python", "python_multiply-p+d+p*p")
AddPlot("Pseudocolor", "simple_with_python", 1, 1)
DrawPlots()
Test("py_exprs_04")
DeleteAllPlots()




Exit()
