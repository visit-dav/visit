#!/usr/bin/env python
# -*- coding: utf-8 -*-

############################################################################################
# @file visitdocs.py
#
# @namespace visitdocs
# @brief Namespace for the visitdocs.py file
#
# @mainpage VisItDocs - extract inline documentation from VisIt Python module.
#
# @author: Alister Maguire
# @date:   Tue Apr  3 09:51:25 PDT 2018
#
# This script is used for generating three sections of the VisIt Python Manual in Sphinx
# (restructuredText). These three sections are function documentation, visit cli events, 
# and attributes reference. 
#
# This script is an evolution from a previous script used to generate documentation in 
# LaTeX and some sections of code were directly or indirectly kept in some form or another. 
# The predecessor script was written by Jakob van Bethlehem on Nov 2010, May 2011. 
#
############################################################################################

import os, sys, re
import visit

################# CLASSES #################

class Row(list):
    """
        A Row object is a python list with a
        variable designating whether or not it 
        is an extension to a previous row. This
        is useful when building tables. 
    """

    def __init__(self, *args, **kwargs):
        self.is_extension = False
        super(Row, self).__init__(*args, **kwargs)


class Table(object):
    """
        A Table object to be used in constructing tables
        in sphinx. 
    """

    def __init__(self):
        self.table_rows  = []
        self.max_col_len = 45
    
    def insert_two_columns(self, col_one, col_two, italic=False):
        """
            Insert two columns into the table. 

            args:
                col_one: the first column as a string. 

                col_two: the second columns as a string. 

                italic: whether or not the last column should
                    be in italic. 
            
        """

        italic_insert = '*' if italic else ''
        words         = col_two.split()
        words         = [word.strip() for word in words]

        #
        # If the length of col_two exceeds our max_col_len, 
        # we need to perform some magic to get sphinx to
        # break the line up to display on multiple lines. 
        # (col_one was ignored because it is generally just
        # an argument. May be implemented in the future.)
        #
        if len(''.join(words)) > self.max_col_len:

            #
            # If col_two is a single word, we're stuck with
            # its length. Just add it. 
            #
            if len(words) == 1:
                col_one = col_one.strip()
                col_two = "%s%s%s" % (italic_insert,
                                      col_two.strip(),
                                      italic_insert)
                nxt_row = Row([col_one, col_two])
                self.table_rows.append(nxt_row)

            #
            # col_two is multiple words. Let's break it up
            # based on our max_col_len. 
            #
            else:
                is_ext        = False
                col_one       = col_one.strip()
                count         = len(words[0])
                col_two       = " %s%s" % (italic_insert, words[0])

                for i in range(1, len(words)):

                    if (count + len(words[i].strip())) > self.max_col_len:
                        col_two += italic_insert
                        nxt_row  = Row([col_one, col_two])

                        #
                        # If this isn't the first line, it's an 
                        # extension of the column we started with. 
                        #
                        nxt_row.is_extension = is_ext
                        self.table_rows.append(nxt_row)
                        col_one = ""
                        col_two = " %s%s" % (italic_insert, words[i])
                        count   = len(words[i])
                        if not is_ext:
                            is_ext = True

                    else:
                        col_two += " %s" % (words[i])
                        count   += len(words[i])

                col_two += italic_insert
                last_row = Row([col_one, col_two])
                last_row.is_extension = True
                self.table_rows.append(last_row) 

        else:
            col_one = col_one.strip()
            col_two = "%s%s%s" % (italic_insert,
                                 col_two.strip(),
                                 italic_insert)
      
            nxt_row = Row([col_one, col_two])
            self.table_rows.append(nxt_row)


    def build_sphinx_table(self, title=False, indent=True):
        """
            Builds a sphinx formated table in string format. 
    
            args:
                title: is the first line of our column a title?

                indent: should we indent the entire table? 
    
            returns: 
                A string form of a sphinx table. 
        """
    
        num_rows = len(self.table_rows)
    
        if num_rows <= 0 or type(self.table_rows[0]) != Row:
            print("ERROR: rows must be > 0 and of type Row")
            return ""
        if len(self.table_rows[0]) <= 0:
            print("ERROR: cannot build an empty table")
            return ""
    
        num_cols = len(self.table_rows[0])
    
        #
        # We need to know how long the rows should be extended for
        # each column. 
        #
        max_lengths = [1]*num_cols
        for i in range(num_rows):
            for j in range(num_cols):
                row_i_j        = len(self.table_rows[i][j]) + 2
                cur            = max_lengths[j]
                max_lengths[j] = row_i_j if row_i_j > cur else cur 
    
        #
        # We need to account for 'bolding' the title.
        #
        if title:
            max_lengths[0] += 4
    
        preclude      = ''
        if indent:
            preclude += " " * 4
    
        table_header     = '='
        standard_top_bot = "+"
        ext_top          = '|' 
        table            = ""
        for _ in range(num_cols):
            standard_top_bot += "%s+"
            ext_top      += "%s|"
    
        standard_top_bot = preclude + standard_top_bot
        ext_top          = preclude + ext_top
    
        standard_inserts = []
        ext_inserts      = []
        for i in range(num_cols):
            standard_inserts.append('-' * max_lengths[i])
            ext_inserts.append(' ' * max_lengths[i])
    
        standard_top_bot = standard_top_bot % tuple(standard_inserts)
        ext_top          = ext_top % tuple(ext_inserts)
    
        #
        # If we have a title, we build this separately. 
        #
        start = 0
        if title:
            table += standard_top_bot
            table += "\n"
            for j in range(num_cols):
                item = self.table_rows[0][j]
                if j > 0:
                    left_over = max_lengths[j] - len(item) - 1
                    inner_pre = ""
    	            table += "%s| %s%s" % (inner_pre, item, " " * (left_over))
                else:
                    left_over = max_lengths[j] - len(item) - 5
                    inner_pre = preclude 
      	            table += "%s| **%s**%s" % (inner_pre, item, " " * (left_over))
            table +=  "|\n"
            start += 1
    
        #
        # Now we can build the actual table. 
        #
        for i in range(start, num_rows):
            #if self.table_rows[i].is_extension:
            #    table += ext_top
            #else:
            #    table += standard_top_bot
            if not self.table_rows[i].is_extension:
                table += standard_top_bot
                table += "\n"
                

            #table += "\n"
            for j in range(num_cols):
                item      = self.table_rows[i][j]
                left_over = max_lengths[j] - len(item) -1
                inner_pre = "" if j > 0 else preclude
                table += "%s| %s%s" % (inner_pre, item, " " * (left_over))
            table +=  "|\n"
    
        table += standard_top_bot
    
        return table


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
            if self.return_type == "AMBIGUOUS":
                output = "return : \n%s\n" %(self.returns)
                return output
            else:
                output  = "return type : %s\n%s\n" %(self.return_type, self.returns)
                return output
        else:
            return ""


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


