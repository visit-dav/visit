#!/usr/bin/env python

import sys
import argparse

parser=argparse.ArgumentParser(
    description='''Auto-generate MethodDoc.C and MethodDoc.h.''')
parser.add_argument('top_level_dir', nargs='?', help='(Optional) Top level directory of the git repo. Needed if running this script from a different directory.')
args=parser.parse_args()

copyright =  '// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt\n'
copyright += '// Project developers.  See the top-level LICENSE file for dates and other\n'
copyright += '// details.  No copyright assignment is required to contribute to VisIt.\n'
copyright += '\n'

note =  '\n'
note += '// ***************************************************************************\n'
note += '// Warning: Autogenerated by /src/doc/functions_to_method_doc.py\n'
note += '//          ALL CHANGES TO THIS FILE WILL BE OVERWRITTEN!\n'
note += '//          Make changes to src/doc/python_scripting/functions.rst and run\n'
note += '//          functions_to_method_doc.py to update this file.\n'
note += '// ***************************************************************************\n'
note += '\n\n'

# --------------- #
# --- Classes --- #
# --------------- #

class HFunction(object):
    def __init__(self, _name):
        if _name[-1] == '\n':
            self.name = _name[0:-1]
        else:
            self.name = _name
        self.pre_text = 'extern const char *visit_'
        self.post_text = '_doc;\n'
        
    def __str__(self):
        return self.pre_text + self.name + self.post_text


class CFunction(object):
    def __init__(self, _name):
        if _name[-1] == '\n':
            self.name = _name[0:-1]
        else:
            self.name = _name
        self.pre_text = 'const char *visit_'
        self.post_text = '_doc = \n'
    
    def __str__(self):
        output  = self.pre_text + self.name + self.post_text
        output += '"' + self.name + r'\n' + '"\n'
        output += '"' + r'\n' + '"\n'
        output += '"' + r'\n' + '"\n'
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
        output  = '"' + self.title + r'\n' + '"\n'
        output += '"' + r'\n' + '"\n'
        for line in self.text:
            output += '"' + line + r'\n' + '"\n'
        if self.last:
            output += ';\n'
        else:
            output += '"' + r'\n' + '"\n'
            output += '"' + r'\n' + '"\n'
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
    
def insert_into_string(text, position, char):
    return text[:position] + char + text[position:]

def insert_backslash(text):
    # Generate a list of indices where " appears in the text
    indices = [i for i, ltr in enumerate(text) if ltr == '"']
    # Insert backslashes at the locations. Because this modifies the string, we
    # need to do this in reverse order
    for ndx in reversed(indices):
        text = insert_into_string(text, ndx, '\\')
    return text

def split_colon_add_spaces(line):
    colon_index = line.find(':')
    if colon_index > -1:
        output = line[:colon_index-1]
    else:
        output = '    ' + line
    return output


def write_state(writer, state_dict):
    if state_dict['synopsis']:
        writer.write(str(state_dict['synopsis']))
    if state_dict['arguments']:
        writer.write(str(state_dict['arguments']))
    if state_dict['returns']:
        writer.write(str(state_dict['returns']))
    if state_dict['description']:
        writer.write(str(state_dict['description']))
    if state_dict['example']:
        writer.write(str(state_dict['example']))
    
    
    
# ------------ #
# --- Main --- #
# ------------ #

if __name__ == '__main__':

    top_level = ''
    if len(sys.argv) == 2:
        top_level = sys.argv[1]
    
    if top_level == '':
        func_file_name = 'python_scripting/functions.rst'
        h_output_name  = '../visitpy/common/MethodDoc.h'
        c_output_name  = '../visitpy/common/MethodDoc.C'
    else:
        func_file_name = top_level + '/src/doc/python_scripting/functions.rst'
        h_output_name  = top_level + '/src/visitpy/common/MethodDoc.h'
        c_output_name  = top_level + '/src/visitpy/common/MethodDoc.C'

    func_file   = open(func_file_name, 'r')
    h_output    = open(h_output_name,'w')
    c_output    = open(c_output_name,'w')
    
    h_output.write(copyright)
    h_output.write(note)
    c_output.write(copyright)
    c_output.write(note)
    
    block_dict = {'synopsis': None, 'arguments': None, 'returns': None, 'description': None, 'example': None}
    cur_block = None
    first_func = True
    
    func_file_lines = func_file.readlines()
    for i in range(0, len(func_file_lines)):
        line = insert_backslash(func_file_lines[i])
        # print line[0:-1]
        
        if line[0] in ['\n', '|', ':', '='] or line == 'Functions\n':
            continue
            
        if line[0] == '-': # The previous line was a function name
            # Output the last state of the block_dict
            if not first_func:
                block_dict[cur_block].set_last(True)
                write_state(c_output, block_dict)
        
            first_func = False
            # Setup the next function
            cur_block = 'Function:'
            block_dict = {'synopsis': None, 'arguments': None, 'returns': None, 'description': None, 'example': None}
            h_output.write(str(HFunction(func_file_lines[i-1])))
            c_output.write(str(CFunction(func_file_lines[i-1])))
        
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
    write_state(c_output, block_dict)
   
    func_file.close()
    h_output.close()
    c_output.close()
    
    
    
    
