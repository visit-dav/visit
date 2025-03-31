#!/bin/sh
"exec" "python3" "-u" "-B" "$0" "$@"
#-----------------------------------------------------------------------
#
# VISIT-CREATE-CHKSUMS - Create lists of the md5 checksums, sha256
#                        checksums, and file sizes for the visit
#                        distribution.
#
#
#
# Author: Cyrus Harrison
# Date:   Thu Apr 11 13:47:17 PDT 2024

#
# Note: This is a python version of `visit-create-chksum`
#
# Usage:
#    visit-create-chksums <version>
#
#-----------------------------------------------------------------------

import sys
import os
import json
import subprocess

def sexe(cmd,ret_output=False,echo=False):
    """ Helper for executing shell commands. """
    if echo:
        print("[exe: {0}]".format(cmd))
    if ret_output:
        p = subprocess.Popen(cmd,
                             shell=True,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT)
        out = p.communicate()[0]
        out = out.decode('utf8')
        return p.returncode,out
    else:
        return subprocess.call(cmd,shell=True)

def parse_args():
    if len(sys.argv) < 2:
        print("usage: python3 visit-create-chksums.py <version>")
        print("example:")
        print(">python3 visit-create-chksums.py 3.4.1")
        sys.exit()
    return sys.argv[1]

def test_files(files):
    res = {}
    for f in files:
        if os.path.isfile(f):
            print("[found: %s]" % f)
            res[f] = {}
            rcode,rout = sexe("md5 < %s" %f ,ret_output=True,echo=True)
            if rcode == 0:
               res[f]["md5"] = rout.split()[0]
            rcode,rout = sexe("shasum -a 256 %s" %f ,ret_output=True,echo=True)
            if rcode == 0:
               res[f]["sha256"] = rout.split()[0]
            res[f]["size"] = os.path.getsize(f)
        else:
            print("[NOT found: %s]" % f)
    return res

def main():
    ver = parse_args()
    res = {}
    res["exe"] = {}
    res["src"] = {}
    ver_underscores = ver.replace(".","_")
    exe_files = [ "visit%s.darwin22-x86_64.dmg" % ver_underscores,
                  "visit%s.darwin23-arm64.dmg" % ver_underscores,
                  "visit%s.darwin22-x86_64.txz" % ver_underscores,
                  "visit%s.darwin23-arm64.txz" % ver_underscores,
                  "visit%s.linux-x86_64-debian11.tar.gz" % ver_underscores,
                  "visit%s.linux-x86_64-debian12.tar.gz" % ver_underscores,
                  "visit%s.linux-x86_64-fedora39.tar.gz" % ver_underscores,
                  "visit%s.linux-x86_64-fedora40.tar.gz" % ver_underscores,
                  "visit%s.linux-x86_64-ubuntu18.tar.gz" % ver_underscores,
                  "visit%s.linux-x86_64-ubuntu20.tar.gz" % ver_underscores,
                  "visit%s.linux-x86_64-ubuntu22.tar.gz" % ver_underscores,
                  "visit%s.linux-x86_64-ubuntu24.tar.gz" % ver_underscores,
                  "visit%s.linux-x86_64-rocky8.tar.gz" % ver_underscores,
                  "visit%s.linux-x86_64-rocky9.tar.gz" % ver_underscores,
                  "visit%s_x64.exe  " % ver,
                  "build_visit%s" % ver_underscores,
                  "visit-install%s" % ver_underscores,
                  "INSTALL_NOTES_%s.txt" % ver_underscores,
                  "jvisit%s.tar.gz" % ver]
    src_files = ["visit%s.tar.gz" % ver] #, TODO ADD THIS
                 # "visit_windowsdev_%s.zip" % ver]

    res["exe"] = test_files(exe_files)
    res["src"] = test_files(src_files)
    print(json.dumps(res,indent=2))
    json.dump(res,open("visit_checksums_and_sizes.json","w"),indent=2)
    ofile = open("visit_checksums_and_sizes.txt","w")
    ofile.write("VisIt executable assets:\n")
    ofile.write("========================\n\n")
    for k,v in res["exe"].items():
        ofile.write(k + "\n")
        ofile.write("  md5 checksum     : " + v["md5"] + "\n")
        ofile.write("  sha256 checksum  : " + v["sha256"] + "\n")
        ofile.write("  file size (bytes): " + str(v["size"]) + "\n")
        ofile.write("\n")
    ofile.write("VisIt source assets:\n")
    ofile.write("====================\n\n")
    for k,v in res["src"].items():
        ofile.write(k + "\n")
        ofile.write("  md5 checksum     : " + v["md5"] + "\n")
        ofile.write("  sha256 checksum  : " + v["sha256"] + "\n")
        ofile.write("  file size (bytes): " + str(v["size"]) + "\n")
        ofile.write("\n")


if __name__ == "__main__":
    main()


