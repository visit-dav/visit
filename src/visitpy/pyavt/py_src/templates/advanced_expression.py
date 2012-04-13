#
# Template for an Advanced Python Expression.
#
class PyExpr(PythonExpression):
    def __init__(self):
        """
        Constructor.
        """
        PythonExpression.__init__(self)
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
    def execute(self,data_sets, domain_ids):
        """
        Called to process all data sets and domain ids local to this process.

        Process data sets and create an appropriate copy with additonal
        vtkDataArray result. Set the names of created vtkDataArrays to:
         self.output_var_name

        Use self.input_var_names & self.arguments to access expression
        variables and const arguments.

        Return two lists:
            one providing result data sets
            one providing the corresponding domain ids.
        """
        pass
    def modify_contract(self,contract):
        """
        Implement this method to modify the pipeline contract.
        """
        pass

py_filter = PyExpr
