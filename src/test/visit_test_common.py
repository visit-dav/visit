# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.
"""
file: visit_test_common.py
description:
author: Cyrus Harrison
date:
"""

import subprocess
import os
import json
import datetime
import socket
import re
import sys

from threading import Timer
from os.path import join as pjoin

# ----------------------------------------------------------------------------
#  Method: _decode_list
#
#  Programmer: Cyrus Harrison
#  Date:       Fri Jan 11 2013
#
#  Helper which decodes json unicode values (in lists) to standard strings.
#
# Recipe from:
#  http://stackoverflow.com/questions/956867/how-to-get-string-objects-instead-unicode-ones-from-json-in-python
# ----------------------------------------------------------------------------
def _decode_list(data):
    if (sys.version_info > (3, 0)):
        return data
    rv = []
    for item in data:
        if isinstance(item, unicode):
            item = item.encode('utf-8')
        elif isinstance(item, list):
            item = _decode_list(item)
        elif isinstance(item, dict):
            item = _decode_dict(item)
        rv.append(item)
    return rv

# ----------------------------------------------------------------------------
#  Method: _decode_dict
#
#  Programmer: Cyrus Harrison
#  Date:       Fri Jan 11 2013
#
#  Helper which decodes json unicode values (in dictonaries) to standard strings.
#
# Recipe from:
#  http://stackoverflow.com/questions/956867/how-to-get-string-objects-instead-unicode-ones-from-json-in-python
# ----------------------------------------------------------------------------
def _decode_dict(data):
    if (sys.version_info > (3, 0)):
        return data
    rv = {}
    for key, value in data.items():
        if isinstance(key, unicode):
           key = key.encode('utf-8')
        if isinstance(value, unicode):
           value = value.encode('utf-8')
        elif isinstance(value, list):
           value = _decode_list(value)
        elif isinstance(value, dict):
           value = _decode_dict(value)
        rv[key] = value
    return rv

# ----------------------------------------------------------------------------
# Method: json_load
#
#  Programmer: Cyrus Harrison
#  Date:       Fri Jan 11 2013
# ----------------------------------------------------------------------------
def json_load(f):
    ifile = open(f,"r")
    return json.load(ifile,object_hook=_decode_dict)

# ----------------------------------------------------------------------------
# Method: json_loads
#
#  Programmer: Cyrus Harrison
#  Date:       Fri Jan 11 2013
# ----------------------------------------------------------------------------
def json_loads(val):
    return json.loads(val,object_hook=_decode_dict)



# ----------------------------------------------------------------------------
# Method: json_dump
#
#  Programmer: Cyrus Harrison
#  Date:       Fri Jan 11 2013
# ----------------------------------------------------------------------------
def json_dump(val,f):
    ofile = open(f,"w")
    return json.dump(val,ofile,indent=2)

# ----------------------------------------------------------------------------
# Method: json_dumps
#
#  Programmer: Cyrus Harrison
#  Date:       Fri Jan 11 2013
# ----------------------------------------------------------------------------
def json_dumps(val):
    return json.dumps(val,indent=2)

# ----------------------------------------------------------------------------
#  Method: abs_path
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
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

# ----------------------------------------------------------------------------
#  Method: timestamp
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def timestamp(t=None,sep="_"):
    """
    Creates a timestamp that can easily be included in a filename.
    """
    if t is None:
        t = datetime.datetime.now()
    sargs = (t.year,t.month,t.day,t.hour,t.minute,t.second)
    sbase = "".join(["%04d",sep,"%02d",sep,"%02d",sep,"%02d",sep,"%02d",sep,"%02d"])
    return  sbase % sargs

# ----------------------------------------------------------------------------
#  Method: kill_sexe
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 23 2013
# ----------------------------------------------------------------------------
def kill_sexe(proc, status):
    """
    Helper used to kill a process launched using sexe with a given timeout.
    """
    status["killed"] = True
    proc.kill()

# ----------------------------------------------------------------------------
#  Method: sexe
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
#
#  Modifications:
#    Kathleen Biagas, Tue June 11, 12:14:27 MST 2013
#    Change creationflags to use only CREATE_NEW_PROCESS_GROUP, as that works
#    whether VisIt was built as a Windows app or a Console app.
# ----------------------------------------------------------------------------
def sexe(cmd,
         ret_output=False,
         suppress_output=False,
         file_stdin = None,
         echo = False,
         timeout=None):
    """
    Helper for executing shell commands.
    """
    res = {}
    res["killed"]       = False
    res["output"]       = None
    res["return_code"]  = -1
    if echo:
        Log("[exe: %s]" % cmd)

    scmd = cmd
    sexe_stdin = subprocess.PIPE
    if not file_stdin is None:
        sexe_stdin = open(os.path.abspath(file_stdin))
    # to create a subprocess w/ killable children
    # the majorty of the args are the same for all three
    # cases, however windows and linux have a slighlty
    # different magic to propogate signals.
    # So, we use a kwargs strategy to create
    # the proper arguments for subprocess.Popen
    popen_args = {"shell":True,
                  "stdin":sexe_stdin}
    if sys.platform.startswith("win"):
        # 0x8   = DETACHED_PROCESS
        # 0x200 = CREATE_NEW_PROCESS_GROUP 
        #         (available via subprocess for python 2.7+)
        # 0x208 (both) does not work if VisIt built as a console app
        # so use the one flag that works for both Console app and Windows app.
        popen_args["creationflags"]=0x200 #subprocess.CREATE_NEW_PROCESS_GROUP
    else:
        popen_args["preexec_fn"] = os.setsid
    if ret_output:
        popen_args["stdout"] = subprocess.PIPE
        popen_args["stderr"] = subprocess.STDOUT
        #p = subprocess.Popen(scmd,
                             #shell=True,
                             #stdin=sexe_stdin,
                             #stdout=subprocess.PIPE,
                             #stderr=subprocess.STDOUT,
                             #preexec_fn=os.setsid)
    elif suppress_output:
        fh_dev_null = open(os.devnull,"w")
        popen_args["stdout"] = fh_dev_null
        popen_args["stderr"] = fh_dev_null
        #p = subprocess.Popen(scmd,
                             #shell=True,
                             #stdin=sexe_stdin,
                             #stdout=fh_dev_null,
                             #stderr=fh_dev_null,
                             #preexec_fn=os.setsid)
    p = subprocess.Popen(scmd,**popen_args)
    timer = Timer(timeout, kill_sexe, [p, res])
    timer.start()
    try:
        sexe_res = p.communicate()
    except KeyboardInterrupt:
        pass
    timer.cancel()
    if ret_output:
        res["output"]  = sexe_res[0]
    res["return_code"] = p.returncode
    return res


# ----------------------------------------------------------------------------
#  Method: hostname
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def hostname(full=True):
    """ Returns the base hostname of the current machine. """
    host_name = socket.gethostname()
    if full:
        return host_name
    else:
        return re.compile("[a-zA-z]*").match(host_name).group()

# ----------------------------------------------------------------------------
#  Method: Log
#
#  Programmer: Cyrus Harrison
#  Date:       Wed May 30 2012
# ----------------------------------------------------------------------------
def Log(msg,echo=True):
    """
    Prints message to screen and also records to a log file.
    """
    if echo:
        print(msg)
    if (os.path.isfile("log")):
        log = open("log", 'a')
        log.write(msg)
        if msg.count("\n") == 0:
            log.write("\n")
        log.close()


