#!/usr/local/apps/bin/python
import os, string, sys

#
# Get the output of a command as a tuple of strings.
#
def GetFilenames(command):
    p = os.popen(command)
    files = p.readlines()
    p.close()
    return files

#
# Get a list of all files that have a .code extension and contain "CreateNode".
# 
def GetCreateNodeFiles():
    files = GetFilenames("grep -i CreateNode *.code")
    retval = []
    for f in files:
        index = string.find(f, ':')
        if(index != -1):
            astr = f[:index]
            if(astr not in retval):
                retval = retval + [astr]
    return retval

#
# Execute a command. It's wrapped for debugging.
#
def systemCommand(command):
    print(command)
    return os.system(command)

###############################################################################
#
# Purpose: This program regenerates all of the state objects in this directory
#          by first checking out the XML files and then calling xml2atts.
#
# Programmer: Brad Whitlock
# Date:       Tue May 20 10:00:56 PDT 2003
#
# Modifications:
#   Brad Whitlock, Fri Dec 14 15:56:16 PST 2007
#   Removed cleartool commands.
#
#   Kathleen Biagas, Tue Apr 23 11:54:25 PDT 2019
#   Made main accept a path to xml2atts.
#
###############################################################################

def main(exeDir):
    # Get the names of the XML files in this directory.
    files = GetFilenames("ls *.xml")

    # Check out each XML file and regenerate the C and h files.
    for f in files:
        bname = f[:-5]
        command = "%s/xml2atts -clobber %s" % (exeDir, f)
        systemCommand(command)

    # Warn about files that have special CreateNode functions.
    cnfiles = GetCreateNodeFiles()
    if(len(cnfiles) > 0):
        print("***")
        print("*** The following code files have special CreateNode methods.")
        print("*** These .code files may need to be updated:")
        print("***")
        for f in cnfiles:
            print(f)

    return 0

#
# Call the main function
#
if len(sys.argv) > 1:
    main(sys.argv[1])
else:
    print("usage: regenerateall.py /path/to/xml2atts")

