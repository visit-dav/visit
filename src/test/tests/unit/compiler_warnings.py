# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  compiler_warnings.py
#
#  Tests:      changes in compiler warnings
#
#  Mark C. Miller, Sun Jul 13 21:45:49 PDT 2014
# ----------------------------------------------------------------------------
import time, os.path, json, subprocess

def ShouldSkip(srcfile, msg):
    srcfile_tmp = "global skip list"
    if srcfile in skip_list:
        srcfile_tmp = srcfile
    for skipmsg in skip_list[srcfile_tmp][0]:
        minlen = min(len(skipmsg),len(msg))
        if msg[0:minlen] == skipmsg[0:minlen]:
            return 1
    return 0

data_dir = test_root_path("..","data")
src_dir = test_root_path("..","src")
tpb_dir = test_root_path("..","src","third_party_builtin")
qtssh_dir = test_root_path("..","src","tools","qtssh")

# skip this test if we don't have the make.err file
if not os.path.exists(test_root_path("..","make.err")):
    Exit(116)

# To reduce noise, only run this test on Sunday evenings
# regressiontest script stuffs a line of the form "DAY_OF_WEEK=Sunday"
# into first line of make.err
mfile = open(test_root_path("..","make.err"))
shouldSkip = 1
for line in mfile:
    if "DAY_OF_WEEK=Sunday" in line:
        shouldSkip = 0
        break
mfile.close()
if shouldSkip:
    Exit(116)

#
# Read per-file skip list and zero any line numbers
# We allow line numbers there so that easy cut-n-paste
# can be used to populate skip list
#
skip_list = {}
try:
    skip_list = json.load(open(test_root_path("tests","unit","compiler_warnings_skips.json")))
except:
    pass
for s in skip_list:
    for l in skip_list[s]:
        for m in skip_list[s][l]:
            skip_list[s][l][m] = 0
        if l == 0:
            continue
        skip_list[s][0] = skip_list[s][l];
        del skip_list[s][l]


#
# Examine warning messages in make output, building a single, large dictionary
# spanning all source files that produced warnings.
#   
mfile = open(test_root_path("..","make.err"))
warning_counts ={}
warning_messages = {}
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

        src_file = warnfile[len(src_dir)+1::1]
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
            if msg[0:len("warning: ")] == "warning: ":
                msg = msg[len("warning: ")::1]
            msg_ascii = msg.decode('unicode_escape').encode('ascii','ignore')
            msg = msg_ascii

            if ShouldSkip(src_file, msg):
                continue

            if src_file in warning_counts:
                warning_counts[src_file] += 1
                if lineno in warning_messages[src_file]:
                    if msg in warning_messages[src_file][lineno]:
                        warning_messages[src_file][lineno][msg] += 1
                    else:
                        warning_messages[src_file][lineno][msg] = 1
                else:
                    warning_messages[src_file][lineno] = {msg:1}
            else:
                warning_counts[src_file] = 1
                warning_messages[src_file] = {lineno:{msg:1}}

mfile.close()

#
# Load in current warning counts baseline data
#
baseline_counts = {}
try:
    baseline_counts = json.load(open(test_baseline_path("unit","compiler_warnings","warning_counts_by_file.txt")))
except:
    pass

improved_counts = {}
worsened_counts = {}
for f in warning_counts:
    if f in baseline_counts:
        if warning_counts[f] > baseline_counts[f]:
            worsened_counts[f] = warning_counts[f]
        elif warning_counts[f] < baseline_counts[f]: 
            improved_counts[f] = warning_counts[f]
            baseline_counts[f] = warning_counts[f]
    else:
        worsened_counts[f] = warning_counts[f]
for f in baseline_counts:
    if f not in warning_counts:
        improved_counts[f] = 0
for f in improved_counts:
    if improved_counts[f] == 0 and f in baseline_counts:
        del baseline_counts[f]

#
# If there were improvements, re-baseline counts file
#
if len(improved_counts):
    bfilename = test_baseline_path("unit","compiler_warnings","warning_counts_by_file.txt")
    bfile = open(bfilename,"w+")
    json.dump(baseline_counts,bfile,indent=4,sort_keys=True)
    bfile.write("\n")
    bfile.close()
    retval = subprocess.call(["/usr/bin/svn","commit","-m","'Updating warning counts'","%s"%bfilename])

#
# Generate the (sorted) warning counts data
#
counts_txt = "{\n"
keys = list(worsened_counts.keys())
keys.sort()
for k in keys:
    counts_txt += "\"%s\": %d,\n"%(k,worsened_counts[k])
counts_txt += "\"last line\": 0\n}\n"
TestText("worsened_counts_by_file", counts_txt)

#
# Ok, tricky here. Append all the warning details to the html file
# so others can actually see it
#
uniq_msgs = {}
tot_files = len(warning_counts)
tot_cnt = 0
tot_uniq_cnt = 0
for srcfile in warning_messages:
    for lineno in warning_messages[srcfile]:
        for msg in warning_messages[srcfile][lineno]:
            if msg not in uniq_msgs:
                uniq_msgs[msg] = 1
            else:
                uniq_msgs[msg] += 1
            tot_uniq_cnt += 1
            tot_cnt += len(warning_messages[srcfile][lineno])


f = open(out_path("html","worsened_counts_by_file.html"),"a")
f.write("<pre>\n")
f.write("\n\n\nTotal warnings %d\n"%tot_cnt)
f.write("Total files with warnings %d\n"%tot_files)
f.write("Total unique warnings %d\n"%tot_uniq_cnt)
f.write("Unique warning messages by count...\n")
sorted_uniq_msgs = sorted(uniq_msgs, key=uniq_msgs.get, reverse=True)
for msg in sorted_uniq_msgs:
    f.write("%d: \"%s\"\n"%(uniq_msgs[msg],msg))
f.write("\n\n\nWarning message details by file and line number...\n");
f.write(json.dumps(warning_messages,indent=4))
f.write("\n\n\nWarning message strings currently being skipped if matched...\n")
f.write(json.dumps(skip_list,indent=4))
f.write("</pre>\n")
f.close()

Exit()
