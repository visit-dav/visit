###############################################################################
# Python Module: filters
#
# Purpose: Base classes for embedded python filters.
# 
# Programmer: Cyrus Harrison
# Creation: Thu Mar  5 09:25:47 PST 2009
#
#
# Modifications:
#
###############################################################################

import sys

class PythonFilter(object):
    def __init__(self,name,description):
        """
        Constructor for the PythonFilter class.
        Sets common info used to interface with the avt pipeline.
        """
        # set by user to interface with avt
        self.name         = name
        self.description  = description
    def __str__(self):
        """
        Pretty print.
        """
        res = "PythonFilter:\n"
        res += " name = %s\n" % self.name
        res += " description = %s\n" % self.description
        return res
    @classmethod
    def load(cls,script_file):
        """
        Simple class method that reads and setups a filter script from disk.
        """
        script = open(script_file).read()
        env =sys.modules['__main__'].__dict__
        exec(script,env)

class PythonExpression(PythonFilter):
    def __init__(self):
        """
        Constructor for the PythonExpression class.
        Sets default values used to interface with the avt pipeline.
        """
        # set by user to interface with avt
        PythonFilter.__init__(self,"avtPythonExpression","Custom Python Expression")
        # set by filter author to interface with avt & the expression parser.
        self.input_num_vars   = 1
        self.output_dimension = 1
        self.output_is_point_var = False
        # set by avt to interface with the user
        self.input_var_names = []
        self.output_var_name = None
        self.arguments       = []
    def execute(self,data_sets, domain_ids):
        """
        Override this method with the guts of your custom python expression.
        Return a list of data_sets and a list of domain_ids.
        """
        return data_sets,domain_ids
    def modify_contract(self,contract):
        """
        Implement this method to modify the pipeline contract.
        """
        return contract
    def __str__(self):
        """
        Pretty print.
        """
        res = "PythonExpression:\n"
        res += " name = %s\n" % self.name
        res += " description = %s\n" % self.description
        res += " Input Info:\n"
        res += "  # of input vars = %d\n" % self.input_num_vars
        res += "  input var names = %s\n" % str(self.input_var_names)
        res += "  input arguments = %s\n" % str(self.arguments)
        res += " Output Info:\n"
        res += "  output var name = %s\n"  % str(self.output_var_name)
        res += "  output var dimension  = %d\n" % self.output_dimension
        res += "  output is point var = %s\n" % str(self.output_is_point_var)
        return res

class SimplePythonExpression(PythonExpression):
    def __init__(self):
        """
        Constructor for the SimplePythonExpression class.
        """
        PythonExpression.__init__(self)
    def execute(self,data_sets, domain_ids):
        """
        Implements an execute analogous to avtSingleInputExpression.
        """
        nsets = len(data_sets)
        res_sets = []
        for i in xrange(nsets):
            res = self.derive_variable(data_sets[i],domain_ids[i])
            res.SetName(self.output_var_name)
            rset = data_sets[i].NewInstance()
            rset.ShallowCopy(data_sets[i])
            if self.output_is_point_var:
                rset.GetPointData().AddArray(res)
            else:
                rset.GetCellData().AddArray(res)
            res_sets.append(rset)
        return res_sets,domain_ids
    def derive_variable(self,ds_in,domain_id):
        """
        Called on each chunk. Return a new vtkDataArray with expression
        result values.
        """
        return None


class PythonQuery(PythonFilter):
    def __init__(self):
        """
        Constructor for the PythonQuery class.
        Sets default values used to interface with the avt pipeline.
        """
        PythonFilter.__init__(self,"avtPythonQuery","Custom Python Query")
        self.result_txt  = None
        self.result_val  = None
        self.result_obj  = None
        self.float_format = "%g"
        self.input_var_names = []
    def pre_execute(self):
        """
        Called before streamining execute of all chunks.
        """
        pass
    def execute(self,data_sets, domain_ids):
        """
        Override this method with the guts of your custom python expression.
        """
        pass
    def post_execute(self):
        """
        Called after streamining execute of all chunks.
        To return results use:
          self.set_result_text()
          self.set_result_value()
          self.set_result_obj()
        """
        pass
    def modify_contract(self,contract):
        """
        Implement this method to modify the pipeline contract.
        """
        return contract
    def set_result_text(self,txt):
        self.result_txt = txt
    def set_result_value(self,val):
        self.result_val = val
    def set_result_obj(self,obj):
        self.result_obj = obj

class SimplePythonQuery(PythonQuery):
    def __init__(self):
        """
        Constructor for the SimplePythonQuery class.
        """
        PythonQuery.__init__(self)
    def execute(self,data_sets, domain_ids):
        """
        Implements an execute analogous to avtDatasetQuery.
        execute_chunk is called on each dataset.
        """
        nsets = len(data_sets)
        for i in xrange(nsets):
            self.execute_chunk(data_sets[i],domain_ids[i])
    def execute_chunk(self,ds_in,domain_id):
        """
        Called on each chunk.
        """
        pass
