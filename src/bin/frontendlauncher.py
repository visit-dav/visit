from __future__ import print_function
import os
import string
import sys
import subprocess

###############################################################################
#
# Purpose:
#   This is the primary front end launcher for programs in the VisIt toolchain.
#   It is separate from the pieces than change on a per-version basis.
#
#   More specifically:
#     Parse out version arguments and determine the version to run.
#     Fall back to legacy launcher if needed.
#     Use argv[0] (i.e. $0) to determine if which program is being run.
#     Determine if someone is trying to use the old "visit -prog" method
#         of launching tools, and if so both warn them and fix things.
#     Keep track of mixing of public and private versions of VisIt.
#     Finally, if all goes well, launch the "internallauncher" which is
#         allowed to contain version-specific pieces.
#
# Note: Place NO version specific code here!
#       Place NOTHING that requires backwards compatibility code here.
#       ...
#       The only exception in this file is that which is just enough to
#       fall back to the pre-version-specific visit launcher script, and
#       by the time you read this note, that should not require changes.
#
# Programmer:  Brad Whitlock
# Date      :  Tue May 15 14:00:17 PDT 2012
#
# Modifications:
#   Eric Brugger, Mon Nov 19 13:22:18 PST 2012
#   Correct an error that caused specifying programs on the command line
#   to fail, such as "visit -mpeg2encode".
#
#   Brad Whitlock, Mon Mar 11 11:49:37 PDT 2013
#   Apply Ilja Honkonen's fix for determining path to frontendlauncher.
#
#   Kathleen Biagas, Mon Jul 15 11:32:00 PDT 2013
#   Added UNSETENV.
#
#   Eric Brugger, Fri Oct 10 10:40:52 PDT 2014
#   I modified the script so that if the user specifies a version number
#   with a minor version number then it uses that specific version for all
#   the components.
#
#   Eric Brugger, Wed Nov 19 11:56:17 PST 2014
#   I modified the script so that arguments that it adds to the argument
#   list are added at the beginning of the list instead of the end of the
#   list so that argument passing for scripts works properly.
#
#   Eric Brugger, Thu Oct  1 12:14:26 PDT 2015
#   I added add_visit_searchpath.
#
###############################################################################

# -----------------------------------------------------------------------------
#                            Utility Functions
# -----------------------------------------------------------------------------

def GETENV(var):
    try:
        return os.environ[var]
    except:
        return ""

def SETENV(var, value):
    try:
        os.environ[var] = value
    except:
        pass

def UNSETENV(var):
    try:
        del os.environ[var]
    except:
        pass

def exit(msg, value):
    if msg != None:
        print(msg, file=sys.stderr)
    sys.exit(value)

def ParseVersion(ver):
    version = [0,0,-1,-1]
    b = string.find(ver, "b")
    if b != -1:
        ver_b = ver[b:]
        ver = ver[:b]
        if ver_b == "b":
            version[3] = 0
        else:
            version[3] = int(ver_b[1:])
    v = string.split(ver, ".")
    if len(v) > 3:
        raise "Invalid version string"
    for i in range(3):
        if len(v) > i:
            version[i] = int(v[i])
    return tuple(version)
# -----------------------------------------------------------------------------
#                            Startup code (adjust command line)
# -----------------------------------------------------------------------------

frontendlauncherpy = sys.argv[0]
sys.argv = sys.argv[1:]

# -----------------------------------------------------------------------------
#                            Check the Python version
# -----------------------------------------------------------------------------

if sys.version_info[0] > 2:
    msg = "\nError: VisIt's launch script is not compatible with Python " + \
          str(sys.version_info[0]) + \
          ". You will need to prepend the path to a Python 2 interpreter to your " + \
          "path in order to start VisIt.\n"
    print(msg) # python 3 code!
    exit(None, -1)

# -----------------------------------------------------------------------------
#                            Figure out visitdir
# -----------------------------------------------------------------------------

progdir, progname = os.path.split(sys.argv[0])
if progdir == "":
    progdir = "."

frontendpath = os.path.realpath(sys.argv[0])

# visitdir is two directories up from frontendlauncher
visitdir = os.path.split(os.path.split(frontendpath)[0])[0]
if visitdir[-1] != os.path.sep:
    visitdir = visitdir + os.path.sep

# -----------------------------------------------------------------------------
#                            Parse the arguments
# -----------------------------------------------------------------------------

# Set some defaults.
add_forceversion = 0
want_version = 0
ver          = ""
ver_set      = 0
beta         = 0
using_dev    = 0
forceversion = ""
forceversion_set = 0
visitargs        = []

