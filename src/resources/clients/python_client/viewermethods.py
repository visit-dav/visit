import sys
import inspect
import base64
if (sys.version_info > (3, 0)):
    import io
else:
    import cStringIO as io
from viewerstate import *
from visitstate import RPCType as RPC

class ViewerMethods:
    def __init__(self,_state):
        self.state = _state

    def sync(self):
        gmap = sys.modules["__main__"].__dict__
        gmap["InvertBackgroundColor"] = self.InvertBackgroundColor
        gmap["AddWindow"] = self.AddWindow
        gmap["DrawPlots"] = self.DrawPlots
        gmap["OpenDatabase"] = self.OpenDatabase
        gmap["AddPlot"] = self.AddPlot
        gmap["AddOperator"] = self.AddOperator
        gmap["SetOperatorOptions"] = self.SetOperatorOptions
        gmap["SetPlotOptions"] = self.SetPlotOptions
        gmap["DeleteActivePlots"] = self.DeleteActivePlots
        gmap["SaveImageWindow"] = self.SaveImageWindow
        gmap["ShowImageWindow"] = self.ShowImageWindow
        gmap["SaveWindow"] = self.SaveWindow

    def find(self,val,lst):
        index = -1

        for (i,n) in enumerate(lst):
            if n == val:
                index = i
                break

        return index
    
    def getKey(self,index,key):
        return int(self.state.api(index)[key])

    def getContents(self,index,key):
        return self.state.data(index)[self.getKey(index,key)]

    def setContents(self,index,key,value):
        self.state.data(index)[self.getKey(index,key)] = value

    def InvertBackgroundColor(self):
        self.setContents(0,"RPCType",RPC.InvertBackgroundRPC)
        self.state.notify(0)

    def AddWindow(self):
        self.setContents(0,"RPCType",RPC.AddWindowRPC)
        self.state.notify(0)

    def DrawPlots(self):
        #self.state.data(0)["RPCType"] = RPC.DrawPlotsRPC
        self.setContents(0,"RPCType",RPC.DrawPlotsRPC)
        self.state.notify(0)

    def OpenDatabase(self, database, timeState = 0, addDefaultPlots = True, forcedFileType = ""):
        self.setContents(0,"RPCType",RPC.OpenDatabaseRPC)
        self.setContents(0,"database",database)
        self.setContents(0,"intArg1",timeState)
        self.setContents(0,"boolFlag",addDefaultPlots)
        self.setContents(0,"stringArg1",forcedFileType)
        self.state.notify(0)

    def GetEnabledID(self,plot_type, name):
        names = self.getContents(14,"name")
        types = self.getContents(14,"type")
        enabled = self.getContents(14,"enabled")
        mapper = []
        for i in range(len(names)):
            if enabled[i] == True and plot_type == types[i]:
                mapper.append(names[i])

        mapper.sort()
        
        for i in range(len(mapper)):
            if name == mapper[i]:
                return i

        return -1

    def AddPlotByID(self, plot_type, plot_var):
        self.setContents(0,"RPCType",RPC.AddPlotRPC)
        self.setContents(0,"plotType",plot_type)
        self.setContents(0,"variable",plot_var)
        self.state.notify(0)

    def AddPlot(self,name, plot_var):
        index = self.GetEnabledID("plot",name)
        if index >= 0:
            self.AddPlotByID(index,plot_var)

    def AddOperatorByID(self, op_type):
        self.setContents(0,"RPCType",RPC.AddOperatorRPC)
        self.setContents(0,"operatorType",op_type)
        self.setContents(0,"boolFlag",False)
        self.state.notify(0)

    def AddOperator(self,name):
        index = self.GetEnabledID("operator",name)
        if index >= 0:
            self.AddOperatorByID(index)

    def SetActivePlots(self,activePlots):
        self.setContents(0,"RPCType",RPC.SetActivePlotsRPC)
        self.setContents(0,"activePlotIds",activePlots)
        self.state.notify(0)

    def DeleteActivePlots(self):
        self.setContents(0,"RPCType",RPC.DeleteActivePlotsRPC)
        self.state.notify(0)

    def HideActivePlots(self):
        self.setContents(0,"RPCType",RPC.HideActivePlotsRPC)
        self.state.notify(0)

    def SetPlotOptionsByID(self,index):
        self.setContents(0,"RPCType",RPC.SetPlotOptionsRPC)
        self.setContents(0,"plotType",index)
        self.state.notify(0)
    
    def SetPlotOptions(self,plot):
        plotname = plot.__class__.__name__
        name = plotname.replace("Attributes","")
        index = self.GetEnabledID("plot",name)
        if index == -1 : return

        plotdict = self.props(plot)
        
        for i in range(len(self.state.states)):
            if self.state.states[i].typename == plotname:
                #print name, index, i, plotdict
                self.state.data(i).update(plotdict)
                self.state.notify(i)
                self.SetPlotOptionsByID(index)
                break
        
    def SetOperatorOptionsByID(self,index):
        self.setContents(0,"RPCType",RPC.SetOperatorOptionsRPC)
        self.setContents(0,"operatorType",index)
        self.state.notify(0)

    def SetOperatorOptions(self,operator):
        opname = operator.__class__.__name__
        name = opname.replace("Attributes","")
        index = self.GetEnabledID("operator",name)
        if index == -1: return
        opdict = self.props(operator)
    
        for i in range(len(self.state.states)):
            if self.state.states[i].typename == opname:
                #print name,index,i,opdict
                self.state.data(i).update(opdict)
                self.state.notify(i)
                self.SetOperatorOptionsByID(index)
                break

    def props(self,obj):
        pr = {} 
        for name in dir(obj):
            value = getattr(obj, name)
            if not name.startswith('__') and not inspect.ismethod(value):
               pr[str(obj.__data__[name])] = value
        return pr

    def ShowImageWindow(self):
        try:
            import PIL
            import PIL.Image

            queryAttr = self.state.data(38)["12"]

            for attr in queryAttr:
                data = io.StringIO(base64.b64decode(attr))
                im = PIL.Image.open(data)
                im.show()
        except:
            print("Showing Window Failed...")

    def SaveImageWindow(self,filename):
        try:
            import PIL
            import PIL.Image

            queryAttr = self.state.data(38)["12"]

            for (i,attr) in enumerate(queryAttr):
                data = io.StringIO(base64.b64decode(attr))
                im = PIL.Image.open(data)
                im.save(filename + "_" + str(i) + ".jpg")
        except:
            print("Saving Window Failed...")

    def decode_base64(self,data):
        """Decode base64, padding being optional.

        :param data: Base64 data as an ASCII byte string
        :returns: The decoded byte string.

        """
        missing_padding = 4 - len(data) % 4
        if missing_padding:
            data += b'='* missing_padding
        return base64.decodestring(data)

    def SaveWindow(self,filename):
        try:

            queryAttr = self.state.data(38)["12"]
            for (i,attr) in enumerate(queryAttr):
                data = base64.b64decode(attr)
                f = open(filename + "_" + str(i) + ".vtk","w")
                f.write(data)
                f.close()
        except:
            print("Saving VTK File Failed...")

