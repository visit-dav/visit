# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.


import fnmatch
import os
import shlex
import stat
import subprocess
import sys

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

def valid_exe_suffix(path):
    # screen things we know are not exes by suffix
    for test_suffix in [".c",
                        ".h",
                        ".hpp",
                        ".cpp",
                        ".hxx",
                        ".cxx",
                        ".py",
                        ".pyc",
                        ".inc",
                        ".a",
                        ".dylib"]:
        if path.lower().endswith(test_suffix):
            return False
    return True

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
          # only do `file` check for files that have exe mode and aren't excluded by suffix
          if mode & exe_flags and valid_exe_suffix(fname):
              # check with otool that this is an exe and not a script
              rcode, chk_out = shexe("file {0}".format(fname), ret_output=True)
              if rcode != 0:
                  print("[warning: failed to obtain file type for '%s']" % fname)
              elif chk_out.count("executable") >= 1:
                  # Note: we could also check for chk_out.lower().count("mach-o")
                  exes.append(fname)
    return exes

def _dedupe_keep_order(seq):
    seen = set()
    out = []
    for x in seq:
        if x not in seen:
            seen.add(x)
            out.append(x)
    return out

def fixup_items(items, lib_maps, prefix_path):
    exe_rpaths = ["@executable_path/..", "@executable_path/../../../.."]

    for item in items:
        item_base = os.path.basename(item)
        print('\n\nosxfixup: Working on item "{0}"'.format(item))

        install_args = ["install_name_tool"]

        # --- read existing LC_RPATHs once (also used for invalid + dedupe) ---
        existing_rpaths = []
        home = os.path.expanduser("~")
        rcode, lc_out = shexe("otool -l {0}".format(shlex.quote(item)), ret_output=True)
        if rcode == 0:
            path_lines = [ln for ln in lc_out.split("\n")[1:] if " path" in ln]
            for ln in path_lines:
                parts = ln.split()
                if len(parts) >= 2:
                    existing_rpaths.append(parts[1])
        else:
            print("[info: no invalid LC_RPATHS for '%s']" % item)

        existing_rpaths = _dedupe_keep_order(existing_rpaths)

        # -id
        if item_base in lib_maps:
            new_id = "@rpath{0}".format(lib_maps[item_base])
        else:
            new_id = "@rpath{0}".format(item.replace(prefix_path, ""))
        install_args += ["-id", new_id]

        # -delete_rpath (home-containing)
        invalid_paths = [p for p in existing_rpaths if home in p]
        for p in invalid_paths:
            install_args += ["-delete_rpath", p]

        # -add_rpath (only if missing to prevent duplication errors)
        for rp in exe_rpaths:
            if rp not in existing_rpaths:
                install_args += ["-add_rpath", rp]

        # -change deps
        rcode, deps_out = shexe("otool -L {0}".format(shlex.quote(item)), ret_output=True)
        if rcode != 0:
            print("[warning: failed to obtain dependencies for '%s']" % item)
        else:
            deps = [d for d in deps_out.split("\n")[1:] if d.strip()]
            print("[# of dependencies for {0} = {1}]".format(item, len(deps)))

            for dep in deps:
                dep = dep.split(" ", 1)[0].strip()
                dep_base = os.path.basename(dep)

                if dep_base not in lib_maps:
                    tdep = os.path.splitext(dep_base)
                    while tdep[1] != "":
                        if tdep[0] in lib_maps:
                            dep_base = tdep[0]
                            break
                        tdep = os.path.splitext(tdep[0])

                if dep_base in lib_maps:
                    install_args += ["-change", dep, "@rpath{0}".format(lib_maps[dep_base])]

        # target must be last
        install_args.append(item)

        cmd = " ".join(shlex.quote(a) for a in install_args) + " 2>&1"
        shexe(cmd)

def main():
    if os.uname().machine == "x86_64":
        prefix_path = "darwin-x86_64"
    elif os.uname().machine == "arm64":
        prefix_path = "darwin-arm64"
    else:
        print("[response from 'os.uname().machine' is not recognized]")
        sys.exit(-1)
    if len(sys.argv) > 1:
        prefix_path = sys.argv[1]
    prefix_path = os.path.abspath(prefix_path)
    ###############
    # find libs
    ###############
    print("[Finding libraries @ %s]" % prefix_path)
    lib_names,lib_maps = find_libs(prefix_path)
    print("[Found %d libraries]" % len(lib_names))
    if len(lib_names) == 0:
        print("[Error found 0 libraries, something is wrong!]")
        sys.exit(-1)
    ###############
    # find exes
    ###############
    print("[Finding executables @ %s]" % prefix_path)
    exe_names = find_exes(prefix_path)
    print("[Found %d executables]" % len(exe_names))
    if len(exe_names) == 0:
        print("[Error found 0 executables, something is wrong!]")
        sys.exit(-1)
    ###############
    # find bundles
    ###############
    print("[Finding bundles @ %s]" % prefix_path)
    bundle_names = find_bundles(prefix_path)
    print("[Found %d bundles]" % len(bundle_names))
    if len(bundle_names) == 0:
        print("[Error found 0 bundles, something is wrong!]")
        sys.exit(-1)
    #############
    # exec fixup
    #############
    print("[Fixing Libraries...]")
    fixup_items(lib_names,lib_maps,prefix_path)
    print("[Fixing Executables...]")
    fixup_items(exe_names,lib_maps,prefix_path)
    print("[Fixing Bundles...]")
    fixup_bundles(bundle_names)


if __name__ == "__main__":
    main()
