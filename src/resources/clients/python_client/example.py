import sys
__system_bytecode_setting = sys.dont_write_bytecode
sys.dont_write_bytecode = True

import visitproxy

host = "localhost"
port = 9002
password = "bob"
BUFSIZE = 4096
visType = "none"
#create a VisIt Proxy
vp = visitproxy.ViewerProxy()

gmap = sys.modules["__main__"].__dict__
gmap["vp"] = vp

#connect to existing host & password
vp.connect(host,port,password,visType)