class AttributesTable(Table):
    """ 
        An attributes table. 
    """ 

    def __init__(self, attribute, title=Row(["Attribute", "**Default/Allowed Values**"])):
        super(AttributesTable, self).__init__()
        self.title       = title
        self.attribute   = attribute
        self.max_col_len = 20
        self.table_rows.append(title)


    def rows_from_hash_match(self, group_one, group_two):
        """
            Create a row from a 'hash match', which is just
            a type of match found while parsing an attribute. 

            args:
                group_one: a string of the form 'attribute = default'
                group_two: a string of the form 'alternative val, 
                    alternative val, ...'
              
        """
        r_one = group_one.split('=')
        
        #
        # Prep the default option. White space needs to be stripped. Also, some defaults
        # will come in with their "full name", such as "colorControlPoints.Linear" instead
        # of just "Linear". So we split it on the period too and use the last string in the
        # split list if it exists.
        #
        default_stripped = r_one[1].strip()
        period_split = default_stripped.split('.')
        default_option = period_split[-1]
        
        #
        # Find the default option within the string of options, add asterisks around it
        # for bold, and move it to the beginning of the list.
        #
        word_start = group_two.find(default_option)
        if word_start != -1:
            word_end = word_start + len(default_option)
            if word_start != 1: # it is not first in the list of options
                group_two_mod = '**' + group_two[word_start:word_end] + '**,' + \
                                group_two[:word_start-2] + group_two[word_end:]
            else: # it is first in the list of options
                group_two_mod = '**' + group_two[word_start:word_end] + '**,' + \
                                group_two[word_end+1:]
        self.insert_two_columns(r_one[0], group_two_mod)
    
    def row_from_basic_match(self, group_one, group_two):
        """
            Create a row from a 'basic match', which is just
            a type of match found while parsing an attribute. 

            args:
                group_one: a string of the form 'attribute'
                group_two: a string of the form 'value'
        """
        self.insert_two_columns(group_one.strip(), group_two.strip())

    def row_from_hash_line(self, hash_line):
        """
            Create a row from a 'hash line', which is just
            a type of match found while parsing an attribute. 

            args:
                hash_lint: a line of the form '# some value'
        """
        line = "%s" % (hash_line.strip("#").strip())
        self.insert_two_columns("", line, True)

    def __str__(self):
        """
            Overridden str method. When str() is called on
            our AttributesTable, it will be converted to 
            a restructuredText formatted string. 

            returns:
                A restructuredText formatted string. 
        """
        char_count  = len(self.attribute) * 2
        char_count += len("****: *Attributes()*")

        output      = "**%s**: *%sAttributes()*\n" % (self.attribute, self.attribute)
        output     += '-' * char_count
        output     += "\n|\n\n%s\n" % (self.build_sphinx_table(title=True))

        return output


class CLIEventsTable(Table):
    """
        A table containing cli events. 
    """

    def __init__(self, title=Row(["EventName", "*ArgCount*"])):
        super(CLIEventsTable, self).__init__()
        self.title = title
        self.table_rows.append(title)

    def add_list_row(self, row):
        """ 
            Add a row to our table. 

            args:
                row: a list of the form [event name, arg count].
        """
        row = Row(row)
        self.table_rows.append(row)

    def __str__(self):
        """
            Overridden str method. When str() is called on
            our CLIEventsTable, it will be converted to 
            a restructuredText formatted string. 

            returns:
                A restructuredText formatted string. 
        """
        output = "\n%s\n\n" % (self.build_sphinx_table(title=True))
        return output


