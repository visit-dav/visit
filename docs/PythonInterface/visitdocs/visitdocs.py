#!/usr/bin/env python
# -*- coding: utf-8 -*-

## @file visitdocs.py
#  @brief Simple, dirty script to extract inline documentation from the Python @e VisIt module

## @namespace visitdocs
#  @brief Namespace for the visitdocs.py file

## @mainpage VisItDocs - extract inline documentation from VisIt Python module.
#
# The script imports the VisIt module and tries to extract the <i>__doc__</i> property
# for everything that is found in that namespace. From that an effort is made to create
# LaTeX code that formats the documentation such that it looks more or less
# like the existing documentation for the Python interface.
#
# Three different sections of function-documentation are created:
#  -# Listing of functions that are available <i>before</i> the <tt>Launch()</tt> command
#     is issued.
#  -# Listing of all functions that are available after <tt>Launch()</tt> (except the ones mentioned in 1)
#    and that have documentation
#  -# Listing of all functions for which no <i>__doc__</i> string was found
#
# Finally a section is created with all the attributes that are returned by functions
# that have the string '<i>Attributes</i>' as part of their name. No pre-selection is done here
# on the presence of documentation for that function. It's just a listing of all the attributes
# that function returns.
#
# If a filename is passed on the cmdline, the results are written in that file,
# otherwise the results are written to <tt>sys.stdout</tt>. Any problems that are
# encountered are sent to <tt>sys.stderr</tt>.
#
# <h1>Problems</h1>
# 'Problems' that are encountered could be any of the following:
#   - some identifier is found for which <i>__doc__</i> is an empty string
#   - some identifier is found for which one of the standard
#   headings <i>Synopsis</i>, <i>Arguments</i>, <i>Returns</i>, <i>Description</i> and <i>Example</i>
#   is not present
#   - An attribute function does not return anything
#
#
#  Jakob van Bethlehem, Nov 2010, May 2011

# debugging needed?
#import pdb

# Here we start:
import re

# FUNCTIONS #

## Lambda function to create a standard header for a documented function.
func_header = lambda name,short: """
\\phantomsection
\\addcontentsline{toc}{subsection}{""" + name + """}
\\noindent {\\bf """ + name + """}: """ + (short[0].lower() + short[1:]) + "\\\\[-3mm]\n"

## Lambda function to create a standard header for an attribute section
attr_header = lambda name, func_name: """
\\phantomsection
\\addcontentsline{toc}{subsection}{""" + name + """}
\\noindent {\\bf """ + name + """}: {\\it """ + func_name + """() }\\\\[-3mm]"""

## Lambda function to create a standard header for an undocumented function.
undoc_header = lambda name: """
\\phantomsection
\\addcontentsline{toc}{subsection}{""" + name + """}
\\noindent {\\bf """ + name + """} \n"""

## Lambda function that puts the passed line in LaTeX-verbatim mode.
make_verb = lambda line: "\\verb!" + line + "!"

## Convert the line into a proper LaTeX-string.
# <tt>string escape_latex(string line)</tt>
#
# Deal with characters that have special meaning in LaTeX.
# Currently:
#   - <,>,-   are surrounded by $$
#   - #,_,%   are escaped by prefixing '\\'
#
def escape_latex(line):
  output = ""
  for c in line:
    if c in ('-', '>', '<'):
      output += ("$"+c+"$")
      continue
    #
    if c in ('#', '_','%'):
      output += ("\\"+c)
      continue
    #
    output += c
    #
  return output
#

## Create LaTeX-code to create a new segment for the documentation of some function.
# <tt>void start_block(string block, string table, string blockline)</tt>
#
# Create segment <tt>block</tt>. We use <tt>blockline</tt> as the header
# to the segment. When the segment to start happens to be the 'Arguments'
# section, we start a new table such that we get a nicely formatted
# list of arguments.
def start_block(block, table, blockline):
  print >> output_fh, "\\noindent {\\it " + blockline + "}",
  if block in ('Synopsis', 'Arguments', 'Returns', 'Description'):
    print >> output_fh, "\\\\ \n\\hangindent=\\parindent",
  #

  if block == 'Arguments':
    table.create_new(False)
  #

  if block == 'Example':
    output_fh.write("\\\\[-6mm]\n\\begin{verbatim}")
  #
