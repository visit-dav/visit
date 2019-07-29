# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: logs.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/24/2012
 description:
    Logging infrastructure for flow.

"""

import logging

def get_log(name = "root"):
    """
    Fetches a logger instance for a given component name.
    """
    if name == "root":
        return logging.getLogger('flow')
    else:
        return logging.getLogger('flow.%s' % name)

def log_msg(msg,components=None,lvl=logging.INFO):
    """
    Logs a message to the root and selected component logs.
    """
    if components is None:
        components = []
    if isinstance(components,str):
        components = [components]
    get_log().info(msg)
    for c in components:
        get_log(c).log(lvl,msg)

def info(msg, components = None):
    """
    Logs an info message to the root and selected component logs.
    """
    log_msg(msg,components)

def error(msg, components = None):
    """
    Logs an error message to the root and selected component logs.
    """
    log_msg(msg,components,lvl=logging.ERROR)
