#
# I usually work from a git worktree that is peer or child dir of the main `develop` repo
# 1. Create a `release` folder there
# 2. Ensure masonry options file is updated with correct
#     - VisIt version number
#     - Git branch name (or tag or commit hash)
#     - architecture
#     - keychain profile name
#     - path to entitlements file
#     - nthreads setting (for make -j)
#
# cd to `release` folder and run command like so...
#
#     env PATH=/usr/local/bin:/usr/bin:/bin:/sbin:/usr/sbin python3 ../src/tools/dev/masonry/bootstrap_visit.py ../src/tools/dev/masonry/opts/mb-3.4.2-darwin-23-arm64-release.json
#
# First try will fail. When it does, cd to the build-mb-x.y.z-arch... folder and create a symlink
# named `visit` to ../.. from that build folder and restart.
#
import sys
import json
import subprocess
import time
import glob

import os
from os.path import join as pjoin

from masonry import *

def load_opts(opts_json):
    opts_data = open(opts_json).read()
    print("[Build Options:]")
    print(opts_data)
    opts = json.loads(opts_data)["bootstrap_visit"]
    # setup platform name
    if not "platform" in opts:
        opts["platform"] = "linux"
        if opts["arch"].count("darwin") > 0: 
            opts["platform"] = "osx"
    # setup paths
    if not "build_dir" in opts:
        opts_json_base = os.path.split(opts_json)[1]
        opts_json_base = os.path.splitext(opts_json_base)[0]
        print(opts_json_base)
        opts["build_dir"] = "build-%s" % (opts_json_base)
    opts["build_dir"] = os.path.abspath(opts["build_dir"])
    print("[build directory: %s]" % opts["build_dir"])
    if not "force_clean" in opts:
        opts["force_clean"] = False
    if "skip_checkout" in opts:
        if opts["skip_checkout"].upper() == "NO":
            opts["skip_checkout"] = False
        else:
            opts["skip_checkout"] = True
    else:
        opts["skip_checkout"] = False
    # Setup bb env vars
    env = {'VISITARCH' : opts["arch"]}
    if "par_compiler" in opts:
        env["PAR_COMPILER"] = opts["par_compiler"]
    if "par_include" in opts:
        env["PAR_INCLUDE"] = opts["par_include"]
    if "par_libs" in opts:
        env["PAR_LIBS"] = opts["par_libs"]
    if "svn" in opts and "nersc_uname" in opts["svn"]:
        env["SVN_NERSC_NAME"] = opts["svn"]["nersc_uname"]
    if "fc_compiler" in opts:
        env["FC_COMPILER"] = opts["fc_compiler"]
    if not "tarball" in opts:
        opts["tarball"] = None
    if not "branch" in opts:
        opts["branch"] = "develop" 
    if not "tag" in opts:
        opts["tag"] = None
    if "env" in opts:
        env.update(opts["env"])
    opts["env"] = env
    if not "cert" in opts:
        opts["cert"] = ""
    return opts

def visit_git_path(git_opts):
    if git_opts["mode"] == "ssh":
        res = "ssh://git@github.com/visit-dav/visit.git"
    else:
        res = "https://github.com/visit-dav/visit.git"
    return res

def cmake_bin(opts):
    if "build_visit" in opts: # use cmake created by build_visit
        cmake_cmd = pjoin(opts["build_dir"],"thirdparty_shared")
        cmake_cmd += "/third_party/cmake/%s/%s/bin/cmake"
        cmake_cmd = cmake_cmd % (opts["build_visit"]["cmake_ver"],opts["arch"])
    else: 
        # assume a suitable cmake exists in the user's PATH
        cmake_cmd = "cmake"
    return cmake_cmd

def build_visit_exe():
    # find build_visit relative to this script
    masonry_dir = os.path.abspath(os.path.split(__file__)[0])
    return os.path.abspath(pjoin(masonry_dir,"..","scripts","build_visit"))