#

## Finish documentation segment <tt>block</tt>.
# <tt>void finish_block(string block, string[] func_args, string funcname)</tt>
#
# Write LaTeX code for finishing segment <tt>block</tt> for
# function <tt>funcname</tt>. When the 'Arguments' block
# is the one to finish, all argument names that were in the
# 'Synopsis' block should now have a value. We check this and
# write a warning if a check fails.
def finish_block(block, func_args, funcname):
  if block == 'Synopsis':
    print >> output_fh, "[-3mm]\n"
  #
  if block == 'Arguments':
    print >> output_fh,"\n"
  #
  if block in ('Returns','Description'):
    print >> output_fh, "\\\\[-3mm] \n"
  #

  if block == 'Arguments':
    for k in func_args.keys():
      if not func_args[k]: # check the arguments:
        print >> sys.stderr, "** Warning: argument",k,"of function",funcname,"was not documented."
    #
    return
  #

  if block == 'Example':
    print >> output_fh, "\\end{verbatim}"
  #
#


## Support class to build LaTeX tables.
# There are different places where LaTeX tables are built
# in the documentation. In some places they really serve as kind of
# a list of some 'mapping', meaning that the first and second column
# are small and we want a border. In other places it serves as a trick
# to nicely format the description that corresponds to some function
# argument. In that case we don't want a border, and the right column
# can contain a whole paragraph of text. While parsing the documentation
# we can't tell which one and how wide the right column will have to
# be.
#
# This class allows to build the table in memory first and then
# write the full table at once as soon as it is known what the
# elements have to be.
class Table:

  ## Create a new object.
  # <tt>__init__(self, bool useBorder=False)</tt>
  #
  #  Create an empty table, with or without a border
  def __init__(self, useBorder=False):

    ## Store the rows of the table.
    self.rows = []
    ## Store the index of the row that was added last.
    self.cur_row = -1
    ## Remember the setting for using a border.
    self.border = useBorder
    ## Remember the waiting-status for the object - by default it is off.
    self.waiting = False
  #

  ## Return <tt>true</tt> if this object is currently building a table.
  # <tt> bool is_building(self)</tt>
  #
  # @return <tt>true</tt> if Table::create_new was called but Table::finish
  #  has not yet been called, <tt>false</tt> otherwise
  def is_building(self):
    return self.cur_row > -1
  #

  ## Put the table into waiting modus before actually finishing
  # <tt>void to_wait_modus(self)</tt>
  #
  # Any subsequent calls to Table::create_new, Table::new_head, Table::extend
  # or Table::extend_row will result in an Exception. The object is
  # made active again after calling Table::finish. By having the table object
  # wait before finishing, it is possible to let the caller make decisions
  # on the output that depend on what actually comes <i>after</i> the table.
  def to_wait_modus(self):
    self.waiting = True
  #

  ## Return <tt>true</tt> if the object is in waiting mode.
  # <tt>bool is_waiting(self)</tt>
  #
  # @sa Table::to_wait_modus
  def is_waiting(self):
    return self.waiting
  #

  ## Return <tt>False</tt> if a call to Table::finish would result in a full table.
  # <tt>bool incomplete(self)</tt>
  #
  #  When the table has heads without a corresponding value return <tt>True</tt>, otherwise
  #  return <tt>False</tt>. An empty table is considered complete, so for an empty
  #  table the result will be <tt>False</tt>.
  def incomplete(self):
    if self.cur_row > -1:
      return not bool(self.rows[self.cur_row][1])
    return False
  #

  ## Start accumulating a new table.
  # <tt>void create_new(self, bool useBorder)</tt>
  #
  # If the object is still in the process of accumulating another table, an exception
  #  will result.
  def create_new(self, useBorder):
    if self.cur_row > -1 or self.waiting:
      raise SystemExit("** Error: starting new table while still building a previous one! Bailing out.")
    #
    self.border = useBorder
  #

  ## Create a new head for a row in the table.
  # <tt>void new_head(self, string line)</tt>
  #
  #  Use <tt>line</tt> as left entry in a new row in the table. If the object
  #  is in waiting mode, an Exception is thrown
  def new_head(self, line):
    if self.waiting:
      raise SystemExit("** Error: new_head() was called while Table is in waiting modus! ")
    self.rows.append([line, ""])
    self.cur_row += 1
  #

  ## Expand the table with a new row of two columns
  # <tt> void add_row(self, string[2] cols) </tt>
  #
  # If the argument <tt>cols</tt> happens to be longer, the rest is simply ignored.
  # The difference with Table::extend is that this function just bluntly adds
  # whatever you want to add. It also allows for example to add an empty
  # first column with a filled second column
  def add_row(self, cols):
    if self.waiting:
      raise SystemExit("** Error: add_row() was called while Table is in waiting modus! ")
    self.rows.append([cols[0], cols[1]])
    self.cur_row += 1
  #

  ## Expand the table either with a new row or expand the current value.
  # <tt>void extend(self, string[] cols)</tt>
  #
  #  If the first element of <tt>cols</tt> is non-empty, a new row is created
  #  with the first and last element as columns.
  #
  #  If the first element is empty, but the last column is not empty, it is assumed to
  #  the continuation of some value and the value on the current row is expanded.
  #
  #  If the list has a length of 1, it is added in the first column with the second column
  #  set to the empty string.
  #
  #  If the object is in wait-modus, an Exception is thrown.
  def extend(self, cols):  # cols has to be an array of string values
    if self.waiting:
      raise SystemExit("** Error: extend() was called while Table is in waiting modus! ")

    if len(cols)==1:
      self.rows.append([cols,""])
      self.cur_row += 1
      return
    #

    if cols[0]:
      self.rows.append([cols[0], cols[-1]])
      self.cur_row += 1
    elif cols[-1]:
      self.rows[self.cur_row][1] += cols[-1]
    else:
      raise SystemExit("Found strange number of columns in Table.new_row")
    #
  #

  ## Fill up a row with a value.
  # <tt>void extend_row(self, string line)</tt>
  #
  #  Finish a row that already has a head with the corresponding value.
  #  If the object is in waiting modus, an Exception is thrown
  def extend_row(self, line):
    if self.waiting:
      raise SystemExit("** Error: extend_row() was called while Table is in waiting modus! ")
    self.extend(["", line])
  #

  ## Finish the table, ie: print it.
  # <tt>void finish(self)</tt>
  #
  #  Determine the size of the second column and print LaTeX output
  #  corresponding to that size and to the value of <tt>self.border</tt>.
  def finish(self):
    if self.cur_row == -1: # no table is stored, so just return
      return
    #
    if self.waiting:
      print >> output_fh, "\\\\"
    #
    # calculate the maximum column-width of the second column:
    maxwidth = 0
    for r in self.rows:
      if len(r[1]) > maxwidth:
        maxwidth = len(r[1])
    #

    # We start with the standard 'll' format for the table
    # This needs to be changed:
    #  - when the second column contains too much text -> need lp{width}
    #  - when a border is needed -> add bars '|'
    tformat = "ll"
    if maxwidth > 40:  # the number is arbitrary; it's a poor man's job of detecting a paragraph of text
      if self.border:
        tformat = "|l|p{9cm}|"
      else:
        tformat = "lp{9cm}"
    elif self.border:
      tformat = "|l|l|"
    #

    # now we print out the table:
    # - LaTeX header
    print >> output_fh, "\n\\begin{tabular}{" + tformat + "}"
    if self.border:
      print >> output_fh, "\\hline"
    #
    # - Table header (adding two horizontal lines)
    print >> output_fh, self.rows[0][0], '&', self.rows[0][1],'\\\\'
    if self.border:
      print >> output_fh, "\\hline \\hline"
    #
    # - rest of the table (adding one horizontal line after each row)
    if self.cur_row > 0:
      for r in self.rows[1:]:
        print >> output_fh, r[0],'&',r[1],"\\\\"
    #
      if self.border:
        print >> output_fh, "\\hline"
    #
    # - finish LaTeX code for the table
    print >> output_fh, "\\end{tabular} \\\\[-2mm]"

    self.__init__() # reset for the next table
  #finish
