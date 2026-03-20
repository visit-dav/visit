#
# This python script will do all the work of compiling and then codesigning and notarizing
# a teeny, tiny, full fledged macOS, Mach-O binary application and then packaging it
# up in a mountable .dmg file.
#
# To run it, be sure your are NOT on an LLNL network or VPN'd into LLNL...
#
#     python3 ./test_notarize.py
#
# The purpose of this script is to exercise all the processes involved in creating,
# codesigning and notarizing a macOS, Mach-O binary application to confirm all the required
# certificates are in place and working.
#
# Because it is small, it is easier to iterate through processes to debug issues than it
# is with the > 1/2 Gigabyte VisIt application.
#
# The final step in the test involves making TestDmgNotarization.dmg file available for
# someone to download through a browser on the internet. For example, copy it to someplace
# in drive.google.com and set permissions so that anyone with a link can read it. Then,
# have someone with a macOS system download, mount and/or install it and run it by double
# clicking on the app icon.
#
# The application intentionally involves a shared .dylib library to ensure that piece of
# things is also working. When it runs correctly it should display a dialog box saying
# "Hello World" and provide the version number of the zlib compression library that is
# embedded in the app, "1.2.13".
#
# Running and re-running this script should be fine. It will just repeatedly overwrite
# all the pieces. The --force option in several of the commands below helps with that.
#
# From time to time, the contents of test.entitlements and/or
# TestDmgNotarization.app/Contents/Info.plist may need to be adjusted to address changes
# in Apple's security policies. For example, macOS apps may need to be explicitly given
# entitlements to access files in a user's Documents or Downloads directory.
#
# Finally, you may need to adjust some of the global params (e.g. username or password key)
#
import os
import subprocess
import json
import time

#
# global values
#
env = os.environ.copy()

params = {
    "username":"miller86@llnl.gov",
    "password":"VisIt",
    "asc_provider":"A827VH86QR"
} 
if os.uname().machine == "arm64":
    params["password"] = "VisIt-arm64"

temp_dmg = "TestDmgNotarization.dmg"
temp_app = "TestDmgNotarization.app"
temp_zip = "TestDmgNotarization.zip"
temp_dylib = "TestDmgNotarization.app/Contents/Frameworks/libz-1.2.13.dylib"

#
# Execute a shell command utility (copied verbatim from masonry)
#
def shexe(cmd,ret_output=False,echo=False,env=None,redirect=None):
        """ Helper for executing shell commands. """
        kwargs = {"shell":True}
        if not env is None:
            kwargs["env"] = env
        if echo:
            print("[exe: %s]" % cmd)
        if ret_output:
            kwargs["stdout"] = subprocess.PIPE
            kwargs["stderr"] = subprocess.STDOUT
            kwargs["universal_newlines"] = True
            p = subprocess.Popen(cmd,**kwargs)
            res = p.communicate()[0]
            return p.returncode,res
        else:
            if redirect is not None:
                with open(redirect, 'w') as file:
                    kwargs["stdout"] = file
                    kwargs["stderr"] = subprocess.STDOUT
                    return subprocess.call(cmd,**kwargs),""
            else:   
                return subprocess.call(cmd,**kwargs),""

#
# Step 0: Compile the application
#
cmd = ["clang", "testdmgnot.c", "-o", "TestDmgNotarization.app/Contents/MacOS/testdmgnot"]
rcode, rout = shexe(" ".join(cmd), ret_output=True, echo=True, env=env)
if rcode != 0:
    print("Compiling the Mach-O application failed with rcode = ", rcode)
    exit(rcode)

#
# Step 1: Codesign the .dylib in the app
#
cmd = [
    "codesign", "--force", "--options", "runtime", "--timestamp",
    "--entitlements", "test.entitlements",
    "-s", '"Developer ID Application: Lawrence Livermore National Laboratory (A827VH86QR)"',
    temp_dylib
]
rcode, rout = shexe(" ".join(cmd), ret_output=True, echo=True, env=env)
if rcode != 0:
    print("Codesigning test dylib failed with rcode = ", rcode)
    exit(rcode)

#
# Step 2: Codesign the app (adds --deep)
#
cmd = [
    "codesign", "--deep", "--force", "--options", "runtime", "--timestamp",
    "--entitlements", "test.entitlements",
    "-s", '"Developer ID Application: Lawrence Livermore National Laboratory (A827VH86QR)"',
    temp_app
]
rcode, rout = shexe(" ".join(cmd), ret_output=True, echo=True, env=env)
if rcode != 0:
    print("Codesigning test app failed with rcode = ", rcode)
    exit(rcode)

