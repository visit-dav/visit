#!/usr/local/apps/bin/python
import os, sys, string

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
    files = GetFilenames("grep -i CreateNode */*.code")
    retval = []
    for f in files:
        index = f.find(':')
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
#   Added optional tool argument, xml2atts is default if not provided.
#   Fix find command for python 3. Support running on Windows via bash.
#
###############################################################################

def main(exeDir, tool):
    # Get the names of the XML files in this directory.
    files = GetFilenames("ls */*.xml")

    # Check out each XML file and regenerate the C and h files.
    for f in files:
        index = f.find("/")
        dirName = ""
        if(index == -1):
            continue
        else:
            dirName = f[:index]
            os.chdir(dirName)

        bname = f[index+1:-5]
        if  sys.platform.startswith("win"):
            command = "%s/visit.exe -%s -clobber %s.xml" % (exeDir,tool,bname)
        else: 
            command = "%s/%s -clobber %s.xml" % (exeDir,tool,bname)
        systemCommand(command)

        # Go back up
        os.chdir("..")

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
if len(sys.argv) > 2:
    main(sys.argv[1], sys.argv[2])
elif len(sys.argv) > 1:
    main(sys.argv[1], "xml2atts")
else:
    print("usage: regenerateatts.py /path/to/xml2atts <tool>")

