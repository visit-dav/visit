import sys
import json
import os

from os.path import join as pjoin

from masonry import *



def load_opts(opts_json):
    opts_data = open(opts_json).read()
    print "[Build Options:]"
    print opts_data
    opts = json.loads(opts_data)["bootstrap_visit"]
    # setup platform name
    if not opts.has_key("platform"):
        opts["platform"] = "linux"
        if opts["arch"].count("darwin") > 0: 
            opts["platform"] = "osx"
    # setup paths
    if not opts.has_key("build_dir"):
        opts_json_base = os.path.split(opts_json)[1]
        opts_json_base = os.path.splitext(opts_json_base)[0]
        print opts_json_base
        opts["build_dir"] = "build-%s" % (opts_json_base)
    opts["build_dir"] = os.path.abspath(opts["build_dir"])
    print "[build directory: %s]" % opts["build_dir"] 
    if not opts.has_key("force_clean"):
        opts["force_clean"] = False
    if opts.has_key("skip_checkout"):
        if opts["skip_checkout"].upper() == "NO":
            opts["skip_checkout"] = False
        else:
            opts["skip_checkout"] = True
    else:
        opts["skip_checkout"] = False
    # Setup bb env vars
    env = {'VISITARCH' : opts["arch"]}
    if opts.has_key("par_compiler"):
        env["PAR_COMPILER"] = opts["par_compiler"]
    if opts.has_key("par_include"):
        env["PAR_INCLUDE"] = opts["par_include"]
    if opts.has_key("par_libs"):
        env["PAR_LIBS"] = opts["par_libs"]
    if opts.has_key("svn") and opts["svn"].has_key("nersc_uname"):
        env["SVN_NERSC_NAME"] = opts["svn"]["nersc_uname"]
    if opts.has_key("fc_compiler"):
        env["FC_COMPILER"] = opts["fc_compiler"]
    if not opts.has_key("tarball"):
        opts["tarball"] = None
    if not opts.has_key("branch"):
        opts["branch"] = "develop" 
    if not opts.has_key("tag"):
        opts["tag"] = None
    if opts.has_key("env"):
        env.update(opts["env"])
    opts["env"] = env
    if not opts.has_key("cert"):
        opts["cert"] = ""
    return opts

def visit_svn_path(path,svn_opts,branch=None,tag=None):
    if svn_opts["mode"] == "anon":
        res = "http://portal.nersc.gov/svn/visit/"
    else:
        nersc_uname = svn_opts["nersc_uname"]
        res = "svn+ssh://%s@cori.nersc.gov/project/projectdirs/visit/svn/visit/"
        res = res % nersc_uname
    if not branch is None:
        res = res + "branches/" + branch + "/" + path
    elif not tag is None:
        res = res + "tags/" + tag + "/" + path
    else:
        res = res + "trunk/" + path
    return res

def visit_git_path(git_opts):
    if git_opts["mode"] == "ssh":
        res = "ssh://git@github.com/visit-dav/visit.git"
    else:
        res = "https://github.com/visit-dav/visit.git"
    return res

def cmake_bin(opts):
    if "build_visit" in opts: # use cmake created by build_visit
        cmake_cmd = "../thirdparty_shared/third_party/cmake/%s/%s/bin/cmake"
        cmake_cmd = cmake_cmd % (opts["build_visit"]["cmake_ver"],opts["arch"])
    else: 
        # assume a suitable cmake exists in the user's PATH
        cmake_cmd = "cmake"
    return cmake_cmd



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
    bv_args = " --console "
    if opts["build_visit"].has_key("make_flags"):
        bv_args    += " --makeflags '%s'" % opts["build_visit"]["make_flags"]
    elif opts.has_key("make_nthreads"):
        bv_args   += " --makeflags '-j%d'" % opts["make_nthreads"]
    bv_args   += " --no-visit"
    if opts.has_key("c_compiler"):
        bv_args += " --cc " + opts["c_compiler"]
    if opts.has_key("cxx_compiler"):
        bv_args += " --cxx " + opts["cxx_compiler"]
    if opts["build_visit"].has_key("args"):
        bv_args += " " + opts["build_visit"]["args"]
    if opts["build_visit"].has_key("libs"):
        bv_args +=  " " + " ".join(["--%s" % l for l in opts["build_visit"]["libs"]])
    bv_cmd   = "echo yes | ../visit/src/tools/dev/scripts/build_visit %s" % bv_args
    ctx.actions["bv_run"] = shell(cmd=bv_cmd,
                                  description="building dependencies",
                                  working_dir=bv_working,
                                  env = opts["env"])
    ctx.triggers["build"].extend(["create_third_party", "bv_run"])