# class Table

## Support class to create tables that span multiple pages
class LongTable(Table):
  ## Create a new object.
  # <tt>__init__(self, bool useBorder=False)</tt>
  #
  #  Create an empty table, with or without a border
  def __init__(self, useBorder=False):
    Table.__init__(self, useBorder)
  #
  ## Finish the table by printing it.
  # <tt>void finish(self, string table_header)</tt>
  #
  # In the visitdocs::LongTable class we use <tt>\\longtable</tt>
  # instead of the normal table layout
  def finish(self, table_header):
    if self.cur_row == -1:
      return

    # We start with the standard 'll' format for the table
    # This needs to be changed:
    #  - when the second column contains too much text -> need lp{width}
    #  - when a border is needed -> add bars '|'
    maxwidth = 0
    for r in self.rows:
      if len(r[1]) > maxwidth:
        maxwidth = len(r[1])
    #
    tformat = "ll"
    if maxwidth > 40:
      if self.border:
        tformat = "|l|p{7.5cm}|"
      else:
        tformat = "lp{7.5cm}"
    elif self.border:
      tformat = "|l|l|"
    #

    # now we print out the table:
    # - LaTeX header
    print >> output_fh, "\n\\begin{longtable}{" + tformat + "}"
    print >> output_fh, table_header

    # - rest of the table (adding one horizontal line after each row)
    for r in self.rows:
      print >> output_fh, r[0],'&',r[1],"\\\\"
      if self.border:
        print >> output_fh, "\\hline"
    #
    # - finish LaTeX code for the table
    print >> output_fh, "\\end{longtable}"

    self.__init__() # reset for the next table
