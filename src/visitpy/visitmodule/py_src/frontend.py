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
#
#
###############################################################################

import sys
import subprocess
import os
import imp

from os.path import join as pjoin

__all__ = ["Launch","AddArgument","SetDebugLevel","GetDebugLevel"]

def Launch(vdir=None):
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
            # if visit does not exist in __main__, the
            # user did an "from visit import *"
            # we need to refresh the entries in __main__
            main_mod = __import__("__main__")
            if not "visit" in main_mod.__dict__:
                for k,v in mod.__dict__.items():
                    # avoid hidden module vars
                    if not k.startswith("__"):
                        main_mod.__dict__[k] = v
            launched = True
        except Exception, e:
            print "ERROR: %s" % e
        return launched
    @classmethod
    def __load_visitmodule(cls,mod_file):
        mod_path = os.path.split(mod_file)[0]
        mfile, mpath, mdes =  imp.find_module('visitmodule',[mod_path ])
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

