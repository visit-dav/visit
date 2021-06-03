# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.


import os
import sys
import subprocess
import glob
import json

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


def main():
    path = sys.argv[1]
    suspect_libs, deps_to_libs_map = find_suspect_libs(path)
    # if we have suspect libs, return error
    if len(suspect_libs) > 0:
        for suspect_lib in suspect_libs:
            print("suspect lib: {0}".format(suspect_lib))
            print("used by:")
            print(json.dumps(deps_to_libs_map[suspect_lib],indent=2))
        sys.exit(-1)

if __name__ == "__main__":
    main()
