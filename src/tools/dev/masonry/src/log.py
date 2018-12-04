#!/usr/bin/env python
#
# file: log.py
# author: Cyrus Harrison <cyrush@llnl.gov>
#
#

import sys
import os
import subprocess
import webbrowser

from os.path import join as pjoin

def view(fname):
    port = 8000
    html_src = pjoin(os.path.split(os.path.abspath(__file__))[0],"html")
    log_dir,log_fname  = os.path.split(os.path.abspath(fname))
    subprocess.call("cp -fr %s/* %s" % (html_src,log_dir),shell=True)
    os.chdir(log_dir)
    try:
        child = subprocess.Popen([sys.executable, 
                                  '-m',
                                  'SimpleHTTPServer',
                                  str(port)])
        url = 'http://localhost:8000/view_log.html?log=%s' % log_fname
        webbrowser.open(url)
        child.wait() 
    except KeyboardInterrupt:
        child.terminate()





