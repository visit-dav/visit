import os.path, json

#
# Examine warning messages in make output, building a single, large dictionary
# spanning all source files that produced warnings.
#   
data_dir = test_root_path("..","data")
src_dir = test_root_path("..","src")
tpb_dir = test_root_path("..","src","third_party_builtin")
qtssh_dir = test_root_path("..","src","tools","qtssh")

if not os.path.exists(test_root_path("..","buildlog")):
    Exit(113)

mfile = open(test_root_path("..","buildlog"))
cur_warnings={}
for line in mfile:
    if "warning" in line.lower():
        warnline = line.partition(":")
        warnfile = warnline[0]
        if warnfile[0:len(src_dir):1] != src_dir: 
            continue # ignore files not in our src dir
        if warnfile[0:len(tpb_dir):1] == tpb_dir: 
            continue # ignore files in third_party_builtin
        if warnfile[0:len(data_dir):1] == data_dir: 
            continue # ignore files in data dir 
        if warnfile[0:len(qtssh_dir):1] == qtssh_dir: 
            continue # ignore files in qtssh dir 
        warnftag = "_".join(warnfile.split("/")[-2::1])
        if warnline[1] == ":" and os.path.exists(warnfile):
            msginfo = warnline[2].partition(":")
            lineno = msginfo[0]
            msg = msginfo[2]
            if warnftag in cur_warnings:
                cur_warnings[warnftag]["count"] += 1
                cur_warnings[warnftag]["warnings"].append((lineno,msg))
            else:
                cur_warnings[warnftag] = {"file":warnfile, "ftag":warnftag, "count":1, "warnings":[(lineno,msg)]}
mfile.close()

#
# Examine all files in current and compare to base
#
for ftag in cur_warnings:
    base_count = 0
    ftag = cur_warnings[ftag]["ftag"]
    cur_count = cur_warnings[ftag]["count"]
    cur_txt = json.dumps(cur_warnings[ftag], indent=3)

    bfile = test_root_path("baseline","unit","compiler_warnings","%s.txt"%ftag)
    if os.path.exists(bfile):
        try:
            bf = open(bfile,"r")
            base_warnings = json.load(bf)
            bf.close()
            base_count = base_warnings["count"]
            base_txt = json.dumps(base_warnings, indent=3)
        except:
            base_count = 0
            base_txt = "Unable to read baseline file"

    failIt = cur_count > base_count
    skipIt = cur_count == base_count

    TestText(ftag, cur_txt, failIt, skipIt)

#
# finish any remaining files that were in base but not in current
#
bdir = test_root_path("baseline","unit","compiler_warnings")
for f in os.listdir(bdir):
    bfn = "%s/%s"%(bdir,f)
    if os.path.isdir(bfn):
        continue
    try:
        bf = open(bfn,"r")
        base_warnings = json.load(bf)
        bf.close()
        readIt = 1
    except:
        readIt = 0
    if readIt:
        if base_warnings["ftag"] in cur_warnings:
            continue
        TestText(base_warnings["ftag"], "", 0, 1)

Exit()
