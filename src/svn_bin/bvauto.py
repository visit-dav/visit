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
    """
    Runs a shell command.
    """
    print "[exe: %s]" % cmd
    os.system(cmd)
    
def parse_args():
    """ 
    Handles argument parsing for bvauto.
    """
    res = {}
    nargs = len(sys.argv)
    if nargs < 2:
        print "usage:"
        print " Test build_visit on current machine:"
        print "  bvauto.py [bvauto.input]"
        print " Test build_visit on current machine and post results"
        print "  bvauto.py [bvauto.input] <--post> [result root] [result group]"
        print "  example: bvauto.py bvauto.input --post cyrush@davinci.nersc.gov:/path/to/results/ visitdev"
        print " Reindex Result Root (local machine only):"
        print "  bvauto.py <--reindex> [result root] [result group]"
        print "  example: bvauto.py --reindex /path/to/results/ visitdev"
        sys.exit(-1)
    if sys.argv[1] == "--reindex":
        if nargs <4:
            print "usage:"    
            print "  bvauto.py <--reindex> [result root] [result group]"
            sys.exit(-1)
        res["op"] = "reindex"
        res["result_root"] = os.path.abspath(sys.argv[2])
        res["result_group"] = sys.argv[3]
    else:
        res["op"] = "test"
        res["input_file"] = sys.argv[1]
        res["mode"] = "svn" # support wget later ? 
        res["post"] = False
        res["cleanup"] = False
        if nargs > 2:
            if sys.argv[2] != "--post" or nargs < 5:
                print "usage:"
                print "  bvauto.py [bvauto.input] <--post> [result root] [result group]"
                sys.exit(-1)
            res["post"] = True
            res["cleanup"] = True
            res["result_root"] = sys.argv[3]
            res["result_group"] = sys.argv[4]
    if "result_root" in res.keys():
        if res["result_root"][-1]== "/":
            res["result_root"]=res["result_root"][:-1]
    return res

def get_current_time():
    """
    Returns human readable string of current time.
    """
    t = datetime.datetime.now()
    return "%d-%02d-%02d.%02d-%02d-%02d" % (t.year,t.month,t.day,t.hour,t.minute,t.second)
    
def get_svn_root():
    """
    Constructs a svn root for the VisIt repo using env var SVN_NERSC_NAME.
    """
    if not "SVN_NERSC_NAME" in os.environ.keys():
        sys.stderr.write("<Error: Could not obtain nersc svn user name from environment.")
        sys.stderr.write("Make sure SVN_NERSC_NAME is set!>\n")
        sys.exit(-1)
    return "svn+ssh://%s@svn.nersc.gov/svn/visit/trunk/src" % os.environ["SVN_NERSC_NAME"]
    
def create_run_dir(mode):
    """
    Creates a temporary directory for running a set of build_visit tests.
    """
    host = socket.gethostname()
    tstr = get_current_time()
    res = "bv.auto.%s.%s.%s" % (mode,tstr,host)
    if not os.path.exists(res):
        sexe("mkdir %s" % res)
    return res

def parse_tests(input_file):
    """
    Parses test commands from an input file.
    Extracts one command per line.
    """
    res = []
    for l in open(input_file).readlines():
        res.append(l.strip())
    return res

def analyze_result(rfile):
    """
    Checks a result file for host, architecture, and success.
    """
    data = open(rfile).read()
    # get host, arch and result color
    host = data[data.find('host="')+len('host="'):]
    host = host[:host.find('"')]
    arch = data[data.find('arch="')+len('arch="'):]
    arch = arch[:arch.find('"')]
    if data.find("failure") >=0:
        result = "red"
    else:
        result = "green"
    return host,arch,result

