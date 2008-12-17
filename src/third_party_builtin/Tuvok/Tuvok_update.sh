#!/bin/bash

#        Author: Josh Stratton
#          Date: Mon Dec 15 10:35:52 MST 2008
#           Use: Updates VisIt with the latest version of Tuvok not
#                including Tuvok's .svn directories
#          Note: This script isn't intended for incorporation into
#                the VisIt build process.  
# Modifications: 
#

svn export --force https://code.sci.utah.edu/svn/Tuvok Tuvok

