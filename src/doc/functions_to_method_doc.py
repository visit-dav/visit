#!/usr/bin/env python

copyright = '/*****************************************************************************\n'
copyright += '*\n'
copyright += '* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC\n'
copyright += '* Produced at the Lawrence Livermore National Laboratory\n'
copyright += '* LLNL-CODE-442911\n'
copyright += '* All rights reserved.\n'
copyright += '*\n'
copyright += '* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The\n'
copyright += '* full copyright notice is contained in the file COPYRIGHT located at the root\n'
copyright += '* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.\n'
copyright += '*\n'
copyright += '* Redistribution  and  use  in  source  and  binary  forms,  with  or  without\n'
copyright += '* modification, are permitted provided that the following conditions are met:\n'
copyright += '*\n'
copyright += '*  - Redistributions of  source code must  retain the above  copyright notice,\n'
copyright += '*    this list of conditions and the disclaimer below.\n'
copyright += '*  - Redistributions in binary form must reproduce the above copyright notice,\n'
copyright += '*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the\n'
copyright += '*    documentation and/or other materials provided with the distribution.\n'
copyright += '*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may\n'
copyright += '*    be used to endorse or promote products derived from this software without\n'
copyright += '*    specific prior written permission.\n'
copyright += '*\n'
copyright += '* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"\n'
copyright += '* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE\n'
copyright += '* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE\n'
copyright += '* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,\n'
copyright += '* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY\n'
copyright += '* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL\n'
copyright += '* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR\n'
copyright += '* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER\n'
copyright += '* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT\n'
copyright += '* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY\n'
copyright += '* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH\n'
copyright += '* DAMAGE.\n'
copyright += '*\n'
copyright += '*****************************************************************************/\n'
copyright += '\n'

class ExampleContainer(object):
    """
        A container to hold and format a function example. 
    """

    def __init__(self, title = "Example:"):
        self.title    = "**%s**\n" % (title)
        self.example  = ""

    def extend(self, extension):
	"""
            Extend our function example. 

            args:
                extension: the extension to be added. 
        """
        #
        # If we have a 'visit -cli' line in our example, we 
        # need to comment it out so that it's proper python
        # format. 
        #
        if "visit -cli" in extension.strip() and self.example == "":
            self.example += "  #%s\n" % (extension)
        else: 
            self.example += "  %s\n" % (extension)

    def __str__(self):
        """
            Overridden str method. When str() is called on
            our ExampleContainer, it will be converted to 
            a restructuredText formatted string. 

            returns:
                A restructuredText formatted string. 
        """
        output  = "\n%s" % (self.title)
        output += "\n::\n\n"
        output += self.example
        return output 


class DescriptionContainer(object):
    """
        A container to hold and format the description of
        a function. 
    """

    def __init__(self, title = "Description:"):
        self.title       = "**%s**\n" % (title)
        self.description = ""

    def extend(self, extension):
        """
            Extend the current function description. 
            This could be a text description or a full
            or piece of a table. 

            args:
                extension: the extension to be added. 
        """

        #  
        # Is the extension a table?
        # 
        if '\t' in extension:
            spaced = extension.replace('\t', '@$@!')
            split  = spaced.split('@$@!')
            row    = Row([el.strip() for el in split if el != ''])
 
        #
        # It's just a description.
        #
        else:
            self.description += "    %s\n" % (extension)

    def __str__(self):
        """
            Overridden str method. When str() is called on
            our DescriptionContainer, it will be converted to 
            a restructuredText formatted string. 

            returns:
                A restructuredText formatted string. 
        """
        output  = self.title
        output += "\n"
        output += self.description

        return output
            
            
class Function(object):
    def __init__(self, _name):
        self.name = _name


class HFunction(Function):
    def __init__(self, _name):
        if _name[-1] == '\n':
            self.name = _name[0:-1]
        else:
            self.name = _name
        self.pre_text = 'extern const char *visit_'
        self.post_text = '_doc;\n'
        
    def __str__(self):
        return self.pre_text + self.name + self.post_text


class CFunction(Function):
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


