


class ExampleContainer(object):
    """
        A container to hold and format a function example. 
    """

    def __init__(self, title = "Example:"):
        self.title    = "**%s**\n" % (title)
        self.example  = ""

    def extend_current_example(self, extension):
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
        
        
class SynopsisContainer(object):
    """
        A container to hold and format a function synopsis. 
    """
 
    def __init__(self, title = "Synopsis:"):
        self.synopsis   = ""
        self.title      = "**%s**\n" % (title)

    def extend_current_synopsis(self, extension):
        """
            Extend the current synopsis. 

            args:
                extension: the extension to be added. 
        """
        self.synopsis += "  %s\n" % (extension)

    def __str__(self):
        """
            Overridden str method. When str() is called on
            our SynopsisContainer, it will be converted to 
            a restructuredText formatted string. 

            returns:
                A restructuredText formatted string. 
        """
        output  = self.title
        output += "\n::\n\n"
        output += self.synopsis
        return output


class DescriptionContainer(object):
    """
        A container to hold and format the description of
        a function. 
    """

    def __init__(self, title = "Description:"):
        self.title       = "**%s**\n" % (title)
        self.description = ""
        self.table = Table()

    def extend_current_description(self, extension):
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
            self.table.table_rows.append(row)
 
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

        if self.table.table_rows != []:
            output += "\n\n%s\n" % (self.table.build_sphinx_table(title=True))

        return output


class ArgumentsContainer(object):
    """
        A container to hold and format all of the 
        arguments of a function. 
    """

    def __init__(self, title = ''):
        self.title           = ""
        self.names           = []
        self.descriptions    = []
        self.cur_idx         = -1
        self.arg_add_ons     = ['(optional)', 'int', 'float', 'bool']
        self.type_keywords   = {'integer': 'integer',
                                'string': 'string',
                                'name': 'string',
                                'string.': 'string',
                                'double':'double',
                                'tuple':'tuple',
                                'list':'list',
                                'dictionary':'dictionary'}

    def find_add_on(self, txt):
        """
            In most instances, an argument can be identified because
            it is a single word. However, there are some cases where 
            the argument has an additional word (an 'add-on') which 
            needs to be identified. 

            args:
                txt: the text to be analyzed.  

            returns: 
                The index of the add-on in self.args_add_ons if 
                an add-on is found. Otherwise, -1.  
        """
        for idx in range(len(self.arg_add_ons)):
            if self.arg_add_ons[idx] in txt:
                return idx
        return -1

    def is_arg_with_add_on(self, txt):
        """
            In most instances, an argument can be identified because
            it is a single word. However, there are some cases where 
            the argument has an additional word (an 'add-on') which 
            needs to be identified. 

            args:
                txt: the text to be analyzed.  

            returns: 
                True if an add-on is found. False otherwise. 
        """
        add_on = self.find_add_on(txt)
        if add_on > -1:
            if ' ' not in txt.replace(self.arg_add_ons[add_on], '').strip():
                return True
        return False
            
    def add_element(self, element):
        """
            Add an element to our container. This could be 
            an argument name (new argument) or a whole or 
            piece of an argument description that will be 
            appended on to the current (last added) argument
            description. 
            
            args:
                element: the element to add (arg name or 
                    description. 
        """
        #
        # Is element a description of an argument?
        #
        space = ' '
        if space in element.strip():

            #
            # People are very inconsistent with designating variables. 
            #
            four_space  = element.strip().find(space*4)
            three_space = element.strip().find(space*3)
            two_space   = element.strip().find(space*2)

            if ((four_space  > 0 and four_space < 10) or
                (three_space > 0 and three_space < 10) or
                (two_space   > 0 and two_space < 10)):

                split      = element.split()
                arg        = split[0].strip()
                definition = ' '.join(split[1:])
                self.add_argument(arg)
                self.extend_current_arg_description(definition)

            #
            # Do we have an arg with an add-on?
            #
            elif self.is_arg_with_add_on(element):
                self.add_argument(element)

            #  
            # We have a description.
            # 
            else:
                self.extend_current_arg_description(element)

        #
        # This is likely the last word in a description. 
        #
        elif '.' in element:
            el_cpy = element
            if element[0] != ' ':
                el_cpy = ' ' + element
            self.extend_current_arg_description(el_cpy)

        #
        # This should be a new argument. 
        #
        else:
            self.add_argument(element)

    def extend_current_arg_description(self, add_on):
        """
            Add to the current argument description. 

            args:
                add_on: the extension to our current 
                    description.
        """
        add_on_cpy = add_on

        #
        # People often don't add spaces after their new lines. 
        #
        if add_on.islower() and add_on[0] != ' ':
            add_on_cpy = ' ' + add_on
        if self.cur_idx > -1:
            self.descriptions[self.cur_idx] += add_on_cpy

    def add_argument(self, name, description=''):
        """
            Add a new argument to our container.  

            args:
                name: the name of the argument.         
                description: the argument description. 
        """
        self.names.append(name)
        self.descriptions.append(description)
        self.cur_idx += 1

    def __str__(self):
        """
            Overridden str method. When str() is called on
            our ArgumentsContainer, it will be converted to 
            a restructuredText formatted string. 

            returns:
                A restructuredText formatted string. 
        """
        lines = ""
        for i in range(len(self.names)):
            name    = "%s" % (self.names[i].strip())
            if name == 'none':
                continue
                
            #
            # Attempt to determine the type of argument from the argument's
            # description.
            #
            type_name = "STARTING_VALUE"
            for word in ''.join(self.descriptions[i]).split():
                if word in self.type_keywords.keys():
                    if type_name == "STARTING_VALUE":
                        type_name = self.type_keywords[word]
                    elif word != type_name:
                        type_name = "AMBIGUOUS"
            
            lines += "\n%s : %s\n    %s\n"%(name, type_name,
                self.descriptions[i])

        output = self.title
        output += lines

        return output
        

