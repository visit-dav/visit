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


class AttributesTable(Table):
    """ 
        An attributes table. 
    """ 

    def __init__(self, attribute, title=Row(["Attribute", "**Default**/Allowed Values"])):
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
        # Prep the default option. White space needs to be stripped. Also, some
        # defaults will come in with their "full name", such as
        # "colorControlPoints.Linear" instead of just "Linear". So we split it
        # on the period too and use the last string in the split list if it
        # exists.
        #
        default_stripped = r_one[1].strip()
        period_split = default_stripped.split('.')
        default_option = period_split[-1]
        
        #
        # Find the default option within the string of options, add asterisks
        # around it for bold, and move it to the beginning of the list.
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
            print("** Warning: function "+attr_names[attr]+" does not return any attributes", file=sys.stderr)
    
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
                print("\nERROR: MISSING CASE!", file=sys.stderr)
                print("LINE: " + str(line) + "\n", file=sys.stderr)
    
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

    atts_pre_file = open("cli_manual/preambles/attributes_preamble", "r")
    cli_pre_file  = open("cli_manual/preambles/events_preamble", "r")
  
    atts_file     = open("cli_manual/attributes.rst", "w")
    cli_file      = open("cli_manual/events.rst", "w")

    atts_preamble = ''.join(atts_pre_file.readlines())
    cli_preamble  = ''.join(cli_pre_file.readlines())
    
    
    missing_documentation = []
    attrlist              = []
    
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
    print("**\n**  Running VisIt", eval('visit.Version()'), "\n**", file=sys.stderr)
    
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
            continue
         elif (func[:3] == 'Get'):
             continue
         else:
           attrlist.append(func)
    
    # sort functions #
    attrlist.sort()
    event_names = visit.GetCallbackNames()
    
    template = "\n\n%s|\n|\n%s"

    atts_doc    = attributes_to_sphinx(attrlist)
    atts_doc    = template % (atts_preamble, atts_doc)
    atts_file.write(atts_doc)

    cli_doc     = cli_events_to_sphinx(event_names)
    cli_doc     = template % (cli_preamble, cli_doc)
    cli_file.write(cli_doc)
   
    atts_file.close()
    cli_file.close()
    atts_pre_file.close()
    cli_pre_file.close()
