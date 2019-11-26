# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  launcher.py
#
#  Tests:      This script tests internallauncher's transformation of visit
#              command line arguments into parallel launch arguments.
#
#  Brad Whitlock, Tue Sep 11 12:31:34 PDT 2012
#
# Modifications:
#   Brad Whitlock, Fri Dec  7 09:08:22 PST 2012
#   I added a little more filtering of the launcher output to replace the host
#   with $HOST now that the noloopback case will always use the real host
#   name instead of 127.0.0.1 for parallel engine launches.
#
# ----------------------------------------------------------------------------
import os
import socket
import subprocess
import string
import getpass

# The launch cases we want to test.
launch_cases = {
"aprun"          : ["-nn", "1"],
"bsub"           : ["-p", "pbatch", "-t", "30:00", "-la", "-arg1 -arg2"],
"bsub/mpirun"    : ["-p", "pbatch", "-t", "30:00", "-la", "-arg1 -arg2", "-sla", "-arg3 -arg4"],
"dmpirun"        : ["-b", "bdivp"],
"ibrun"          : [],
"mpirun"         : [],
"msub/aprun"     : ["-nn", "1", "-sla", "-arg1 -arg2", "-hw-pre", "startx", "-hw-post", "stopx"],
"msub/ibrun"     : ["-hw-pre", "startx", "-hw-post", "stopx"],
"msub/mpiexec"   : ["-slpre", "echo 'slprecommand'", "-slpost", "echo 'slpostcommand'", "-sla", "-arg1 -arg2", "-machinefile", "machine.txt", "-hw-pre", "startx", "-hw-post", "stopx"],
"msub/mpirun"    : ["-sla", "-arg1 -arg2", "-machinefile", "machine.txt", "-hw-pre", "startx", "-hw-post", "stopx"],
"msub/srun"      : ["-sla", "-arg1 -arg2", "-hw-pre", "startx", "-hw-post", "stopx"],
"poe"            : ["-nn", "1", "-p", "pbatch"],
"prun"           : ["-nn", "1", "-p", "pbatch"],
"psub"           : ["-n", "JOB_NAME", "-p", "pbatch", "-b", "bdivp", "-t", "30:00", "-expedite", "-slpre", "echo 'pre command'", "-slpost", "echo 'post command'"],
"psub/mpirun"    : ["-nn", "1", "-n", "JOB_NAME", "-p", "pbatch", "-b", "bdivp", "-t", "30:00", "-expedite", "-slpre", "echo 'pre command'", "-slpost", "echo 'post command'"],
"psub/srun"      : ["-nn", "1", "-n", "JOB_NAME", "-p", "pbatch", "-b", "bdivp", "-t", "30:00", "-expedite", "-slpre", "echo 'pre command'", "-slpost", "echo 'post command'"],


"salloc"         : ["-p", "pbatch", "-t", "30:00", "-nn", "1"],
"sbatch"         : ["-p", "pbatch", "-b", "bdivp", "-nn", "1"],
"sbatch/aprun"   : ["-p", "pbatch", "-b", "bdivp", "-nn", "1", "-sla", "-arg1 -arg2"],
"srun"           : ["-nn", "1", "-p", "pbatch", "-b", "bdivp", "-n", "JOB_NAME", "-t", "30:00"],
"yod"            : []}

# Some debugger arguments.
debuggers = [[], ["-totalview", "engine_par"], ["-valgrind", "engine_par"], ["-strace", "engine_par"]]

# Get the launcher command for new versions of VisIt.
def GetLauncherCommand(args):
    p = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = p.communicate()
    ru = string.find(stdout, "RUN USING")
    if ru != -1:
        cmd = stdout[ru + 11:-2]
    else:
        cmd = stdout
    return cmd

def FilterLauncherOutput(text, replacements):
    for k in list(replacements.keys()):
        text = string.replace(text, k, replacements[k])
    return text

def FilterHostName(text):
    host = string.find(text, "-host")
    port = string.find(text, "-port")
    if host != -1 and port != -1:
        return text[:host + 6] + "$HOST " + text[port:]
    return text

def hostname():
    return socket.gethostname()

def nodename():
    return string.split(hostname(), ".")[0]

def sectorname():
    s = nodename()
    for d in "0123456789":
        s = string.replace(s, d, "")
    return s

