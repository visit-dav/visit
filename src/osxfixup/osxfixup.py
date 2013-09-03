import os
import sys
import subprocess

if len(sys.argv) < 2:
    print "arguments are osxfixup.py <dir-name> [rpath] [verbose]"
    pass

verbose = int(sys.argv[3]) if len(sys.argv) > 3 else 0

#if new path is blank it will apply to full path..

# new_path is the prefix to the new path. For example: @executable_path/../
new_path = sys.argv[2] if len(sys.argv) > 2 else ""

print verbose, new_path
prefix_path = sys.argv[1] #should normally be the darwin-x86_64 directory, but not necessary

if not os.path.exists(sys.argv[1]):
    print sys.argv[1],"does not exist"
    pass

prefix_path = os.path.abspath(prefix_path) 
prefix_bin_path = os.path.join(prefix_path, "bin") 


def log(verbosity, message):
    if verbosity <= verbose: print message

#####################################################

# Generate library map and convert ids of libraries

#####################################################

def generate_libs(libnames, libmaps):
    global new_path

    #add frameworks..
    frameworks = subprocess.check_output("find {} -name \"*.framework\"".format(prefix_path), shell=True)
    frameworks = frameworks.split("\n")

    #map dylib and so ..
    #assume framework library is right after the path..
    for i,fwork in enumerate(frameworks):
        if len(fwork) == 0: continue
        fwork_basename = os.path.splitext(os.path.basename(fwork))[0]

        fwork = os.path.join(fwork, fwork_basename)
        frameworks[i] = fwork

        if len(new_path) == 0:
            libmaps[fwork_basename] = frameworks[i]
            continue

        fwork = fwork.replace(prefix_path,"")

        if fwork_basename in libmaps:
            print "warning: ", fwork_basename,"has multiple old:",libmaps[fwork_basename],"new:", fwork

        libmaps[fwork_basename] = fwork
        

    libs = subprocess.check_output("find {} -name \"*.dylib*\" -or -name \"*.so*\"".format(prefix_path), shell=True)
    libs = libs.split("\n")

    for i,lib in enumerate(libs) :
        if len(lib) == 0: continue

        bname = os.path.basename(lib)
        if len(new_path) == 0:
            libmaps[bname] = libs[i]
            continue

        lib = lib.replace(prefix_path,"")

        #create map...
        if bname in libmaps:
            print "warning: ", bname,"has multiple old:",libmaps[bname],"new:",lib

        libmaps[bname] = lib

    libnames.extend(frameworks)
    libnames.extend(libs)

####################################

# fixup libraries to new location

####################################

def fixup(libnames, libmaps):
    global new_path

    for lib in libnames :
        if len(lib) == 0: continue

        bname = os.path.basename(lib)
        log(1,bname)

        #only change ids if it is not a symlinked file..
        idcmd = ""
        idcmd2 = ""

        #evaluating symlinks causes add_rpath errors for duplication
        #not evaluating it skips frameworks..
        #for now I am okay with duplication error
        #if not os.path.islink(lib):
        if len(new_path) == 0:
            idcmd  = "install_name_tool -id {0} {1}".format(lib, lib)
        elif bname in libmaps:
            idcmd  = "install_name_tool -id @rpath/{0} {1}".format(libmaps[bname], lib)
            idcmd2  = "install_name_tool -add_rpath {0} {1} 2>&1".format(new_path, lib)
        else:
            idcmd = "install_name_tool -id @rpath/{0} {1}".format(lib.replace(prefix_path,""), lib)    
            idcmd2  = "install_name_tool -add_rpath {0} {1} 2>&1".format(new_path, lib)

        log(2,idcmd)
        os.system(idcmd)

        if len(idcmd2) > 0:
            log(2,idcmd2)
            os.system(idcmd2)

        dependencies = subprocess.check_output("otool -L {0}".format(lib), shell=True)
        dependencies = dependencies.split("\n")
    
        for dep in dependencies[1:]:
            if len(dep) == 0: continue

            dep = dep.strip()
            index = dep.find(" ")
            if index >= 0: dep = dep[0:index].strip()

            depname = os.path.basename(dep)

            #sometimes dependencies can have extensions, find the appropriate one..
            if depname not in libmaps:
                tdep = os.path.splitext(depname)
                while tdep[1] != '':
                    if tdep[0] in libmaps:
                        depname = tdep[0]
                        break
                    tdep = os.path.splitext(tdep[0])
            if depname in libmaps:
                log(1, "\t" + depname)
                depcmd = "install_name_tool "
                if len(new_path) == 0:
                    depcmd += "-change {0} {1} {2}".format(dep,libmaps[depname],lib)
                else:
                    depcmd += "-change {0} @rpath/{1} {2}".format(dep,libmaps[depname],lib)
                log(2, "\t" + depcmd)
                os.system(depcmd)
    
##########################################################

# convert executable (for now convert the bin directory)

##########################################################

def generate_exelist(exelist):
    if not os.path.exists(prefix_bin_path):
        return

    execnames = subprocess.check_output("find {} -perm -u+x ! -type d -exec file {{}} \; | grep executable | grep -v text".format(prefix_bin_path), shell=True)
    execnames = execnames.split("\n")

    for exe in execnames:
        exe = exe[0:exe.find(":")]
        exelist.append(exe)

libnames = []
libmaps = {}
exelist = []

print "Generating Library Names..."
generate_libs(libnames, libmaps)

print "Collecting executable list..."
generate_exelist(exelist)

print "Fixing Libraries..."
fixup(libnames, libmaps)

print "Fixing Executables..."
fixup(exelist, libmaps)
