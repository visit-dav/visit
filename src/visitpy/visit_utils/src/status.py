# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: log.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 4/9/2010
 description:
    For displaying status messages to std out & optionally to a file.

"""

from __future__ import print_function


__lvl_sep = ["|******************************************************************************",
             "|------------------------------------------------------------------------------"]

__ofile = None

def __center(sval,ident):
    """ Helper for centering a string. """
    pad = (80 - len(ident) - len(sval))/2 - 1
    res = "".join(" " for i in range(pad))
    return res + sval

def __format(msg,lvl,ident,center):
    """ Helper for formatting a message. """
    res = __lvl_sep[lvl] + "\n"
    toks = msg.split("\n")
    for tok in toks:
        if center:
            tok = __center(tok,ident)
        res += "|%s%s\n" % (ident,tok)
    res += __lvl_sep[lvl] + "\n"
    return res

def info(msg,lvl=1,ident=" ",center=False):
    """ Display a message. """
    global __ofile
    msg = __format(msg,lvl,ident,center)
    print(msg, end=' ')
    if not __ofile is None:
        __ofile.write(msg)

def open_file(fname):
    """ Open log file. """
    global __ofile
    close_file()
    __ofile = open(fname,"w")

def close_file():
    """ Close log file. """
    global __ofile
    if not __ofile is None:
        __ofile.close()
    __ofile = None
