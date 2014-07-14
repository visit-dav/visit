# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  compiler_warnings.py
#
#  Tests:      changes in compiler warnings
#
#  Mark C. Miller, Tue Jan 11 10:19:23 PST 2011
# ----------------------------------------------------------------------------
import os.path, json

data_dir = test_root_path("..","data")
src_dir = test_root_path("..","src")
tpb_dir = test_root_path("..","src","third_party_builtin")
qtssh_dir = test_root_path("..","src","tools","qtssh")

if not os.path.exists(test_root_path("..","make.err")):
    Exit(113)

#
# Examine warning messages in make output, building a single, large dictionary
# spanning all source files that produced warnings.
#   
mfile = open(test_root_path("..","make.err"))
cur_warnings={}
tot_warn_cnt = 0
tot_file_cnt = 0
for line in mfile:
    if "warning" in line.lower():

        # get name of file generating warning
        line = line[0:-1] # remove trailing newline
        warnline = line.partition(":")
        warnfile1 = warnline[0]
        warnfile2 = warnfile1.partition("]")
        if warnfile2[1] == "]":
            warnfile = warnfile2[2].replace(" ","")
        else:
            warnfile = warnfile2[0]

        if warnfile[0:len(src_dir):1] != src_dir: 
            continue # ignore files not in our src dir, not our code
        if warnfile[0:len(tpb_dir):1] == tpb_dir: 
            continue # ignore files in third_party_builtin, not our code
        if warnfile[0:len(data_dir):1] == data_dir: 
            continue # ignore files in data dir, not relevant to VisIt for users
        if warnfile[0:len(qtssh_dir):1] == qtssh_dir: 
            continue # ignore files in qtssh dir, not our code 

        warnfilesrc = warnfile[len(src_dir)+1::1]
        if warnline[1] == ":" and os.path.exists(warnfile):
            msginfo = warnline[2].partition(":")
            if not msginfo[0].isdigit():
                continue # probably a garbled message in make.err
            lineno = int(msginfo[0])
            rawmsg = msginfo[2]
            idx = rawmsg.find("warning")
            if idx > -1:
                msg = rawmsg[idx::1]
            else:
                msg = rawmsg
            if warnfilesrc in cur_warnings:
                if lineno in cur_warnings[warnfilesrc]["warnings"]:
                    if msg not in cur_warnings[warnfilesrc]["warnings"][lineno]:
                        cur_warnings[warnfilesrc]["warnings"][lineno].append(msg)
                        tot_warn_cnt += 1
                else:
                    cur_warnings[warnfilesrc]["warnings"][lineno] = [msg]
                    tot_warn_cnt += 1
            else:
                cur_warnings[warnfilesrc] = {"file":warnfilesrc, "warnings":{lineno:[msg]}}
                tot_warn_cnt += 1
                tot_file_cnt += 1

mfile.close()

TestCWText("TOTAL_WARNINGS", "%d"%tot_warn_cnt, tot_warn_cnt)
TestCWText("TOTAL_FILES", "%d"%tot_file_cnt, tot_file_cnt)

#
# Examine all files in current
#
for srcfile in cur_warnings:
    cur_count = 0
    for lineno in cur_warnings[srcfile]["warnings"]:
        cur_count += len(cur_warnings[srcfile]["warnings"][lineno])
    cur_txt = json.dumps(cur_warnings[srcfile],indent=2)
    TestCWText(srcfile, cur_txt, cur_count)

#
# finish any remaining files that are in base but not in current
#
bdir = test_root_path("baseline","unit","compiler_warnings")
for f in os.listdir(bdir):
    bfn = "%s/%s"%(bdir,f)
    srcfile=""
    try:
        base_warnings = json.load(open("%s/%s"%(bdir,f)))
        srcfile = base_warnings["file"]
    except:
        pass
    if not srcfile or srcfile in cur_warnings:
        continue
    TestCWText(srcfile, "", 0)

Exit()