class SynopsisContainer(object):
    """
        A container to hold and format a function synopsis. 
    """
 
    def __init__(self):
        self.title  = 'Synopsis:'
        self.text   = []

    def extend(self, extension):
        """
            Extend the current synopsis. 

            args:
                extension: the extension to be added. 
        """
        
        self.text.append(extension)

    def __str__(self):
        """
            Overridden str method. When str() is called on
            our SynopsisContainer, it will be converted to 
            a restructuredText formatted string. 

            returns:
                A restructuredText formatted string. 
        """
        output  = '"' + self.title + r'\n' + '"\n'
        output += '"' + r'\n' + '"\n'
        for line in self.text:
            output += '"' + line + r'\n' + '"\n'
        output += '"' + r'\n' + '"\n'
        output += '"' + r'\n' + '"\n'
        return output


class ArgumentsContainer(object):
    """
        A container to hold and format a function synopsis. 
    """
 
    def __init__(self):
        self.title  = 'Arguments:'
        self.text   = []
 
    def __init__(self, arg_name):
        self.title  = 'Arguments:'
        self.text   = []
        if arg_name.find(':') > -1:
            arg_name = arg_name.split()[0]
        self.text.append(arg_name)

    def extend(self, extension):
        """
            Extend the current synopsis. 

            args:
                extension: the extension to be added. 
        """
        if extension.find(':') > -1:
            # Strip off the argument type
            extension = extension.split()[0]
            
        self.text.append(extension)

    def __str__(self):
        """
            Overridden str method. When str() is called on
            our SynopsisContainer, it will be converted to 
            a restructuredText formatted string. 

            returns:
                A restructuredText formatted string. 
        """
        output  = '"' + self.title + r'\n' + '"\n'
        output += '"' + r'\n' + '"\n'
        for line in self.text:
            output += '"' + line + r'\n' + '"\n'
        output += '"' + r'\n' + '"\n'
        output += '"' + r'\n' + '"\n'
        return output


class ReturnsContainer(object):
    """
        A container to hold and format a function synopsis. 
    """
 
    def __init__(self):
        self.title  = 'Returns:'
        self.text   = []

    def extend(self, extension):
        """
            Extend the current synopsis. 

            args:
                extension: the extension to be added. 
        """
        
        self.text.append(extension)

    def __str__(self):
        """
            Overridden str method. When str() is called on
            our SynopsisContainer, it will be converted to 
            a restructuredText formatted string. 

            returns:
                A restructuredText formatted string. 
        """
        output  = '"' + self.title + r'\n' + '"\n'
        output += '"' + r'\n' + '"\n'
        for line in self.text:
            output += '"' + line + r'\n' + '"\n'
        output += '"' + r'\n' + '"\n'
        output += '"' + r'\n' + '"\n'
        return output



