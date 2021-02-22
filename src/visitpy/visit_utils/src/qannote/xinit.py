# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: xinit.py
 author: Cyrus Harrison (cyrush@llnl.gov)
 description:
    Handle x setup using Xvfb if necessary
 modification:
    Kathleen Biagas, Tue Feb 16, 2021
    Add universal_newlines to subprocess.Popen.

"""

import os
import subprocess

def check_x():
    cmd = "xset -g"
    p = subprocess.Popen(cmd,shell=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,universal_newlines=True)
    res = p.communicate()[0]
    if res.count("xset:  unable to open display") >0:
        return False
    return True

def launch_x():
    if not check_x():
        print("[x-server not found: launching Xvfb]")
        subprocess.call("Xvfb :0 -screen 0 500x500x24 &",shell=True)
        os.environ["DISPLAY"] = "127.0.0.1:0"


