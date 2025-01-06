#!/usr/bin/env python
#
# file: core.py
# author: Cyrus Harrison <cyrush@llnl.gov>
#
#

import sys
import os
import subprocess
import datetime
import json
import errno
import webbrowser
import time

import glob
import re

from os.path import join as pjoin

__all__ = ["Context",
           "shell",
           "svn",
           "git",
           "cmake",
           "make",
           "inorder",
           "notarize",
           "view_log",
           "log_to_text"]
# ----------------------------------------------------------------------------
#  Method: mkdir_p
#
#  Programmer: Cyrus Harrison
#  Date:       Mon Sept 30 2013
#
#  Helper that does recursive folder creation.
#
# Recipe from:
#  http://stackoverflow.com/questions/600268/mkdir-p-functionality-in-python
# ----------------------------------------------------------------------------
def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: 
            raise

# ----------------------------------------------------------------------------
#  Method: _decode_list
#
#  Programmer: Cyrus Harrison
#  Date:       Fri Jan 11 2013
#
#  Helper which decodes json unicode values (in lists) to standard strings.
#
# Recipe from:
#  http://stackoverflow.com/questions/956867/how-to-get-string-objects-instead-unicode-ones-from-json-in-python
# ----------------------------------------------------------------------------
def _decode_list(data):
    if (sys.version_info > (3, 0)):
        return data
    rv = []
    for item in data:
        if isinstance(item, unicode):
            item = item.encode('utf-8')
        elif isinstance(item, list):
            item = _decode_list(item)
        elif isinstance(item, dict):
            item = _decode_dict(item)
        rv.append(item)
    return rv

# ----------------------------------------------------------------------------
#  Method: _decode_dict
#
#  Programmer: Cyrus Harrison
#  Date:       Fri Jan 11 2013
#
#  Helper which decodes json unicode values (in dictonaries) to standard strings.
#
# Recipe from:
#  http://stackoverflow.com/questions/956867/how-to-get-string-objects-instead-unicode-ones-from-json-in-python
# ----------------------------------------------------------------------------
def _decode_dict(data):
    if (sys.version_info > (3, 0)):
        return data
    rv = {}
    for key, value in data.items():
        if isinstance(key, unicode):
           key = key.encode('utf-8')
        if isinstance(value, unicode):
           value = value.encode('utf-8')
        elif isinstance(value, list):
           value = _decode_list(value)
        elif isinstance(value, dict):
           value = _decode_dict(value)
        rv[key] = value
    return rv

# ----------------------------------------------------------------------------
#  Method: filter_files 
#
#  Programmer: Kevin Griffin
#  Date:       Fri Dec 18 2020
#
#  Generates a list of binary files, frameworks, and bundles on macOS for code signing. 
#  Items in the list are placed in the correct order for proper inside/out code signing. 
#
# ----------------------------------------------------------------------------
def filter_files(sub_dir):
    filtered_files = []
    app_bundles = []
    app_frameworks = []

    files = glob.glob(pjoin(sub_dir, "*"))

    # RE Patterns
    headers_pattern = re.compile(r'/Headers/')
    framework_pattern = re.compile(r'\.framework', flags=re.IGNORECASE)
    bundle_pattern = re.compile(r'\.app', flags=re.IGNORECASE)
    binary_pattern = re.compile(r'\.dylib|\.so|\.a', flags=re.IGNORECASE)

    # Search for binaries
    for f in files:
        if headers_pattern.search(f) is not None:
            continue

        basename = os.path.basename(f)
        endpos = len(basename)
        pos = endpos - len(".app")

        # Add app bundles last to ensure proper inside/out signing
        if pos > 0 and bundle_pattern.search(basename, pos, endpos) is not None:
            app_bundles.append(f)
        else:
            pos = endpos - len(".framework")
            if pos > 0 and framework_pattern.search(basename, pos, endpos) is not None:
                app_frameworks.append(f)

        if os.path.isdir(f):
            filtered_files += filter_files(f)
        else:
            if basename.find('.') == -1:
                filtered_files.append(f)
            else:
                pos = endpos - len(".dylib") # length of the longest binary extension
                if pos < 0:
                    pos = 0

                if binary_pattern.search(basename, pos, endpos) is not None:
                    filtered_files.append(f)

    filtered_files += app_frameworks
    filtered_files += app_bundles
    return filtered_files

