#!/bin/bash

# remove old source tarball if it exists

###############################################################################
# Copyright (c) 2015-2019, Lawrence Livermore National Security, LLC.
#
# Produced at the Lawrence Livermore National Laboratory
#
# LLNL-CODE-716457
#
# All rights reserved.
#
# This file is part of Ascent.
#
# For details, see: http://ascent.readthedocs.io/.
#
# Please also read ascent/LICENSE
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the disclaimer below.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the disclaimer (as noted below) in the
#   documentation and/or other materials provided with the distribution.
#
# * Neither the name of the LLNS/LLNL nor the names of its contributors may
#   be used to endorse or promote products derived from this software without
#   specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY,
# LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
# IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
###############################################################################
import os
import sys
import subprocess
import shutil

from os.path import join as pjoin

def remove_if_exists(path):
    if os.path.isfile(path):
        os.remove(path) 
    if os.path.isdir(path):
        shutil.rmtree(path)

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

def main():
    # clean up tarballs we push into the container
    remove_if_exists("visit.masonry.docker.src.tar")
    remove_if_exists("visit.build_visit.docker.src.tar")
    orig_dir = os.path.abspath(os.getcwd())
    os.chdir("../../../src/tools/dev")
    # get current copy of masonry
    sexe('tar -czvf {0} --exclude "build-*" --exclude "*.pyc" masonry'.format(pjoin(orig_dir,
                                                                              "visit.masonry.docker.src.tar")))
    # get current copy of build_visit from this branch
    os.chdir("scripts")
    sexe('tar -czvf {0} --exclude "build-*" --exclude "*.pyc" build_visit bv_support'.format(pjoin(orig_dir,
                                                                                             "visit.build_visit.docker.src.tar")))
    os.chdir(orig_dir)
    # exec docker build to create image
    # note: --squash requires docker runtime with experimental 
    # docker features enabled. It combines all the layers into
    # a more compact final image to save disk space.
    sexe('docker build -t visit-ci-develop:current . --squash')

if __name__ == "__main__":
    main()

