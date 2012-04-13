#
# Template for a Simple Python Query.
#
class PyQuery(SimplePythonQuery):
    def __init__(self):
        """
        Constructor.
        """
        SimplePythonQuery.__init__(self)
        # set to provide a name & description for your query.
        self.name = "PyQuery"
        self.description = "Custom Python Query"
    def pre_execute(self):
        """
        Called prior to any calls to execute_chunk().

        Use to initialize query state.
        """
        pass
    def execute_chunk(self,ds_in,domain_id):
        """
        Called to process each chunk.

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