#

## Create output for all functions in the given list and return undocumented functions.
# <tt>string[] print_file_list(string[] funclist)</tt>
#
#  Foreach function in <tt>funclist</tt>:
#    - try to extract inline documentation string <tt>func.__doc__</tt>
#    - if not available, add to 'undocumented' collection
#    - otherwise try to parse the string and create LaTeX output for
#      each
#
#  @return a list of functions that had no (inline) documentation associated
def print_file_list(funclist):
  undocumented = []

  for func in funclist:
    # extract documentation for this function:
    fulldoc = eval(func).__doc__

    # functions that are not documented, are appended to 'undocumented'
    if not fulldoc:
      undocumented.append(func)
      continue
    #

    # Otherwise: try to parse the string #
    fulldoc = fulldoc.splitlines()

    # documentation blocks that each function should have:
    blocks = {'Synopsis': False, 'Arguments': False, 'Returns': False, 'Description': False, 'Example':False}
    current_block = ''

    # The first line should hold the short description - do away with silly "s and -s and
    # print a header for the new function:
    short = fulldoc[1]
    if short[0]=='"':
      print >> sys.stderr, "Got \" for ",func
      short = short[1:]
    if short[-1]=='"':
      print >> sys.stderr, "Got \" for ",func
      short = short[:-2]
    #
    print >> output_fh, func_header(func, short[1:]) # don't print the ugly '-'

    # We will try to extract function arguments as well as we can, in order
    # to compare against the list given in the documentation:
    func_arguments = {}
    funcArg_matcher = re.compile(r'["[]?(\w+)[]"]?\s*(?:=\s*[^,)]+)?[,)]')

    # Set up a Table-object in case we come across a table
    table_collector = Table()

    found_no_lines = False # this flag will tell us whether we found any non-empty lines in a documentation block
    for line in fulldoc[2:]: # and here we go:
      if (not line or line.isspace()) and not (table_collector.is_building() and current_block == 'Description'):
        # skip empty lines, unless we're in the middle op parsing a table (in Description)
        # -> the end of the table is signaled by an empty line
        continue
      #

      # This flag will be set to True, when the start of a new block is found: #
      newblock = False

      # work-around for the 'AlterDatabaseCorrelation' function
      # This function _does_ have an example, but the 'Example'-keyword is _not_ there, so we
      # trigger on the first line of the example:
      if func == 'AlterDatabaseCorrelation' and line.find('dbs = ("/usr/gapps/visit/data/wave.visit",') == 0:
        table_collector.finish()
        if found_no_lines:
          print >> output_fh, "None"
        #
        finish_block(current_block, func_arguments, func)

        current_block = 'Example'
        blocks['Example'] = True

        start_block(current_block, table_collector, 'Example:')
        found_no_lines = True
      # AlterDatabaseCorrelation work-around

      # Proceed with the general case: try to match one of the keywords that
      # signify the start of a new block
      for k in blocks.keys():
        if line.find(k) == 0 and line.endswith(':'):
          table_collector.finish()
          if found_no_lines:
            print >> output_fh, "None"
          #
          finish_block(current_block, func_arguments, func)

          current_block = k
          blocks[k] = True

          start_block(current_block, table_collector, line)
          newblock = True
          found_no_lines = True
          break
      #

      # if we found a new piece, we're finished now with the line
      if newblock:
        continue

      # The current line is not the start of a new block
      # So process the line according to the current state of 'current_block' which
      # tells us which documentation block we are currently parsing

      # Reading a 'Synopsis' piece
      if current_block == 'Synopsis':
        # This block holds a single line with a formal function call
        found_no_lines = False

        # First we try to match the names of the arguments
        # They should recide between the '()' brackets
        arg_section = line[line.find('('):line.rfind(')')]
        args = funcArg_matcher.findall(arg_section)
        for a in args:
          if a and not a in func_arguments:
            func_arguments[a] = False
          #
        #

        # Next we print the line in verbatim environment; this gives rise
        # to problems when the line is really long, because LaTeX doesn't automatically
        # cut off verbatim lines at the edge of the paper. So we do it manually here:
        max_line = 65
        if len(line) > max_line:
          output_fh.write("\n\\verb!")

          counter = 0
          parts = line.split(',')
          for p in parts[:-1]:
            if (counter + len(p)) > max_line:
              print >> output_fh, "\\ ! \\\\ \n\\verb!        ",
              counter = 8
            #
            output_fh.write(p+",")
            counter += (len(p) + 1)
          #
          if (counter + len(parts[-1])) > max_line:
            print >> output_fh, "\\ ! \\\\ \n\\verb!   ",
          #
          print >> output_fh, parts[-1] + "!\\\\",
        else:
          print >> output_fh, "\n\\verb!" + line + "!\\\\",
        continue
      #

      # Reading an 'Arguments' piece:
      if current_block == 'Arguments':
        found_no_lines = False

        # need to be extra smart here, because PythonQuery and DefinePythonExpression
        # print the start of the description of an argument name on the same
        # line as the argument itself, while all other functions put the name
        # and the description on separate lines:
        if line.find('   ') == 0:
          # this only happens for the *Python* functions; it is the continuation of
          # the description of a parameter:
          for ic,c in enumerate(line):
            if not c.isspace():
              table_collector.extend_row(escape_latex(line[ic:]))
              break
          #
          continue
        #

        # back to normal business: check if one of the arguments is at
        # the beginning of the line, otherwise just print
        finished = False
        for k in func_arguments.keys():
          if line.find(k) == 0:
            # First hint that we found the name of some argument:
            # Let's make sure:
            if not table_collector.incomplete():
              # Yes! We must have found the argument itself, not
              # a description that happens to start with the name of the argument:
              func_arguments[k] = True

              # in the case of the *Python* functions, the description will start at the same line
              parts = line.split('  ')
              if len(parts) > 1:
                # *Python* case
                table_collector.extend([make_verb(parts[0]), escape_latex(parts[-1])])
              else:
                table_collector.new_head(make_verb(line))
              #
            else:
              # we found the continuation of a description
              table_collector.extend_row(escape_latex(line))
            #
            finished = True
            break
          #
        #
        if finished:
          continue
        #

        # Try a bit harder; in some cases the name in the table doesn't match the actual name
        # Try to find out anyway, such that we can emit a warning:
        if len(line.split()) <= 2 and not line.endswith('.'):
          table_collector.new_head(make_verb(line))
          continue
        #
        parts = line.split('  ')
        if len(parts) > 1 and not parts[0].endswith('.') and not table_collector.incomplete():
          table_collector.extend([make_verb(parts[0]), escape_latex(parts[-1])])
          continue
        #

        # Really not a single piece of evidence was found that something out of the
        # ordinary is going on - so add the line to the description of the current
        # argument
        table_collector.extend_row(escape_latex(line))
        continue
      #if 'Arguments'

      # Reading a 'Returns' piece
      if current_block == 'Returns':
        found_no_lines = False
        print >> output_fh, escape_latex(line),
        continue
      # if 'Returns'

      # Reading a 'Description' piece
      if current_block == 'Description':
        # An empty line signifies the end of a table:
        # Put the table in waiting modus; this allows us to do take special care
        # for what follows:
        #   - if a new table follows immediately this one, we will not create a new paragraph
        #   - otherwise we will
        if not line or line.isspace() and table_collector.is_building():
          table_collector.to_wait_modus()
          continue
        #

        if line and table_collector.is_waiting():
         table_collector.finish()
         # line following the table is a normal piece of text - create a new paragraph
         # otherwise the next if-block will detect the new table and start a new one
         if line.find('\t') == -1:
           print >> output_fh, "\\\\\n\n\\hangindent=\\parindent"
        #

        # we're not at the end of a table - maybe at the beginning or building one?
        if line.find('\t') > 0 or table_collector.is_building():
          cols = line.split('\t')
          nocols = sum([bool(c) for c in cols])

          if not (nocols == 2 or nocols == 1):
            # detect anything we haven't taken into account yet
            raise SystemExit("** Error: found an unsupported table format in " + func + " documentation:\n"+line)
          #
          if not table_collector.is_building():
            table_collector.create_new(True)
          #

          if len(cols) == 1 and len(line.split()) < 3 and not line.endswith('.'):
            table_collector.new_head(make_verb(line))
          elif nocols == 1:
            table_collector.extend_row(escape_latex(cols[-1]))
          else:
            table_collector.extend([make_verb(cols[0]), escape_latex(cols[-1])])

          continue
        # if table
        found_no_lines = False
        # nothing special: duplicate the line:
        print >> output_fh, escape_latex(line),
        continue
      # if 'Description'


      # Reading an 'Example' piece
      if current_block == 'Example':
        found_no_lines = False
        # copy all contents verbatim:
        print >> output_fh, line
        continue
      # if 'Example'
    # for line in fulldoc[2:]

    # parsed all lines in the __doc__ string -
    for k,ktest in blocks.iteritems():
      # Test for the presence of all blocks - In the special case
      # that the function has no arguments, it is allowed that Arguments is empty
      # Otherwise print a warning message:
      if k == "Arguments" and not len(func_arguments):
        ktest = True
      if not ktest:
        print >> sys.stderr, "** Warning: missing documentation block '" + k + "' for '" + func + "'"
    #
    finish_block(current_block, func_arguments, func)
    print >> output_fh, "\\newpage\n"
  # for func
  # finally return the accumulated list of undocumented functions:
  return undocumented
