# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#
# Helper script to cleanup tpl build dirs to reduce
# our docker image size
#
import sys
import glob
import os
import shutil

from os.path import join as pjoin

def tpl_build_dir():
    return "/masonry/build-mb-develop-ci-smoke/thirdparty_shared"

def check_to_keep(path):
    res = False
    if path.count("third_party") > 0:
        res = True
    return res
    
def cleanup_tpl_build_dirs():
    paths = glob.glob( pjoin(tpl_build_dir(),"*") )
    for path in paths:
        if not check_to_keep(path):
            print("[removing %s]" % path)
            if os.path.isdir(path):
                shutil.rmtree(path)
            else:
                os.remove(path)

def copy_config_site():
     cfg_site = glob.glob( pjoin(tpl_build_dir(),"*.cmake") )[0]
     dest = "/visit-ci-cfg.cmake"
     print("[copying %s to %s]" % (cfg_site,dest))
     shutil.copyfile(cfg_site,dest)

def main():
    copy_config_site()
    cleanup_tpl_build_dirs()


if __name__ == "__main__":
    main()
