# Example VisIt Python Expression
#
# Labels cells with the id of the process they reside on.
#
# Usage:
#  Load in the Expressions window or run with provided driver:
#   visit -o path/to/rect2d.silo -nowin -cli -s visit_py_expr_procid_test.py
#
from math import sin, pi
class PyProcidExpression(SimplePythonExpression):
    def __init__(self):
        SimplePythonExpression.__init__(self)
        self.name = "PyProcidExpression"
        self.description = "Labeling mesh by processor id"
        self.output_is_point_var  = False
        self.output_dimension = 1
    def derive_variable(self,ds_in,domain_id):
        ncells = ds_in.GetNumberOfCells()
        res = vtk.vtkFloatArray()
        res.SetNumberOfComponents(1)
        res.SetNumberOfTuples(ncells)
        for i in xrange(ncells):
            res.SetTuple1(i,float(mpicom.rank()))
        return res

py_filter = PyProcidExpression