def steps_bv(opts,ctx):
    if(opts["force_clean"] == True):
        cmd_clean = "rm -rf *"
    else:
        cmd_clean = "pwd"
    bv_working = pjoin(opts["build_dir"],"thirdparty_shared")
    thirdparty_dir = pjoin(bv_working,"third_party")

    ctx.actions["create_third_party"]  =  shell(cmd=cmd_clean,
                                                working_dir=thirdparty_dir,
                                                description="create %s" % thirdparty_dir)
    bv_args = ""
    if "make_flags" in opts["build_visit"]:
        bv_args    += " --makeflags '%s'" % opts["build_visit"]["make_flags"]
    elif "make_nthreads" in opts:
        bv_args   += " --makeflags '-j%d'" % opts["make_nthreads"]
    bv_args   += " --no-visit"
    if "c_compiler" in opts:
        bv_args += " --cc " + opts["c_compiler"]
    if "cxx_compiler" in opts:
        bv_args += " --cxx " + opts["cxx_compiler"]
    if "args" in opts["build_visit"]:
        bv_args += " " + opts["build_visit"]["args"]
    if "libs" in opts["build_visit"]:
        bv_args +=  " " + " ".join(["--%s" % l for l in opts["build_visit"]["libs"]])
    bv_cmd   = "echo yes | %s %s" % (build_visit_exe(), bv_args)
    ctx.actions["bv_run"] = shell(cmd=bv_cmd,
                                  description="building dependencies",
                                  working_dir=bv_working,
                                  env = opts["env"])
    ctx.triggers["build"].extend(["create_third_party", "bv_run"])


def steps_checkout(opts,ctx):
    git_working = pjoin(opts["build_dir"], "visit")
    git_cmd = "clone --recursive"
    if "depth" in opts["git"]:
        git_cmd += " --depth=%s" % opts["git"]["depth"]
    ctx.actions["src_checkout"] = git(git_url=visit_git_path(git_opts=opts["git"]),
                                      git_cmd=git_cmd,
                                      description="checkout visit src",
                                      working_dir=opts["build_dir"],
                                      halt_on_error=False)
    ctx.triggers["build"].append("src_checkout");
    if opts["branch"] != "develop":
        ctx.actions["switch_branch"] = shell(cmd="git checkout %s" % opts["branch"],
                                             description="switch to branch",
                                             working_dir=git_working,
                                             halt_on_error=False)
        ctx.triggers["build"].append("switch_branch");


def steps_untar(opts,ctx):
    tar_base = os.path.basename(opts["tarball"])
    ctx.actions["src_copy_tar"] = shell(cmd="cp %s ." % opts["tarball"],
                                      description="copy source tar",
                                      working_dir=build_dir)
    ctx.actions["src_untar"] = shell(cmd="tar -xzvf %s" % tar_base,
                                      description="untar source",
                                      working_dir=build_dir)

def steps_configure(opts,build_type,ctx):
    if(opts["force_clean"] == True):
        cmd_clean = "rm -rf *"
    else:
        cmd_clean = "pwd"
    build_dir   = pjoin(opts["build_dir"],"build.%s"   % build_type.lower())
    install_dir = pjoin(opts["build_dir"],"install.%s" % build_type.lower())
    config_dir = pjoin(opts["build_dir"], "thirdparty_shared")
    ctx.actions["create_build.%s"   % build_type.lower() ]    = shell(cmd=cmd_clean,
                                                  working_dir=build_dir,
                                                  description="create %s" % build_dir)
    ctx.actions["create_install.%s" % build_type.lower() ]  = shell(cmd=cmd_clean,
                                                 working_dir=install_dir,
                                                 description="create %s" % install_dir)
    cmake_opts  = " -DCMAKE_BUILD_TYPE:STRING=%s" % build_type
    cmake_opts += " -DVISIT_INSTALL_THIRD_PARTY:BOOL=ON"
    cmake_opts += " -DCMAKE_INSTALL_PREFIX:PATH=%s" % install_dir
    if "c_compiler" in opts:
        cmake_opts += ' -DVISIT_C_COMPILER:PATH="%s"' % opts["c_compiler"]
    if "cxx_compiler" in opts:
        cmake_opts += ' -DVISIT_CXX_COMPILER:PATH="%s"' % opts["cxx_compiler"]
    if "boost_dir" in opts:
        cmake_opts += ' -DVISIT_USE_BOOST:BOOL="ON"'
        cmake_opts += ' -DBOOST_ROOT:PATH="%s"' % opts["boost_dir"]
    if "build_xdb" in opts:
        if opts["build_xdb"]:
            cmake_opts += " -DVISIT_ENABLE_XDB:BOOL=ON"    
    if "build_visit" in opts:
        cmake_opts += " -DVISIT_CONFIG_SITE:PATH=%s/$(hostname).cmake" % config_dir
    elif "config_site" in opts:
        cfg_site = opts["config_site"]
        cfg_site_abs = os.path.abspath(cfg_site)
        if not os.path.isfile(cfg_site_abs):
            cfg_site = pjoin("..","visit/src","config-site",cfg_site)
        else:
            cfg_site = cfg_site_abs
        cmake_opts += " -DVISIT_CONFIG_SITE:PATH=%s" % cfg_site
    if "cmake_extra_args" in opts:
        cmake_opts += opts["cmake_extra_args"]
    ctx.actions["cmake_" + build_type ] = cmake(src_dir=pjoin(opts["build_dir"],"visit/src"),
                                                cmake_bin=cmake_bin(opts),
                                                cmake_opts=cmake_opts,
                                                working_dir=build_dir,
                                                description="configuring visit")
    ctx.triggers["build"].extend(["create_build.%s"   % build_type.lower() ,
                                  "create_install.%s" % build_type.lower(),
                                  "cmake_" + build_type])


