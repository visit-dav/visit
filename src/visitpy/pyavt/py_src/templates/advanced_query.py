#
# Template for an Advanced Python Query.
#
class PyQuery(PythonQuery):
    def __init__(self):
        """
        Constructor.
        """
        PythonQuery.__init__(self)
        # set to provide a name & description for your query.
        self.name = "PyQuery"
        self.description = "Custom Python Query"
    def pre_execute(self):
        """
        Called prior to execute().

        Override to initialize query state.
        """
        pass
    def execute(self,data_sets, domain_ids):
        """
        Called to process all data sets and domain ids local to this process.

        Use self.input_var_names to access variable names.
        """
        pass
    def post_execute(self):
        """
        Called after all chunks were processed.

        To set final query results use:
            self.set_result_text()
            self.set_result_value()
        """
        pass

py_filter = PyQuery

