# [VisIt Python Script]
#
# Driver to test example python query.
#
# Usage:
#  visit -o path/to/rect2d.silo -nowin -cli -s visit_py_query_test.py
#

def script_path():
    # path magic to make sure we can locate the script file
    # no matter where we run visit from
    script_dir = os.path.split(__visit_source_file__)[0]
    return os.path.join(script_dir,"py_query.py")

if __name__ == "__main__":
    AddPlot("Pseudocolor","d")
    DrawPlots()
    if not PythonQuery(file=script_path()):
        print("Error Executing Python Query!")
    else:
        print("Python Query Results:")
        print(" ", GetQueryOutputString())
        print(" ", GetQueryOutputValue())
    sys.exit(0)


