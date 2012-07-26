###############################################################################
#
# Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-442911
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#
###############################################################################
# file: frontend.py
# Purpose: Front for the visit module.
#  (python port of magic from visitfrontend.c)
#
# Programmer: Cyrus Harrison
# Creation: Wed Apr  4 10:13:25 PDT 2012
#
#
# Modifications:
#  Brad Whitlock, Thu Jul 26 11:57:51 PDT 2012
#  Explicitly load shared library dependencies when VisIt is installed on a 
#  platform that needs that. That way we don't have to force the user to set
#  LD_LIBRARY_PATH before running a system Python. 
#
#  Brad Whitlock, Thu Jul 26 15:05:16 PDT 2012
#  Make "from visit import *" add the VisIt functions to the main namespace
#  as was intended.
#
###############################################################################

import sys
import subprocess
import os
import imp

from os.path import join as pjoin

__all__ = ["Launch","LaunchNowin","AddArgument","SetDebugLevel","GetDebugLevel"]

def Launch(vdir=None):
    return VisItModuleState.launch(vdir)

def LaunchNowin(vdir=None):
    VisItModuleState.add_argument("-nowin")
    return VisItModuleState.launch(vdir)

def AddArgument(arg):
    return VisItModuleState.add_argument(arg)

def SetDebugLevel(lvl):
    return VisItModuleState.set_debug_level(lvl)

def GetDebugLevel(lvl):
    return VisItModuleState.get_debug_level()

class VisItModuleState(object):
    """
    Class used to hold module state.
    """
    debug_lvl   = None
    launch_args = []
    @classmethod
    def launch(cls,vdir=None):
        launched = False
        try:
            vcmd = cls.__visit_cmd(vdir,cls.launch_args)
            env  = cls.__read_visit_env(vcmd)
            mod  = cls.__visit_module_path(env["LIBPATH"])
            print "Using visitmodule: %s" % mod
            for k in env.keys():
                if k != "LIBPATH" and k != "VISITARCHHOME":
                    print k, env[k]
                    os.environ[k] = env[k]
            #print os.environ
            mod = cls.__load_visitmodule(mod)
            for arg in cls.launch_args:
                mod.AddArgument(arg)
            if not cls.debug_lvl is None:
                mod.SetDebugLevel(str(cls.debug_lvl))
            vcmd = cls.__visit_cmd(vdir,[])
            
            # this will add functions to the current
            # 'visit' module
            mod.Launch(vcmd)

            # if SetDebugLevel exists in __main__, then the user did
            # did "from visit import *". the Launch() added functions to
            # the 'visit' module so we need to copy over the functions
            # from the 'visit' module into __main__. We check for
            # the SetDebugLevel function because it's in the frontend
            # 'visit' module and because whichever way we import the 
            # 'visit' module, there seems to be a 'visit' entry in
            # __main__. 
            main_mod = __import__("__main__")
            if "SetDebugLevel" in main_mod.__dict__.keys():
                for k,v in mod.__dict__.items():
                    # avoid hidden module vars
                    if not k.startswith("__"):
                        main_mod.__dict__[k] = v
                # Tell the VisIt module that it uses a local namespace,
                # which helps it get command recording right.
                mod.LocalNameSpace()
            launched = True
        except Exception, e:
            print "ERROR: %s" % e
        return launched
    @classmethod
    def __load_visitmodule(cls,mod_file):
        mod_path = os.path.split(mod_file)[0]
        mfile, mpath, mdes =  imp.find_module('visitmodule',[mod_path ])
        # If VisIt is installed, preemptively try to dlopen the libraries 
        # upon which the visitmodule depends. During package creation, VisIt
        # libraries get their rpath stripped so the path to the various
        # VisIt libraries is no longer known. Without setting LD_LIBRARY_PATH
        # before running Python, the module import would fail. To avoid
        # having to set that variable, we explicitly load the shared libraries
        # we need (in the right order).
        installed_on_afflicted_platform = False
        for platform in ("linux-intel", "linux-x86_64"):
            if platform in mod_path:
                installed_on_afflicted_platform = True
                break
        if installed_on_afflicted_platform:
            import ctypes
            site_pkg = os.path.split(mod_path)[0]
            libdir = os.path.split(site_pkg)[0]
            ext = ".so"
            libs = ("libvisitcommon","libavtdbatts","libviewerrpc","libviewerproxy","libvisitpy")
            for lib in libs:
                libfile = pjoin(libdir,lib + ext)
                a = ctypes.cdll.LoadLibrary(libfile)
        res = None
        try:
            res = imp.load_module("visit", mfile, mpath, mdes)
        finally:
            mfile.close()
        return res
    @classmethod
    def add_argument(cls,arg):
        cls.launch_args.append(arg)
    @classmethod
    def set_debug_level(cls,lvl):
        cls.debug_lvl = lvl
    @classmethod
    def get_debug_level(cls,lvl):
        return cls.debug_lvl
    @classmethod
    def __visit_cmd(cls,vdir,args):
        if sys.platform.startswith("win"):
            vcmd = "visit.exe"
        else:
            vcmd = "visit"
        if not vdir is None:
            vcmd = pjoin(os.path.abspath(vdir),vcmd)
        for arg in args:
            vcmd += " %s" % str(arg)
        return vcmd
    @classmethod
    def __visit_module_path(cls,libpath):
        libpath = os.path.abspath(libpath)
        if sys.platform.startswith("win"):
            mfile = "visitmodule.pyd"
        else:
            mfile = "visitmodule.so"
        mpaths = [ pjoin(libpath,"site-packages","visit",mfile), # new style
                   pjoin(libpath,mfile)] # old style
        for mp in mpaths:
            if os.path.isfile(mp):
                return mp
        msg  = "Could not find the visit python module (%s)\n"
        msg += " Tested paths:\n"
        for mp in mpaths:
            msg += "  %s\n" % mp
        raise Exception(msg)
    @classmethod
    def __read_visit_env(cls,vcmd):
        vcmd += " -env"
        p = subprocess.Popen(vcmd,
                             shell=True,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT)
        pout =p.communicate()[0]
        if p.returncode != 0:
            msg  = "Could not execute VisIt to determine proper env settings!\n"
            msg += "Is VisIt in your shell's PATH?" 
            raise Exception(msg)
        pout = pout.split("\n")
        # we want to know LIBPATH, and VISITPLUGINDIR
        res = {"LIBPATH":"",
               "VISITPLUGINDIR":"",
               "VISITARCHHOME":""}
        for l in pout:
            for k in res.keys():
                if l.startswith(k + "="):
                    r = l[len(k)+1:]
                    r = r.strip(":")
                    res[k] = r
        # extra setup for OSX (we may need to tweak this)
        if sys.platform.startswith("darwin"):
            vhome = res["VISITARCHHOME"]
            dyld = "%s/lib:%s/plugins/operators:%s/plugins/plots" % (vhome,vhome,vhome)
            res["DYLD_LIBRARY_PATH"] = dyld
        return res

