# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

###############################################################################
# file: convert2to3.py
# Purpose: Python helper function for converting a python 2 script string
#          to be python 3 compatible using lib2to3
#
#
# Programmer: Cyrus Harrison
# Creation: Tue Jul 21 16:53:00 PDT 2020
#
#
# Modifications:
#
#
###############################################################################

from lib2to3 import refactor

# this class holds static state
class AutoPy2to3Flag:
    enabled = False


def SetAutoPy2to3(val):
    AutoPy2to3Flag.enabled = val

def GetAutoPy2to3():
    return AutoPy2to3Flag.enabled

def ConvertPy2to3(py_script_text):
    """
    Converts contents of input string py_script_text to python 3 using lib2to3
    and returns the result as a string.
    """
    # As far as I can tell, lib2to3 isn't documented that well but once
    # you find the right recipe it's very easy to use for this case.
    # ref: 
    # https://stackoverflow.com/questions/30340151/using-2to3-on-in-memory-scripts
    fixes = refactor.get_fixers_from_package('lib2to3.fixes')
    converter = refactor.RefactoringTool(fixes)
    ast = converter.refactor_string(py_script_text, '<script>')
    return str(ast)
