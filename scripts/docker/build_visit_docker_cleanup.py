#
# Helper script to cleanup tpl build dirs to reduce
# our docker image size
#
import sys
import glob
import os
import shutil

from os.path import join as pjoin

def home_dir():
    return "/home/visit/"

def check_to_keep(path):
    res = False
    if path.count("third-party") > 0:
        res = True
    elif path.count("build_visit") > 0:
        res = True
    elif path.count("run_build_visit.sh") > 0:
        res = True
    elif path.count("test_visit.py") > 0:
        res = True
    elif path.count("build_test_visit.sh") > 0:
        res = True
    elif path.endswith(".cmake"):
        res = True
    return res
    
def cleanup_tpl_build_dirs():
    paths = glob.glob( pjoin(home_dir(),"*") )
    for path in paths:
        if not check_to_keep(path):
            print "[removing %s]" % path
            if os.path.isdir(path):
                shutil.rmtree(path)
            else:
                os.remove(path)

def copy_config_site():
     cfg_site = glob.glob( pjoin(home_dir(),"*.cmake") )[0]
     dest = pjoin(home_dir(),"visit-config.cmake")
     print "[copying %s to %s]" % (cfg_site,dest)
     shutil.copyfile(cfg_site,dest)

def main():
    cleanup_tpl_build_dirs()
    copy_config_site()

if __name__ == "__main__":
    main()
