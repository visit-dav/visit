###############################################################################
# Program: build_visitmodule.py
#
# Purpose: This program builds the VisIt Python module.
#
# Programmer: Brad Whitlock
# Creation: Wed Nov 22 12:41:39 PDT 2006
#
# Notes: This is the "visit" module and it allows us to connect to VisIt
#        and control it by loading a user-settable "visitmodule" shared
#        library under the covers. This lets one small "visit" module
#        connect to many versions of VisIt and allows the module to include
#        their interfaces in its own.
#
# Modifications:
#   Brad Whitlock, Wed Mar 14 12:19:32 PDT 2007
#   Changed path to version file.
#
###############################################################################
from distutils.core import setup, Extension
import string

# Determine the version
f = open("../../VERSION")
lines = f.readlines()
f.close()
ver = string.replace(lines[0], "\n", "")


visitmod = Extension("visit",
    include_dirs=['.'],
    sources = ["visitfrontend.c"])

setup(name = "visit",
      version = ver,
      description = "This module contains functions to control VisIt.",
      ext_modules = [visitmod])