def json_loads(jsons):
    if os.path.isfile(jsons):
        jsons=open(jsons).load()
    return json.loads(jsons,object_hook=_decode_dict)

def json_dumps(v):
    return json.dumps(v,indent=2)

def timenow():
    return datetime.datetime.now()
    
def timestamp(t=None,sep="_"):
    """ Creates a timestamp that can easily be included in a filename. """
    if t is None:
        t = timenow()    
    sargs = (t.year,t.month,t.day,t.hour,t.minute,t.second)
    sbase = "".join(["%04d",sep,"%02d",sep,"%02d",sep,"%02d",sep,"%02d",sep,"%02d"])
    tstamp = sbase % sargs
    return      {"key":    tstamp,
                 "year":   t.year,
                 "month":  t.month,
                 "day":    t.day,
                 "hour":   t.hour,
                 "minute": t.minute,
                 "second": t.second}

def timedelta(t_start,t_end):
    t_delta = t_end - t_start
    days, seconds = t_delta.days, t_delta.seconds
    hours = days * 24 + seconds // 3600
    minutes = (seconds % 3600) // 60
    seconds = seconds % 60
    return {"key": "%02d_%02d_%02d" % (hours, minutes, seconds),
            "hours":hours,
            "minutes":minutes,
            "seconds": seconds}

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

class Context(object):
    def __init__(self,enable_logging=True,log_dir=None):
        self.enable_logging = enable_logging
        if enable_logging and log_dir is None:
                log_dir = pjoin(os.getcwd(),"_logs")
        self.log_dir    = log_dir
        if not self.log_dir is None and not os.path.isdir(log_dir):
            mkdir_p(log_dir)
        self.actions  = {}
        self.triggers = {}
    def to_dict(self):
        r = {"context":{"actions":{},
                     "triggers":{},
                     "log_dir":self.log_dir,
                     "enable_logging":self.enable_logging}}
        for k,v in list(self.actions.items()):
            r["context"]["actions"][k] = v.params
        for k,v in list(self.triggers.items()):
            r["context"]["triggers"][k] = v.params
        return r
    def to_json(self):
        return json_dumps(self.to_dict())
    @classmethod 
    def load_dict(cls,params):
        if "context" in params:
            res = Context(enable_logging = params["context"]["enable_logging"],
                          log_dir        = params["context"]["log_dir"])
            for k,v in list(params["context"]["actions"].items()):
                res.actions[k]  = Action.load_dict(v)
            for k,v in list(params["context"]["triggers"].items()):
                res.triggers[k] = Action.load_dict(v)
        return res
    @classmethod 
    def load_json(cls,jsons):
        if os.path.isfile(jsons):
            jsons = open(jsons).read()
        params = json_loads(jsons)
        return cls.load_dict(params)
    def fire(self,trigger_name,key=None):
        t   = self.triggers[trigger_name]
        if key is None:
            key = self.unique_key()
        key += "_" + trigger_name
        res = t.fire(self,trigger_name,key)
        self.log(key,res)
        return res
    def log(self,key,result,tag=None):
        if self.log_dir is None:
            return
        if not tag is None:
            ofname = pjoin(self.log_dir,"log_" + key + "_" + tag +".json")
        else:
            ofname = pjoin(self.log_dir,"log_" + key + ".json")
        try:
            ofile = open(ofname,"w")
            ofile.write(json.dumps(result,indent=2))
            ofile.close()
            if not sys.platform.startswith('win'):
                # if not on windows, create syml link
                lastlink = pjoin(self.log_dir,"last.json")
                if os.path.islink(lastlink):
                    os.unlink(lastlink)
                os.symlink(ofname,lastlink)
        except Exception as e:
            print("<logging error> failed to write results to %s" % ofname)
            raise e
    def unique_key(self):
        return timestamp()["key"]

class Action(object):
    def __init__(self):
        self.params = {}
    def to_dict(self):
        return dict(self.params)
    def to_json(self):
        return json_dumps(self.to_dict())
    @classmethod
    def load_dict(cls,params):
        if "type" in params:
            atype = params["type"]
            aparams = dict(params)
            del aparams["type"]
            return globals()[atype](**aparams)
    @classmethod
    def load_json(cls,jsons):
        if os.path.isfile(jsons):
            jsons = open(jsons).read()
        params = json_loads(jsons)
        return cls.load_dict(params)

