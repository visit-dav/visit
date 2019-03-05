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
    if path.count("third_party") == 0:
        res = True
    elif path.endswith(".cmake"):
        res = True
    return res
    
def cleanup_tpl_build_dirs():
    paths = glob.glob( pjoin(tpl_build_dir(),"*") )
    for path in paths:
        if check_to_keep(path):
            print "[removing %s]" % path
            #shutil.rmtree(path)

def copy_config_site():
     cfg_site = glob.glob( pjoin(tpl_build_dir(),"*.cmake") )[0]
     print "[copying %s to /visit-ci-cfg.cmake]" % cfg_site
     shutil.copyfile(cfg_site,"/visit-ci-cfg.cmake")

def main():
    cleanup_tpl_build_dirs()
    copy_config_site()

if __name__ == "__main__":
    main()