#


# MAIN SCRIPT #

# Define ALL variables we'll need:
# Do this before we import all the visit stuff,
# such that after we import visit, we can tell the difference
## @brief Store function names with missing documentation
missing_documentation = []
## @brief Store names of functions that are available before @c visit.Launch() is called
startup_funcs = []
## @brief Store functions that deal with Attributes
attrlist=[]
## @brief Store general functions
funclist=[]

import sys
## @brief Global filehandle to which all output is printed
output_fh = sys.stdout
if len(sys.argv) > 1:
  try:
    output_fh = open(sys.argv[1], 'w')
  except IOError,e:
    output_fh = sys.stdout
#


# DONT CREATE ANY NEW VARIABLES AFTER THIS POINT! #####
## @brief Store all identifiers in the global namespace @b before importing VisIt identifiers
system_funcs = dir()
system_funcs.extend(['system_funcs', 'func'])

# Everything that corresponds to the default Python stuff and this particular script
# is now in system_funcs - hence everything that is in dir() after importing
# from visit that is Not in system_funcs, must have come from visit
#
# We need to take this tricky road to circumvent bug #457: import visit doesn't load
# the Eval* functions in the visit-namespace
sys.path.append("/usr/gapps/visit/2.2.2/linux-x86_64/lib")
from visit import *
AddArgument("-nowin")
# Now extract functions that are available before @c Launch() is called and store them in startup_funcs
for func in dir():
  if func in system_funcs:
    continue
  startup_funcs.append(func)