def reindex_results(result_root,result_group):
    """
    Rebuilds the index.html file at a given result_root and sets 
    group read permisions.
    """
    print "[bvauto: reindex %s %s]" % (result_root,result_group)
    rdirs = glob.glob("%s/bv.auto.*/*.xml" % result_root)
    rdirs.sort()
    rdirs.reverse()
    res = "<html>\n"
    res += "<head>\n"
    res += "<title>\n"
    res += "bvauto results" 
    res += "</title>\n"
    res += '<link rel="stylesheet" type="text/css" href="bvtest.css" title="Style">\n'
    res += "</head>\n"
    res += '<table class="wikitable">\n'
    res +='<tr><td colspan=3><b>bvauto results: updated %s</b></td></tr>\n' % get_current_time()
    if len(rdirs) == 0:
        res += '<tr class="red"><td>No results?</td></tr>\n'
    cdate = ""
    for r in rdirs:
        host,arch,rval = analyze_result(r)
        # date is currently between 3th & 3th "."
        rfile = r[r.rfind("/")+1:]
        rdate = rfile.split(".")[3]
        if cdate != rdate:
            cdate = rdate
            res += '<tr class="rh"><td colspan=3>%s</td></tr>\n' % cdate
        res += '<tr class="%s">' % rval 
        res += '<td>%s</td><td>%s</td>' % (host,arch)
        res += '<td><a href="%s">%s</a></td></tr>\n' %(r,rfile)
    res +="</table>\n"
    res+="</html>\n"
    f = open("%s/index.html" % result_root,"w")
    f.write(res)
    f.close()
    sexe("chgrp -R %s %s " % (result_group,result_root))
    sexe("chmod -R +rx %s " % result_root)
   
def run_tests(input_file,mode):
    """
    Rebuilds the index.html file at a given result_root and sets 
    group read permisions.
    """
    print "[bvauto: test %s %s]" % (input_file,mode)
    svn_root = get_svn_root()
    run_dir = create_run_dir(mode)
    libcmds = parse_tests(input_file)
    print "[Svn Root:    %s]" % svn_root
    print "[Run Dir:     %s]" % run_dir
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
    os.chdir(cwd);
    return run_dir

def post(pattern,result_dir,result_group):
    """
    Posts a set of files from the current directory to a result directory.
    Handles posting to local file system (via cp) or remote system (via ssh/scp).
    """
    if result_dir.find("@") > 0 and result_dir.find(":") > 0:
        print "[Posting to remote path]"
        remote_host, remote_path = result_dir.split(":")
        sexe("ssh %s mkdir %s" % (remote_host,remote_path))
        sexe("scp %s %s" % (pattern,result_dir))
        sexe("ssh %s chgrp -R %s %s" % (remote_host,result_group,remote_path))
        sexe("ssh %s chmod -R g+rx %s" % (remote_host,remote_path))
    else:
        print "[Posting to local path]"
        # copy results to public dir (xml,xsl,css,bv_log_*txt)
        if not os.path.exists(result_dir):
            sexe("mkdir %s/" % result_dir)
        sexe("cp %s %s" % (pattern,result_dir))
        sexe("chgrp -R %s %s"  % (result_group,result_dir))
        sexe("chmod -R g+rx %s"  % result_dir)

def post_results(run_dir,result_root,result_group):
    """
    Posts the result of a test run.
    """
    result_dir="%s/%s" % (result_root,run_dir)
    cwd = os.getcwd()
    os.chdir(run_dir);
    post("*xml *xsl bv_log*.txt *css",result_dir,result_group) 
    os.chdir(cwd);
    
def cleanup_run_dir(run_dir):
    """
    Removes temporary run directory.
    """
    sexe("rm -rf %s/" % run_dir)
    
def main():
    """
    Main driver routine.
    """
    args = parse_args()
    if args["op"] == "reindex":
        result_root  = args["result_root"]
        result_group = args["result_group"]
        reindex_results(result_root,result_group)
    elif args["op"] == "test":
        input_file = args["input_file"]
        mode = args["mode"]
        run_dir = run_tests(input_file,mode)
        if args["post"] == True:
            result_root  = args["result_root"]
            result_group = args["result_group"]
            post_results(run_dir,result_root,result_group)
        if args["cleanup"]:
            cleanup_run_dir(run_dir)
    else:
        print args

if __name__ == "__main__":
    main()


