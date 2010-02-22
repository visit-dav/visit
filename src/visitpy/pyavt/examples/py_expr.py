# Example VisIt Python Expression
#
# Example embedded python expression that creates a scalar wave
# pattern on a rectilinear mesh.
#
# This expression works on the rect2d.silo & rect3d.silo datasets.
#
# Usage:
#  Load in the Expressions window or run with provided driver:
#   visit -o path/to/rect2d.silo -nowin -cli -s visit_py_expr_test.py
#
from math import sin, pi
class MyExpression(SimplePythonExpression):
    def __init__(self):
        SimplePythonExpression.__init__(self)
        self.name = "MyExpression"
        self.description = "constructing simple wave pattern"
        self.output_is_point_var  = False
        self.output_dimension = 1
    def modify_contract(self,contract):
        pass
    def derive_variable(self,ds_in,domain_id):
        # ds_in is a vtk dataset, we want
        # to create and return a new vtkDataArray
        # that contains a simple sine wave pattern
        ds_bounds = ds_in.GetBounds()
        x_ext = ds_bounds[1] - ds_bounds[0]
        y_ext = ds_bounds[3] - ds_bounds[2]
        z_ext = ds_bounds[5] - ds_bounds[4]
        ncells = ds_in.GetNumberOfCells()
        res = vtk.vtkFloatArray()
        res.SetNumberOfComponents(1)
        res.SetNumberOfTuples(ncells)
        for i in xrange(ncells):
            cell = ds_in.GetCell(i)
            bounds = cell.GetBounds()
            xv = bounds[0] + bounds[1] / 2.0
            yv = bounds[2] + bounds[3] / 2.0
            zv = bounds[4] + bounds[5] / 2.0
            val = sin(xv*3*pi/x_ext) + sin(yv * 3*pi / y_ext)
            if z_ext != 0:
                val+= sin(zv * 3*pi / z_ext)
            res.SetTuple1(i,.2 * val)
        return res

py_filter = MyExpression