def steps_checkout(opts,ctx):
    git_working = pjoin(opts["build_dir"], "visit")
    ctx.actions["src_checkout"] = git(git_url=visit_git_path(git_opts=opts["git"]),
                                      git_cmd="clone --depth=1",
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
    if opts.has_key("c_compiler"):
        cmake_opts += ' -DVISIT_C_COMPILER:PATH="%s"' % opts["c_compiler"]
    if opts.has_key("cxx_compiler"):
        cmake_opts += ' -DVISIT_CXX_COMPILER:PATH="%s"' % opts["cxx_compiler"]
    if opts.has_key("boost_dir"):
        cmake_opts += ' -DVISIT_USE_BOOST:BOOL="ON"'
        cmake_opts += ' -DBOOST_ROOT:PATH="%s"' % opts["boost_dir"]
    if opts.has_key("build_xdb"):
        if opts["build_xdb"]:
            cmake_opts += " -DVISIT_ENABLE_XDB:BOOL=ON"    
    if opts.has_key("build_visit"):
        cmake_opts += " -DVISIT_CONFIG_SITE:PATH=%s/$(hostname).cmake" % config_dir
    if opts.has_key("cmake_extra_args"):
        cmake_opts += opts["cmake_extra_args"]
    elif opts.has_key("config_site"):
        cfg_site = opts["config_site"]
        cfg_site_abs = os.path.abspath(cfg_site)
        if not os.path.isfile(cfg_site_abs):
            cfg_site = pjoin("..","visit/src","config-site",cfg_site)
        else:
            cfg_site = cfg_site_abs
        cmake_opts += " -DVISIT_CONFIG_SITE:PATH=%s" % cfg_site
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

def steps_install(opts,build_type,ctx):
    build_dir      = pjoin(opts["build_dir"],"build.%s" % build_type.lower())
    a_make_install = "install_" + build_type.lower()
    ctx.actions[a_make_install] = make(description="installing visit",
                                   nthreads=opts["make_nthreads"],
                                   working_dir=build_dir,
                                   target="install")
    ctx.triggers["build"].append(a_make_install)

def steps_package(opts,build_type,ctx):
    build_dir  = pjoin(opts["build_dir"],"build.%s" % build_type.lower())
    a_make_pkg = "package_" + build_type.lower()
    ctx.actions[a_make_pkg] = make(description="building visit package",
                                   nthreads=opts["make_nthreads"],
                                   working_dir=build_dir,
                                   target="package")
    ctx.triggers["build"].append(a_make_pkg)
    if opts["platform"] == "osx":
        cmake_opts = " -DVISIT_CREATE_APPBUNDLE_PACKAGE:BOOL=ON"
        if opts.has_key("cert"):
            cmake_opts += ' -DCPACK_BUNDLE_APPLE_CERT_APP="%s"' % opts["cert"]
        a_cmake_bundle = "cmake_cfg_bundle_" + build_type
        a_make_bundle  = "package_osx_bundle." + build_type
        ctx.actions[a_cmake_bundle] = cmake(src_dir=pjoin(opts["build_dir"],"visit/src"),
                                            cmake_bin=cmake_bin(opts),
                                            cmake_opts=cmake_opts,
                                            working_dir=build_dir,
                                            description="configuring visit (osx bundle)")
        ctx.actions[a_make_bundle] = make(description="packaging visit (osx bundle)",
                                          nthreads=opts["make_nthreads"],
                                          working_dir=build_dir,
                                          target="package")
        ctx.triggers["build"].extend([a_cmake_bundle,
                                      a_make_bundle])

def steps_sanity_checks(opts,build_type,ctx):
    if opts["platform"] == "osx":
        steps_osx_sanity_check(opts,build_type,ctx)    

def steps_osx_sanity_check(opts,build_type,ctx):
    """
    Post build check of OSX install names.
    Checks the names of the vtkRendering.dylib in the resulting DMG. 
    """
    build_dir  = pjoin(opts["build_dir"],"build.%s" % build_type.lower())
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
    test_cmd   = "hdiutil attach -mountpoint mount VisIt-%s.dmg\n"
    test_dylib = "libvtkRenderingCore-6.*.dylib"

    test_cmd += "spctl -a -t exec -vv mount/VisIt.app\n"
    test_cmd += "otool -L %s/lib/%s | grep @exe\n"
    test_cmd += "otool -L %s/lib/%s | grep build-mb\n"
    test_cmd += "otool -L %s/lib/%s | grep build-mb | wc -l\n"
    test_cmd += "otool -L %s/lib/%s | grep RPATH\n"
    test_cmd += "hdiutil detach mount\n"
    test_cmd = test_cmd %  (actual_version,test_base,test_dylib,
                                           test_base,test_dylib,
                                           test_base,test_dylib,
                                           test_base,test_dylib)
    saction = "osx_sanity_" + build_type.lower()
    ctx.actions[saction] = shell(cmd=test_cmd,
                                      description="sanity check",
                                      working_dir=build_dir)
    ctx.triggers["build"].append(saction)


def steps_visit(opts,ctx):
    ctx.triggers["build"] = inorder()
    #if not opts["tarball"] is None:
    #    steps_untar(opts,ctx)
    #else:
    #    steps_checkout(opts,ctx)
    if not opts["skip_checkout"]:
        steps_checkout(opts,ctx)

    if opts.has_key("build_visit"):
        steps_bv(opts,ctx)

    for build_type in opts["build_types"]:
        steps_configure(opts,build_type,ctx)
        steps_build(opts,build_type,ctx)
        steps_install(opts,build_type,ctx)
        steps_package(opts,build_type,ctx)
        steps_sanity_checks(opts,build_type,ctx)

def main(opts_json):
    opts = load_opts(opts_json)
    ctx = Context()
    steps_visit(opts,ctx)
    ctx.fire("build")
    return ctx


if __name__ == "__main__":
    ctx = main(sys.argv[1])
