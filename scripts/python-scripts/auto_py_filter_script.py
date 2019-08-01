
import os
import errno

# ----------------------- #
# --- User Input Area --- # 
# ----------------------- #

input_variables = [
    'd',
    'p',
    'my_expr',
]

expressions = [
    """my_python_expr = my_expr-d+p*p""",
    """my_python_expr_2 = d-p""",
]

output_file_name = 'example.py'

# ----------------------- #















# --------------------------- #
# --- Script details here --- #
# --------------------------- #

imports_str = """
import numpy as np
import vtk.util.numpy_support as vnp
"""

get_data_str = """
def smart_get_data(ds_in, var_name):
    out = ds_in.GetCellData().GetArray(var_name)
    if out is None:
        out = ds_in.GetPointData().GetArray(var_name)
    if out is None:
        raise Exception('Could not fetch %s' % var_name)
    res = vnp.vtk_to_numpy(out)
    return res
"""

class_body_str = """
    def __init__(self):
        SimplePythonExpression.__init__(self)
        self.name = "AutoPythonExpression"
        self.description = "Auto-generated python expression from auto_py_filter_script.py"
        self.output_is_point_var = False
        self.output_dimension = 1
        self.input_num_vars = """ + str(len(input_variables)) + """

    def derive_variable(self, ds_in, domain_id):
        # Globalize the names
        for var_name in self.input_var_names:
            globals()[var_name] = smart_get_data(ds_in, var_name)
        
        # Perform calculation
        out = self.user_expression()

        # Convert result to vtk
        res = vnp.numpy_to_vtk(out, deep=1)
        return res

    def user_expression(self):
        # Expression created by user and written here
"""


if __name__ == "__main__":
    print "Executing auto_py_filter_script.py..."

    # -------------------------- #
    # --- Parse the "inputs" --- #
    # -------------------------- #

    # Ensure .py extension
    if output_file_name[-3:] != '.py':
        if output_file_name.find('.'):
            print "ERROR: Extension for the output_file_name must be '.py'."
            raise SystemExit
        else:
            # Append '.py' to the end of the file
            output_file_name += '.py'


    # if output_file_name starts with directory, then it can work with os.
    # if it does not start with directory, then we need to append './' to the front.
    if output_file_name[0] not in ['.', '/', '~']:
        output_file_name = './' + output_file_name
    

    # name the sub directory the same as the file minus the extension
    sub_dir_name = output_file_name[2:-3] + '_sub'

    
    # ------------------------------------ #
    # --- Generate the controller file --- #
    # ------------------------------------ #

    if not os.path.exists(os.path.dirname(output_file_name)):
        try:
            os.makedirs(os.path.dirname(output_file_name))
        except OSError as exc:
            if exc.errno != errno.EEXIST:
                raise
        
    controller_file = open(output_file_name, 'w+') # Create the controller file
    if not os.path.exists(sub_dir_name):
        os.makedirs(sub_dir_name) # Create the subdirectory to store the additional files


    # Write the input variables
    controller_file.write('import visit\n\n')
    controller_file.write('input_variables = [\n')
    for input_var in input_variables:
        controller_file.write("    '" + input_var + "',\n")
    controller_file.write(']\n\n')


    # ---------------------------------------------- #
    # --- Generate sub files and controller file --- #
    # ---------------------------------------------- #
    for expr in expressions:
        # Get the output variable
        out_var = expr.split('=')[0].strip()

        # Populate the subfile
        sub_file_str = sub_dir_name + '/' + out_var + '.py'
        sub_file = open(sub_file_str, 'w+')
        sub_file.write(imports_str)
        sub_file.write(get_data_str + '\n')
        sub_file.write('class ' + out_var + '_PythonExpression(SimplePythonExpression):')
        sub_file.write(class_body_str)
        sub_file.write('        ' + expr + '\n\n')
        sub_file.write('        #Return the expression\n')
        sub_file.write('        return ' + out_var + '\n\n')
        sub_file.write('py_filter = ' + out_var + '_PythonExpression')

        # Populate the controller file
        def_python_str = 'visit.DefinePythonExpression("' + out_var + '", input_variables, file = "' + sub_file_str + '")\n'
        controller_file.write(def_python_str)

        # Close subfile
        sub_file.close()


    # Cleanup
    controller_file.close()

    print "Done!"