class NotarizeAction(Action):
    def __init__(self,
                 build_dir,
                 build_type,
                 build_version,
                 build_arch,
                 entitlements,
                 cert,
                 bundle_id,
                 username,
                 password,
                 asc_provider,
                 type="notarize",
                 description=None,
                 halt_on_error=True,
                 env=None,
                 redirect=None):
        super(NotarizeAction,self).__init__()
        self.params["build_dir"] = build_dir
        self.params["build_type"] = build_type
        self.params["build_version"] = build_version
        self.params["build_arch"] = build_arch
        self.params["entitlements"] = entitlements
        self.params["cert"] = cert
        self.params["bundle_id"] = bundle_id
        self.params["username"] = username
        self.params["password"] = password
        self.params["asc_provider"] = asc_provider
        self.params["type"] = type
        self.params["description"] = description
        self.params["halt_on_error"] = halt_on_error
        self.params["env"] = env
        self.params["redirect"] = redirect

    def execute(self,base,key,tag,parent_res):
        t_start = timenow();
        res = {"action":
               {"key": key,
                "type":self.params["type"],
                "name":tag,
                "description": self.params["description"],
                "build_dir": self.params["build_dir"],
                "build_type": self.params["build_type"],
                "build_version": self.params["build_version"],
                "build_arch": self.params["build_arch"],
                "entitlements": self.params["entitlements"],
                "cert": self.params["cert"],
                "bundle_id": self.params["bundle_id"],
                "username": self.params["username"],
                "password": self.params["password"],
                "asc_provider": self.params["asc_provider"],
                "env": self.params["env"],
                "redirect": self.params["redirect"],
                "start_time":  timestamp(t_start),
                "halt_on_error": self.params["halt_on_error"],
                "finish_time":  None,
                "elapsed_time": None,
                "output": None}
                }
        parent_res["trigger"]["active_actions"] = [res]
        base.log(key=key,result=parent_res)
        cwd = os.path.abspath(os.getcwd())
        env = os.environ.copy()

        if not self.params["env"] is None:
            env.update(self.params["env"])

        try:
            build_dir = pjoin(self.params["build_dir"], "build.%s" % self.params["build_type"])
            bundle_dir = pjoin(build_dir, "_CPack_Packages/Darwin/Bundle")

            ######################################
            # Inside/Out Code Signing
            ######################################
            sub_dirs_base = pjoin(bundle_dir, "VisIt-%s/VisIt.app/Contents/Resources/%s/%s" % (self.params["build_version"], 
                                                                                               self.params["build_version"], 
                                                                                               self.params["build_arch"]))
            sub_dirs = [pjoin(sub_dirs_base, "bin")]
            sub_dirs.append(pjoin(sub_dirs_base, "lib"))
            sub_dirs.append(pjoin(sub_dirs_base, "plugins"))
            sub_dirs.append(pjoin(sub_dirs_base, "libsim"))

            # Get the list of binaries in each directory and sign them
            for sub_dir in sub_dirs:
                binaries = filter_files(sub_dir)
                for binary in binaries:
                    cmd = 'codesign --force --options runtime --timestamp'
                    cmd += ' --entitlements %s' % self.params["entitlements"]
                    cmd += ' -s "%s" %s' % (self.params["cert"], binary)
                    rcode, rout = shexe(cmd, ret_output=True, env=env)

            # codesign VisIt.app
            visit_app = pjoin(bundle_dir, "VisIt-%s/VisIt.app" % self.params["build_version"])
            cmd = 'codesign --force --options runtime --timestamp'
            cmd += ' --entitlements %s' % self.params["entitlements"]
            cmd += ' -s "%s" %s' % (self.params["cert"], visit_app) 
            rcode, rout = shexe(cmd, ret_output=True, echo=True, env=env)

            # Create DMG to upload to Apple
            notarize_dir = pjoin(self.params["build_dir"], "notarize.%s" % self.params["build_type"])
            if not os.path.isdir(notarize_dir):
                os.makedirs(notarize_dir)
            os.chdir(notarize_dir)

            src_folder = pjoin(bundle_dir, "VisIt-%s" % self.params["build_version"])
            temp_dmg = pjoin(notarize_dir, "VisIt.dmg")
            if os.path.isfile(temp_dmg):
                print("[removing existing temporary dmg file: {0}]".format(temp_dmg))
                os.remove(temp_dmg)

            cmd = "hdiutil create -srcFolder %s -o %s" % (src_folder, temp_dmg)

            ##########################################################################
            # NOTE (cyrush) 2021-05-27
            # the dmg creation process is unreliable, it can often fail with:
            #   hdiutil: create failed - Resource busy 
            # but then works fine on subsequent tries, so we try here multiple times
            #
            # NOTE (miller86) Mark C. Miller, Fri Dec 13 19:12:02 PST 2024
            # I believe the "Resource busy" condition we sometimes hit is actually not
            # from hdiutil commands here but instead down in CMake's `make package`
            # logic when large parallel task counts are used (e.g. -j8 or more).
            # So, in the trigger in the bootstrap, we override nthreads to 1 there in
            # hopes of preventing the "Resource busy" error in hdiutil commands.
            ##########################################################################

            dmg_created = False
            dmg_create_max_attempts = 5
            dmg_create_attempts = 0
            dmg_create_output = ""
            while not dmg_created and dmg_create_attempts < dmg_create_max_attempts:
                rcode, rout = shexe(cmd, ret_output=True, echo=True, env=env)
                print("[res: %s]" % rout)
                dmg_create_output = rout
                if rcode == 0:
                    dmg_created = True
                else:
                    dmg_create_attempts += 1

            if not dmg_created:
                msg = "[error creating VisIt dmg for notarization ({0} attempts)]".format(dmg_create_attempts)
                raise RuntimeError(msg, cmd, dmg_create_output)

            ######################################
            # Submit to Apple Notary Service 
            ######################################
            cmd = 'xcrun notarytool submit'
            cmd += ' --apple-id "%s"' % self.params["username"]
            cmd += ' --keychain-profile %s' % self.params["password"]
            cmd += ' --team-id %s' % self.params["asc_provider"]
            cmd += ' --output-format json'
            cmd += ' %s' % temp_dmg
            rcode, rout = shexe(cmd, ret_output=True, echo=True, env=env)
            if rcode != 0:
                raise RuntimeError("[error submitting VisIt dmg for notarization, error='%s']"%rout, cmd)

            jr = json.loads(rout)
            uuid = jr.get("id")
            print("[id: %s]" % uuid)

            # Check status of notarization request
            cmd = 'xcrun notarytool info'
            cmd += ' --apple-id "%s"' % self.params["username"]
            cmd += ' --keychain-profile %s' % self.params["password"]
            cmd += ' --team-id %s' % self.params["asc_provider"]
            cmd += ' --output-format json'
            cmd += ' %s' % uuid

            status = "in progress"
            while "progress" in status:
                time.sleep(120) # check status every two minutes
                rcode, rout = shexe(cmd, ret_output=True, echo=True, env=env)
                jr = json.loads(rout)
                status = jr.get("status").strip().lower()
                print('Status check result: %s ("%s")'%(status,rout))
             
            #################################################################
            # Staple notarization ticket to app bundle
            # NOTE: Mark C. Miller, Sat Dec 14 09:06:34 PST 2024
            # Stapling helps users who are not connected to a network still
            # be able to validate the VisIt .dmg download before using it.
            #################################################################

            if "accepted" in status:
                cmd = "xcrun stapler staple %s" % visit_app
                rcode, rout = shexe(cmd, ret_output=True, echo=True, env=env)
                print('[stapler result: "%s"]' % rout)
                if rcode != 0:
                    raise RuntimeError("[error stapling VisIt (bad network or on VPN?)]", cmd)

                # Create new DMG with stapled containing notarized app
                dmg_stapled = pjoin(notarize_dir, "VisIt.stpl.dmg")
                cmd = "hdiutil create -srcFolder %s -o %s" % (src_folder, dmg_stapled)
                rcode, rout = shexe(cmd, ret_output=True, echo=True, env=env)
                print('[hdiutil result: "%s"]' % rout)
                if rcode != 0:
                    raise RuntimeError("[error creating stapled VisIt.stpl.dmg]", cmd)

                final_dmg_name = "visit%s.darwin%s-%s.dmg" % (self.params["build_version"].replace('.','_'),os.uname().release[0:2],os.uname().machine)
                dmg_release = pjoin(notarize_dir, final_dmg_name)
                cmd = "hdiutil convert %s -format UDZO -o %s" % (dmg_stapled, dmg_release)
                rcode, rout = shexe(cmd, ret_output=True, echo=True, env=env)
                print('[hdiutil convert result: "%s"]' % rout)
                if rcode != 0:
                    raise RuntimeError("[error creating final {0}]".format(final_dmg_name), cmd)
            else:
                raise RuntimeError("Notarization Failed!")
        except KeyboardInterrupt as e:
            res["action"]["error"] = "notarize command interrupted by user (ctrl-c)"
        except Exception as e:
            traceback.print_exc(file=sys.stdout)
            print(e)
            res["action"]["error"] = str(e)

        t_end = timenow()
        res["action"]["finish_time"]  = timestamp(t_end)
        res["action"]["elapsed_time"] = timedelta(t_start,t_end) 
        os.chdir(cwd)
        parent_res["trigger"]["active_actions"] = []
        base.log(key=key,result=parent_res)
        return res