#

# next Launch() VisIt in order to extract other functions:
Launch()
print >> sys.stderr, "**\n**  Running VisIt", Version(), "\n**"
for func in dir():
  if func in system_funcs or func in startup_funcs:
    continue

  # all functions that have 'Attributes' in their name and don't start with
  # 'Set' will be collected seperately:
  if func.find('Attributes') > -1 and func[:3] != 'Set':
    attrlist.append(func)
  else:
    funclist.append(func)
  #
#

# sort functions #
startup_funcs.sort()
funclist.sort()
attrlist.sort()

# start creating output #

# start with the latex header:
print >> output_fh, \
"""\\documentclass[11ptr]{article}
\\usepackage{longtable}
\\usepackage[colorlinks=true, linkcolor=blue]{hyperref}
\\begin{document}
\\pagenumbering{roman}
\\title{VisIt Python CLI function reference for VisIt """+Version() + """}
\\author{Generated from visitdoc.py}
\\maketitle

\\phantomsection
\\addcontentsline{toc}{section}{Contents}
\\renewcommand{\contentsname}{}
\\tableofcontents \\newpage

\\pagenumbering{arabic}"""

# first chapter of functions available before Launch()
print >> output_fh, "\\section{Functions available before Launch()}"
print >> output_fh, \
"""These are functions that are available after importing the visit module:
\\begin{verbatim}
from visit import *    # either this way
import visit           # or this way
                       # (Eval* functions will fail to load this way though)
\\end{verbatim}
"""

