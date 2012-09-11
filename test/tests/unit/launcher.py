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
# ----------------------------------------------------------------------------
import os, subprocess, string

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

"qsub/mpiexec"   : ["-np", "16", "-nn", "4", "-t", "1:00:00", "-sla", "-env MV2_ON_DEMAND_THRESHOLD 1024", "-slpre", "cd $PBS_O_WORKDIR; setenv NUM_NODES `wc -l $PBS_NODEFILE | cut -f 1 --delimiter=\" \"`; mpdboot -n $NUM_NODES -f $PBS_NODEFILE --remcons", "-slpost", "cd $PBS_O_WORKDIR; mpdallexit"],

"salloc"         : ["-p", "pbatch", "-t", "30:00", "-nn", "1"],
"sbatch"         : ["-p", "pbatch", "-b", "bdivp", "-nn", "1"],
"sbatch/aprun"   : ["-p", "pbatch", "-b", "bdivp", "-nn", "1", "-sla", "-arg1 -arg2"],
"sbatch/mpiexec" : ["-p", "pbatch", "-b", "bdivp", "-nn", "1", "-sla", "-arg1 -arg2"],
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
    for k in replacements.keys():
        text = string.replace(text, k, replacements[k])
    return text

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
keys = launch_cases.keys()
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
        visitdir = pjoin(TestEnv.params["visit_top_dir"],"src")
        visittestdir = pjoin(TestEnv.params["visit_top_dir"],"test")
        visit = pjoin(visitdir,"bin","visit")
        output = FormatLauncherOutput(GetLauncherCommand([visit] + args))

        # Filter out visitdir and some related directories.
        output = FilterLauncherOutput(output, {pjoin(visitdir,"bin") : "$VISIT_EXE_DIR"})
        output = FilterLauncherOutput(output, {pjoin(visitdir,"exe") : "$VISIT_EXE_DIR"})
        output = FilterLauncherOutput(output, {visittestdir : "$VISIT_TEST_DIR"})
        output = FilterLauncherOutput(output, {visitdir : "$VISITDIR"})

        # Filter out $HOME.
        try:
            output = FilterLauncherOutput(output, {os.environ["HOME"] : "$HOME"})
        except:
            pass

        # Filter out some other stuff.
        replacements = {os.getlogin()   : "$USER",
                        Version()       : "$VERSION",
                        "linux-intel"   : "$PLATFORM", 
                        "linux-x86_64"  : "$PLATFORM", 
                        "darwin-i386"   : "$PLATFORM", 
                        "darwin-x86_64" : "$PLATFORM"}
        output = FilterLauncherOutput(output, replacements)

        # Do the test
        text = text + "="*80 + "\n"
        text = text + "CASE: %s\n\nINPUT: visit %s\n\nRESULTS:\n" % (k + " " + string.join(debuggers[j]), cmd[:-1])
        text = text + output + "\n"*2

    name = string.replace(k, "/", "_")
    TestText(name, text)
    i = i + 1

Exit()

