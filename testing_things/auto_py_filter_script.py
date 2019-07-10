#!/usr/tce/bin/python

import os
import errno

input_variables = [
    'den',
    'tke',
    'zdvxdx',
    'zdvydy',
]

expressions = [
    'tauxx = 0.6667*den',
    'tauyy = 0.813*tke+zdvxdx',
    'tauxy = den*tke - zdvydy'
]

output_file_name = 'Boussinesq_expr.py'
sub_dir_name = ''

if __name__ == "__main__":
    print "Starting script"

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
    
    # If the subdirectory starts with a directory character, then we must ensure that the directories
    # for the subdirectory and the filename are the same.
    if sub_dir_name == '':
        # name the sub directory the same as the file minus the extension
        sub_dir_name = output_file_name[2:-3] + '_dir'
    elif sub_dir_name[0] in ['.', '/', '~']:
        if os.path.dirname(sub_dir_name) != os.path.dirname(output_file_name):
            print "ERROR: sub_dir_name and output_file_name must be in the same directory."
            print "sub_dir_dirname: " + os.path.dirname(sub_dir_name)
            print "output_file_dirname: " + os.path.dirname(output_file_name)
            raise SystemExit



    print 'output_file_name is : ' + output_file_name
    print 'sub_dir_name is : ' + sub_dir_name

    
    # generate the controller file

    print 'directory name is: ' + os.path.dirname(output_file_name)

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
    controller_file.write('input_variables = [\n')
    for input_var in input_variables:
        controller_file.write("\t'" + input_var + "',\n")
    controller_file.write(']\n\n')


    # define python expressions in subfiles
    for expr in expressions:
        out_var = expr.split('=')[0].strip()
        sub_file_str = sub_dir_name + '/' + out_var + '.py'
        sub_file = open(sub_file_str, 'w+')

        def_python_str = 'DefinePythonExpression("' + out_var + '", input_variables, file = ' + sub_file_str + ')\n'
        controller_file.write(def_python_str)
        sub_file.close()


    # Cleanup
    controller_file.close()







