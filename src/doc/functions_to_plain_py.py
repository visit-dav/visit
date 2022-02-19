#!/usr/bin/env python
#
# file: functions_to_plain_py.py
# author: cyrush (but most code taken from functions_to_method_doc.py)
#

import sys

##############################################################################
# This is a hacked up version of functions_to_method_doc.py
# it creates a single output file:
#   PY_RST_FUNCTIONS_TO_PYTHON.py 
# with all of the python examples from functions.rst
#
# Why is it useful? You can use this to check python syntax 
# for all of our examples, and to check python 3 compat with 2to3.
##############################################################################


# --------------- #
# --- Classes --- #
# --------------- #

class PyFunction(object):
    def __init__(self, _name):
        if _name[-1] == '\n':
            self.name = _name[0:-1]
        else:
            self.name = _name
        self.pre_text = ''
        self.post_text = ''
    
    def __str__(self):
        output = "# " + self.name  + " (EXAMPLE)\n"
        output += "\n"
        output += "\n"
        return output


class Container(object):
    """
        A container to hold and format documentation.
    """
    def __init__(self):
        self.text = []
        self.last = False
    
    def extend(self, extension):
        self.text.append(extension)
    
    def set_last(self, is_last):
        self.last = is_last
    
    def __str__(self):
        """
            Overridden str method. When str() is called on
            the Container, it will be converted to a string
            formatted for MethodDoc.C

            returns:
                A formatted string. 
        """
        if self.last:
            self.text = self.text[0:-1]
        output = ""
        output += "\n"
        for line in self.text:
            output +=  line  + "\n"
        if self.last:
            output += "\n"
        else:
            output += "\n\n"
        return output


class SynopsisContainer(Container):
    def __init__(self):
        self.title = 'Synopsis:'
        Container.__init__(self)


class ArgumentsContainer(Container):
    """
        A container to hold and format a function synopsis. 
    """ 
    def __init__(self, arg_name):
        self.title = 'Arguments:'
        Container.__init__(self)
        self.text.append(split_colon_add_spaces(arg_name))

    def extend(self, extension):
        self.text.append(split_colon_add_spaces(extension))


class ReturnsContainer(Container):
    def __init__(self):
        self.title = 'Returns:'
        Container.__init__(self)
    
    def extend(self, extension):
        self.text.append(split_colon_add_spaces(extension))


class DescriptionContainer(Container):
    def __init__(self):
        self.title = 'Description:'
        Container.__init__(self)


class ExampleContainer(Container): 
    def __init__(self):
        self.title = 'Example:'
        Container.__init__(self)



# ----------------- #
# --- Functions --- #
# ----------------- #

def split_colon_add_spaces(line):
    colon_index = line.find(':')
    if colon_index > -1:
        output = line[:colon_index-1]
    else:
        output = '    ' + line
    return output


def write_state(writer, state_dict):
    # 
    if state_dict['example']:
        writer.write(str(state_dict['example']))

# ------------ #
# --- Main --- #
# ------------ #

if __name__ == '__main__':

    func_file_name = 'python_scripting/functions.rst'
    py_output_name = 'PY_RST_FUNCTIONS_TO_PYTHON.py'

    func_file   = open(func_file_name, 'r')
    py_output    = open(py_output_name,'w')
        
    block_dict = {'synopsis': None, 'arguments': None, 'returns': None, 'description': None, 'example': None}
    cur_block = None
    first_func = True
    
    func_file_lines = func_file.readlines()
    for i in range(0, len(func_file_lines)):
        line = func_file_lines[i]
        # print line[0:-1]
        
        if line[0] in ['\n', '|', ':', '='] or line == 'Functions\n':
            continue
            
        if line[0] == '-': # The previous line was a function name
            # Output the last state of the block_dict
            if not first_func:
                block_dict[cur_block].set_last(True)
                write_state(py_output, block_dict)
        
            first_func = False
            # Setup the next function
            cur_block = 'Function:'
            block_dict = {'synopsis': None, 'arguments': None, 'returns': None, 'description': None, 'example': None}
            py_output.write(str(PyFunction(func_file_lines[i-1])))
        
        elif line == '.. end of the file':
            break;
            
        elif line[0:13] == '**Synopsis:**':
            cur_block = 'synopsis'
            block_dict['synopsis'] = SynopsisContainer()
            
        elif cur_block != 'arguments' and line[0:6] != 'return' and line.find(' : ') > -1:
            cur_block = 'arguments'
            block_dict['arguments'] = ArgumentsContainer(line.strip())
            
        elif line[0:6] == 'return':
            cur_block = 'returns'
            block_dict['returns'] = ReturnsContainer()
            
        elif line[0:16] == '**Description:**':
            cur_block = 'description'
            block_dict['description'] = DescriptionContainer()
            
        elif line[0:12] == '**Example:**':
            cur_block = 'example'
            block_dict['example'] = ExampleContainer()

        elif cur_block is not None:
            # for examples sources, we need to preserve spaces in 
            # the example script to keep python code valid
            if cur_block == 'example' and line.strip() != "":
                # find the leading number of spaces using lstrip
                padding = len(line) - len(line.lstrip())
                # assume padding of 2 spaces in rst
                padding = padding - 2
                next_text = ""
                for i in range(padding):
                    next_text += " "
                next_text += line.strip()
            else:
                next_text = line.strip()
            block_dict[cur_block].extend(next_text)    
    # Write the last function
    block_dict[cur_block].set_last(True)
    write_state(py_output, block_dict)
   
    func_file.close()
    py_output.close()

