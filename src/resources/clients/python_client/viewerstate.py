import json

global_viewerstate = None

class Struct(object):

  def __init__(self):
    for k, v in self.__data__.items():
      if isinstance(v, dict):
        #setattr(self, str(k), Struct(str(k),v))
        #self.__dict__[str(k)] = Struct(str(k),v)
        #TODO: handle recursive objects
        self.__dict__[str(k)] = Struct(str(k),v)
      else:
        self.__dict__[str(k)] = self.__values__[v]

  def __repr__(self):
    return '{%s}' % str(', '.join('%s : %s' % (k, repr(v)) for
      (k, v) in self.__dict__.items()))

  def __str__(self):
    return '%s' % str('\n'.join('%s = %s' % (k, repr(v)) for
      (k, v) in self.__dict__.items()))

  def __setattr__(self,name,value):
    if name in self.__dict__:
        # python 3 path
        if (sys.version_info > (3, 0)):
            if type(value) == type(self.__dict__[name]):
                self.__dict__[name] = value
            elif isinstance(value,(bool,int,float)) \
                 and isinstance(self.__dict__[name],(bool,int,float)):
                 if isinstance(self.__dict__[name],bool) : 
                    self.__dict__[name] = bool(value)
                 elif isinstance(self.__dict__[name],int) : 
                    self.__dict__[name] = int(value)
                 elif isinstance(self.__dict__[name],int) : 
                    self.__dict__[name] = int(value)
                 elif isinstance(self.__dict__[name],float) : 
                    self.__dict__[name] = float(value)
                 #if isinstance(self.__dict__[name],complex) : 
                 #   self.__dict__[name] = complex(value)
            else:
                raise ValueError("Types mismatch {0} and {1}".format(type(value),type(self.__dict__[name])))
        else: # python 2 path (includes long)
            if type(value) == type(self.__dict__[name]):
                self.__dict__[name] = value
            elif isinstance(value,(bool,int,long,float)) \
                 and isinstance(self.__dict__[name],(bool,int,long,float)):
                 if isinstance(self.__dict__[name],bool) : 
                    self.__dict__[name] = bool(value)
                 elif isinstance(self.__dict__[name],int) : 
                    self.__dict__[name] = int(value)
                 elif isinstance(self.__dict__[name],long) : 
                    self.__dict__[name] = int(value)
                 elif isinstance(self.__dict__[name],float) : 
                    self.__dict__[name] = float(value)
                 #if isinstance(self.__dict__[name],complex) : 
                 #   self.__dict__[name] = complex(value)
            else:
                raise ValueError("Types mismatch {0} and {1}".format(type(value),type(self.__dict__[name])))
    else:
        raise RuntimeError("Unable to set unknown attribute: '{0}'".format(name))

class AttributeSubject:
    def __init__(self,inputState=None):
        self.obj = None
        self.data = None
        self.api = None
        self.listeners = []

        if inputState is not None:
            self.update(inputState)

    def addListener(self,l):
        if l not in self.listeners:
            self.listeners.append(l)

    def removeListener(self,l):
        if l in self.listeners:
            self.listeners.remove(l)

    def dict2obj(self,typename,dictionary):
        obj = type(str(typename),(object,),dictionary)
        for k, v in dictionary.items():
            if isinstance(v, dict):
                setattr(obj,str(k), self.dict2obj(k,v))
            else:
                setattr(obj,str(k), v)
        return obj

    def update(self,updateState):
        try:
            if self.api is None and "api" in updateState:
                self.api = updateState
                self.typename = self.api["typename"]
                # return after api has set..
                return

            if self.data is None:
                self.data = updateState
                self.typename = self.data["typename"]
            else:
                self.data.update(updateState)
        except:
            print(self.typename, "failed to parse", updateState["contents"])

        # tell all listeners of update
        try:
            for i in range(len(self.listeners)):
                self.listeners[i](self)
        except:
            print("updating listeners failed")

    def sync(self):
        if self.obj is None:
            self.obj = type(str(self.typename),(Struct,),{}) #self.api["api"])
            self.obj.__data__ = self.api["api"]
            self.obj.__values__ = self.data["contents"]
        return self.obj

class ViewerState:
    def __init__(self):
        self.states = []
        self.conn = None
        self.listeners = []

    def get(self,index):
        if index >= 0 and index < len(self.states):
            return self.states[index]
        return None

    def data(self,index):
        #print "m: ", self.states[index].data["contents"]
        if index >= 0 and index < len(self.states):
            return self.states[index].data["contents"]
        return None

    def api(self,index):
        if index >= 0 and index < len(self.states):
            return self.states[index].api["api"]
        return None

    def setConnection(self,connection):
        self.conn = connection

    def notify(self,index,tag=""):

        if self.conn is None: 
            print("No connection to send results")
            return

        if index < len(self.states):
            if tag != "" and tag != self.states[index].typename:
                print("tags do not match", tag, " vs ", self.states[index].typename)
                return
            res = json.dumps(self.states[index].data)
            self.conn.send(res)
        else:
            print("non-existent index: ", index, " tag = ", tag)

    def update(self,inputState):

        index = inputState["id"]

        #print "key = ", key, " index = ", index
        if index < len(self.states):
            if self.states[index] is None:
                self.states[index] = AttributeSubject(inputState)
            else:
                self.states[index].update(inputState)
        else:
            #create empty AttributeSubjects in between..
            diff = index-len(self.states)

            #include case when state is next element in line
            self.states.append(None)

            #create empty AttributeSubjects in between..
            for i in range(diff):
                self.states.append(None)
            self.states[index] = AttributeSubject(inputState)

        for l in self.listeners:
            if self.states[index] is not None:
                l(self.states[index])
            
    def addListener(self,l):
        if l not in self.listeners:
            self.listeners.append(l)

    def removeListener(self,l):
        if l in self.listeners:
            self.listeners.remove(l)