class ReturnsContainer(object):
    """
        A container to hold and format the return 
        information of a function. 
    """

    def __init__(self, _return_type, title = ""):
        self.title   = ""
        self.returns = ""
        self.return_type = _return_type

    def extend_current_returns(self, extension):
	"""
            Extend our current return information. 

            args:
                extension: the extension to add on. 
        """
        self.returns += "    %s\n" % (extension)

    def __str__(self):
        """
            Overridden str method. When str() is called on
            our ReturnsContainer, it will be converted to 
            a restructuredText formatted string. 

            returns:
                A restructuredText formatted string. 
        """
        if self.return_type != "NONE":
            if self.return_type == "integer":
                if self.returns.find("1 for success and 0 for failure"):
                    self. return_type = "CLI_return_t"
            output  = "return type : %s\n%s\n" %(self.return_type, self.returns)
            return output
        else:
            return ""
        

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
    

if __name__ == '__main__':

    func_pre_file = open("cli_manual/preambles/functions_preamble", "r")
    func_file     = open("cli_manual/functions.rst", "w")
    func_preamble = ''.join(func_pre_file.readlines())
    
    funclist              = []
    
    system_funcs = dir()
    system_funcs.extend(['system_funcs', 'func'])
    
    #
    # Everything that corresponds to the default Python stuff and this particular script
    # is now in system_funcs - hence everything that is in dir() after importing
    # from visit that is Not in system_funcs, must have come from visit.
    #
    # We need to take this tricky road to circumvent bug #457: import visit doesn't load
    # the Eval* functions in the visit-namespace.
    #
    
    visit.AddArgument("-nowin")
    visit.AddArgument("-noconfig")
    visit.Launch()
    print >> sys.stderr, "**\n**  Running VisIt", visit.Version(), "\n**"
    
    for func in dir(visit):
      # Deprecated
      if (func == "ConstructDDFAttributes"):
         continue
      if (func == "ConstructDDF"):
         continue
      if (func == "string"):
         continue
      if (func == "sys"):
         continue
      if (func[:2] == "__"):  # __doc__, __file__, etc.
         continue
    
      # all functions that have 'Attributes' in their name and don't start with
      # 'Set' or 'Get' will be collected seperately:
      if (func.find('Attributes') > -1):
         if (func[:3] == 'Set'):
           funclist.append(func)
         elif (func[:3] == 'Get'):
           funclist.append(func)
      else:
         funclist.append(func)
    
    # sort functions #
    funclist.sort()
    
    template = "\n\n%s|\n|\n%s"

    func_doc, _ = functions_to_sphinx(funclist)
    func_doc    = template % (func_preamble, func_doc)
    func_file.write(func_doc)
   
    func_file.close()
    func_pre_file.close()
    
    
