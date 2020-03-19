# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: windows.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 7/2/2010
 description:
    Window creation & managment.

"""


import os
import shutil

from .common import VisItException, require_visit, sexe
from .property_tree import PropertyTree

try:
    import visit
except:
    pass


class WindowManager(object):
    __windows = {}
    @classmethod
    def register_window(cls,win):
        if len(cls.registered_windows()) > 0:
            # visit will always have one valid window
            # when more than one Window instance is registered
            # we need to add an addtional window.
            visit.AddWindow()
        # get active window id from global atts
        gatts = visit.GetGlobalAttributes()
        rid = gatts.windows[gatts.activeWindow]
        visit.SetActiveWindow(rid)
        cls.__windows[rid] = win
        return rid
    @classmethod
    def remove_window(cls,win):
        win.clear_plots()
        # make sure we don't delete the last window
        if len(list(cls.__windows.keys())) > 1:
            prev_lvl = visit.SuppressMessages(2)
            visit.DeleteWindow()
            visit.SuppressMessages(prev_lvl)
        # remove key from active windows dict
        del cls.__windows[win.window_id]
    @classmethod
    def registered_windows(cls):
        return list(cls.__windows.values())
    @classmethod
    def cleanup_windows(cls):
        for win in cls.registered_windows():
            win.remove()

class Window(object):
    @require_visit
    def __init__(self):
        self.window_id = WindowManager.register_window(self)
        self.ts = None
    @require_visit
    def activate(self):
        visit.SetActiveWindow(self.window_id)
    @require_visit
    def remove(self):
        WindowManager.remove_window(self)
        self.window_id = -1
    @require_visit
    def show(self,ts=None):
        self.activate()
        if not ts is None:
            visit.TimeSliderSetState(ts)
            self.ts = ts
        visit.DrawPlots()
        self.set_view()
    @require_visit
    def clear_plots(self):
        self.activate()
        visit.DeleteAllPlots()
    @require_visit
    def set_view(self):
        pass
    @require_visit
    def export(self,format,obase,vnames=None,seq_id=None):
        ts = self.ts
        if not seq_id is None:
            ts = seq_id
        prev_lvl = visit.SuppressMessages(2)
        if vnames is None:
            vnames = ("default",)
        obase = os.path.abspath(obase)
        odir, ofile = os.path.split(obase)
        if not ts is None:
            ofile = "%s%04d" %(ofile,ts)
        stargs = (format,str(vnames),odir,ofile)
        print("[exporting format: %s vars: %s path: %s/%s ]" % stargs)
        eatts = visit.ExportDBAttributes()
        eatts.db_type = format
        eatts.filename = ofile
        eatts.dirname = odir
        eatts.variables = tuple(vnames)
        visit.ExportDatabase(eatts)
        visit.SuppressMessages(prev_lvl)
    @require_visit
    def render(self,obase,res,ores=None,screen_cap=False,seq_id=None):
        ts = self.ts
        if not seq_id is None:
            ts = seq_id
        return self.__save_window(obase,res,ores,screen_cap,ts)
    @require_visit
    def __save_window(self,obase,res,ores,screen_cap,ts):
        prev_lvl = visit.SuppressMessages(2)
        res = [int(v) for v in res]
        if ores is None:
            ores = res
        obase = os.path.abspath(obase)
        odir, ofile = os.path.split(obase)
        if ts is None:
            print("[rendering %s/%s.png]" % (odir,ofile))
            tmp_ofile = "%s___.tmp" % ofile
        else:
            print("[rendering %s/%s%04d.png]" % (odir,ofile,ts))
            tmp_ofile = "%s.%04d___.tmp" % (ofile,ts)
        sa = visit.SaveWindowAttributes()
        sa.outputToCurrentDirectory = 0
        sa.outputDirectory = odir
        sa.fileName = tmp_ofile
        sa.format = sa.PNG
        sa.width, sa.height = res
        sa.screenCapture = 0
        sa.saveTiled = 0
        sa.resConstraint = sa.NoConstraint
        visit.SetSaveWindowAttributes(sa)
        a = visit.GetAnnotationAttributes()
        a.userInfoFlag = 0
        visit.SetAnnotationAttributes(a)
        fname = visit.SaveWindow()
        if fname == "/dev/null/SaveWindow_Error.txt" or not os.path.isfile(fname):
            raise VisItException("Error saving window.")
        des = fname[:fname.find("___.tmp")]
        des += ".png"
        shutil.move(fname,des)
        if ores[0] != res[0] or ores[1] != res[1]:
            stargs = (res[0],res[1],ores[0],ores[1])
            print("[resizing output (from %dx%d to %dx%d)]" % stargs)
            sexe("convert -resize %dx%d %s %s" % (ores[0],ores[1],des,des))
        visit.SuppressMessages(prev_lvl)
        return des


