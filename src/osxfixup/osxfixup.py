# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.


import os
import sys
import stat
import subprocess
import fnmatch

from os.path import join as pjoin

def find_matches(sdir,pattern):
    """
    Walk sdir and find all files and dirs the match given pattern.
    """
    matches = []
    for root, dirs, files in os.walk(sdir):
        all = []
        all.extend(dirs)
        all.extend(files)
        for item in fnmatch.filter(all, pattern):
            matches.append(os.path.join(root, item))
    return matches

def real_lib(lib):
    """
    Returns the proper dylib name, needed to handle frameworks.
    """
    if lib.endswith(".framework"):
        lbase = os.path.split(lib)[1]
        lbase = lbase[:lbase.find(".framework")]
        lib = pjoin(lib,lbase)
    return lib    


def find_libs(sdir):
    """
    Find all libs we need to modify.
    """
    lib_names = []
    lib_maps  = {}
    libs = []
    libs.extend(find_matches(sdir,"*.framework"))
    libs.extend(find_matches(sdir,"*.dylib"))
    libs.extend(find_matches(sdir,"*.so"))
    for lib in libs:
        full_lib = real_lib(lib)
        if os.path.isfile(os.path.realpath(full_lib)):
            lib_basename = os.path.basename(full_lib)
            lib_names.append(full_lib)
            #if lib_basename in lib_maps:
            #    print "warning: ", lib_basename,"has multiple mappings old:",
            #    print lib_maps[lib_basename],"new:", lib
            lib_maps[lib_basename] = full_lib.replace(sdir,"")   
    return lib_names,lib_maps

def find_bundles(sdir):
    """
    Walk sdir and find all OSX bundles.
    """
    bundles = find_matches(sdir,"*.app")
    return bundles

def fixup_bundles(bundles):
    """
    Apply OSX bundle specific fixes.
    """ 
    add_qt_conf(bundles)

def add_qt_conf(bundles):
    """
    Adds a qt.conf config file to all bundles to avoid using any bad built-in plugin 
    paths. 
    """
    for bundle in bundles:
        resources_dir = pjoin(bundle,"Contents","Resources")
        if not os.path.isdir(resources_dir):
            os.mkdir(resources_dir)
        qtconf_fname = pjoin(resources_dir,"qt.conf")
        print("[creating: %s]" % qtconf_fname)
        qtconf = open(qtconf_fname,"w")
        qtconf.write("[Paths]\nPlugins=\n")

def find_exes(sdir):
    """
    Walk sdir and find all exes.
    """
    exes = []
    exe_flags = stat.S_IEXEC | stat.S_IXGRP | stat.S_IXOTH
    for root, dirs, files in os.walk(sdir):
        for fname in  files:
          fname = pjoin(root,fname)
          if not os.path.exists(fname):
              print('WARNING: not found %s -- skipped.'%(fname))
              continue
          st = os.stat(fname)
          mode = st.st_mode
          if mode & exe_flags:
              # check with otool that this is an exe and not a script
              chk_cmd = "file {0}"
              try:
                  chk = subprocess.check_output(chk_cmd.format(fname), shell=True)
                  if chk.count("executable") >= 1:
                      exes.append(fname)
              except:
                    print("[warning: failed to obtain file type for '%s']" % fname)
    return exes

def fixup_items(items,lib_maps,prefix_path):
    """
    Fixup libs / exes with the proper paths.
    """
    exe_rpaths = ["@executable_path/..", # standard exe
                  "@executable_path/../../../.." # bundle
                  ]
    for item in items:
        item_base = os.path.basename(item)
       
        #evaluating symlinks causes add_rpath errors for duplication
        #not evaluating it skips frameworks..
        #for now I am okay with duplication error
        #if os.path.islink(lib):             continue
        
        id_cmd = "install_name_tool -id @rpath{0} {1}"
        if item_base in list(lib_maps.keys()):
            id_cmd  = id_cmd.format(lib_maps[item_base], item)
        else:
            id_cmd  =  id_cmd.format(item.replace(prefix_path,""), item) 
        subprocess.call(id_cmd,shell=True)

        # Remove rpaths containing user home directory 
        load_cmd = "otool -l {0}"
        home = os.path.expanduser("~")
        invalid_paths = []
        try:
            lc_rpaths = subprocess.check_output(load_cmd.format(item), shell=True)
            lc_rpaths = [ path for path in lc_rpaths.split("\n")[1:] if path.find(" path") != -1]
            for lc_rpath in lc_rpaths:
                invalid_path = lc_rpath.split()[1]
                if invalid_path.find(home) != -1:
                    invalid_paths.append(invalid_path)
        except:
            print("[info: no invalid LC_RPATHS for '%s']" % item)

        del_rpath_cmd = "install_name_tool -delete_rpath {0} {1} 2>&1"
        for invalid_path in invalid_paths:
            subprocess.call(del_rpath_cmd.format(invalid_path, item), shell=True)

        # Add rpaths relative to the bundle
        deps_cmd = "otool -L {0}"
        try:
            dependencies = subprocess.check_output(deps_cmd.format(item), shell=True)
            dependencies = [ d for d in dependencies.split("\n")[1:] if d.strip() != ""]
        except:
            print("[warning: failed to obtain dependencies for '%s']" % item)
            dependencies = []
        
        # if we have an exe exe_rpaths[0]
        # if we have a bundle exe_rpaths[1]
        # it doens't hurt to add both for now
        rpath_base_cmd =  "install_name_tool -add_rpath {0} {1} 2>&1"
        rpath_cmds = [rpath_base_cmd.format(rp, item) for rp in exe_rpaths]
        for rp_cmd in rpath_cmds:
            subprocess.call(rp_cmd,shell=True)
        
        for dep in dependencies:
            index = dep.find(" ")
            if index >= 0: dep = dep[0:index].strip()
            dep_base = os.path.basename(dep)

            #sometimes dependencies can have extensions, find the appropriate one..
            if dep_base not in lib_maps:
                tdep = os.path.splitext(dep_base)
                while tdep[1] != '':
                    if tdep[0] in lib_maps:
                        dep_base = tdep[0]
                        break
                    tdep = os.path.splitext(tdep[0])
            if dep_base in lib_maps:
                dep_cmd = "install_name_tool -change {0} @rpath{1} {2}"
                dep_cmd = dep_cmd.format(dep,lib_maps[dep_base],item)
                subprocess.call(dep_cmd,shell=True)

def main():
    prefix_path = "darwin-x86_64"
    if len(sys.argv) > 1:
        prefix_path = sys.argv[1]
    prefix_path = os.path.abspath(prefix_path)
    print("[Finding libraries @ %s]" % prefix_path)
    lib_names,lib_maps = find_libs(prefix_path)
    print("[Found %d libraries]" % len(lib_names))
    print("[Finding executables @ %s]" % prefix_path)
    exe_names = find_exes(prefix_path)
    print("[Found %d executables]" % len(exe_names))
    print("[Finding bundles @ %s]" % prefix_path)
    bundle_names = find_bundles(prefix_path)
    print("[Found %d bundles]" % len(bundle_names))
    print("[Fixing Libraries...]")
    fixup_items(lib_names,lib_maps,prefix_path)
    print("[Fixing Executables...]")
    fixup_items(exe_names,lib_maps,prefix_path)
    print("[Fixing Bundles...]")
    fixup_bundles(bundle_names)


if __name__ == "__main__":
    main()