def FormatLauncherOutput(cmd):
    tmpvisit = string.find(cmd, "/tmp/visit")
    text = ""
    if tmpvisit == -1:
        text = cmd
    else:
        # The launcher made a script. Get the contents of the script.
        index = tmpvisit
        try:
            while cmd[index] != '\n':
                index = index + 1
            filename = cmd[tmpvisit:index]
        except:
            filename = cmd[tmpvisit:]

        cmd = string.replace(cmd, filename, "$LAUNCHSCRIPT")
        text = text + cmd

        try:
            lines = open(filename, "rt").readlines()
            text = text + '\n\nContents of $LAUNCHSCRIPT:\n'
            for line in lines:
                text = text + line
            os.unlink(filename)
        except:
            pass

    return text


# For each launcher
i = 0
keys = list(launch_cases.keys())
keys.sort()
for k in keys:
    # Test the launcher with each debugger.
    j = 0
    text = ""
    for j in range(len(debuggers)):
        np = []
        if not "-np" in launch_cases[k]:
            np = ["-np", "8"]
        args = ["-engine", "-norun", "engine_par", "-l", k] + np + launch_cases[k] + ["-host", "127.0.0.1", "-port", "5600"] + debuggers[j]

        # Come up with a visit command
        cmd = ""
        for a in args:
            if " " in a:
                cmd = cmd + '"%s" ' % a
            else:
                cmd = cmd + a + ' '

        # Run the launcher and get the output.
        visitdir = pjoin(TestEnv.params["top_dir"],"src")
        visittestdir = pjoin(TestEnv.params["top_dir"],"test")
        visit =  visit_bin_path("visit")
        output = FormatLauncherOutput(GetLauncherCommand([visit] + args))
        # filter the run dir, since there are multiple variants for nightly tests
        # (serial, par, etc)
        output = FilterLauncherOutput(output, {TestEnv.params["run_dir"]: "$VISIT_TEST_DIR"})
        output = FilterLauncherOutput(output, {TestEnv.params["result_dir"] : "$VISIT_TEST_DIR"})
        # Filter out visitdir and some related directories.
        output = FilterLauncherOutput(output, {visit_bin_path() : "$VISIT_EXE_DIR"})
        output = FilterLauncherOutput(output, {visit_bin_path("..","exe") : "$VISIT_EXE_DIR"})
        output = FilterLauncherOutput(output, {visittestdir : "$VISIT_TEST_DIR"})
        output = FilterLauncherOutput(output, {visit_bin_path("..") : "$VISITDIR"})
        output = FilterLauncherOutput(output, {visitdir : "$VISITDIR"})
        # special case filter to resolve csh vs bash env differences
        bash_case   = "ulimit -c 0 ;"
        bash_case  += " LIBPATH=$VISITDIR/lib ;"
        bash_case  += " export LIBPATH ;"
        bash_case  += " LD_LIBRARY_PATH=$VISITDIR/lib ;"
        bash_case  += " export LD_LIBRARY_PATH"

        csh_case  = "limit coredumpsize 0 ;"
        csh_case += " setenv LIBPATH $VISITDIR/lib ;"
        csh_case += " setenv LD_LIBRARY_PATH $VISITDIR/lib"
        shell_filter = {bash_case : csh_case}
        output = FilterLauncherOutput(output, shell_filter)
        # Filter out $HOME.
        try:
            output = FilterLauncherOutput(output, {os.environ["HOME"] : "$HOME"})
        except:
            pass

        cdcmd = "cd $VISIT_TEST_DIR"

        # Filter out some other stuff.
        replacements = {getpass.getuser() : "$USER",
                        Version()         : "$VERSION",
                        "linux-intel"     : "$PLATFORM",
                        "linux-x86_64"    : "$PLATFORM",
                        "darwin-i386"     : "$PLATFORM",
                        "darwin-x86_64"   : "$PLATFORM"}

        output = FilterLauncherOutput(output, replacements)

        output = FilterHostName(output)

        # Do the test
        text = text + "="*80 + "\n"
        text = text + "CASE: %s\n\nINPUT: visit %s\n\nRESULTS:\n" % (k + " " + string.join(debuggers[j]), cmd[:-1])
        text = text + output + "\n"*2

    name = string.replace(k, "/", "_")
    TestText(name, text)
    i = i + 1

Exit()

