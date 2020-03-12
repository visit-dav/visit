#!/usr/local/apps/bin/python
import sys, os 

#
# Get the output of a command as a tuple of strings.
#
def GetFilenames(command):
    p = os.popen(command)
    files = p.readlines()
    p.close()
    return files

#
# Execute a command. It's wrapped for debugging.
#
def systemCommand(command):
    print(command)
    return os.system(command)

###############################################################################
#
# Purpose: This program regenerates all of the java state objects in this 
#          directory by first finding the XML files and then calling xml2java.
#
# Programmer: Kathleen Biagas 
# Date:       June 26, 2013
#
# Modifications:
#
###############################################################################

def main(exeDir):
    # Get the names of the XML files in the common/state directory.
    files = GetFilenames("ls ../common/state/*.xml")

    # regenerate the java files
    for f in files:
        command = "%s/xml2java -clobber %s" % (exeDir, f)
        systemCommand(command)

    # Get the names of the XML files in the avt/DBAtts/MetaData directory.
    files = GetFilenames("ls ../avt/DBAtts/MetaData/*.xml")

    # regenerate the java files
    for f in files:
        command = "%s/xml2java -clobber %s" % (exeDir, f)
        systemCommand(command)

    # Get the names of the XML files in the viewer/rpc directory.
    files = GetFilenames("ls ../viewer/rpc/*.xml")

    # regenerate the java files
    for f in files:
        command = "%s/xml2java -clobber %s" % (exeDir, f)
        systemCommand(command)

    return 0

#
# Call the main function
#
if len(sys.argv) > 1:
    main(sys.argv[1])
else:
    print("usage: regeneratejava.py /path/to/xml2java")