programs = (
"-add_visit_searchpath",
"-convert",
"-makemili_driver",
"-mpeg2encode",
"-mpeg_encode",
"-silex",
"-surfcomp",
"-text2polys",
"-time_annotation",
"-visitconvert",
"-visitconvert_par",
"-xml2atts",
"-xml2avt",
"-xml2info",
"-xml2java",
"-xml2makefile",
"-xml2plugin",
"-xml2projectfile",
"-xml2python",
"-xml2window",
"-xmledit",
"-xmltest")

programsWithOtherNames = {"convert_par" : "visitconvert_par", 
"-composite" : "visit_composite",
"-protocol" : "visitprotocol",
"-transition" : "visit_transition"
}

# Parse the arguments 
i = 1
while i < len(sys.argv):
    arg = sys.argv[i]
    if arg == "-v":
        if i+1 < len(sys.argv):
            ver = sys.argv[i+1]
            ver_set = 1           
        else:
            exit("The -v argument requires a value", -1)
        i = i + 1
    elif arg == "-forceversion":
        if i+1 < len(sys.argv):
            forceversion = sys.argv[i+1]
            forceversion_set = 1           
        else:
            exit("The -forceversion argument requires a value", -1)
        i = i + 1
    elif arg == "-beta":
        beta = 1
    elif arg == "-dv":
        using_dev = 1
    elif arg == "-version":
        want_version = 1
    elif arg in programs:
        progname = arg[1:]
        print("NOTE:  Specifying tools as an argument to VisIt is ", file=sys.stderr)
        print("no longer necessary.\nIn the future, you should ", file=sys.stderr)
        print("just run '%s' instead.\n" % progname, file=sys.stderr)
    elif arg in list(programsWithOtherNames.keys()):
        progname = programsWithOtherNames[arg]
    else:
        visitargs.append(arg)
    i = i +1

# -----------------------------------------------------------------------------
#                        Parse the requested version
# -----------------------------------------------------------------------------
# A forced version: (a) overrides any other -v argument, and (b) makes
# no attempt to parse the version string, which bypasses the rest of
# the intelligent version selection logic in this launcher script.
if forceversion_set:
    ver_set = 1
    ver = forceversion
    if progname == "visit":
        visitargs = ["-forceversion", forceversion] + visitargs

# -----------------------------------------------------------------------------
#                          Find the right version
# -----------------------------------------------------------------------------
# If we have a top-level "exe" directory, then don't bother looking
# for versions to use; this is a development executable.
if os.path.exists(visitdir + "exe"):
    if want_version:
        exit("The version of Visit in the directory %sexe/ will be launched.\n" % visitdir, 0)

    # They may have specified the version, but we need to ignore it
    # because development trees don't have version directories.
    ver = ""

    # The version that we use for plugins. Since we have a development
    # version, let's try to get the version from the VERSION file.
    visitpluginver = ""
    VERSIONFILE = visitdir + "VERSION"
    VISIT_CONFIG_H = visitdir + "include" + os.path.sep + "visit-config.h"
    if os.path.exists(VERSIONFILE):
        visitpluginver = open(VERSIONFILE).readlines()[0][:-1]
    elif os.path.exists(VISIT_CONFIG_H):
        tok = "#define VISIT_VERSION"
        vline = [x for x in open(VISIT_CONFIG_H).readlines() if string.find(x, tok) == 0]
        visitpluginver = vline[0][23:-2]

    # We want to make sure we know if we are trying to launch a public
    # version from under a development version.  Keep track of this.
    # (Note -- don't add it on our own if we're launching a tool.)
    if using_dev or progname in ("visit", "xml2cmake"):
        # stick the -dv at beginning of args to ensure it comes
        # before any '-s' option.
        visitargs = ["-dv"] + visitargs

    version = ParseVersion(visitpluginver)
