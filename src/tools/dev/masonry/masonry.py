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

from os.path import join as pjoin

__all__ = ["Context",
           "shell",
           "svn",
           "git",
           "cmake",
           "make",
           "inorder",
           "view_log"]
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

def sexe(cmd,ret_output=False,echo = False,env=None):
        """ Helper for executing shell commands. """
        kwargs = {"shell":True}
        if not env is None:
            kwargs["env"] = env
        if echo:
            print("[exe: %s]" % cmd)
        if ret_output:
            kwargs["stdout"] = subprocess.PIPE
            kwargs["stderr"] = subprocess.STDOUT
            p = subprocess.Popen(cmd,**kwargs)
            res =p.communicate()[0]
            return p.returncode,res
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
            # create link
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

class ShellAction(Action):
    def __init__(self,
                 cmd,
                 type="shell",
                 working_dir=None,
                 description=None,
                 halt_on_error=True,
                 env=None):
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
            if not os.path.isdir(self.params["working_dir"]):
                mkdir_p(self.params["working_dir"])

            print("[chdir to: %s]" % self.params["working_dir"])
            os.chdir(self.params["working_dir"])
            rcode, rout = sexe(self.params["cmd"],
                               ret_output=True,
                               echo=True,
                               env=env)
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
                 env=None):
        cmd = " ".join([cmake_bin,cmake_opts,src_dir])
        super(CMakeAction,self).__init__(cmd=cmd,
                                         type="cmake",
                                         working_dir=working_dir,
                                         description=description,
                                         halt_on_error=halt_on_error,
                                         env=env)
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
                 env=None):
        cmd = " ".join([make_bin,
                        "-j%d" % nthreads,
                        target])
        super(MakeAction,self).__init__(cmd=cmd,
                                        type="make",
                                        working_dir=working_dir,
                                        description=description,
                                        halt_on_error=halt_on_error,
                                        env=env)
        self.params["target"]    = target
        self.params["nthreads"]  = nthreads
        self.params["make_bin"]   =make_bin

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


def view_log(fname):
    port = 8000
    html_src = pjoin(os.path.split(os.path.abspath(__file__))[0],"html")
    log_dir,log_fname  = os.path.split(os.path.abspath(fname))
    subprocess.call("cp -fr %s/* %s" % (html_src,log_dir),shell=True)
    os.chdir(log_dir)
    try:
        child = subprocess.Popen([sys.executable, 
                                  '-m',
                                  'SimpleHTTPServer',
                                  str(port)])
        url = 'http://localhost:8000/view_log.html?log=%s' % log_fname
        webbrowser.open(url)
        child.wait() 
    except KeyboardInterrupt:
        child.terminate()

