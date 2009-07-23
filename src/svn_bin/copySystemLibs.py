#!/usr/bin/python

# This script can be called on a final VisIt install to make it run
# on Compute Node Linux (CNL). CNL supports dynamic libraries, but
# these must reside on a Lustre file system to be accessible. To run
# VisIt on compute nodes, it needs to be installed on a Lustre file
# system. This script examines the executable via ldd and copies all
# system libraties used by VisIt and its plugins to a subdirectory
# "system_libs" in the VisIt directory tree, making them accesible to
# compute nodes. 
#
# NOTE:
# - Currently, the architecture (linux-x86_64) is harcoded in the
#   script.
# - The prefix of the Lustre directory is hardcoded as well.
#
# To run this script, change "lustrePrefix" to the mount point of
# the Lustre file system that contains the VisIt install and run
# copySystemLibs.py <root of VisIt install>.

import os
import sys
import glob

# The following functions returns true if cont (file or directory) is
# contained in dir.
def dirContains(dir,cont):
  return os.path.commonprefix([os.path.abspath(dir), os.path.abspath(cont)]) == os.path.abspath(dir)

# ---- Main ----

# Check number of arguments
if len(sys.argv) != 2:
  sys.exit("Usage: %s <visitDir>" % sys.argv[0])

# Locations
visitDir = sys.argv[1]
visitExecDir = visitDir + "/current/linux-x86_64"
execName = visitExecDir + "/bin/engine_par"
pluginNameList = glob.glob(visitExecDir + "/plugins/*/*.so")
# Change this to the Lustre mount point on you machine
lustrePrefix = "/scratch2"

# Check if the VisIt parallel engine executable exists
if not os.access(execName, os.X_OK):
  sys.exit("Invalid VisIt executable %s!" % execName)

# We need to start checking the parall engine and all plugins
libList = [ execName ] + pluginNameList

# Examine dependencies
for lib in libList:
   print "Examining dependencies for " + lib
   lddOut = os.popen("env LD_LIBRARY_PATH=%s ldd %s" % (os.environ["LD_LIBRARY_PATH"]+":"+visitExecDir+"/lib", lib))
   for line in lddOut.readlines():
     if line.strip() != "statically linked":
       if len(line.strip().split()) == 4:
         (libname, sep, libpath, addr) = line.strip().split()
         if sep != "=>":
            sys.exit("Error: Unexpected separator %s!" % sep)
       else:
         (libpath, addr) = line.strip().split()
       if libpath != "not":
         if not os.access(libpath, os.R_OK):
            sys.exit("Error: File \"%s\" is not readable!" % libpath)
         else:
            if not libpath in libList:
                print "Adding " + libpath + " to library list."
                libList.append(libpath)
       else:
         print "Couldn't parse library location in ldd output!"
         print "Offending line: %s" % (line.strip())
         sys.exit()

# Create direcotory for system libraties and copy all
# libraries that are not on Lustre into it. (Note: Copying
# only libraries not on Lustre will automatically eliminate
# the VisIt executable and all VisIt plugins from the list
# since the VisIt install needs to be on Lustre)
os.system("mkdir -p " + visitExecDir + "/system_libs")
for lib in libList:
  if not dirContains(lustrePrefix, lib):
    print "Copying lib " + lib
    os.system("cp " + lib + " " + visitExecDir + "/system_libs/")
  else:
    print "Skipping lib " + lib

# Copy env command
print "Copying env command"
os.system("cp /usr/bin/env " + visitExecDir + "/bin/")

# Change permissions
print "Changing permissions"
os.system("chmod -R a+rX " + visitExecDir + "/system_libs/")
os.system("chmod -R a+rX " + visitExecDir + "/bin/env")
# The following is franklin.nersc.gov specific. Make VisIt  install
# also writable by USG
os.system("chmod -R g+w " + visitExecDir + "/system_libs/")
os.system("chmod -R g+w " + visitExecDir + "/bin/env")
os.system("chgrp -R usg " + visitExecDir + "/system_libs/")
os.system("chgrp -R usg " + visitExecDir + "/bin/env")
