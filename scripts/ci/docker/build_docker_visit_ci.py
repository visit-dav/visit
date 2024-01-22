# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#
# Helper script that drives our docker build and tag for ci
#

import os
import subprocess
import shutil
import datetime
import tarfile

from os.path import join as pjoin

def remove_if_exists(path):
    """
    Removes a file system path if it exists.
    """
    if os.path.isfile(path):
        os.remove(path) 
    if os.path.isdir(path):
        shutil.rmtree(path)

def timestamp(t=None,sep="-"):
    """ Creates a timestamp that can easily be included in a filename. """
    if t is None:
        t = datetime.datetime.now()
    sargs = (t.year,t.month,t.day)
    sbase = "".join(["%04d",sep,"%02d",sep,"%02d"])
    return  sbase % sargs

def sexe(cmd,ret_output=False,echo = True):
    """ Helper for executing shell commands. """
    if echo:
        print("[exe: {}]".format(cmd))
    if ret_output:
        p = subprocess.Popen(cmd,
                             shell=True,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT)
        res = p.communicate()[0]
        res = res.decode('utf8')
        return p.returncode,res
    else:
        return subprocess.call(cmd,shell=True)

def git_hash():
    """
    Returns the current git repo hash, or UNKNOWN
    """
    res = "UNKNOWN"
    rcode,rout = sexe("git rev-parse HEAD",ret_output=True)
    if rcode == 0:
        res = rout
    return res;

def gen_docker_tag():
    """
    Creates a useful docker tag for the current build.
    """
    ghash = git_hash()
    if ghash != "UNKNOWN":
        ghash = ghash[:6]
    return timestamp() + "-sha" + ghash

def main():
    # clean up tarballs we push into the container
    remove_if_exists("visit.masonry.docker.src.tar")
    remove_if_exists("visit.build_visit.docker.src.tar")

    # save current working dir so we can get back here
    orig_dir = os.path.abspath(os.getcwd())

    # move to dir with masonry and build_viist
    os.chdir("../../../src/tools/dev")

    # get current copy of masonry
    with tarfile.open(pjoin(orig_dir, "visit.masonry.docker.src.tar"),'w') as fout_mas:
        fout_mas.add("masonry",recursive=True)

    # get current copy of build_visit from this branch
    os.chdir("scripts")
    with tarfile.open(pjoin(orig_dir, "visit.build_visit.docker.src.tar"),'w') as fout_bv:
        fout_bv.add("build_visit")
        fout_bv.add("bv_support",recursive=True)
    
    # change back to orig working dir
    os.chdir(orig_dir)

    # exec docker build to create image
    # note: --squash requires docker runtime with experimental 
    # docker features enabled. It combines all the layers into
    # a more compact final image to save disk space.
    # tag with date + git hash
    sexe('docker build -t visitdav/visit-ci-develop:{0} . --squash'.format(gen_docker_tag()))

if __name__ == "__main__":
    main()

