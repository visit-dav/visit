#!/usr/bin/python

###############################################################################
# Purpose:
#    This script is part of the coding to automatically determine segment
#    offsets for dynamic libraries on MacOS X so we can have prebound
#    libraries so VisIt launches faster.
#
# Notes:
#    When this program is called with the -readlines argument, we print the
#    segment offset. Otherwise, we read in the link edit file and compute the
#    next segment offset.
#
# Programmer: Brad Whitlock
# Date: Wed Apr 21 17:22:24 PST 2004
#
# Modifications:
#
###############################################################################

import os, sys, string

def ReadLines(fileName):
    f = open(fileName, "r")
    lines = f.readlines()
    f.close()
    return lines

def StringToHex(s):
    retval = 0
    e = 1
    for i in range(len(s)):
        pos = len(s) - i - 1
        c = s[pos]
        val = 0
        if(c >= '0' and c <= '9'):
            val = int(c)
        elif c == 'a' or c == 'A':
            val = 10
        elif c == 'b' or c == 'B':
            val = 11
        elif c == 'c' or c == 'C':
            val = 12
        elif c == 'd' or c == 'D':
            val = 13
        elif c == 'e' or c == 'E':
            val = 14
        elif c == 'f' or c == 'F':
            val = 15
        retval = retval + (val * e)
        e = e * 16
    return retval

#
# Writes a library's starting address to a log file.
#
def AppendLog(logFile, s):
    try:
        f = open(logFile, "a")
	f.write("%s\n" % s)
	f.close()
    except:
        return

#
# Return whether or not the file exists.
#
def FileExists(filename):
    retval = 1
    try:
        # Try and stat the file. If it does not exist then OSError is raised.
        a = os.stat(filename)
    except OSError:
        retval = 0
    return retval

#
# Return whether a library is prebound.
#
def LibraryIsPreBound(libFile):
    retval = 1
    try:
        # Read the output of otool to determine if the library is prebound
        f = os.popen("otool -hv %s" % libFile)
        lines = f.readlines()
        f.close()

        # Add all of the lines together
        wholeLine = ""
        for line in lines:
            wholeLine = wholeLine + line

        # Now that we have the entire string, see if it contains the 
        # word PREBOUND. All prebound libraries should contain that word.
        retval = string.find(wholeLine, "PREBOUND") != -1
    except:
        retval = 0
    return retval	

#
# Attempts to rename the backup file back to the addressFile, thus restoring
# it. If the file cannot be restored, return 0.
#
def RestoreBackup(backup, addressFile):
    retval = 0
    if FileExists(backup):
        try:
	    os.rename(backup, addressFile)
	    retval = 1
        except:
	    AppendLog(logFile, "Could not restore %s" % addressFile)
    return retval

#
# Constants
#
PROCESS_ADDRESS = 0
PRINT_ADDRESS = 1

#
# Default values
#
mode = PROCESS_ADDRESS
libFile = ""
addressFile = "address.txt"
logFile = "linkedit.log"

#
# Process the command line arguments
#
i = 1
ARGVLEN = len(sys.argv)
while i < ARGVLEN:
    arg = sys.argv[i]
    if (arg == "-input") and (i+1 < ARGVLEN):
        addressFile = sys.argv[i+1]
	i = i + 2
    elif (arg == "-lib") and (i+1 < ARGVLEN):
        libFile = sys.argv[i+1]
	i = i + 2
    elif (arg == "-log") and (i+1 < ARGVLEN):
        logFile = sys.argv[i+1]
	i = i + 2
    elif (arg == "-readstart"):
        mode = PRINT_ADDRESS
	i = i + 1
    else:
        i = i + 1

backup = "%s.bak" % addressFile

#
# Read the lines out of the address file.
#
lines = ["0x00000000"]
try:
    lines = ReadLines(addressFile)
except IOError:
    lines = ["0x00000000"]
except OSError:
    lines = ["0x00000000"]

#
# Do the work that is appropriate for the script's mode.
#
if mode == PRINT_ADDRESS:
    #print "readlines!"
    if(len(lines) > 0):
        print lines[-1]
    else:
        print "0x00000000"

    # Rename the address file to a backup so if the build fails then
    # we can restore it for next time.
    try:
        os.rename(addressFile, backup)
    except:
        AppendLog(logFile, "Problem renaming %s" % addressFile)
else:
    if FileExists(libFile):
        if not LibraryIsPreBound(libFile):
	    print "***"
   	    print "*** %s is not prebound!" % libFile
	    print "***"
	    AppendLog(logFile, "*** %s is not prebound!" % libFile)
	    # Since the library was not prebound, try and restore the 
	    # address file so we can use the same offset for the next
	    # library (or subsequent attempts to build the failed library)
	    # If we can't restore the backup file then write a new
	    # address file.
            if RestoreBackup(backup, addressFile):
	        sys.exit(0)

        #print "nextstart"
        # The file should now contain __LINKEDIT   addr  size
        addr = 0
        size = 0
        newaddr = 0
        if(len(lines) > 0):
            tokens = string.split(lines[-1], " ")
            realtokens = []
            for t in tokens:
                if(t != ''):
                    newT = t
                    if(newT[-1] == '\n'):
                        newT = newT[:-1]
                    realtokens = realtokens + [newT]
            #print realtokens
            if(len(realtokens) == 3 and realtokens[0] == "__LINKEDIT"):
                addr = StringToHex(realtokens[1][2:])
                size = StringToHex(realtokens[2][2:])
                #print "addr = 0x%08x  size = 0x%08x" % (addr, size)
                newaddr = addr + size
        # Align the new address to 0x1000 segment boundaries
        newaddr = newaddr + (0x1000 - newaddr % 0x1000)	    
        # Write out the new file so it only has the address of the library in it.
        f = open(addressFile, "w")
        f.write("0x%08x" % newaddr)
        f.close()
        # print "The next starting address is: %08x." % newaddr
        # Write the starting address of the library to the log file.
        if(libFile != ""):
            AppendLog(logFile, "%s   0x%08x" % (libFile, addr))
    else:
        print "***"
	print "*** Not updating the prebinding segment offset because "
	print "*** %s " % libFile
	print "*** does not exist! It must not have built."
	print "***"
	RestoreBackup(backup, addressFile)
	    
sys.exit(0)




