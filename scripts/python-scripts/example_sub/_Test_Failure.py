
import numpy as np
import vtk.util.numpy_support as vnp

class _TEST1PythonExpression(SimplePythonExpression):

    def __init__(self):
        SimplePythonExpression.__init__(self)
        self.name = "AutoPythonExpression"
        self.description = "Auto-generated python expression from auto_py_filter_script.py"
        self.output_is_point_var = False
        self.output_dimension = 1
    def derive_variable(self, ds_in, domain_id):

        # d = vnp.vtk_to_numpy(ds_in.GetCellData().GetArray(self.input_var_names[0]))
        # p = vnp.vtk_to_numpy(ds_in.GetCellData().GetArray(self.input_var_names[1]))
        # n = vnp.vtk_to_numpy(ds_in.GetCellData().GetArray(self.input_var_names[2]))
        
        d = ds_in.GetCellData().GetArray(self.input_var_names[0])
        p = ds_in.GetCellData().GetArray(self.input_var_names[1])
        n = ds_in.GetCellData().GetArray(self.input_var_names[2])

        # Convert result to vtk
        res = vtk.vtkFloatArray()
        res.SetNumberOfComponents(1)
        res.SetNumberOfTuples(ds_in.GetNumberOfCells())
        return res

py_filter = _TEST1PythonExpression