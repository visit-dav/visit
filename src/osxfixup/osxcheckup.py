# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.


import os
import sys
import subprocess
import glob
import json
import zipfile

from os.path import join as pjoin

def shexe(cmd,ret_output=False,echo = True,env=None):
        """ Helper for executing shell commands. """
        kwargs = {"shell":True}
        if not env is None:
            kwargs["env"] = env
        if echo:
            print("[exe: %s]" % cmd)
        if ret_output:
            kwargs["stdout"] = subprocess.PIPE
            kwargs["stderr"] = subprocess.STDOUT
            kwargs["universal_newlines"] = True
            p = subprocess.Popen(cmd,**kwargs)
            res = p.communicate()[0]
            return p.returncode,res
        else:
            return subprocess.call(cmd,**kwargs),""

def find_suspect_libs(path):
    path = pjoin(path,"*")
    print("[checking: {0}]".format(path))
    fs = glob.glob(path)
    lib_to_deps_map  = {}
    deps_to_libs_map = {}

    # look at all dylibs
    for lib in fs:
        print(lib)
        lib_to_deps_map[lib] = []
        rcode, rout = shexe("otool -L {0}".format(lib),ret_output=True)
        if rcode == 0:
            for l in rout.split("\n"):
                # check if we have a lib
                if l.count("(compatibility version") > 0:
                    dep = l.strip().split()[0]
                    lib_to_deps_map[lib].append(dep)
                    if not dep in deps_to_libs_map.keys():
                        deps_to_libs_map[dep] = []
                    deps_to_libs_map[dep].append(lib)
        else:
            print("[skipping {0}, bad otool resut]".format(lib))

    print(json.dumps(lib_to_deps_map,indent=2))
    print(json.dumps(deps_to_libs_map,indent=2))

    rpath_libs   = []
    other_libs   = []
    suspect_libs = []

    for k in deps_to_libs_map.keys():
        if k.startswith("@rpath"):
            rpath_libs.append(k)
        else:
            other_libs.append(k)

    print("Non @rpath libs")
    print(json.dumps(other_libs,indent=2))

    for lib in other_libs:
        ok_prefixes = ["/usr/lib/", "/System/Library/"]
        ok = False
        for prefix in ok_prefixes:
            if lib.startswith(prefix):
                ok = True
        if not ok:
            suspect_libs.append(lib)

    print("libs we suspect should have @rpath:")
    print(json.dumps(suspect_libs,indent=2))
    return suspect_libs, deps_to_libs_map

def find_suspect_eggs(path):
    # if our python modules are eggs that are zipfiles
    # that will pose a problem for our signing.
    #so check for egg zips 
    # full traverse of the dir
    eggs = []
    suspect_eggs = {}
    for path, directories, files in os.walk(path):
         for f in files:
              if f.endswith(".egg"):
                  eggs.append(pjoin(path,f))
         # we know these are ok, but still want to find some eggs
         for d in directories:
             if d.endswith(".egg"):
                 eggs.append(pjoin(path,d))
    for egg in eggs:
        rcode, rout = shexe("file {0}".format(egg),ret_output=True)
        if rcode != 0:
            print("[error running egg file check!]")
            sys.exit(-1)
        if rout.lower().count("zip") > 0:
            # list contents of the zip file
            z = zipfile.ZipFile(egg)
            # list available files in the container
            z_items = z.namelist()
            z_libs = []
            for z_item in z_items:
                if z_item.endswith(".so") or  z_item.endswith(".dylib"):
                    z_libs.append(z_item)
            if len(z_libs) > 0:
                suspect_eggs[egg] = z_libs
    return suspect_eggs

def main():
    path = sys.argv[1]
    ok = True
    suspect_libs, deps_to_libs_map = find_suspect_libs(path)
    # if we have suspect libs, return error
    if len(suspect_libs) > 0:
        ok = False
        for suspect_lib in suspect_libs:
            print("suspect lib: {0}".format(suspect_lib))
            print("used by:")
            print(json.dumps(deps_to_libs_map[suspect_lib],indent=2))
    suspect_eggs = find_suspect_eggs(path)
    if len(suspect_eggs) > 0:
        print("Error: python egg zip files with shared libs detected!")
        print(json.dumps(suspect_eggs,indent=2))
        ok = False
    if not ok:
        sys.exit(-1)

if __name__ == "__main__":
    main()
