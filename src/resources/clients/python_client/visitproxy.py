import time
import socket
if (sys.version_info > (3, 0)):
    import _thread
else:
    import thread as _thread
import threading
import visitstate
import atexit
import inspect
import json
import sys

from viewerstate import *
from viewermethods import *

# Event records reciept of last VisIt API call
LastEvent = False

class ViewerProxy:
    ASCIIFORMAT=0
    BINARYFORMAT=1
    BUFSIZE = 4096

    def __init__(self):

        self.state = ViewerState()
        self.methods = ViewerMethods(self.state)

        #internal data
        self.visit_host = ""
        self.visit_port = -1
        self.visit_security_key = ""
        self.visit_socket_key = ""
        self.inputConnection = None
        self.outputConnection = None
        self.headerData = bytearray('')
        self.globalList = None

    #create syncState before anything else
    #SyncCount = 0
    #def handleCallback(self,subject):
    #    if subject.typename == "SyncAttributes":
    #        #ignore first since that is initial connection
    #        if ViewerProxy.SyncCount >= 1 and subject.data["syncTag"] == 10000:
    #            print "releasing lock",subject.data["syncTag"]
    #        ViewerProxy.SyncCount += 1

    #def synchronize(self,value):
    #    #send VisIt sync signal..
    #    self.state.data(2)["syncTag"] = int(value)
    #    self.state.notify(2,"SyncAttributes")
    #    self.state.data(2)["syncTag"] = -1
    #    self.event.wait()

    def handshake(self,host,port,password,visType):
        # connect to visit given host, port, and password
        # VisIt must be listener mode for this to work
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # connect to VisIt
        s.connect((host,port))

        # VisIt expects password to continue
        header = {}
        header["password"] = password
        header["canRender"] = visType
        s.send(json.dumps(header))

        data = s.recv(ViewerProxy.BUFSIZE) # read handshake message

        s.close() # close connection after message

        if not data:
            print("Connection to VisIt failed")
            return False

        message = json.loads(data)
        self.visit_host = str(message["host"])
        self.visit_port = int(message["port"])
        self.visit_security_key = str(message["securityKey"])
        return True

    # Header information contains:
    # 5 bytes for format information
    # 1 byte for failcode,
    # 10 bytes for version number
    # 21 bytes for security key
    # 21 bytes for socket key
    
    def readHeader(self,conn):

        # the header should be exactly 100 bytes
        self.headerData = conn.recv(ViewerProxy.BUFSIZE)

        #computed from header info (not really needed, but kept for debugging)
        self.visit_socket_key = self.headerData[5+1+10+21:5+1+10+21+21]

    def writeHeader(self,conn):
        # modify header data for response..
        response = bytearray(self.headerData)
        
        # set ascii format as response type
        response[0] = ViewerProxy.ASCIIFORMAT
        
        # write security key in response
        for i in range(len(self.visit_security_key)):
            response[5+1+10+i] = self.visit_security_key[i] #21st byte is NULL?

        # send 100 byte response
        conn.send(response)

    #TODO: I need a way of telling that the last api has come in
    #before adding the all of VisIt's functions to the global modules

    count = 0
    def updateEvent(self,subject):
        global LastEvent

        if subject.typename == "ClientInformationList":
            self.count = self.count + 1
            if self.count == 3:
                LastEvent = True

    def connect(self,host,port,password,visType):
        global LastEvent

        # attempt handshake
        if not self.handshake(host,port,password,visType):
            print("HandShake failed: unable to connect")
            return False

        #self.visit_host = "localhost";
        # create connections from handshake information
        self.inputConnection = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        self.inputConnection.connect((self.visit_host,self.visit_port))

        self.outputConnection = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        self.outputConnection.connect((self.visit_host,self.visit_port))

        # read header
        self.readHeader(self.outputConnection)

        # write header
        self.writeHeader(self.inputConnection)

        #give state the connection
        self.state.setConnection(self.outputConnection)

        self.state.addListener(self.updateEvent)

        #run eventloop
        _thread.start_new_thread(self.eventLoop,())

        #wait until VisIt API has loaded
        #TODO: replace this logic with semaphore wait..
        print("Loading VisIt API...")
        while(not LastEvent): time.sleep(.1)

        self.state.removeListener(self.updateEvent)

        self.sync()
        print("The viewer proxy has loaded...")

    def close(self):
        self.inputConnection.close()
        self.outputConnection.close()

    def notify(self,index,tag=""):
        self.state.notify(index,tag)

    def printXml(self,index,tag=""):
        self.state.printXml(index,tag)

    def sync(self):
            gmap = sys.modules["__main__"].__dict__
            if type(gmap) is not dict : return

            for i in range(len(self.state.states)):
                typename = self.state.states[i].typename
                if typename not in gmap:
                    gmap[typename] = self.state.states[i].sync() 
            #add viewer method functions
            self.methods.sync()

    def eventLoop(self):
        partial_entry = "" #stitches together one map_node entry
        input_buffer = "" #holds input data buffer

        while True:
            data = self.inputConnection.recv(ViewerProxy.BUFSIZE)

            if not data:
                print("Quitting ...")
                break;
        
            input_buffer += data
            
            # for now JSON parser has to start with object..
            mnsi = input_buffer.find("{");
            mnei = input_buffer.find("}");

            while mnsi >= 0 or mnei >= 0:
                if mnsi < 0 and mnei >= 0:
                    mnei += len("}")
                    partial_entry += input_buffer[0:mnei]
                    input_buffer = input_buffer[mnei:]
                elif mnsi >= 0 and mnei < 0:
                    mnsi += len("{")
                    partial_entry += input_buffer[0:mnsi]
                    input_buffer = input_buffer[mnsi:]
                else:
                    if mnsi < mnei:
                        mnsi += len("{")
                        partial_entry += input_buffer[0:mnsi]
                        input_buffer = input_buffer[mnsi:]
                    else:
                        mnei += len("}")
                        partial_entry += input_buffer[0:mnei]
                        input_buffer = input_buffer[mnei:]
                
                tmp = partial_entry.strip()

                if tmp.count("{") > 0 and tmp.count("{") == tmp.count("}"):
                    try:
                        partial_entry = ""
                        tmp=tmp.replace("\n","")
                        tmp=tmp.replace("\\\"","")
                        res = json.loads(tmp)
                        self.state.update(res)
                    except:
                        print("failed input", tmp)

                tmp = input_buffer.strip()

                #print partial_entry
                mnsi = input_buffer.find("{") #don't include current node
                mnei = input_buffer.find("}")
    