'''
def functions_to_sphinx(funclist):
    """
        Create a sphinx document from a VisIt generated
        list of functions and function documentation. 

        args:
            funclist: a list of functions and documentation
                generated by VisIt. 

        returns: 
            A tuple of the form (functions_doc, undocumented), 
            where functions_doc is a restructuredText formatted
            documentation of VisIt python functions, and undocumented
            is a list of undocumented functions. 
    """

    undocumented  = []
    functions_doc = ""

    for func in funclist:
        #
        # Extract documentation for this function.
        #
        visitStr="visit."+func
        fulldoc = eval(visitStr).__doc__

        #
        # Functions that are not documented, are appended to 'undocumented'. 
        #
        if not fulldoc:
            undocumented.append(func)
            continue

        fulldoc    = fulldoc.splitlines()
        full_doc   = [el for el in fulldoc if el != ''] 

        block_dict = {'Synopsis:': None, 'Arguments:': None, 'Returns:': None, 'Description:': None, 'Example:':None}
        block_list = block_dict.keys()

        func_name  = str(func)
        return_type = "STARTING_VALUE"
        full_doc   = full_doc[1:]
        if len(full_doc) == 0:
            continue
        start      = 0

        #
        # Ignore the brief explanation
        #
        while start < len(full_doc) and full_doc[start].strip() not in block_list:
            start += 1
        full_doc = full_doc[start:]
    
        #
        # Iterate over and parse the blocks. Then format them to sphinx and write.  
        #
        cur_block = None
        for element in full_doc:
            element = element.strip()
  
            #
            # First block 
            #
            if cur_block == None:
                if element not in block_list:
                    print >> stderr, "ERROR: incorrect start point!!"
                    continue
                else:
                    cur_block = element
                    continue

            #
            # Starting new block
            # 
            if element in block_list:
                cur_block = element
                continue

            elif cur_block == 'Example:':
                if not block_dict[cur_block]:
                    block_dict[cur_block] = ExampleContainer()
                block_dict[cur_block].extend_current_example(element)

            elif cur_block == 'Synopsis:':
                if not block_dict[cur_block]:
                    block_dict[cur_block] = SynopsisContainer()
                    
                #
                # Attempt to extract the return type from the synopsis
                #
                if element.find(str(func_name + '(')) > -1:
                    # Grab the output type from the synopsis. Look for
                    # func_name(args) -> output_type
                    arrow_index = element.find('->')
                    if arrow_index < 0:
                        return_type_helper = "NONE"
                    else:
                        return_type_helper = element[arrow_index+3:]
                    if return_type == "STARTING_VALUE":
                        return_type = return_type_helper
                    elif return_type != return_type_helper:
                        return_type = "AMBIGUOUS"
                        
                block_dict[cur_block].extend_current_synopsis(element)

            elif cur_block == 'Description:':
                if not block_dict[cur_block]:
                    block_dict[cur_block] = DescriptionContainer()
                block_dict[cur_block].extend_current_description(element)

            elif cur_block == 'Arguments:':
                if not block_dict[cur_block]:
                    block_dict[cur_block] = ArgumentsContainer()
                block_dict[cur_block].add_element(element)

            elif cur_block == 'Returns:':
                if not block_dict[cur_block]:
                    block_dict[cur_block] = ReturnsContainer(return_type)
                block_dict[cur_block].extend_current_returns(element)
        
        #
        # Build our sphinx output.
        #
        output = build_function_header(func_name)
        if block_dict['Synopsis:']:
            output += str(block_dict['Synopsis:'])
            output += '\n'
        if block_dict['Arguments:']:
            output += str(block_dict['Arguments:'])
            output += '\n'
        if block_dict['Returns:']:
            output += str(block_dict['Returns:'])
            output += '\n'
        if block_dict['Description:']:
            output += str(block_dict['Description:'])
            output += '\n'
        if block_dict['Example:']:
            output += str(block_dict['Example:'])
            output += '\n'

        functions_doc += "\n%s" % str(output)

    return (functions_doc, undocumented)
'''


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
    
    

if __name__ == '__main__':

    func_file   = open('cli_manual/functions.rst', 'r')
    h_output    = open('DELETE_ME.h','w')
    c_output    = open('DELETE_ME.C','w')
    
    h_output.write(copyright)
    c_output.write(copyright)
    
    block_dict = {'synopsis': None, 'arguments': None, 'returns': None, 'description': None, 'example': None}
    cur_block = None
    
    func_file_lines = func_file.readlines()
    for i in range(0, len(func_file_lines)):
        line = func_file_lines[i]
        
        if line[0] in ['\n', '|', ':', '='] or line == 'Functions\n':
            continue
            
        if line[0] == '-': # The previous line was a function name
            # Output the last state of the block_dict
            write_state(c_output, block_dict)
            # c_output.write(gather_state(block_dict)) if gather_state returns a string
            
            # Setup the next function
            cur_block = 'Function:'
            block_dict = {'synopsis': None, 'arguments': None, 'returns': None, 'description': None, 'example': None}
            h_output.write(str(HFunction(func_file_lines[i-1])))
            c_output.write(str(CFunction(func_file_lines[i-1])))
            
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
            block_dict[cur_block].extend(line.strip())
   
    func_file.close()
    h_output.close()
    c_output.close()
    
    