missing_documentation.extend(print_file_list(startup_funcs))

# proceeding to general functions #
print >> output_fh, "\\section{General functions}"
print >> output_fh, \
"""After setting up the correct options for the compute engine,
one issues the \\verb!Launch()! command:
\\begin{verbatim}
Launch()               # after 'from visit import *'
visit.Launch()         # after 'import visit'
\\end{verbatim}
After launching the compute engine, all functions in this and the following
sections become available.\\\\[5mm]
"""

missing_documentation.extend(print_file_list(funclist))

# Proceeding to a chapter with all Attribute functions #
print >> output_fh, "\\section{Attribute functions}"
missing_documentation.extend(print_file_list(attrlist))

missing_documentation.sort()

# We finish the function listing with functions that had no documentation at all
print >> output_fh, "\\section{Undocumented functions}"
for func in missing_documentation:
  print >> output_fh, undoc_header(func)
#

# We add a listing of attributes:
## Compiled regex to detect all function names that have the string '<i>Attributes</i>' in it
attr_matcher = re.compile(r'(Get)?(.*)Attributes$')
## Store all the attribute names with the functions that will return them
attr_names = {}
for func in attrlist:
  ## Store the matched attribute set
  aname = attr_matcher.match(func)
  attr_names[aname.group(2)] = func
#

# sort them and create documentation
print >> output_fh, "\\newpage\n\\section{Attribute listings}"
print >> output_fh,\
"""These sections show all the attributes that can be set to
control the behaviour of VisIt. The attributes themselves are
not documented, but their names are usually quite explanatory. When
an attribute can have values from a given list of options, the default
option is printed and the other available options are printed
in a column beneath in italic.

\\noindent The listing is ordered in alphabetical ordering of the
name of the attribute set. For each set the function that will provide
you with these attributes is printed in italic.\\\\[5mm]
"""

