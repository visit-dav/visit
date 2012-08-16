#*****************************************************************************
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
#*****************************************************************************
"""
file: visit_test_common.py
description: 
author: Cyrus Harrison
date:
"""

import subprocess
import os

from os.path import join as pjoin


def abs_path(*args):
    """
    Helper for constructing absolute paths from a string, or lists of strings.
    """
    rargs = []
    for arg in args:
        if os.path.isabs(arg) or arg.count("/")  == 0:
            rargs.append(arg)
        else:
            toks = arg.split("/")
            rargs.extend(toks)
    res = os.path.abspath(pjoin(*rargs))
    # for now, avoid unicode b/c it 
    # doesn't work w/ RestoreSession w/ diff sources
    res = res.encode('ascii','ignore')
    return res


def sexe(cmd,ret_output=False,suppress_output=False,echo = False):
    """
    Helper for executing shell commands.
    """
    if echo:
        Log("[exe: %s]" % cmd)
    if ret_output:
        p = subprocess.Popen(cmd,
                             shell=True,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT)
        res =p.communicate()[0]
        return p.returncode,res
    elif suppress_output:
        null_fp = open(os.devnull, "w")
        p = subprocess.Popen(cmd,
                             shell=True,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT)
        res = p.communicate()[0]
        return p.returncode
    else:
        return subprocess.call(cmd,shell=True)

def Log(msg,echo=True):
    """
    Prints message to screen and also records to a log file.
    """
    if echo:
        print msg
    if (os.path.isfile("log")):
        log = open("log", 'a')
        log.write(msg)
        if msg.count("\n") == 0:
            log.write("\n")
        log.close()