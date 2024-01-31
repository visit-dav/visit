# ----------------------------------------------------------------------------
# Example VisIt Python Expression with numpy
#
# Converts input variables into numpy arrays, performs element-wise
# arithmetic, and returns result as vtk array. One of the arguments is a
# python expression.
#
#  Programmer: Eddie Rusu
#  Date:       Tue Jul 30 13:22:17 PDT 2019
#
# ----------------------------------------------------------------------------

import numpy as np
import vtk.util.numpy_support as vnp
class Test3PythonExpression(SimplePythonExpression):

    def __init__(self):
        SimplePythonExpression.__init__(self)
        self.name = "AutoPythonExpression"
        self.description = "Auto-generated python expression from auto_py_filter_script.py"
        self.output_is_point_var = False
        self.output_dimension = 1
        self.input_num_vars   = 3
    def derive_variable(self, ds_in, domain_id):
        # Globalize the names
        for var_name in self.input_var_names:
            globals()[var_name] = vnp.vtk_to_numpy(ds_in.GetCellData().GetArray(var_name))
        
        # Perform calculation
        out = self.user_expression()

        # Convert result to vtk
        res = vnp.numpy_to_vtk(out, deep=1)
        return res
    def user_expression(self):
        # Expression created by user and written here
        res = python_multiply-p+d+p*p

        #Return the expression
        return res

py_filter = Test3PythonExpression