#
# Step 3: zip the app for notarization upload (we'll do notarization too on the dmg file)
#
cmd = [
    "zip", "-r", "-y", temp_zip, temp_app
]
rcode, rout = shexe(" ".join(cmd), ret_output=True, echo=True, env=env)
if rcode != 0:
    print("Zipping test app failed with rcode = ", rcode)
    exit(rcode)

#
# Step 4: notarize the zipped app
#
cmd = [
    "xcrun", "notarytool", "submit",
    "--apple-id", params["username"],
    "--keychain-profile", params["password"],
    "--team-id", params["asc_provider"],
    "--output-format", "json",
    temp_zip
]
rcode, rout = shexe(" ".join(cmd), ret_output=True, echo=True, env=env)
if rcode != 0:
    print("notarizing zipped app failed with rcode = ", rcode)
    exit(rcode)
jr = json.loads(rout)
uuid = jr.get("id")

# 
# Loop, checking status of notarization request until its been completed
#
cmd = [
    "xcrun", "notarytool", "info",
    "--apple-id", params["username"],
    "--keychain-profile", params["password"],
    "--team-id", params["asc_provider"],
    "--output-format", "json",
    uuid
]
status = "in progress"
while "in progress" in status:
    time.sleep(15)
    rcode, rout = shexe(" ".join(cmd), ret_output=True, echo=True, env=env)
    if rcode != 0:
        print("checking status of notarization of zipped app failed with rcode = ", rcode)
        exit(rcode)
    jr = json.loads(rout)
    status = jr.get("status")
    status = status.strip()
    status = status.lower()
    print("[status: %s]" % status)

if status != "accepted":
    print("Final status of zipfile notarization not 'accepted', rcode = ", rcode)
    exit(rcode)

#
# Staple the app
#
cmd = "xcrun stapler staple %s" % temp_app
rcode, rout = shexe(cmd, ret_output=True, echo=True, env=env)
if rcode != 0:
    print("Stapling the app failed, rcode = ", rcode)
    exit(rcode)

#
# Create dmg file
#
cmd = [
    "hdiutil", "create", "-volname", "TestDmgNotarization",
    "-srcfolder", "TestDmgNotarization.app",
    "-ov", "-format", "ULMO", "TestDmgNotarization.dmg"
]
rcode, rout = shexe(" ".join(cmd), ret_output=True, echo=True, env=env)
if rcode != 0:
    print("Creating the dmg file failed, rcode = ", rcode)
    exit(rcode)

#
# Notarize the dmg file
#
cmd = [
    "xcrun", "notarytool", "submit",
    "--apple-id", params["username"],
    "--keychain-profile", params["password"],
    "--team-id", params["asc_provider"],
    "--output-format", "json",
    temp_dmg
]
rcode, rout = shexe(" ".join(cmd), ret_output=True, echo=True, env=env)
if rcode != 0:
    print("notarizing dmg failed with rcode = ", rcode)
    exit(rcode)
jr = json.loads(rout)
print(jr)
uuid = jr.get("id")

# 
# Loop, checking status of notarization request until its been completed
#
cmd = [
    "xcrun", "notarytool", "info",
    "--apple-id", params["username"],
    "--keychain-profile", params["password"],
    "--team-id", params["asc_provider"],
    "--output-format", "json",
    uuid
]
status = "in progress"
while "in progress" in status:
    time.sleep(15)
    rcode, rout = shexe(" ".join(cmd), ret_output=True, echo=True, env=env)
    if rcode != 0:
        print("checking status of notarization of zipped app failed with rcode = ", rcode)
        exit(rcode)
    jr = json.loads(rout)
    status = jr.get("status")
    status = status.strip()
    status = status.lower()
    print("[status: %s]" % status)

if status != "accepted":
    print("Final status of dmg notarization not 'accepted', rcode = ", rcode)
    exit(rcode)

#
# Staple the dmg
#
cmd = "xcrun stapler staple %s" % temp_dmg
rcode, rout = shexe(cmd, ret_output=True, echo=True, env=env)
if rcode != 0:
    print("Stapling the dmg failed, rcode = ", rcode)
    exit(rcode)

#
# Now, copy the dmg to a new mac and install it there and try to run it
#
