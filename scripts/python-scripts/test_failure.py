
import numpy as np
import vtk.util.numpy_support as vnp

def smart_get_data(ds_in, var_name):
    out = ds_in.GetCellData().GetArray(var_name)
    if out is None:
        out = ds_in.GetPointData().GetArray(var_name)
    if out is None:
        raise Exception("Could not fetch %s" % var_name)
    return out

class _TEST1PythonExpression(SimplePythonExpression):

    def __init__(self):
        SimplePythonExpression.__init__(self)
        self.name = "AutoPythonExpression"
        self.description = "Auto-generated python expression from auto_py_filter_script.py"
        self.output_is_point_var = False
        self.output_dimension = 1
    def derive_variable(self, ds_in, domain_id):

        d = smart_get_data(ds_in, self.input_var_names[0])
        p = smart_get_data(ds_in, self.input_var_names[1])
        # n = smart_get_data(ds_in, self.input_var_names[2])

        # Output an empty array
        res = vtk.vtkFloatArray()
        res.SetNumberOfComponents(1)
        res.SetNumberOfTuples(ds_in.GetNumberOfCells())
        return res

py_filter = _TEST1PythonExpression