else:
    # look for the version-specific visit script to determine viable versions
    exeversions = []
    try:
        files = os.listdir(visitdir)
        for f in files:
            filename = visitdir + f + os.path.sep + "bin" + os.path.sep + "internallauncher"
            if os.path.exists(filename):
                exeversions.append(f)
    except:
        pass

    try:
        current_version = os.readlink(visitdir + "current");
    except OSError:
        # If there is no current link then pick the latest version.
        sorted_versions = sorted(exeversions)
        try:
            current_version = sorted_versions[-1]
        except:
            current_version = ""

    try:
        beta_version = os.readlink(visitdir + "beta");
    except OSError:
        beta_version = ""

    if want_version:
        if current_version == "":
            exit("There is no current version of VisIt.", 1)
        exit("The current version of VisIt is %s.\n" % current_version, 0)

    if not ver_set:
        if beta:
            if beta_version == "":
                exit("There is no beta version of VisIt.", 1)
            ver = beta_version
        else:
            if current_version == "":
                exit("There is no current version of VisIt.", 1)
            ver = current_version

    # If they requested no minor version (i.e. $ver_patch is -1)
    # then fill it in with the most recent bugfix release.
    # We don't want to attempt this trick with beta versions.
    # This behavior is new for 1.7.
    version = ParseVersion(ver)
    if ver_set:
        add_forceversion = 1
    if (not (version[0] == 1 and version[1] < 7)) and version[2] == -1 and version[3] == -1:
        add_forceversion = 0
        unsorted_matches = []
        for v in exeversions:
            try:
                thisver = ParseVersion(v)
                if thisver[0] == version[0] and thisver[1] == version[1]:
                    unsorted_matches.append(v)
            except:
                continue
        def by_patch_version(a,b):
            v1 = string.split(a, ".")
            v2 = string.split(b, ".")
            if len(v1) < 3: return -1
            if len(v2) < 3: return +1
            if v1[2] < v2[2]: return -1
            if v1[2] > v2[2]: return +1
            return 0
        if len(unsorted_matches) > 0:
            sorted_matches = sorted(unsorted_matches, cmp=by_patch_version)
            ver = sorted_matches[-1]
            version = ParseVersion(ver)

    # If there was no internal laucher for that version, then either
    # that version wasn't installed, or it is an old version that
    # didn't have a version-specific launcher script.
    if ver not in exeversions:
        exit("Version %s of VisIt does not exist." % ver, 1)

    # Warn if we mixed public and private development versions.
    if using_dev:
        print("", file=sys.stderr);
        print("WARNING: You are launching a public version of VisIt", file=sys.stderr);
        print("         from within a development version!", file=sys.stderr);
        print("", file=sys.stderr);
        visitargs = ["-dv"] + visitargs

    # The actual visit directory is now version-specific
    visitdir = visitdir + ver

    # The version that we use for plugins. It will always match $ver for
    # installed versions.
    visitpluginver = ver

# Make sure that the visitdir path ends in a separator.
if visitdir[-1] != os.path.sep:
    visitdir = visitdir + os.path.sep

# -----------------------------------------------------------------------------
#     Set the environment variables needed for the internal visit launcher
# -----------------------------------------------------------------------------
path = list(string.split(GETENV("PATH"), ":"))
path = path + ["/bin","/usr/bin", "/usr/sbin", "/usr/local/bin", "/usr/bsd","/usr/ucb"]
path = [progdir] + path
SETENV("PATH",               string.join(path, ":"))
SETENV("VISITVERSION",       ver)
SETENV("VISITPLUGINVERSION", visitpluginver)
SETENV("VISITPROGRAM",       progname)
SETENV("VISITDIR",           visitdir[:-1])

# -----------------------------------------------------------------------------
#     If launched from an MacOS app bundle, change directory to users home
# -----------------------------------------------------------------------------
# Otherwise all file dialogs in the gui default to a location inside the
# appbundle, which is confusing to users.
if GETENV("VISIT_STARTED_FROM_APPBUNDLE") == "TRUE":
    os.chdir(GETENV("HOME"))

# -----------------------------------------------------------------------------
#     If the user specified the minor version then add a -forceversion with
#     the minor version
# -----------------------------------------------------------------------------
if progname != "mpeg2encode" and forceversion_set == 0 and add_forceversion == 1:
    visitargs = ["-forceversion", ver] + visitargs

# -----------------------------------------------------------------------------
#                       Run the internal launcher!
# -----------------------------------------------------------------------------
launcher = visitdir + "bin" + os.path.sep + "internallauncher"
ret = 0
if version[0] <= 1 or (version[0] == 2 and version[1] < 6):
    # Pre 2.6, run the internallauncher.
    try:
        ret = subprocess.call([launcher] + visitargs)
    except:
        exit("Can't execute visit launcher script: %s!" % launcher, -1)
else:
    if (sys.version_info > (3, 0)):
        exec(compile(open(launcher).read(), launcher, 'exec'))
    else:
        execfile(launcher)
    
    # Create a launcher object
    launcher = MainLauncher()
    
    # Look for custom launcher code.
    customlauncher = visitdir + "bin" + os.path.sep + "customlauncher"
    if os.path.exists(customlauncher):
        if (sys.version_info > (3, 0)):
            exec(compile(open(customlauncher).read(), customlauncher, 'exec'))
        else:
            execfile(customlauncher)
        try:
            newlauncher = createlauncher()
            launcher = newlauncher
        except:
            print("Could not create custom launcher", file=sys.stderr)

    # Now, call the regular internallauncher function with the launcher
    # object that we created.
    ret = internallauncher(launcher, visitdir, progname, ver, visitpluginver, visitargs)

exit(None, ret)




