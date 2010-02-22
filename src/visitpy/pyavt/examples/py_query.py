# Example VisIt Python Expression
#
# Example embedded python query that calcultes the average cell value.
# (currently serial only)
# 
#
# Usage:
#  Load in the Query window or run with provided driver:
#   visit -o path/to/rect2d.silo -nowin -cli -s visit_py_query_test.py
#
# simple cell average query
class CellAvgQuery(SimplePythonQuery):
    def __init__(self):
        SimplePythonQuery.__init__(self)
        self.name = "CellAvgQuery"
        self.description = "Calculate the average cell value."
    def pre_execute(self):
        # init vars used to compute the average
        self.total_ncells = 0
        self.total_sum    = 0.0
    def execute_chunk(self,ds_in,domain_id):
        # sum over cell data array passed to query args
        ncells = ds_in.GetNumberOfCells()
        self.total_ncells += ncells
        cell_data = ds_in.GetCellData().GetArray(self.input_var_names[0])
        for i in xrange(ncells):
            self.total_sum += cell_data.GetTuple1(i)
    def post_execute(self):
        # calculate average and set results
        res_val = self.total_sum / float(self.total_ncells)
        res_txt = "The average value = " + self.float_format
        res_txt = res_txt % res_val
        self.set_result_text(res_txt)
        self.set_result_value(res_val)

py_filter = CellAvgQuery

