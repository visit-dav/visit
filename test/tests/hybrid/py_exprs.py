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

Exit()