def steps_build(opts,build_type,ctx):
    build_dir  = pjoin(opts["build_dir"],"build.%s" % build_type.lower())
    a_vbuild = "build_" + build_type.lower()
    ctx.actions[a_vbuild] = make(description="building visit",
                                 nthreads=opts["make_nthreads"],
                                 working_dir=build_dir)
    ctx.triggers["build"].append(a_vbuild)

def steps_manuals(opts,build_type,ctx):
    build_dir      = pjoin(opts["build_dir"],"build.%s" % build_type.lower())
    a_make_manuals = "manuals_" + build_type.lower()
    ctx.actions[a_make_manuals] = make(description="creating manuals",
                                   nthreads=opts["make_nthreads"],
                                   working_dir=build_dir,
                                   target="manuals")
    ctx.triggers["build"].append(a_make_manuals)

def steps_install(opts,build_type,ctx):
    build_dir      = pjoin(opts["build_dir"],"build.%s" % build_type.lower())
    a_make_install = "install_" + build_type.lower()
    ctx.actions[a_make_install] = make(description="installing visit",
                                   nthreads=opts["make_nthreads"],
                                   working_dir=build_dir,
                                   target="install")
    ctx.triggers["build"].append(a_make_install)

########################################################################
# NOTE: Mark C. Miller, Fri Dec 13 19:13:32 PST 2024
# I believe CMake's `make package` target tries to also use hdiutil.
# When make is invoked with a lot of parallelism (e.g. -j8 or more),
# I believe a race condition occurs in CMake sometimes causing the
# `make package` operation to fail during `hdiutil` command with
# "Resource busy" error. So, here we override nthreads to force use
# of just a single thread.
########################################################################

def steps_package(opts,build_type,ctx):
    build_dir  = pjoin(opts["build_dir"],"build.%s" % build_type.lower())
    a_make_pkg = "package_" + build_type.lower()
    ctx.actions[a_make_pkg] = make(description="building visit package",
                                   nthreads=1,
                                   working_dir=build_dir,
                                   target="package")
    ctx.triggers["build"].append(a_make_pkg)
    if opts["platform"] == "osx":
        cmake_opts = " -DVISIT_CREATE_APPBUNDLE_PACKAGE:BOOL=ON"
        a_cmake_bundle = "cmake_cfg_bundle_" + build_type
        a_make_bundle  = "package_osx_bundle." + build_type
        ctx.actions[a_cmake_bundle] = cmake(src_dir=pjoin(opts["build_dir"],"visit/src"),
                                            cmake_bin=cmake_bin(opts),
                                            cmake_opts=cmake_opts,
                                            working_dir=build_dir,
                                            description="configuring visit (osx bundle)")
        ctx.actions[a_make_bundle] = make(description="packaging visit (osx bundle)",
                                          nthreads=1,
                                          working_dir=build_dir,
                                          target="package")
        ctx.triggers["build"].extend([a_cmake_bundle,
                                      a_make_bundle])

def steps_notarize(opts,build_type,ctx):
    if opts["platform"] == "osx":
        a_notarize_visit  = "notarize_visit." + build_type
        ctx.actions[a_notarize_visit] = notarize(build_dir=opts["build_dir"],
                                                 build_type=build_type.lower(),
                                                 build_version=opts["version"],
                                                 build_arch=opts["arch"],
                                                 entitlements=opts["entitlements"],
                                                 cert=opts["cert"],
                                                 bundle_id=opts["notarize"]["bundle_id"],
                                                 username=opts["notarize"]["username"],
                                                 password=opts["notarize"]["password"],
                                                 asc_provider=opts["notarize"]["asc_provider"],
                                                 description="notarizing visit")
        ctx.triggers["build"].append(a_notarize_visit)

