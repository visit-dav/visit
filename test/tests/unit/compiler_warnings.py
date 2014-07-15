# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  compiler_warnings.py
#
#  Tests:      changes in compiler warnings
#
#  Mark C. Miller, Sun Jul 13 21:45:49 PDT 2014
# ----------------------------------------------------------------------------
import os.path, json

skip_messages = \
[ \
    "missing initializer for member '_typeobject::", \
    "missing initializer for member 'PyMethodDef::", \
    "ISO C++ forbids casting between pointer-to-function and pointer-to-object", \
    "ISO C forbids conversion of object pointer to function pointer type", \
    "extra ';'" \
]

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

            shouldSkip = 0
            for skipmsg in skip_messages:
                minlen = min(len(skipmsg),len(msg))
                if msg[0:minlen] == skipmsg[0:minlen]:
                    shouldSkip = 1
                    break
            if shouldSkip:
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
# Generate the (sorted) warning counts data
#
counts_txt = "{\n"
keys = warning_counts.keys()
keys.sort()
for k in keys:
    counts_txt += "\"%s\": %d,\n"%(k,warning_counts[k])
counts_txt += "\"last line\": 0\n}\n"
TestText("warning_counts_by_file", counts_txt)

#
# Ok, tricky here. Append all the warning details to the html file
# so others can actually see it
#
f = open(test_root_path("html","warning_counts_by_file.html"),"a")
f.write("<pre>\n")
f.write("\n\n\nWarning message strings currently being skipped if matched...\n");
f.write(json.dumps(skip_messages,indent=4))
f.write("\n\n\nWarning message details by file and line number...\n");
f.write(json.dumps(warning_messages,indent=4))
f.write("</pre>\n")
f.close()

Exit()