## visitdocs::LongTable collector
ltable_collector = LongTable()
## Compiled regex to detect lines of the form 'value = something'
attr_finder = re.compile(r'([^=]+)=(.*)')
## Compiled regex to detect lines of the form 'value = default # alternative1, alternative2, ...'
value_finder = re.compile(r'([^#]+)#(.+)')
for attr in sorted(attr_names.keys()):
  if attr in ('ResetPick', 'ResetQueryOverTime'):
    continue

  ltable_collector.create_new(False)
  ## Store the full listing for a attribute set by calling the stored function
  alist = str(eval(attr_names[attr])()).splitlines()
  if not alist:
    print >> sys.stderr, "** Warning: function "+attr_names[attr]+" does not return any attributes"

  for line in alist:

    ## Store the result of trying to match with visitdocs::attr_finder
    attr_match = attr_finder.match(line)
    ## Store the value for this attribute, initially the full line, later the part that stores the value plus possible list of values that is behind a '#'
    value_list = line
    if attr_match:
      ltable_collector.new_head(escape_latex(attr_match.group(1)))
      value_list = attr_match.group(2)
    else: # this only happens for ContourAttributes and VolumeAttributes and the following code will suffice for those:
      ltable_collector.add_row(["", "{\\it "+escape_latex(line[1:])+"}"])
      continue
    #

    # Some cases are of the form 'entry = default # other1, other2, ...'
    ## Store part of the line behind a '#' that would store a list of allowed options, found by visitdocs::value_finder
    list_match = value_finder.match(value_list)
    if list_match:
      ## Store the actual list of attribute values after splitting the part behind the '#' on the commas
      opt_list = list_match.group(2).split(',')
      ltable_collector.extend(["", escape_latex(list_match.group(1))])
      for o in opt_list:
        ltable_collector.add_row(["", "{\\it "+escape_latex(o)+"}"])
    else:
      ltable_collector.extend(["", escape_latex(value_list)])
  #

  # print the table:
  print >> output_fh, attr_header(attr, attr_names[attr])
  ## Store the (formatted) string used as header for the longtable for the current attribute name
  attribute_table_header = \
"""{\\bf Attribute} & {\\bf Default/Allowed values} \\\\
\\hline \\hline
\\endfirsthead
\\multicolumn{2}{l}{{\\it ... """+attr+""" attributes continued}} \\\\
{\\bf Attribute} & {\\bf Default/Allowed values} \\\\
\\hline \\hline
\\endhead
\\hline
\\multicolumn{2}{l}{{\\it Continued on next page...}} \\\\
\\endfoot
\\hline
\\endlastfoot
"""
  ltable_collector.finish(attribute_table_header)

  print >> output_fh, "\n\\newpage"
#

# Then follows the list of Events:
print >> output_fh, "\\newpage\n\\section{VisIt CLI Events}"
print >> output_fh,\
"""These section shows a table with all events that the VisIt GUI could potentially
generate. Different plugins create different events, so the list
will depend on the user configuration. The list in this section is generated
from a call to the {\\it GetCallbackNames()} function and will therefore list
just the events that are applicable to the user that generates this documentation.

The list is alphabetically ordered. The left column, labeled {\\it EventName}
displays each event or callback name. The right column, labeled {\\it ArgCount}
displays the result of calling {\\it GetCallbackArgumentCount(EventName) }
for the corresponding event, which returns the number of arguments a callback function
for that event should accept.
"""
## Store a list of event names in the VisIt CLI
event_names = GetCallbackNames()
ltable_collector.create_new(False)
for ev in sorted(event_names):
  ltable_collector.add_row([ev, str(GetCallbackArgumentCount(ev))])
#
## Store the (formatted) string used as header for the longtable for the Event listing
event_table_header = \
"""{\\bf EventName} & {\\it ArgCount } \\\\
\\hline \\hline
\\endfirsthead
\\multicolumn{2}{l}{{\\it ... CLI Events continued }} \\\\
{\\bf EventName} & {\\it ArgCount } \\\\
\\hline \\hline
\\endhead
\\multicolumn{2}{l}{{\\it Continued on next page...}} \\\\
\\endfoot
\\hline
\\endlastfoot
"""
ltable_collector.finish(event_table_header)

print >> output_fh, """\\end{document}"""

# and we're finished! #