class ShellAction(Action):
    def __init__(self,
                 cmd,
                 type="shell",
                 working_dir=None,
                 description=None,
                 halt_on_error=True,
                 env=None,
                 redirect=None):
        super(ShellAction,self).__init__()
        self.params["cmd"]  = cmd
        self.params["type"] = type
        self.params["halt_on_error"] = halt_on_error
        if working_dir is None:
            working_dir  = os.getcwd()
        self.params["working_dir"] = working_dir
        if description is None:
            description = ""
        self.params["description"] = description
        self.params["env"] = env
        self.params["redirect"] = redirect
    def execute(self,base,key,tag,parent_res):
        t_start = timenow();
        res = {"action":
               {"key": key,
                "type":self.params["type"],
                "name":tag,
                "cmd": self.params["cmd"],
                "description": self.params["description"],
                "working_dir": self.params["working_dir"],
                "env": self.params["env"],
                "redirect": self.params["redirect"],
                "start_time":  timestamp(t_start),
                "halt_on_error": self.params["halt_on_error"],
                "finish_time":  None,
                "elapsed_time": None,
                "output": None}
                }
        parent_res["trigger"]["active_actions"] = [res]
        base.log(key=key,result=parent_res)
        cwd = os.path.abspath(os.getcwd())
        env = os.environ.copy()
        redirect=self.params["redirect"]
        ret_output = True if redirect is None else False
        if not self.params["env"] is None:
            env.update(self.params["env"])
        try:
            if not os.path.isdir(self.params["working_dir"]):
                mkdir_p(self.params["working_dir"])

            print("[chdir to: %s]" % self.params["working_dir"])
            os.chdir(self.params["working_dir"])
            rcode, rout = shexe(self.params["cmd"],
                               ret_output=ret_output,
                               echo=True,
                               env=env,
                               redirect=self.params["redirect"])
            res["action"]["output"] = rout
            res["action"]["return_code"]  = rcode
        except KeyboardInterrupt as e:
            res["action"]["error"] = "shell command interrupted by user (ctrl-c)"
        except Exception as e:
            print(e)
            res["action"]["error"] = str(e)
        t_end = timenow()
        res["action"]["finish_time"]  = timestamp(t_end)
        res["action"]["elapsed_time"] = timedelta(t_start,t_end) 
        os.chdir(cwd)
        parent_res["trigger"]["active_actions"] = []
        base.log(key=key,result=parent_res)
        return res