def steps_osx_install_sanity_checks(opts,build_type,ctx):
    """
    Post build check of OSX install names.
    Uses "osxcheckup.py" script to do a full check of install names 
    """
    # install dir
    #
    # we need to read the actual version b/c even if we select "trunk", the package
    # names will include what is in the src/VERSION file.
    #
    actual_version = opts["version"] 
    actual_version_file = pjoin(opts["build_dir"],"visit/src","VERSION")
    if os.path.isfile(actual_version_file):
        actual_version = open(actual_version_file).readline().strip()
    install_dir = pjoin(opts["build_dir"],"install.%s" % build_type.lower())
    test_base_dir = "%s/%s" % (actual_version,opts["arch"])

    osxcheckup_script = pjoin(opts["build_dir"],"visit","src","osxfixup","osxcheckup.py")

    for check_dir in [ "lib", "bin"]:
        full_test_dir = pjoin(test_base_dir,check_dir)
        test_cmd = "{0} {1} {2}".format(sys.executable,osxcheckup_script,full_test_dir)
        saction = "osx_install_sanity_" + check_dir + "_" + build_type.lower()
        ctx.actions[saction] = shell(cmd=test_cmd,
                                     description="Trying with python install names check for " + check_dir,
                                     working_dir=install_dir)
        ctx.triggers["build"].append(saction)

def steps_install_sanity_checks(opts,build_type,ctx):
    if opts["platform"] == "osx":
        steps_osx_install_sanity_checks(opts,build_type,ctx)

def steps_osx_dmg_sanity_checks(opts,build_type,ctx):
    """
    Post build check of OSX install names.
    Checks the names of the vtkRendering.dylib in the resulting DMG. 
    """
    notarize_dir = pjoin(opts["build_dir"],"notarize.%s" % build_type.lower())
    #
    # we need to read the actual version b/c even if we select "trunk", the package
    # names will include what is in the src/VERSION file.
    #
    actual_version = opts["version"] 
    actual_version_file = pjoin(opts["build_dir"],"visit/src","VERSION")
    if os.path.isfile(actual_version_file):
        actual_version = open(actual_version_file).readline().strip()
    test_base = "mount/VisIt.app/Contents/Resources/%s/%s" % (actual_version, 
                                                              opts["arch"])
    # stop at any error
    final_dmg_name = "visit%s.darwin%s-%s.dmg" % (opts["version"].replace('.','_'),os.uname().release[0:2],os.uname().machine)

    test_cmd  = ""
    test_cmd  += "hdiutil attach -mountpoint mount %s\n" % final_dmg_name
    test_dylib = "libvtkRenderingCore*.*.dylib "
    test_cmd += "otool -L %s/lib/%s | grep @exe\n"
    test_cmd += "otool -L %s/lib/%s | grep build-mb\n"
    test_cmd += "otool -L %s/lib/%s | grep build-mb | wc -l\n"
    test_cmd += "otool -L %s/lib/%s | grep RPATH\n"
    test_cmd += "set -e\n"
    # check for code sign
    test_cmd += 'codesign --test-requirement="=notarized" --verify --verbose mount/VisIt.app/\n'
    # check for any bad symlinks
    test_cmd += 'find . -type l ! -exec test -e {} \; -print | wc -l\n'
    # verify the app
    test_cmd += "spctl -a -t exec -vv mount/VisIt.app\n"
    test_cmd += "hdiutil detach mount\n"
    test_cmd = test_cmd %  (test_base,test_dylib,
                            test_base,test_dylib,
                            test_base,test_dylib,
                            test_base,test_dylib)
    saction = "osx_sanity_" + build_type.lower()
    ctx.actions[saction] = shell(cmd=test_cmd,
                                      description="sanity check",
                                      working_dir=notarize_dir)
    ctx.triggers["build"].append(saction)

def steps_package_sanity_checks(opts,build_type,ctx):
    if opts["platform"] == "osx":
        steps_osx_dmg_sanity_checks(opts,build_type,ctx)


def steps_visit(opts,ctx):
    ctx.triggers["build"] = inorder()

    if not opts["tarball"] is None:
        steps_untar(opts,ctx)
    elif not opts["skip_checkout"]:
        steps_checkout(opts,ctx)

    if "build_visit" in opts:
        steps_bv(opts,ctx)

    for build_type in opts["build_types"]:
        steps_configure(opts,build_type,ctx)
        steps_build(opts,build_type,ctx)
        steps_manuals(opts,build_type,ctx)
        steps_install(opts,build_type,ctx)
        steps_install_sanity_checks(opts,build_type,ctx)
        steps_package(opts,build_type,ctx)
        steps_notarize(opts,build_type,ctx)
        steps_package_sanity_checks(opts,build_type,ctx)

def main(opts_json):
    opts = load_opts(opts_json)
    ctx = Context()
    steps_visit(opts,ctx)
    res = ctx.fire("build")
    # print out any results
    print(res["trigger"]["results"][-1]["action"]["output"])
    # forward return code 
    return res["trigger"]["results"][-1]["action"]["return_code"]


if __name__ == "__main__":
    sys.exit(main(sys.argv[1]))
