# [VisIt Python Script]
#
# Driver to test mpicom module via a python query.
# 
# Usage:
#  visit -o path/to/rect2d.silo -nowin -cli -s visit_mpicom_test.py
#
if __name__ == "__main__":
    AddPlot("Pseudocolor","d")
    DrawPlots()
    Query("Python","PythonFilter.load('py_test_mpicom_query.py')");
    print GetQueryOutputString()
    print GetQueryOutputValue()
    sys.exit(0)