class SVNAction(ShellAction):
    def __init__(self,
                 svn_url,
                 svn_cmd,
                 svn_bin="svn",
                 working_dir=None,
                 description=None,
                 halt_on_error=True,
                 env=None):
        cmd = " ".join([svn_bin,svn_cmd,svn_url])
        super(SVNAction,self).__init__(cmd=cmd,
                                       type="svn",
                                       working_dir=working_dir,
                                       description=description,
                                       halt_on_error=halt_on_error,
                                       env=env)
        self.params["svn_url"] = svn_url
        self.params["svn_cmd"] = svn_cmd
        self.params["svn_bin"] = svn_bin


class GitAction(ShellAction):
    def __init__(self,
                 git_url,
                 git_cmd,
                 git_bin="git",
                 working_dir=None,
                 description=None,
                 halt_on_error=True,
                 env=None):
        cmd = " ".join([git_bin,git_cmd,git_url])
        super(GitAction,self).__init__(cmd=cmd,
                                       type="git",
                                       working_dir=working_dir,
                                       description=description,
                                       halt_on_error=halt_on_error,
                                       env=env)
        self.params["git_url"] = git_url
        self.params["git_cmd"] = git_cmd
        self.params["git_bin"] = git_bin


class CMakeAction(ShellAction):
    def __init__(self,
                 src_dir,
                 cmake_opts="",
                 cmake_bin="cmake",
                 working_dir=None,
                 description=None,
                 halt_on_error=True,
                 env=None,
                 redirect="cmake.out"):
        cmd = " ".join([cmake_bin,cmake_opts,src_dir])
        super(CMakeAction,self).__init__(cmd=cmd,
                                         type="cmake",
                                         working_dir=working_dir,
                                         description=description,
                                         halt_on_error=halt_on_error,
                                         env=env,
                                         redirect="cmake.out")
        self.params["src_dir"]    = src_dir
        self.params["cmake_opts"] = cmake_opts
        self.params["cmake_bin"]  = cmake_bin

