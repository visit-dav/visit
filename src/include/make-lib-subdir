# This is the makefile for a subdirectory of a library which is built
# from source files split across multiple subdirectories
#
# Modified:
#    Jeremy Meredith, Wed Jul  3 14:05:25 PDT 2002
#    Made it use braces for the PWD environment variable, helping portability.
#

DIR=`basename ${PWD}`

all:
	(cd .. && $(MAKE) $(DIR)_obj)
	(cd .. && $(MAKE) all)
