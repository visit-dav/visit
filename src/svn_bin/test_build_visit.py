#!/usr/bin/env python
# *****************************************************************************
#   Script: test_build_visit.py
#
#   Purpose:
#       Provide an easy (and cron-able) way to test the build_visit script.
#   Usage:
#       test_build_visit.py [output_dir] [lib1] <lib2> ... <libn>
#   
#   Results:
#       Outputs a xml summary of build results to stdout.
# 
# *****************************************************************************
# Current Functionality: 
# foreach lib invoke build_visit with the following options:
#  build_visit --console --no-visit --no-thirdparty --no-hostconf --stdout \ 
#          --[libname] --thirdparty-path  [output] > [output]/bv_log_[libname]
#  Check for "Finished!" at tail of output log
#
# Possible Future Functionality:
#  Cleanup after (each?) build.
#  Figure out $VISITARCH and version of each lib &check for lib installed
#
# *****************************************************************************

import sys,os,time,socket

class BuildResult:
    """
    Class that holds the results for an attemped build of a library.
    """
    def __init__(self,lib,cmd,build_start,build_end,result):
        self.lib = lib
        self.cmd = cmd
        self.build_start = build_start
        self.build_end   = build_end
        self.result = result 
    def to_xml(self,space=""):
        res =  space + "<build_result>\n"
        res += space + space +"<lib>%s</lib>\n" % self.lib
        res += space + space +"<cmd>%s</cmd>\n" % self.cmd
        res += space + space +"<start>%s</start>\n" % self.build_start
        res += space + space +"<end>%s</end>\n"  % self.build_end
        res += space + space +"<result>%s</result>\n" % self.result
        res += space + "</build_result>"
        return res

def get_current_time():
    """
    Helper to return a human readble string /w the current time.
    """
    return time.asctime(time.gmtime())
        
def sexe(cmd):
    """
    Execute a shell command.
    """
    sys.stderr.write("[exe:%s]\n" % cmd)
    os.system(cmd)

def log_file(lib,odir=""):
    """
    Construct the output log file name.
    """
    if odir == "":
        return "bv_log_%s" % lib
    else:
        return "%s/bv_log_%s" % (odir,lib)
    
def build_visit_command(libs,odir=""):
    """
    Construct the proper build_visit command.
    """
    options = "--console --no-visit --no-thirdparty --no-hostconf --stdout --makeflags -j4"
    blibs=""
    for l in libs:
        blibs+="--%s " % l
    lib = libs[-1]
    if odir == "":
        cmd = "echo yes | ./build_visit %s %s >> %s"
        cmd = cmd % (options,blibs,log_file(lib))
    else:
        cmd = "echo yes | ./build_visit %s %s --thirdparty-path  %s >> %s"
        cmd = cmd % (options,blibs,odir,log_file(lib,odir))
    return cmd

def check_build(lib,odir):
    """
    Checks for sucessful build by examining at the end of the build log.
    """
    try:
        lines = open(log_file(lib,odir)).readlines()
        nlines = len(lines)
        last = lines[nlines-1].strip()
        return last == "Finished!"
    except:
        return False

def build(libcmd,odir):
    """
    Build a library using build_visit and return a BuildResult.
    """
    libs = parse_libcmd(libcmd)
    bvc = build_visit_command(libs,odir);
    lib = libs[-1]
    bresult = "failure"
    bstart = get_current_time()
    sexe(bvc)
    bend = get_current_time()
    if check_build(lib,odir):
        bresult = "success"
    return BuildResult(lib,libcmd,bstart,bend,bresult)
   
def build_libs(libcmds,odir):
    """
    Build a collection of libraries using build_visit and return a BuildResult for each.
    """
    if odir != "":
        if not os.path.exists(odir):
            os.mkdir(odir)
    return [ build(libcmd,odir) for libcmd in libcmds ]

    
def parse_libcmd(vstr):
    # lib >>> [lib[
    # lib_3:lib_1,lib_2 >>> [lib_1,lib_2,lib_3]
    #
    res = []
    cidx = vstr.find(":")
    if cidx >0:
        pre  = vstr[cidx+1:].split(",")
        post = vstr[:cidx]
        for p in pre:
            res.append(p)
        res.append(post)
    else:
        res.append(vstr)
    return res
    
    
def main():
    """
    Main driver routine.
    """
    nargs = len(sys.argv)
    if nargs < 3:
        sys.stderr.write("usage: test_build_visit.py [output_dir] [lib_1] <lib_2> .... <lib_n>\n")
        sys.exit(1)
    odir = sys.argv[1]
    libcmds = sys.argv[2:]
    if odir[-1] == "/":
        odir = odir[:-1]
    stime = get_current_time()
    results = build_libs(libcmds,odir)
    etime = get_current_time()
    print '<?xml version="1.0"?>'
    host=socket.gethostname()
    info = 'host="%s" odir="%s" libs="%s" start="%s" end="%s"' % (host,str(odir),str(libcmds),stime,etime)
    print "<test_build_visit %s >" % info
    print "  <results>"
    for r in results:
        print r.to_xml("  ")
    print "  </results>"
    print "</test_build_visit>"
    

if __name__ == "__main__":
    main()
    
