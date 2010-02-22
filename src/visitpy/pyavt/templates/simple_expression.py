#
# Template for a Simple Python Expression.
#
class PyExpr(SimplePythonExpression):
    def __init__(self):
        """
        Constructor.
        """
        SimplePythonExpression.__init__(self)
        # set to provide a name & description for your expression.
        self.name = "PyExpr"
        self.description = "Custom Python Expression"
        # output_is_point_var:
        # true  if output centering is nodal
        # false if output centering is zonal
        self.output_is_point_var  = False
        # output_dimension:
        # set to indicate number of components in the output tuple.
        self.output_dimension = 1
    def derive_variable(self,ds_in,domain_id):
        """
        Called to process each chunk.

        Use self.input_var_names & self.arguments to access expression
        variable names and const arguments.

        Return a new vtkDataArray with expression result values.
        """
        pass
    def modify_contract(self,contract):
        """
        Implement this method to modify the pipeline contract.
        """
        pass

py_filter = PyExpr