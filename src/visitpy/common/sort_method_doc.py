#!/usr/bin/env python

copyright =  '// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt\n'
copyright += '// Project developers.  See the top-level LICENSE file for dates and other\n'
copyright += '// details.  No copyright assignment is required to contribute to VisIt.\n'
copyright += '\n'



# --------------- #
# --- Classes --- #
# --------------- #

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
    
    def extend(self, extension):
        self.text.append(extension)
    
    def __str__(self):
        """
            Overridden str method. When str() is called on
            the Container, it will be converted to a string
            formatted for MethodDoc.C

            returns:
                A formatted string. 
        """
        output  = '"' + self.title + r'\n' + '"\n'
        output += '"' + r'\n' + '"\n'
        for line in self.text:
            output += '"' + line + r'\n' + '"\n'
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
        self.text.append(split_colon(arg_name))

    def extend(self, extension):
        self.text.append(split_colon(extension))


class ReturnsContainer(Container):
    def __init__(self):
        self.title = 'Returns:'
        Container.__init__(self)


class DescriptionContainer(Container):
    def __init__(self):
        self.title = 'Description:'
        Container.__init__(self)


class ExampleContainer(Container): 
    def __init__(self):
        self.title = 'Example:'
        Container.__init__(self)
        
    def __str__(self):
        output  = '"' + self.title + r'\n' + '"\n'
        output += '"' + r'\n' + '"\n'
        for line in self.text[0:-1]:
            output += '"' + line + r'\n' + '"\n'
        output += ';\n'
        return output



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

def split_colon(line):
    if line.find(':') > -1:
        output = line.split()[0]
    else:
        output = line
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

    func_file   = open('MethodDoc.C', 'r')
    
    func_dict = {}
    func = ''
    func_name = ''
    
    func_file_lines = func_file.readlines()
    func_file.close()
    
    for line in func_file_lines:
        if line[0] == '*' or line[0] == '/':
            continue
            
        func += line
        if line[:18] == 'const char *visit_':
            indices = [i for i, ltr in enumerate(line) if ltr == '_']
            ndx1 = indices[0]+1
            ndx2 = indices[1]
            func_name = line[ndx1:ndx2]
        if line[0] == ';': # end of the function
            func_dict.update({func_name: func})
            func = ''
    
    func_file = open('MethodDoc.C','w')
    func_file.write(copyright)
    for function in sorted(func_dict.keys()):
        func_file.write(func_dict[function])
    func_file.close()
    
    
    
    
