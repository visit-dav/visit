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
    params["password"]:"VisIt-arm64"

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