################# END CLASSES #################


################# FUNCTIONS #################

def build_function_header(function_name):
    """
        Build a header for a function. 

        args:
            function_name: the name of our function.
    """
    function_name = function_name.strip()
    num_char      = len(function_name)
    header        = function_name + '\n'
    header       += "-"*num_char + '\n\n'
    return header


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


def attributes_to_sphinx(atts):
    """
        Convert a VisIt generated list of attributes to a
        restructuredText formatted string. 

        args:
            atts: a VisIt generated list of attributes. 
   
        returns: 
            attributes_doc: a restructuredText formatted string 
                documenting attributes. 
    """
    attributes_doc = ""

    #
    # Compiled regex to detect all function names that 
    # have the string '<i>Attributes</i>' in it.
    #
    attr_matcher = re.compile(r'(Get)?(.*)Attributes$')

    #
    # Store all the attribute names with the functions that 
    # will return them.
    #
    attr_names = {}
    for func in atts:
        aname = attr_matcher.match(func)
        attr_names[aname.group(2)] = func
    
    #
    # Compiled regex to detect lines of the form 'value = something'.
    #
    basic_finder = re.compile(r'([^=]+)=(.*)')
    #
    # Compiled regex to detect lines of the form 
    # 'value = default # alternative1, alternative2, ...'
    #
    hash_finder = re.compile(r'([^#]+)#(.+)')
    
    for attr in sorted(attr_names.keys()):
        if attr in ('ResetPick', 'ResetQueryOverTime'):
            continue
    
        table       = AttributesTable(attr) 

        visitString = "visit."+attr_names[attr]
        alist       = str(eval(visitString)()).splitlines()
        if not alist:
            print >> sys.stderr, "** Warning: function "+attr_names[attr]+" does not return any attributes"
    
        for line in alist:
            basic_match = basic_finder.match(line)
            hash_match  = hash_finder.match(line)
    
            if hash_match:
                 table.rows_from_hash_match(hash_match.group(1), 
                                            hash_match.group(2))
            elif basic_match:
                 table.row_from_basic_match(basic_match.group(1), 
                                            basic_match.group(2))
            elif ''.join(line).strip()[0] == '#':
                 str_form = ''.join(line).strip()
                 table.row_from_hash_line(str_form)
            else:
                print >> sys.stderr, "\nERROR: MISSING CASE!"
                print >> sys.stderr, "LINE: " + str(line) + "\n"
    
        attributes_doc += "\n|\n\n%s\n|\n" % (str(table))

    return attributes_doc


def cli_events_to_sphinx(event_names):
    """
        Convert a VisIt gerenated list of cli events to
        a restructuredText formatted document. 

        args:
            event_names: a list of cli event names. 

        returns: 
            A restructuredText formatted string. 
    """
    table = CLIEventsTable()
    for ev in sorted(event_names):
        table.add_list_row([ev, str(visit.GetCallbackArgumentCount(ev))]) 
    
    return "\n|\n%s|\n" % (str(table))
            
################ END FUNCTIONS ################



if __name__ == '__main__':

    func_pre_file = open("cli_manual/preambles/functions_preamble", "r")
    atts_pre_file = open("cli_manual/preambles/attributes_preamble", "r")
    cli_pre_file  = open("cli_manual/preambles/events_preamble", "r")
  
    func_file     = open("cli_manual/functions.rst", "w")
    atts_file     = open("cli_manual/attributes.rst", "w")
    cli_file      = open("cli_manual/events.rst", "w")

    func_preamble = ''.join(func_pre_file.readlines())
    atts_preamble = ''.join(atts_pre_file.readlines())
    cli_preamble  = ''.join(cli_pre_file.readlines())
    
    
    missing_documentation = []
    attrlist              = []
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
      # 'Set' will be collected seperately:
      if (func.find('Attributes') > -1):
         if (func[:3] == 'Set'):
           funclist.append(func)
         elif (func[:3] == 'Get'):
           funclist.append(func)
         else:
           attrlist.append(func)
      else:
         funclist.append(func)
    
    # sort functions #
    funclist.sort()
    attrlist.sort()
    event_names = visit.GetCallbackNames()
    
    template = "\n\n%s|\n|\n%s"

    func_doc, _ = functions_to_sphinx(funclist)
    func_doc    = template % (func_preamble, func_doc)
    func_file.write(func_doc)

    atts_doc    = attributes_to_sphinx(attrlist)
    atts_doc    = template % (atts_preamble, atts_doc)
    atts_file.write(atts_doc)

    cli_doc     = cli_events_to_sphinx(event_names)
    cli_doc     = template % (cli_preamble, cli_doc)
    cli_file.write(cli_doc)
   
    func_file.close()
    atts_file.close()
    cli_file.close()
    atts_pre_file.close()
    func_pre_file.close()
    cli_pre_file.close()