class MakeAction(ShellAction):
    def __init__(self,
                 target="",
                 nthreads=1,
                 make_bin="make",
                 working_dir=None,
                 description=None,
                 halt_on_error=True,
                 env=None,
                 redirect="make.out"):
        cmd = " ".join([make_bin,
                        "-j%d" % nthreads,
                        target])
        super(MakeAction,self).__init__(cmd=cmd,
                                        type="make",
                                        working_dir=working_dir,
                                        description=description,
                                        halt_on_error=halt_on_error,
                                        env=env,
                                        redirect="make.out")
        self.params["target"]   = target
        self.params["nthreads"] = nthreads
        self.params["make_bin"] = make_bin

class InorderTrigger(Action):
    def __init__(self,actions=None):
        super(InorderTrigger,self).__init__()
        self.params["actions"] = []
        if not actions is None:
            self.params["actions"] = actions
        self.params["type"]    = "inorder" 
    def append(self,action_name):
        self.params["actions"].append(action_name)
    def extend(self,action_list):
        self.params["actions"].extend(action_list)
    def fire(self,base,trigger_name,key):
        t_start = timenow();
        res = {"trigger":{"name":trigger_name,
                          "key": key,
                          "start_time":  timestamp(t_start),
                          "end_time": None,
                          "elapsed_time": None,
                          "actions": self.params["actions"],
                          "active_actions":[],
                          "type":"inorder",
                          "results" :[]}}
        base.log(key,res)
        try:
            for  action in self.params["actions"]:
                print("[fire: %s]" % action)
                a = base.actions[action]
                r = a.execute(base,key,action,res)
                res["trigger"]["results"].append(r)
                base.log(key,res)
                if "return_code" in r["action"]:
                    print("[rcode: %d]" % r["action"]["return_code"])
                if "error" in list(r["action"].keys()) or \
                    ("return_code" in r["action"] and r["action"]["return_code"] != 0) :
                    emsg = "[action failed: %s]" % json.dumps(r,indent=2)
                    print(emsg)
                    if "halt_on_error" in r["action"] and r["action"]["halt_on_error"]:
                        raise Exception(emsg)
        except Exception as emsg:
            res["trigger"]["error"] = str(emsg)
        t_end = timenow()
        res["end_time"] = timestamp(t_end)
        res["elapsed_time"] = timedelta(t_start,t_end)
        base.log(key,res)
        return res


shell   = ShellAction
svn     = SVNAction
git     = GitAction
cmake   = CMakeAction
make    = MakeAction
inorder = InorderTrigger
notarize = NotarizeAction


def log_to_text(fname):
    log = json.load(open(fname))
    for t in log["trigger"]["results"]:
        for k, v in t.items():
            print()
            print("++++++++++++++++++++++++++++++")
            print(k)
            print("++++++++++++++++++++++++++++++")
            print()
            for kk,vv in v.items():
                if kk == "output":
                    print("========")
                    print("{0}: output:".format(kk))
                    print("========")
                    print(vv.replace("\\n","\n"))
                else:
                    print("{0}: {1}".format(kk,vv))


def view_log(fname):
    port = 8000
    html_src = pjoin(os.path.split(os.path.abspath(__file__))[0],"html")
    log_dir,log_fname  = os.path.split(os.path.abspath(fname))
    subprocess.call("cp -fr %s/* %s" % (html_src,log_dir),shell=True)
    os.chdir(log_dir)
    try:
        if sys.version_info[0] == 2:
            svr_cmd = "SimpleHTTPServer"
        else:
            svr_cmd = "http.server"
        child = subprocess.Popen([sys.executable, 
                                  '-m',
                                  svr_cmd,
                                  str(port)])
        url = 'http://localhost:8000/view_log.html?log=%s' % log_fname
        webbrowser.open(url)
        child.wait() 
    except KeyboardInterrupt:
        child.terminate()

