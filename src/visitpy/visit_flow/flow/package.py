# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: package.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 3/25/20112
 description:
    Exports the git repo at the current working directory to a tarball.

"""

import subprocess
import sys
import datetime
import os


def create_package(output_file=None):
    if output_file is None:
        t = datetime.datetime.now()
        repo_name = os.path.basename(os.path.dirname(os.path.abspath(__file__)))
        output_file = "%s.%04d.%02d.%02d.tar" % (repo_name,t.year,t.month,t.day)
    cmd = "git archive --format=tar --prefix=%s/ HEAD > %s; " % (repo_name,output_file)
    cmd += "cd ../; tar -rf %s/%s %s/.git; cd %s; " % (repo_name,output_file,repo_name,repo_name)
    cmd += "gzip %s; " % output_file
    print("[exe: %s]" % cmd)
    subprocess.call(cmd,shell=True)

if __name__ == "__main__":
    output_file = None
    if len(sys.argv) > 1:
        output_file = sys.argv[1]
    create_package(output_file)


