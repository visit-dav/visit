#!/usr/bin/env python
# *****************************************************************************
#   Script: bvauto.py
#
#   Purpose:
#       Helps automate bvtest.py.
#
#   Usage:
#       bvauto.py [result_dir]
#   
#   Results:
#       Executes build tests and updates build summary results in result_dir.
# 
# *****************************************************************************

import sys,os,time,datetime,glob,socket

def sexe(cmd):
    print "[exe: %s]" % cmd
    os.system(cmd)

def get_svn_root():
    if not "SVN_NERSC_NAME" in os.environ.keys():
        sys.stderr.write("<Error: Could not obtain nersc svn user name from enviroment. Make sure SVN_NERSC_NAME is set!>\n")
        sys.exit(-1)
    return "svn+ssh://%s@svn.nersc.gov/svn/visit/trunk/src" % os.environ["SVN_NERSC_NAME"]
    
def create_run_dir(mode):
    t = datetime.datetime.now()
    host = socket.gethostname()
    res = "bv.auto.%s.%d-%02d-%02d.%02d-%02d-%02d.%s" % (mode,t.year,t.month,t.day,t.hour,t.minute,t.second,host)
    if not os.path.exists(res):
        sexe("mkdir %s" % res)
    return res

def parse_tests():
    res = []
    for l in open("bvauto.input").readlines():
        res.append(l.strip())
    return res

def check_success(rfile):
    data = open(rfile).read()
    if data.find("failure") >=0:
        return "red"
    return "green"

def rsort(a,b):
    r = cmp(a,b)
    if r == 0:
        return r
    elif a == -1:
        return 1
    else:
        return -1
    
def update_index(result_root):
    rdirs = glob.glob("%s/bv.auto.*/*.xml" % result_root)
    rdirs.sort(rsort)
    res = "<html>\n"
    res += "<head>\n"
    res += "<title>\n"
    res += "bvauto results" 
    res += "</title>\n"
    res += '<link rel="stylesheet" type="text/css" href="bvtest.css" title="Style">\n'
    res += "</head>\n"
    res += '<table class="wikitable">\n'
    res +='<tr class="rh"><td>Updated: %s</td></tr>\n' % time.asctime(time.gmtime())
    if len(rdirs) == 0:
        res += '<tr class="red"><td>No results?</td></tr>\n'
    for r in rdirs:
        val   = check_success(r)
        rfile = r[r.rfind("/")+1:]
        res += '<tr class="%s"><td><a href="%s">%s</a></td></tr>\n' %(val,r,rfile)
    res +="</table>\n"
    res+="</html>\n"
    f = open("%s/index.html" % result_root,"w")
    f.write(res)
    f.close()

def run(result_root,result_group,mode):
    svn_root = get_svn_root()
    run_dir = create_run_dir(mode)
    libcmds = parse_tests()
    print "[Result Root: %s]" % result_root
    print "[Svn Root:    %s]" % svn_root
    print "[Run Dir:     %s]" % run_dir
    result_dir="%s/%s" % (result_root,run_dir)
    result_file="%s.xml" % run_dir
    cwd = os.getcwd()
    os.chdir(run_dir);
    sexe("svn export %s/svn_bin/build_visit >> export.txt " % svn_root)
    sexe("svn export %s/svn_bin/bvtest.py   >> export.txt " % svn_root)
    sexe("svn export %s/svn_bin/bvtest.xsl  >> export.txt " % svn_root)
    sexe("svn export %s/svn_bin/bvtest.css  >> export.txt " % svn_root)
    # Run test cases from bvauto.input
    res_file = "%s.xml" % run_dir
    sys.path.append(".")
    import bvtest
    bvt = bvtest.BuildTest( "_bv_output",libcmds,"./build_visit",mode)
    bvt.execute()
    open(res_file,"w").write(bvt.result_xml())
    
    # copy results to public dir (xml,xsl,css,bv_log_*txt)
    if not os.path.exists(result_dir):
        sexe("mkdir %s/" % result_dir)
    sexe("cp *xml *xsl _bv_output/bv_log*.txt *css %s/" % result_dir)
    sexe("cp *css %s/" % result_root)
    os.chdir(cwd);
    sexe("rm -rf %s/" % run_dir)
    update_index(result_root)
    sexe("chgrp -R %s %s " % (result_group,result_root))
    sexe("chmod -R +rx %s " % result_root)
    
if __name__ == "__main__":
    nargs = len(sys.argv)
    if nargs < 2:
        print "usage: bvauto.py [result path]"
        sys.exit(-1)
    result_dir = os.path.abspath(sys.argv[1])
    result_group = sys.argv[2]
    if result_dir[-1] == "/":
        result_dir[:-1]
    mode = "svn"
    if nargs ==4:
        mode = sys.argv[3]
    run(result_dir,result_group,mode)



